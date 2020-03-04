// 1752149 �����
#ifndef  BufferManager_H
#define BufferManager_H

#include <unordered_map>

#include "buffer.h"
#include "device_manager.h"

using namespace std;

class BufferManager 
{
public:
    /* ������ƿ顢������������ */
    static const int BUF_NUM = 100;

private:
    /* ���ɻ�����п��ƿ� */
    BufferCtrl buf_free_list_;

    /* ������ƿ����� */
    BufferCtrl buf_ctrls_[BUF_NUM];

    /* ���������� */
    Buffer buffers_[BUF_NUM];
    unordered_map<int, BufferCtrl*> buffer_map_;

public:
    BufferManager();
    ~BufferManager();

    /* ����һ�黺�棬���ڶ�д�豸�ϵĿ�block_no��*/
    BufferCtrl* GetBlk(int block_no);

    /* �ͷŻ�����ƿ�buf */
    void BRelease(BufferCtrl* p_buffer);

    /* ��һ�����̿飬block_noΪĿ����̿��߼���š� */
    BufferCtrl* BRead(int block_no);

    /* дһ�����̿� */
    void BWrite(BufferCtrl* p_buffer);

    /* �ӳ�д���̿� */
    void BDWrite(BufferCtrl* p_buffer);

    /* ��ջ��������� */
    void BClear(BufferCtrl* p_buffer);

    /* ���������ӳ�д�Ļ���ȫ����������� */
    void BFlush();

    /* ��ȡ���п��ƿ�Buf�������� */
    //BufferCtrl& GetFreeBuffer();				

    void FormatBuffer();

private:
    /* ��ʼ�� */
    void Initialize();
    void NotAvail(BufferCtrl* p_buffer);
};

#endif