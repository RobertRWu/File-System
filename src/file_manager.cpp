// 1752149 �����

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


/* �ı䵱ǰ����Ŀ¼ */
void FileManager::Cd() 
{
    INode* p_inode;

    p_inode = NameI(FileManager::FM_OPEN);
    if (NULL == p_inode) 
        return;
    
    /* ���������ļ�����Ŀ¼�ļ� */
    if ((p_inode->i_mode_ & INode::I_MASK) != INode::I_DIR) {
        gUser.u_error_ = User::U_ERR_NOT_DIR;
        gINodeTable.IPut(p_inode);

        return;
    }

    gUser.cur_dir_node_ = p_inode;  // �޸ĵ�ǰĿ¼���

    /* ���·����������·��������ϵ�ǰ·������ */
    if (gUser.dirp_[0] != '/') 
        gUser.cur_dir_path_ += gUser.dirp_;
    
    /* ����·����ȡ��ԭ�й���Ŀ¼ */
    else 
        gUser.cur_dir_path_ = gUser.dirp_;
}

/* �г���ǰINode�ڵ���ļ��� */
void FileManager::Ls() 
{
    INode* p_inode, * temp;
    BufferCtrl* p_buffer;
    int phy_blk_no;

    p_inode = gUser.cur_dir_node_;
    p_buffer = NULL;
    gUser.io_param_.offset_ = 0;
    gUser.io_param_.count_ = p_inode->i_size_ / sizeof(DirectoryEntry);  // Ŀ¼��

    /* ��Ŀ¼������ѭ�� */
    while (gUser.io_param_.count_) {

        /* �Ѷ���Ŀ¼�ļ��ĵ�ǰ�̿飬��Ҫ������һĿ¼�������̿� */
        if (0 == gUser.io_param_.offset_ % INode::BLOCK_SIZE) {
            if (p_buffer) 
                gBufferManager.BRelease(p_buffer);
            
            /* ����Ҫ���������̿�� */
            phy_blk_no = p_inode->BMap(gUser.io_param_.offset_ / INode::BLOCK_SIZE);
            p_buffer = gBufferManager.BRead(phy_blk_no);
        }

        /* û�ж��굱ǰĿ¼���̿飬���ȡ��һĿ¼�� */
        memcpy(&gUser.dir_entry_, p_buffer->b_addr_ + (gUser.io_param_.offset_ % INode::BLOCK_SIZE), sizeof(DirectoryEntry));
        gUser.io_param_.offset_ += sizeof(DirectoryEntry);
        gUser.io_param_.count_--;

        /* ������Ŀ¼�� */
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
* ���ܣ����ļ�
* Ч�����������ļ��ṹ���ڴ�i�ڵ㿪�� ��i_count Ϊ������i_count ++��
* */
void FileManager::Fopen() 
{
    INode* p_inode;

    p_inode = NameI(FileManager::FM_OPEN);

    /* û���ҵ���Ӧ��Inode */
    if (NULL == p_inode) 
        return;

    Open1(p_inode, gUser.arg_[1], 0);
}

void FileManager::Fcreat() 
{
    INode* p_inode;
    unsigned int mode;

    mode = gUser.arg_[1];

    /* ����Ŀ¼��ģʽΪ1����ʾ����������Ŀ¼����д�������� */
    p_inode = NameI(FileManager::FM_CREATE);

    /* û���ҵ���Ӧ��INode����NameI���� */
    if (NULL == p_inode) {
        if (gUser.u_error_)
            return;

        p_inode = MakNode(mode);
        if (NULL == p_inode)
            return;

        /* ������������ֲ����ڣ�ʹ�ò���trf = 2������open1()��*/
        Open1(p_inode, File::FWRITE, 2);
        return;
    }

    /* ���NameI()�������Ѿ�����Ҫ�������ļ��������� */
    Open1(p_inode, File::FWRITE, 1);
    p_inode->i_mode_ |= mode;
}

/*
* trf == 0��open����
* trf == 1��creat���ã�creat�ļ���ʱ��������ͬ�ļ������ļ�
* trf == 2��creat���ã�creat�ļ���ʱ��δ������ͬ�ļ������ļ��������ļ�����ʱ��һ������
* mode���������ļ�ģʽ����ʾ�ļ������� ����д���Ƕ�д
*/
void FileManager::Open1(INode* p_inode, int mode, int trf)
{
    File* p_file;
    int fd;

    /* ��creat�ļ���ʱ��������ͬ�ļ������ļ��������� */
    if (1 == trf) {
        gINodeTable.IPut(p_inode);
        return;
    }

    /* ������ļ����ƿ�File�ṹ */
    p_file = gOpenFileTable.FAlloc();
    if (NULL == p_file) {
        gINodeTable.IPut(p_inode);
        return;
    }

    /* ���ô��ļ���ʽ������File�ṹ���ڴ�INode�Ĺ�����ϵ */
    p_file->flag_ = mode & (File::FREAD | File::FWRITE);
    p_file->inode_ = p_inode;

    /* Ϊ�򿪻��ߴ����ļ��ĸ�����Դ���ѳɹ����䣬�������� */
    if (gUser.u_error_ == User::U_NO_ERROR)
        return;
    /* ����������ͷ���Դ */
    else {
        /* �ͷŴ��ļ������� */
        fd = gUser.ar0_[User::EAX];
        if (fd != NO_OPEN_FILE_SLOT) {
            gUser.ofiles_.SetF(fd, NULL);
            /* �ݼ�File�ṹ��INode�����ü��� ,File�ṹû���� f_countΪ0�����ͷ�File�ṹ��*/
            p_file->count_--;
        }
        gINodeTable.IPut(p_inode);
    }
}

/* ����NULL��ʾĿ¼����ʧ�ܣ�δ�ҵ�u.dirp��ָ��Ŀ¼ȫ·��
 * �����Ǹ�ָ�룬ָ���ļ����ڴ��i�ڵ�
 */
INode* FileManager::NameI(enum DirectorySearchMode mode) 
{
    INode* p_inode;
    BufferCtrl* p_buffer;
    int free_entry_offset;  // �˱�������˿���Ŀ¼��λ��Ŀ¼�ļ��е�ƫ����
    int phy_blk_no;  // ������
    unsigned int index;  // ·������ʼλ������
    unsigned int next_index;  //·��������λ������

    p_inode = gUser.cur_dir_node_;
    index = 0;
    next_index = 0;

    /* ���Ϊ����·�����л�����Ŀ¼��� */
    if ('/' == gUser.dirp_[0]) {
        index = 1;
        next_index = 1;
        p_inode = root_dir_node_;
    }

    /* ���ѭ��ÿ�δ���pathname��һ��·������ */
    while (true) {

        /* ����������ͷŵ�ǰ��������Ŀ¼�ļ�Inode�����˳� */
        if (gUser.u_error_ != User::U_NO_ERROR) 
            break;

        /* ������� */
        if (next_index >= gUser.dirp_.length()) 
            return p_inode;   

        /* ����������Ĳ���Ŀ¼���ͷ����INode��Դ���˳� */
        if ((p_inode->i_mode_ & INode::I_MASK) != INode::I_DIR) {
            gUser.u_error_ = User::U_ERR_NOT_DIR;
            break;
        }

        // �ҵ�ǰ·������β���Ҳ���˵��Ϊ���һ��
        next_index = gUser.dirp_.find_first_of('/', index);
        memset(gUser.dir_buf_, 0, sizeof(gUser.dir_buf_));  // ��ʼ����ʱ·��

        /* ����ǰ·����д����ʱ·����*/
        if (next_index == (unsigned int)string::npos)
            memcpy(gUser.dir_buf_, gUser.dirp_.data() + index, gUser.dirp_.length() - index);
        else
            memcpy(gUser.dir_buf_, gUser.dirp_.data() + index, next_index - index);

        index = next_index + 1;  // ��������ֵΪ��һ��·�����

        /* �ڲ�ѭ�����ֶ���u.dbuf[]�е�·���������������Ѱƥ���Ŀ¼�� */
        gUser.io_param_.offset_ = 0;

        /* ����ΪĿ¼����� �����հ׵�Ŀ¼��*/
        gUser.io_param_.count_ = p_inode->i_size_ / sizeof(DirectoryEntry);
        free_entry_offset = 0;
        p_buffer = NULL;

        /* ��һ��Ŀ¼��Ѱ�� */
        while (true) {

            /* ��Ŀ¼���Ѿ�������� */
            if (0 == gUser.io_param_.count_) {
                if (NULL != p_buffer) 
                    gBufferManager.BRelease(p_buffer);

                /* ����Ǵ������ļ�������ȫ�������� */
                if (FileManager::FM_CREATE == mode && next_index >= gUser.dirp_.length()) {

                    /* ����Ŀ¼Inodeָ�뱣���������Ժ�дĿ¼��WriteDir()�������õ� */
                    gUser.p_dir_node_ = p_inode;

                    /* ������Ŀ¼��ƫ��������gUser���У�дĿ¼��WriteDir()���õ� */
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

            /* �Ѷ���Ŀ¼�ļ��ĵ�ǰ�̿飬��Ҫ������һĿ¼�������̿� */
            if (0 == gUser.io_param_.offset_ % INode::BLOCK_SIZE) {
                if (p_buffer) 
                    gBufferManager.BRelease(p_buffer);
                
                /* ����Ҫ���������̿�� */
                phy_blk_no = p_inode->BMap(gUser.io_param_.offset_ / INode::BLOCK_SIZE);
                p_buffer = gBufferManager.BRead(phy_blk_no);
            }

            /* û�ж��굱ǰĿ¼���̿飬���ȡ��һĿ¼����gUser.dir_entry_ */
            memcpy(&gUser.dir_entry_, p_buffer->b_addr_ + (gUser.io_param_.offset_ % INode::BLOCK_SIZE), sizeof(DirectoryEntry));
            gUser.io_param_.offset_ += sizeof(DirectoryEntry);
            gUser.io_param_.count_--;

            /* ����ǿ���Ŀ¼���¼����λ��Ŀ¼�ļ���ƫ���� */
            if (0 == gUser.dir_entry_.inode_no_) {
                if (0 == free_entry_offset) 
                    free_entry_offset = gUser.io_param_.offset_ - sizeof(DirectoryEntry);

                continue;
            }

            /* ƥ��ɹ������� */
            if (0 == memcmp(gUser.dir_buf_, &gUser.dir_entry_.dir_name_, DirectoryEntry::DIR_MAX_SIZE)) 
                break;
        }

        if (p_buffer) 
            gBufferManager.BRelease(p_buffer);

        /* �����ɾ���������򷵻ظ�Ŀ¼INode����Ҫɾ���ļ���INode����u.dent.m_ino�� */
        if (FileManager::FM_DELETE == mode && next_index >= gUser.dirp_.length()) 
            return p_inode;

        /* ƥ��Ŀ¼��ɹ������ͷŵ�ǰĿ¼INode������ƥ��ɹ���
         * Ŀ¼��inode_no�ֶλ�ȡ��Ӧ��һ��Ŀ¼���ļ���INode��*/
        gINodeTable.IPut(p_inode);
        p_inode = gINodeTable.IGet(gUser.dir_entry_.inode_no_);

        /* ��ȡʧ�� */
        if (NULL == p_inode) 
            return NULL;
    }

    gINodeTable.IPut(p_inode);

    return NULL;
}



/* Ϊ�´������ļ�д�µ�i�ڵ�͸�Ŀ¼���µ�Ŀ¼��(��Ӧ������User�ṹ��)
 * ���ص�p_inode�����������ڴ�i�ڵ㣬���е�i_count�� 1��
 */
INode* FileManager::MakNode(unsigned int mode) 
{
    INode* p_inode;

    /* ����һ������DiskInode������������ȫ����� */
    p_inode = gFileSystem.IAlloc();
    if (NULL == p_inode) 
        return NULL;
   
    p_inode->i_flag_ |= (INode::I_ACCESS | INode::I_UPDATE);

    /* ��ע��INode�����ļ� */
    p_inode->i_mode_ = mode | INode::I_ALLOC;
    p_inode->i_nlink_ = 1;

    /* ��Ŀ¼��д��gUser.u_dir_entry_�����д��Ŀ¼�ļ� */
    WriteDir(p_inode);

    return p_inode;
}

/* �������Լ���Ŀ¼��д����Ŀ¼���޸ĸ�Ŀ¼�ļ���inode�ڵ� ������д�ش��̡�*/
void FileManager::WriteDir(INode* p_inode) 
{
    /* ����Ŀ¼����INode��Ų��� */
    gUser.dir_entry_.inode_no_ = p_inode->i_number_;

    /* ����Ŀ¼����pathname�������� */
    memcpy(gUser.dir_entry_.dir_name_, gUser.dir_buf_, DirectoryEntry::DIR_MAX_SIZE);

    gUser.io_param_.count_ = sizeof(DirectoryEntry);
    gUser.io_param_.base_ = (unsigned char*)&gUser.dir_entry_;

    /* ��Ŀ¼��д�븸Ŀ¼�ļ� */
    gUser.p_dir_node_->WriteI();
    gINodeTable.IPut(gUser.p_dir_node_);
}

void FileManager::Fclose() 
{
    File* p_file;
    int fd;

    /* �ļ�������ͨ��arg[0]���� */
    fd = gUser.arg_[0];

    /* ��ȡ���ļ����ƿ�File�ṹ */
    p_file = gUser.ofiles_.GetF(fd);
    if (NULL == p_file) 
        return;
    
    /* �ͷŴ��ļ�������fd���ݼ�File�ṹ���ü��� */
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
    
    /* д��������Ŀ¼�� */
    gUser.io_param_.offset_ -= sizeof(DirectoryEntry);
    gUser.io_param_.base_ = (unsigned char*)&gUser.dir_entry_;
    gUser.io_param_.count_ = sizeof(DirectoryEntry);

    gUser.dir_entry_.inode_no_ = 0;
    p_delete_node->WriteI();

    /* �޸�inode�� */
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

    fd = gUser.arg_[0];  /* ����ͨ��arg_[0]���� */

    p_file = gUser.ofiles_.GetF(fd);
    /* ��FILE�����ڣ�GetF��������� */
    if (NULL == p_file) 
        return;  
    
    offset = gUser.arg_[1];
    whence = gUser.arg_[2];

    switch (whence) {
    /* ���ļ���ͷ��ʼ */
    case SEEK_SET:
        p_file->offset_ = offset;
        break;

    /* �ӵ�ǰλ�ÿ�ʼ */
    case SEEK_CUR:
        p_file->offset_ += offset;
        break;
        /* ��дλ�õ���Ϊ�ļ����ȼ�offset */
    case SEEK_END:
        p_file->offset_ = p_file->inode_->i_size_ + offset;
        break;
    }
}

/* ֱ�ӵ���Rdwr()�������� */
void FileManager::Fread() 
{
    Rdwr(File::FREAD);
}

/* ֱ�ӵ���Rdwr()�������� */
void FileManager::Fwrite() 
{
    Rdwr(File::FWRITE);
}

void FileManager::Rdwr(enum File::FileFlags mode) 
{
    File* p_file;

    /* ����Read()/Write()��ϵͳ���ò���fd��ȡ���ļ����ƿ�ṹ */
    p_file = gUser.ofiles_.GetF(gUser.arg_[0]);	

    /* �����ڸô��ļ���GetF�Ѿ����ù������룬�������ﲻ��Ҫ�������� */
    if (NULL == p_file) 
        return;
    
    /* ��д��ģʽ����ȷ */
    if ((p_file->flag_ & mode) == 0) {
        gUser.u_error_ = User::U_ERR_ACCES;
        return;
    }

    /* Ŀ�껺������ַ */
    gUser.io_param_.base_ = (unsigned char*)gUser.arg_[1];     

    /* Ҫ���/д���ֽ��� */
    gUser.io_param_.count_ = gUser.arg_[2];		

    /* �����ļ���ʼ��λ�� */
    gUser.io_param_.offset_ = p_file->offset_;
    if (File::FREAD == mode) 
        p_file->inode_->ReadI();
    else 
        p_file->inode_->WriteI();

    /* ���ݶ�д�������ƶ��ļ���дƫ��ָ�� */
    p_file->offset_ += (gUser.arg_[2] - gUser.io_param_.count_);

    /* ����ʵ�ʶ�д���ֽ������޸Ĵ��ϵͳ���÷���ֵ�ĺ���ջ��Ԫ */
    gUser.ar0_[User::EAX] = gUser.arg_[2] - gUser.io_param_.count_;
}

