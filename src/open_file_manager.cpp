// 1752149 吴子睿

#include <iostream>

#include "main.h"
#include "utility.h"
#include "open_file_manager.h"
#include "user.h"

using namespace std;

extern BufferManager gBufferManager;
extern FileSystem gFileSystem;
extern INodeTable gINodeTable;
extern User gUser;

/* 格式化 */
void OpenFileTable::Format() 
{
    File file;
    int i;

    for (i = 0; i < OpenFileTable::MAX_FILES; i++)
        memcpy(files_ + i, &file, sizeof(File));
}

/* 作用：进程打开文件描述符表中找的空闲项之下标写入 ar0[EAX] */
File* OpenFileTable::FAlloc() 
{
    int fd;
    int i;

    fd = gUser.ofiles_.AllocFreeSlot();
    if (fd == NO_OPEN_FILE_SLOT) 
        return NULL;
    
    for (i = 0; i < OpenFileTable::MAX_FILES; i++) {

        /* count==0表示该项空闲 */
        if (files_[i].count_ == 0) {
            gUser.ofiles_.SetF(fd, &files_[i]);
            files_[i].count_++;
            files_[i].offset_ = 0;

            return &files_[i];
        }
    }

    gUser.u_error_ = User::U_ERR_NFILE;

    return NULL;
}

/* 对打开文件控制块File结构的引用计数count减1，若引用计数count为0，则释放File结构。*/
void OpenFileTable::CloseF(File* pFile) 
{
    pFile->count_--;
    if (pFile->count_ <= 0) 
        gINodeTable.IPut(pFile->inode_);
}


void INodeTable::Format() 
{
    int i;
    INode inode;

    for (i = 0; i < INodeTable::INODE_NUM; i++) 
        memcpy(inodes_ + i, &inode, sizeof(INode));
}


/*
 * 检查编号为inode_num的外存INode是否有内存拷贝，
 * 如果有则返回该内存INode在内存INode表中的索引
 */
int INodeTable::IsLoaded(int inode_num) 
{
    int i;

    for (i = 0; i < INODE_NUM; i++) {
        if (inodes_[i].i_number_ == inode_num && inodes_[i].i_count_) 
            return i;
    }

    return INODE_NOT_LOADED;
}

/* 在内存INode表中寻找一个空闲的内存INode */
INode* INodeTable::GetFreeINode() 
{
    int i;

    for (i = 0; i < INodeTable::INODE_NUM; i++) {

        /* 如果该内存Inode引用计数为零，则该Inode表示空闲 */
        if (inodes_[i].i_count_ == 0) 
            return inodes_ + i;
    }

    return NULL;
}

/*
 * 根据外存INode编号获取对应INode。如果该INode已经在内存中，返回该内存INode；
 * 如果不在内存中，则将其读入内存并返回该内存INode，返回NULL:INode Table OverFlow!
 */
INode* INodeTable::IGet(int inode_num) 
{
    INode* p_inode;
    int index;
    BufferCtrl* p_buffer;

    index = IsLoaded(inode_num);  // 检查是否已载入内存

    /* 已载入内存 */
    if (index >= 0) {
        p_inode = inodes_ + index;
        ++p_inode->i_count_;

        return p_inode;
    }

    /* 没有Inode的内存拷贝，则分配一个空闲内存Inode */
    else {
        p_inode = GetFreeINode(); 

        /* 没有可用的inode */
        if (NULL == p_inode) {
            cout << "INode Table Overflow!" << endl;
            gUser.u_error_ = User::U_ERR_NFILE;

            return NULL;
        }

        /* 设置Inode编号，增加引用计数 */
        p_inode->i_number_ = inode_num;
        p_inode->i_count_++;
        p_buffer = gBufferManager.BRead(FileSystem::INODE_ZONE_START_SECTOR + inode_num / FileSystem::INODE_NUMBER_PER_SECTOR);
        p_inode->ICopy(p_buffer, inode_num);
        gBufferManager.BRelease(p_buffer);
    }

    return p_inode;
}

/*
 * 减少该内存INode的引用计数，如果此INode已经没有目录项指向它，
 * 且无进程引用该INode，则释放此文件占用的磁盘块。
 */
void INodeTable::IPut(INode* p_inode) 
{
    /* 当前进程为引用该内存INode的唯一进程，且准备释放该内存INode */
    if (p_inode->i_count_ == 1) {

        /* 该文件已经没有目录路径指向它 */
        if (p_inode->i_nlink_ <= 0) {

            /* 释放该文件占据的数据盘块 */
            p_inode->ITrunc();
            p_inode->i_mode_ = 0;

            /* 释放对应的外存INode */
            gFileSystem.IFree(p_inode->i_number_);
        }

        /* 清除内存INode的所有标志位 */
        p_inode->i_flag_ = 0;

        /* 这是内存inode空闲的标志之一，另一个是i_count == 0 */
        p_inode->i_number_ = -1;
    }

    /* 更新外存INode信息 */
    p_inode->IUpdate((int)time(NULL));

    p_inode->i_count_--;
}

/* 将所有被修改过的内存INode更新到对应外存INode中 */
void INodeTable::UpdateINodeTable() 
{
    int i;

    for (i = 0; i < INodeTable::INODE_NUM; i++) {
        if (inodes_[i].i_count_) 
            inodes_[i].IUpdate((int)time(NULL));
    }
}