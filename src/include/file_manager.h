// 1752149 �����
#ifndef  FILE_MANAGER_H
#define FILE_MANAGER_H

#include "file_system.h"
#include "open_file_manager.h"

/*
* �ļ�������(FileManager)
* ��װ���ļ�ϵͳ�ĸ���ϵͳ�����ں���̬�´�����̣�
* ����ļ���Open()��Close()��Read()��Write()�ȵ�
* ��װ�˶��ļ�ϵͳ���ʵľ���ϸ�ڡ�
*/
class FileManager
{
public:
    /* Ŀ¼����ģʽ������NameI()���� */
    enum DirectorySearchMode
    {
        FM_OPEN = 0,  // �Դ��ļ���ʽ����Ŀ¼
        FM_CREATE = 1,  // ���½��ļ���ʽ����Ŀ¼
        FM_DELETE = 2  // ��ɾ���ļ���ʽ����Ŀ¼
    };

public:
    /* ��Ŀ¼�ڴ�INode */
    INode* root_dir_node_;

public:
    FileManager();

    /* �л���ǰ����Ŀ¼ */
    void Cd();

    /* �г���ǰINode�ڵ���ļ��� */
    void Ls();

    /* Open()ϵͳ���ô������ */
    void Fopen();

    /* Creat()ϵͳ���ô������ */
    void Fcreat();

    /* Open()��Creat()ϵͳ���õĹ������� */
    void Open1(INode* p_inode, int mode, int trf);

    /* Close()ϵͳ���ô������ */
    void Fclose();

    /* Seek()ϵͳ���ô������ */
    void Fseek();

    /* Read()ϵͳ���ô������ */
    void Fread();

    /* Write()ϵͳ���ô������ */
    void Fwrite();

    /* ��дϵͳ���ù������ִ��� */
    void Rdwr(enum File::FileFlags mode);

    /* Ŀ¼��������·��ת��Ϊ��Ӧ��INode�������������INode */
    INode* NameI(enum DirectorySearchMode mode);

    /* ��Creat()ϵͳ����ʹ�ã�����Ϊ�������ļ������ں���Դ */
    INode* MakNode(unsigned int mode);

    /* ȡ���ļ� */
    void UnLink();

    /* ��Ŀ¼��Ŀ¼�ļ�д��һ��Ŀ¼�� */
    void WriteDir(INode* p_inode);
};

/*  */
class DirectoryEntry 
{
public:
    /* Ŀ¼����·�����ֵ�����ַ������� */
    static const int DIR_MAX_SIZE = 28;

public:
    /* Ŀ¼����INode��Ų��� */
    int inode_no_;

    /* Ŀ¼����·�������� */
    char dir_name_[DIR_MAX_SIZE];
};

#endif