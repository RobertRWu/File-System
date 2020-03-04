// 1752149 吴子睿

#include <iostream>

#include "include/main.h"
#include "include/utility.h"
#include "include/file_manager.h"
#include "include/buffer_manager.h"
#include "include/file_system.h"

using namespace std;

extern BufferManager gBufferManager;
extern OpenFileTable gOpenFileTable;
extern FileSystem gFileSystem;
extern INodeTable gINodeTable;
extern User gUser;


FileManager::FileManager() 
{
    root_dir_node_ = gINodeTable.IGet(FileSystem::ROOT_INODE_NO);
    root_dir_node_->i_count_ += 0xffff;
}


/* 改变当前工作目录 */
void FileManager::Cd() 
{
    INode* p_inode;

    p_inode = NameI(FileManager::FM_OPEN);
    if (NULL == p_inode) 
        return;
    
    /* 搜索到的文件不是目录文件 */
    if ((p_inode->i_mode_ & INode::I_MASK) != INode::I_DIR) {
        gUser.u_error_ = User::U_ERR_NOT_DIR;
        gINodeTable.IPut(p_inode);

        return;
    }

    gUser.cur_dir_node_ = p_inode;  // 修改当前目录结点

    /* 相对路径，在现有路径后面加上当前路径分量 */
    if (gUser.dirp_[0] != '/') 
        gUser.cur_dir_path_ += gUser.dirp_;
    
    /* 绝对路径，取代原有工作目录 */
    else 
        gUser.cur_dir_path_ = gUser.dirp_;
}

/* 列出当前INode节点的文件项 */
void FileManager::Ls() 
{
    INode* p_inode, * temp;
    BufferCtrl* p_buffer;
    int phy_blk_no;

    p_inode = gUser.cur_dir_node_;
    p_buffer = NULL;
    gUser.io_param_.offset_ = 0;
    gUser.io_param_.count_ = p_inode->i_size_ / sizeof(DirectoryEntry);  // 目录数

    /* 对目录数进行循环 */
    while (gUser.io_param_.count_) {

        /* 已读完目录文件的当前盘块，需要读入下一目录项数据盘块 */
        if (0 == gUser.io_param_.offset_ % INode::BLOCK_SIZE) {
            if (p_buffer) 
                gBufferManager.BRelease(p_buffer);
            
            /* 计算要读的物理盘块号 */
            phy_blk_no = p_inode->BMap(gUser.io_param_.offset_ / INode::BLOCK_SIZE);
            p_buffer = gBufferManager.BRead(phy_blk_no);
        }

        /* 没有读完当前目录项盘块，则读取下一目录项 */
        memcpy(&gUser.dir_entry_, p_buffer->b_addr_ + (gUser.io_param_.offset_ % INode::BLOCK_SIZE), sizeof(DirectoryEntry));
        gUser.io_param_.offset_ += sizeof(DirectoryEntry);
        gUser.io_param_.count_--;

        /* 跳过空目录项 */
        if (0 == gUser.dir_entry_.inode_no_)
            continue;

        if (gUser.cur_dir_path_[0] == '/')
            gUser.dirp_ = gUser.cur_dir_path_ + gUser.dir_entry_.dir_name_;
        else
            gUser.dirp_ = gUser.cur_dir_path_ + '/' + gUser.dir_entry_.dir_name_;
        temp = NameI(FileManager::FM_OPEN);
        if((temp->i_mode_ & temp->I_DIR) == temp->I_DIR)
            cout << "<DIR>  " << gUser.dir_entry_.dir_name_ << endl;
        else
            cout << "<FILE> " << gUser.dir_entry_.dir_name_ << endl;
        gINodeTable.IPut(temp);
    }

    if (p_buffer) {
        gBufferManager.BRelease(p_buffer);
    }
}

/*
* 功能：打开文件
* 效果：建立打开文件结构，内存i节点开锁 、i_count 为正数（i_count ++）
* */
void FileManager::Fopen() 
{
    INode* p_inode;

    p_inode = NameI(FileManager::FM_OPEN);

    /* 没有找到相应的Inode */
    if (NULL == p_inode) 
        return;

    Open1(p_inode, gUser.arg_[1], 0);
}

