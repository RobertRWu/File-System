// 1752149 �����

#ifndef FILE_H
#define FILE_H

#include "inode.h"

/*
 * ���ļ����ƿ�File�ࡣ
 * �ýṹ��¼�˽��̴��ļ��Ķ���д�������ͣ��ļ���дλ�õȵȡ�
 */
class File {
public:
    enum FileFlags {
        FREAD = 0x1,			/* ���������� */
        FWRITE = 0x2,			/* д�������� */
    };

public:
    File();

    /* �Դ��ļ��Ķ���д����Ҫ�� */
    unsigned int flag_;

    /* ��ǰ���ø��ļ����ƿ�Ľ������� */
    int	count_;

    /* ָ����ļ����ڴ�INodeָ�� */
    INode* inode_;

    /* �ļ���дλ��ָ�� */
    int	offset_;
};

/*
 * ���̴��ļ���������(OpenFiles)�Ķ���
 * ά���˵�ǰ���̵����д��ļ���
 */
class OpenFiles {
public:
    static const int MAX_FILES = 100;		/* ��������򿪵�����ļ��� */

private:
    /* File�����ָ�����飬ָ��ϵͳ���ļ����е�File���� */
    File* process_open_file_table[MAX_FILES];

public:
    OpenFiles();

    /* ����������ļ�ʱ���ڴ��ļ����������з���һ�����б��� */
    int AllocFreeSlot();

    /* �����û�ϵͳ�����ṩ���ļ�����������fd���ҵ���Ӧ�Ĵ��ļ����ƿ�File�ṹ */
    File* GetF(int fd);

    /* Ϊ�ѷ��䵽�Ŀ���������fd���ѷ���Ĵ��ļ����п���File������������ϵ */
    void SetF(int fd, File* pFile);
};

/*
 * �ļ�I/O�Ĳ�����
 * ���ļ�����дʱ���õ��Ķ���дƫ�������ֽ����Լ�Ŀ�������׵�ַ������
 */
class IOParameter 
{
public:
    /* ��ǰ����д�û�Ŀ��������׵�ַ */
    unsigned char* base_;

    /* ��ǰ����д�ļ����ֽ�ƫ���� */
    int offset_;

    /* ��ǰ��ʣ��Ķ���д�ֽ����� */
    int count_;
};



#endif 
