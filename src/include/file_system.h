// 1752149 �����

#ifndef  FILE_SYSTEM_H
#define FILE_SYSTEM_H

#include "inode.h"
#include "device_manager.h"
#include "buffer_manager.h"

/*
 * �ļ�ϵͳ�洢��Դ�����(Super Block)�Ķ��塣
 */
class SuperBlock 
{
public:
    const static int MAX_FREE_NUM = 100;
    const static int MAX_INODE_NUM = 100;

public:
    /* ���INode��ռ�õ��̿��� */
    int	s_isize_;

    /* �̿����� */
    int	s_fsize_;

    /* ֱ�ӹ���Ŀ����̿����� */
    int	s_nfree_;

    /* ֱ�ӹ���Ŀ����̿������� */
    int	s_free_[MAX_FREE_NUM];

    /* ֱ�ӹ���Ŀ������INode���� */
    int	s_ninode_;

    /* ֱ�ӹ���Ŀ������INode������ */
    int	s_inode_[MAX_INODE_NUM];

    /* ���������̿��������־������ɾ����Ȼ����1024 */
    int	s_flock_;

    /* ��������INode���־ */
    int	s_ilock_;

    /* �ڴ���super block�������޸ı�־����ζ����Ҫ��������Ӧ��Super Block */
    int	s_fmod_;

    /* ���ļ�ϵͳֻ�ܶ��� */
    int	s_ronly_;

    /* ���һ�θ���ʱ�� */
    int	s_time_;

    /* ���ʹSuperBlock���С����1024�ֽڣ�ռ��2������ */
    int	padding_[47];
};



class FileSystem 
{
public:
    // Block���С
    static const int BLOCK_SIZE = 512;

    // ����������������
    static const int DISK_SIZE = 16384;

    // ����SuperBlockλ�ڴ����ϵ������ţ�ռ����������
    static const int SUPERBLOCK_START_SECTOR = 0;

    // ���INode��λ�ڴ����ϵ���ʼ������
    static const int INODE_ZONE_START_SECTOR = 2;

    // ���������INode��ռ�ݵ�������
    static const int INODE_ZONE_SIZE = 1022;

    // ���INode���󳤶�Ϊ64�ֽڣ�ÿ�����̿���Դ��512/64 = 8�����INode
    static const int INODE_NUMBER_PER_SECTOR = BLOCK_SIZE / sizeof(DiskINode);

    // �ļ�ϵͳ��Ŀ¼���INode���
    static const int ROOT_INODE_NO = 0;

    // ���INode���ܸ���
    static const int INode_NUMBERS = INODE_ZONE_SIZE * INODE_NUMBER_PER_SECTOR;

    // ����������ʼ������
    static const int DATA_ZONE_START_SECTOR = INODE_ZONE_START_SECTOR + INODE_ZONE_SIZE;

    // �����������������
    static const int DATA_ZONE_END_SECTOR = DISK_SIZE - 1;

    // ������ռ�ݵ���������
    static const int DATA_ZONE_SIZE = DISK_SIZE - DATA_ZONE_START_SECTOR;

public:
    FileSystem();
    ~FileSystem();

    /* ��ʽ��SuperBlock */
    void FormatSuperBlock();

    /* ��ʽ��DiskINode�� */
    void FormatDiskINodes();

    /* ��ʽ���̿��� */
    void FormatDiskBlocks();

    /* ��ʽ�������ļ�ϵͳ */
    void FormatDevice();

    /* ϵͳ��ʼ��ʱ����SuperBlock */
    void LoadSuperBlock();

    /* ��SuperBlock������ڴ渱�����µ��洢�豸��SuperBlock��ȥ */
    void Update();

    /* �ڴ洢�豸dev�Ϸ���һ���������INode��һ�����ڴ����µ��ļ���*/
    INode* IAlloc();

    /* �ͷű��Ϊnumber�����INode��һ������ɾ���ļ���*/
    void IFree(int number);

    /* �ڴ洢�豸�Ϸ�����д��̿� */
    BufferCtrl* Alloc();

    /* �ͷŴ洢�豸dev�ϱ��Ϊblock_no�Ĵ��̿� */
    void Free(int block_no);

};

#endif