void FileManager::Fcreat() 
{
    INode* p_inode;
    unsigned int mode;

    mode = gUser.arg_[1];

    /* 搜索目录的模式为1，表示创建；若父目录不可写，出错返回 */
    p_inode = NameI(FileManager::FM_CREATE);

    /* 没有找到相应的INode，或NameI出错 */
    if (NULL == p_inode) {
        if (gUser.u_error_)
            return;

        p_inode = MakNode(mode);
        if (NULL == p_inode)
            return;

        /* 如果创建的名字不存在，使用参数trf = 2来调用open1()。*/
        Open1(p_inode, File::FWRITE, 2);
        return;
    }

    /* 如果NameI()搜索到已经存在要创建的文件，则跳过 */
    Open1(p_inode, File::FWRITE, 1);
    p_inode->i_mode_ |= mode;
}

/*
* trf == 0由open调用
* trf == 1由creat调用，creat文件的时候搜索到同文件名的文件
* trf == 2由creat调用，creat文件的时候未搜索到同文件名的文件，这是文件创建时更一般的情况
* mode参数：打开文件模式，表示文件操作是 读、写还是读写
*/
void FileManager::Open1(INode* p_inode, int mode, int trf)
{
    File* p_file;
    int fd;

    /* 在creat文件的时候搜索到同文件名的文件，则跳过 */
    if (1 == trf) {
        gINodeTable.IPut(p_inode);
        return;
    }

    /* 分配打开文件控制块File结构 */
    p_file = gOpenFileTable.FAlloc();
    if (NULL == p_file) {
        gINodeTable.IPut(p_inode);
        return;
    }

    /* 设置打开文件方式，建立File结构和内存INode的勾连关系 */
    p_file->flag_ = mode & (File::FREAD | File::FWRITE);
    p_file->inode_ = p_inode;

    /* 为打开或者创建文件的各种资源都已成功分配，函数返回 */
    if (gUser.u_error_ == User::U_NO_ERROR)
        return;
    /* 如果出错则释放资源 */
    else {
        /* 释放打开文件描述符 */
        fd = gUser.ar0_[User::EAX];
        if (fd != NO_OPEN_FILE_SLOT) {
            gUser.ofiles_.SetF(fd, NULL);
            /* 递减File结构和INode的引用计数 ,File结构没有锁 f_count为0就是释放File结构了*/
            p_file->count_--;
        }
        gINodeTable.IPut(p_inode);
    }
}

/* 返回NULL表示目录搜索失败，未找到u.dirp中指定目录全路径
 * 否则是根指针，指向文件的内存打开i节点
 */
