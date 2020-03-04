// 1752149 �����

#ifndef OPEN_FILE_MANAGER_H
#define OPEN_FILE_MANAGER_H

#include "file.h"
#include "file_system.h"

/*
 * ���ļ�������(OpenFileManager)�����ں��жԴ��ļ������Ĺ���Ϊ���̴��ļ�������
 * �����ݽṹ֮��Ĺ�����ϵ��
 * ������ϵָ����u���д��ļ�������ָ����ļ����е�File���ļ����ƽṹ���Լ���File
 * �ṹָ���ļ���Ӧ���ڴ�INode��
 */
class OpenFileTable 
{
public:
    static const int MAX_FILES = 100;	/* ���ļ����ƿ�File�ṹ������ */

public:
    /* ϵͳ���ļ���Ϊ���н��̹������̴��ļ���������
     * �а���ָ����ļ����ж�ӦFile�ṹ��ָ�롣
     */
    File files_[MAX_FILES];

public:

    /* ��ϵͳ���ļ����з���һ�����е�File�ṹ */
    File* FAlloc();

    /* �Դ��ļ����ƿ�File�ṹ�����ü���count��1�������ü���countΪ0�����ͷ�File�ṹ��*/
    void CloseF(File* pFile);

    /* ��ʽ�� */
    void Format();
};

/*
 * �ڴ�INode��
 * �����ڴ�INode�ķ�����ͷš�
 */
class INodeTable 
{
public:
    /* �ڴ�INode������ */
    static const int INODE_NUM = 100;

private:
    /* �ڴ�INode���飬ÿ�����ļ�����ռ��һ���ڴ�INode */
    INode inodes_[INODE_NUM];

public:
    /* �������INode��Ż�ȡ��ӦINode�������INode�Ѿ����ڴ��У����ظ��ڴ�INode��
    ��������ڴ��У���������ڴ沢���ظ��ڴ�INode������NULL:INode Table OverFlow! */
    INode* IGet(int inode_num);

    /* ���ٸ��ڴ�INode�����ü����������INode�Ѿ�û��Ŀ¼��ָ������
    ���޽������ø�INode�����ͷŴ��ļ�ռ�õĴ��̿顣*/
    void IPut(INode* pNode);

    /* �����б��޸Ĺ����ڴ�INode���µ���Ӧ���INode�� */
    void UpdateINodeTable();

    /* �����Ϊinode_num�����INode�Ƿ����ڴ濽����������򷵻ظ��ڴ�INode���ڴ�INode���е����� */
    int IsLoaded(int inode_num);

    /* ���ڴ�INode����Ѱ��һ�����е��ڴ�INode */
    INode* GetFreeINode();

    
    void Format();
};

#endif
