// 1752149 �����
#ifndef  INODE_H
#define INODE_H

#include "buffer.h"

class INode {
public:
    /* INodeFlag�б�־λ */
    enum INodeFlag {
        I_UPDATE = 0x2,		// �ڴ�INode���޸Ĺ�����Ҫ������Ӧ���INode
        I_ACCESS = 0x4,		// �ڴ�INode�����ʹ�����Ҫ�޸����һ�η���ʱ��
    };
    /* �ļ����� */
    static const unsigned int I_FREE = 0x0;

    /* �ļ���ʹ�� */
    static const unsigned int I_ALLOC = 0x8000;

    /* �ļ��������� */
    static const unsigned int I_MASK = 0x6000;

    /* �ļ����ͣ�Ŀ¼�ļ� */
    static const unsigned int I_DIR = 0x4000;

    /* ���豸���������ļ���Ϊ0��ʾ���������ļ� */
    static const unsigned int I_BLK = 0x6000;

    /* �ļ��������ͣ����ͻ�����ļ� */
    static const unsigned int I_LARGE = 0x1000;

    /* ���ļ��Ķ�Ȩ�� */
    static const unsigned int I_READ = 0x100;

    /* ���ļ���дȨ�� */
    static const unsigned int I_WRITE = 0x80;

    /* ���ļ���ִ��Ȩ�� */
    static const unsigned int I_EXEC = 0x40;	

    /* �ļ������ļ��Ķ���д��ִ��Ȩ�� */
    static const unsigned int I_RWX_U = (I_READ | I_WRITE | I_EXEC);	

    /* �ļ���ͬ���û����ļ��Ķ���д��ִ��Ȩ�� */
    static const unsigned int I_RWX_G = ((I_RWX_U) >> 3);

    /* �����û����ļ��Ķ���д��ִ��Ȩ�� */
    static const unsigned int I_RWX_O = ((I_RWX_U) >> 6);

    /* �ļ��߼����С: 512�ֽ� */
    static const int BLOCK_SIZE = 512;

    /* ÿ�����������(��������)�����������̿�� */
    static const int ADDRESS_PER_INDEX_BLOCK = BLOCK_SIZE / sizeof(int);

    /* С���ļ���ֱ������������Ѱַ���߼���� */
    static const int SMALL_FILE_BLOCK = 6;

    /* �����ļ�����һ�μ������������Ѱַ���߼���� */
    static const int LARGE_FILE_BLOCK = 128 * 2 + 6;

    /* �����ļ��������μ����������Ѱַ�ļ��߼���� */
    static const int HUGE_FILE_BLOCK = 128 * 128 * 2 + 128 * 2 + 6;

public:
    /* ״̬�ı�־λ�������enum INodeFlag */
    unsigned int i_flag_;

    /* �ļ�������ʽ��Ϣ */
    unsigned int i_mode_;

    /* ���ü��� */
    int		i_count_;

    /* �ļ���������������ļ���Ŀ¼���в�ͬ·���������� */
    int		i_nlink_;
    
    /* ���INode���еı�� */
    int		i_number_;

    /* �ļ������ߵ��û���ʶ�� */
    short	i_uid_;

    /* �ļ������ߵ����ʶ�� */
    short	i_gid_;

    /* �ļ���С���ֽ�Ϊ��λ */
    int		i_size_;

    /* �����ļ��߼���ź�������ת���Ļ��������� */
    int		i_addr_[10];

public:
    INode();

    /* ����Inode�����е�������̿���������ȡ��Ӧ���ļ����� */
    void ReadI();

    /* ����Inode�����е�������̿�������������д���ļ� */
    void WriteI();

    /* ���ļ����߼����ת���ɶ�Ӧ�������̿�� */
    int BMap(int logic_block_num);

    /* �������Inode�����ķ���ʱ�䡢�޸�ʱ�� */
    void IUpdate(int time);

    /* �ͷ�INode��Ӧ�ļ��ڴ����ϵ��������� */
    void ITrunc();

    /* ���Inode�����е����� */
    void Clean();

    /* ���������Inode�ַ�������Ϣ�������ڴ�Inode�� */
    void ICopy(BufferCtrl* p_buffer, int inode_num);
};

/* ��������ڵ� */
class DiskINode {
public:
    /* ״̬�ı�־λ�������INodeһ�Ѿ�̬���� */
    unsigned int d_mode_;

    /* ���ļ���Ŀ¼���в�ͬ·���������� */
    int	d_nlink_;

    /* �ļ������ߵ��û���ʶ�� */
    short d_uid_;	
    
    /* �ļ������ߵ����ʶ�� */
    short d_gid_;
    
    /* �ļ���С���ֽ�Ϊ��λ */
    int	d_size_;
    
    /* �ļ��߼���ź�������ת���Ļ��������� */
    int	d_addr_[10];
    
    /* ������ʱ�䣬����ɾ���ôչ�64kb */
    int	d_atime_;
    
    /* ����޸�ʱ�� */
    int	d_mtime_;

public:
    DiskINode();
};

#endif