INode* FileManager::NameI(enum DirectorySearchMode mode) 
{
    INode* p_inode;
    BufferCtrl* p_buffer;
    int free_entry_offset;  // 此变量存放了空闲目录项位于目录文件中的偏移量
    int phy_blk_no;  // 物理块号
    unsigned int index;  // 路径名开始位置索引
    unsigned int next_index;  //路径名结束位置索引

    p_inode = gUser.cur_dir_node_;
    index = 0;
    next_index = 0;

    /* 如果为绝对路径，切换到根目录结点 */
    if ('/' == gUser.dirp_[0]) {
        index = 1;
        next_index = 1;
        p_inode = root_dir_node_;
    }

    /* 外层循环每次处理pathname中一段路径分量 */
    while (true) {

        /* 如果出错则释放当前搜索到的目录文件Inode，并退出 */
        if (gUser.u_error_ != User::U_NO_ERROR) 
            break;

        /* 搜索完毕 */
        if (next_index >= gUser.dirp_.length()) 
            return p_inode;   

        /* 如果搜索到的不是目录，释放相关INode资源则退出 */
        if ((p_inode->i_mode_ & INode::I_MASK) != INode::I_DIR) {
            gUser.u_error_ = User::U_ERR_NOT_DIR;
            break;
        }

        // 找当前路径名结尾，找不到说明为最后一级
        next_index = gUser.dirp_.find_first_of('/', index);
        memset(gUser.dir_buf_, 0, sizeof(gUser.dir_buf_));  // 初始化临时路径

        /* 将当前路径名写入临时路径中*/
        if (next_index == (unsigned int)string::npos)
            memcpy(gUser.dir_buf_, gUser.dirp_.data() + index, gUser.dirp_.length() - index);
        else
            memcpy(gUser.dir_buf_, gUser.dirp_.data() + index, next_index - index);

        index = next_index + 1;  // 设置索引值为下一级路径起点

        /* 内层循环部分对于u.dbuf[]中的路径名分量，逐个搜寻匹配的目录项 */
        gUser.io_param_.offset_ = 0;

        /* 设置为目录项个数 ，含空白的目录项*/
        gUser.io_param_.count_ = p_inode->i_size_ / sizeof(DirectoryEntry);
        free_entry_offset = 0;
        p_buffer = NULL;

        /* 在一个目录下寻找 */
        while (true) {

            /* 对目录项已经搜索完毕 */
            if (0 == gUser.io_param_.count_) {
                if (NULL != p_buffer) 
                    gBufferManager.BRelease(p_buffer);

                /* 如果是创建新文件，且已全部搜索完 */
                if (FileManager::FM_CREATE == mode && next_index >= gUser.dirp_.length()) {

                    /* 将父目录Inode指针保存起来，以后写目录项WriteDir()函数会用到 */
                    gUser.p_dir_node_ = p_inode;

                    /* 将空闲目录项偏移量存入gUser区中，写目录项WriteDir()会用到 */
                    if (free_entry_offset) 
                        gUser.io_param_.offset_ = free_entry_offset;
                    else 
                        p_inode->i_flag_ |= INode::I_UPDATE;
                    
                    return NULL;
                }
                gUser.u_error_ = User::U_ERR_NO_ENT;
                gINodeTable.IPut(p_inode);

                return NULL;
            }

            /* 已读完目录文件的当前盘块，需要读入下一目录项数据盘块 */
            if (0 == gUser.io_param_.offset_ % INode::BLOCK_SIZE) {
                if (p_buffer) 
                    gBufferManager.BRelease(p_buffer);
                
                /* 计算要读的物理盘块号 */
                phy_blk_no = p_inode->BMap(gUser.io_param_.offset_ / INode::BLOCK_SIZE);
                p_buffer = gBufferManager.BRead(phy_blk_no);
            }

            /* 没有读完当前目录项盘块，则读取下一目录项至gUser.dir_entry_ */
            memcpy(&gUser.dir_entry_, p_buffer->b_addr_ + (gUser.io_param_.offset_ % INode::BLOCK_SIZE), sizeof(DirectoryEntry));
            gUser.io_param_.offset_ += sizeof(DirectoryEntry);
            gUser.io_param_.count_--;

            /* 如果是空闲目录项，记录该项位于目录文件中偏移量 */
            if (0 == gUser.dir_entry_.inode_no_) {
                if (0 == free_entry_offset) 
                    free_entry_offset = gUser.io_param_.offset_ - sizeof(DirectoryEntry);

                continue;
            }

            /* 匹配成功就跳出 */
            if (0 == memcmp(gUser.dir_buf_, &gUser.dir_entry_.dir_name_, DirectoryEntry::DIR_MAX_SIZE)) 
                break;
        }

        if (p_buffer) 
            gBufferManager.BRelease(p_buffer);

        /* 如果是删除操作，则返回父目录INode，而要删除文件的INode号在u.dent.m_ino中 */
        if (FileManager::FM_DELETE == mode && next_index >= gUser.dirp_.length()) 
            return p_inode;

        /* 匹配目录项成功，则释放当前目录INode，根据匹配成功的
         * 目录项inode_no字段获取相应下一级目录或文件的INode。*/
        gINodeTable.IPut(p_inode);
        p_inode = gINodeTable.IGet(gUser.dir_entry_.inode_no_);

        /* 获取失败 */
        if (NULL == p_inode) 
            return NULL;
    }

    gINodeTable.IPut(p_inode);

    return NULL;
}



/* 为新创建的文件写新的i节点和父目录中新的目录项(相应参数在User结构中)
 * 返回的p_inode是上了锁的内存i节点，其中的i_count是 1。
 */
INode* FileManager::MakNode(unsigned int mode) 
{
    INode* p_inode;

    /* 分配一个空闲DiskInode，里面内容已全部清空 */
    p_inode = gFileSystem.IAlloc();
    if (NULL == p_inode) 
        return NULL;
   
    p_inode->i_flag_ |= (INode::I_ACCESS | INode::I_UPDATE);

    /* 标注该INode已有文件 */
    p_inode->i_mode_ = mode | INode::I_ALLOC;
    p_inode->i_nlink_ = 1;

    /* 将目录项写入gUser.u_dir_entry_，随后写入目录文件 */
    WriteDir(p_inode);

    return p_inode;
}

