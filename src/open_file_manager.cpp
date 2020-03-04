// 1752149 �����

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

/* ��ʽ�� */
void OpenFileTable::Format() 
{
    File file;
    int i;

    for (i = 0; i < OpenFileTable::MAX_FILES; i++)
        memcpy(files_ + i, &file, sizeof(File));
}

/* ���ã����̴��ļ������������ҵĿ�����֮�±�д�� ar0[EAX] */
File* OpenFileTable::FAlloc() 
{
    int fd;
    int i;

    fd = gUser.ofiles_.AllocFreeSlot();
    if (fd == NO_OPEN_FILE_SLOT) 
        return NULL;
    
    for (i = 0; i < OpenFileTable::MAX_FILES; i++) {

        /* count==0��ʾ������� */
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

/* �Դ��ļ����ƿ�File�ṹ�����ü���count��1�������ü���countΪ0�����ͷ�File�ṹ��*/
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
 * �����Ϊinode_num�����INode�Ƿ����ڴ濽����
 * ������򷵻ظ��ڴ�INode���ڴ�INode���е�����
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

/* ���ڴ�INode����Ѱ��һ�����е��ڴ�INode */
INode* INodeTable::GetFreeINode() 
{
    int i;

    for (i = 0; i < INodeTable::INODE_NUM; i++) {

        /* ������ڴ�Inode���ü���Ϊ�㣬���Inode��ʾ���� */
        if (inodes_[i].i_count_ == 0) 
            return inodes_ + i;
    }

    return NULL;
}

/*
 * �������INode��Ż�ȡ��ӦINode�������INode�Ѿ����ڴ��У����ظ��ڴ�INode��
 * ��������ڴ��У���������ڴ沢���ظ��ڴ�INode������NULL:INode Table OverFlow!
 */
INode* INodeTable::IGet(int inode_num) 
{
    INode* p_inode;
    int index;
    BufferCtrl* p_buffer;

    index = IsLoaded(inode_num);  // ����Ƿ��������ڴ�

    /* �������ڴ� */
    if (index >= 0) {
        p_inode = inodes_ + index;
        ++p_inode->i_count_;

        return p_inode;
    }

    /* û��Inode���ڴ濽���������һ�������ڴ�Inode */
    else {
        p_inode = GetFreeINode(); 

        /* û�п��õ�inode */
        if (NULL == p_inode) {
            cout << "INode Table Overflow!" << endl;
            gUser.u_error_ = User::U_ERR_NFILE;

            return NULL;
        }

        /* ����Inode��ţ��������ü��� */
        p_inode->i_number_ = inode_num;
        p_inode->i_count_++;
        p_buffer = gBufferManager.BRead(FileSystem::INODE_ZONE_START_SECTOR + inode_num / FileSystem::INODE_NUMBER_PER_SECTOR);
        p_inode->ICopy(p_buffer, inode_num);
        gBufferManager.BRelease(p_buffer);
    }

    return p_inode;
}

/*
 * ���ٸ��ڴ�INode�����ü����������INode�Ѿ�û��Ŀ¼��ָ������
 * ���޽������ø�INode�����ͷŴ��ļ�ռ�õĴ��̿顣
 */
void INodeTable::IPut(INode* p_inode) 
{
    /* ��ǰ����Ϊ���ø��ڴ�INode��Ψһ���̣���׼���ͷŸ��ڴ�INode */
    if (p_inode->i_count_ == 1) {

        /* ���ļ��Ѿ�û��Ŀ¼·��ָ���� */
        if (p_inode->i_nlink_ <= 0) {

            /* �ͷŸ��ļ�ռ�ݵ������̿� */
            p_inode->ITrunc();
            p_inode->i_mode_ = 0;

            /* �ͷŶ�Ӧ�����INode */
            gFileSystem.IFree(p_inode->i_number_);
        }

        /* ����ڴ�INode�����б�־λ */
        p_inode->i_flag_ = 0;

        /* �����ڴ�inode���еı�־֮һ����һ����i_count == 0 */
        p_inode->i_number_ = -1;
    }

    /* �������INode��Ϣ */
    p_inode->IUpdate((int)time(NULL));

    p_inode->i_count_--;
}

/* �����б��޸Ĺ����ڴ�INode���µ���Ӧ���INode�� */
void INodeTable::UpdateINodeTable() 
{
    int i;

    for (i = 0; i < INodeTable::INODE_NUM; i++) {
        if (inodes_[i].i_count_) 
            inodes_[i].IUpdate((int)time(NULL));
    }
}