/* 把属于自己的目录项写进父目录，修改父目录文件的inode节点 、将其写回磁盘。*/
void FileManager::WriteDir(INode* p_inode) 
{
    /* 设置目录项中INode编号部分 */
    gUser.dir_entry_.inode_no_ = p_inode->i_number_;

    /* 设置目录项中pathname分量部分 */
    memcpy(gUser.dir_entry_.dir_name_, gUser.dir_buf_, DirectoryEntry::DIR_MAX_SIZE);

    gUser.io_param_.count_ = sizeof(DirectoryEntry);
    gUser.io_param_.base_ = (unsigned char*)&gUser.dir_entry_;

    /* 将目录项写入父目录文件 */
    gUser.p_dir_node_->WriteI();
    gINodeTable.IPut(gUser.p_dir_node_);
}

void FileManager::Fclose() 
{
    File* p_file;
    int fd;

    /* 文件描述符通过arg[0]传入 */
    fd = gUser.arg_[0];

    /* 获取打开文件控制块File结构 */
    p_file = gUser.ofiles_.GetF(fd);
    if (NULL == p_file) 
        return;
    
    /* 释放打开文件描述符fd，递减File结构引用计数 */
    gUser.ofiles_.SetF(fd, NULL);
    gOpenFileTable.CloseF(p_file);
}

void FileManager::UnLink() 
{
    INode* p_inode;
    INode* p_delete_node;

    p_delete_node = NameI(FileManager::FM_DELETE);
    if (NULL == p_delete_node) 
        return;

    p_inode = gINodeTable.IGet(gUser.dir_entry_.inode_no_);
    if (NULL == p_inode) 
        return;
    
    /* 写入清零后的目录项 */
    gUser.io_param_.offset_ -= sizeof(DirectoryEntry);
    gUser.io_param_.base_ = (unsigned char*)&gUser.dir_entry_;
    gUser.io_param_.count_ = sizeof(DirectoryEntry);

    gUser.dir_entry_.inode_no_ = 0;
    p_delete_node->WriteI();

    /* 修改inode项 */
    p_inode->i_nlink_--;
    p_inode->i_flag_ |= INode::I_UPDATE;

    gINodeTable.IPut(p_delete_node);
    gINodeTable.IPut(p_inode);
}

void FileManager::Fseek() 
{
    File* p_file;
    int fd;
    int offset;
    int whence;

    fd = gUser.arg_[0];  /* 参数通过arg_[0]传递 */

    p_file = gUser.ofiles_.GetF(fd);
    /* 若FILE不存在，GetF有设出错码 */
    if (NULL == p_file) 
        return;  
    
    offset = gUser.arg_[1];
    whence = gUser.arg_[2];

    switch (whence) {
    /* 从文件开头开始 */
    case SEEK_SET:
        p_file->offset_ = offset;
        break;

    /* 从当前位置开始 */
    case SEEK_CUR:
        p_file->offset_ += offset;
        break;
        /* 读写位置调整为文件长度加offset */
    case SEEK_END:
        p_file->offset_ = p_file->inode_->i_size_ + offset;
        break;
    }
}

/* 直接调用Rdwr()函数即可 */
void FileManager::Fread() 
{
    Rdwr(File::FREAD);
}

/* 直接调用Rdwr()函数即可 */
void FileManager::Fwrite() 
{
    Rdwr(File::FWRITE);
}

void FileManager::Rdwr(enum File::FileFlags mode) 
{
    File* p_file;

    /* 根据Read()/Write()的系统调用参数fd获取打开文件控制块结构 */
    p_file = gUser.ofiles_.GetF(gUser.arg_[0]);	

    /* 不存在该打开文件，GetF已经设置过出错码，所以这里不需要再设置了 */
    if (NULL == p_file) 
        return;
    
    /* 读写的模式不正确 */
    if ((p_file->flag_ & mode) == 0) {
        gUser.u_error_ = User::U_ERR_ACCES;
        return;
    }

    /* 目标缓冲区首址 */
    gUser.io_param_.base_ = (unsigned char*)gUser.arg_[1];     

    /* 要求读/写的字节数 */
    gUser.io_param_.count_ = gUser.arg_[2];		

    /* 设置文件起始读位置 */
    gUser.io_param_.offset_ = p_file->offset_;
    if (File::FREAD == mode) 
        p_file->inode_->ReadI();
    else 
        p_file->inode_->WriteI();

    /* 根据读写字数，移动文件读写偏移指针 */
    p_file->offset_ += (gUser.arg_[2] - gUser.io_param_.count_);

    /* 返回实际读写的字节数，修改存放系统调用返回值的核心栈单元 */
    gUser.ar0_[User::EAX] = gUser.arg_[2] - gUser.io_param_.count_;
}

