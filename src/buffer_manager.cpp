// 1752149 �����

#include <iostream>

#include "include/buffer_manager.h"
#include "include/utility.h"
#include "include/main.h"

using namespace std;

extern DeviceManager gDeviceManager;


/*
 *	BufferCtrlֻ�õ���������־��B_DONE��B_DEL_WRI���ֱ��ʾ�Ѿ����IO���ӳ�д�ı�־��
 *	����BufferCtrl���κα�־
*/
BufferManager::BufferManager()
{
    Initialize();
}

BufferManager::~BufferManager() 
{
    BFlush();
}

/* ��ʽ������BufferCtrl */
void BufferManager::FormatBuffer() 
{
    int i;
    BufferCtrl temp;

    for (i = 0; i < BUF_NUM; i++) 
        memcpy(buffers_ + i, &temp, sizeof(BufferCtrl));
    
    Initialize();
}

/* ��ʼ�� */
void BufferManager::Initialize() 
{
    int i;
    BufferCtrl* p_buffer;

    buf_free_list_.b_p_forw_ = &buf_free_list_;
    buf_free_list_.b_p_back_ = &buf_free_list_;

    for (i = 0; i < BUF_NUM; i++) {
        p_buffer = &buf_ctrls_[i];
        p_buffer->b_addr_ = buffers_[i];

        /* ��ʼ�����ɶ��� */
        BRelease(p_buffer);
    }

    return;
}

/* LRU��ȡ������� */
void BufferManager::NotAvail(BufferCtrl* p_buffer)
{
    p_buffer->b_p_back_->b_p_forw_ = p_buffer->b_p_forw_;
    p_buffer->b_p_forw_->b_p_back_ = p_buffer->b_p_back_;
}

/* ����һ�黺�棬�ӻ��������ȡ�������ڶ�д�豸�ϵĿ�block_no��*/
BufferCtrl* BufferManager::GetBlk(int block_no) 
{
    BufferCtrl* p_buffer;

    if (!buffer_map_.empty() && buffer_map_.find(block_no) != buffer_map_.end()) {
        p_buffer = buffer_map_[block_no];
        NotAvail(p_buffer);

        return p_buffer;
    }

    p_buffer = buf_free_list_.b_p_forw_;
    if (p_buffer == &buf_free_list_) {
        cout << "��BufferCtrl�ɹ�ʹ��" << endl;
        return NULL;
    }

    NotAvail(p_buffer);
    buffer_map_.erase(p_buffer->b_block_no_);

    /* �����ӳ�д��ǣ���д�� */
    if (p_buffer->b_flags_ & BufferCtrl::B_DEL_WRI) 
        gDeviceManager.Write(p_buffer->b_addr_, BUFFER_SIZE, p_buffer->b_block_no_ * BUFFER_SIZE);
    
    p_buffer->b_flags_ &= ~BufferCtrl::B_DEL_WRI;
    p_buffer->b_block_no_ = block_no;
    buffer_map_[block_no] = p_buffer;

    return p_buffer;
}

/* �ͷŻ�����ƿ�buf��������뵽�ײ� */
void BufferManager::BRelease(BufferCtrl* p_buffer) 
{
    buf_free_list_.b_p_back_->b_p_forw_ = p_buffer;
    p_buffer->b_p_back_ = buf_free_list_.b_p_back_;
    p_buffer->b_p_forw_ = &buf_free_list_;
    buf_free_list_.b_p_back_ = p_buffer;
}

/* ��һ�����̿飬block_noΪĿ����̿��߼���š� */
BufferCtrl* BufferManager::BRead(int block_no) 
{
    BufferCtrl* p_buffer;

    p_buffer = GetBlk(block_no);

    /* �����ӳ�д��ǣ���д�� */
    if (p_buffer->b_flags_ & BufferCtrl::B_DEL_WRI) 
        return p_buffer;
    
    gDeviceManager.Read(p_buffer->b_addr_, BUFFER_SIZE, p_buffer->b_block_no_ * BUFFER_SIZE);

    return p_buffer;
}

/* дһ�����̿� */
void BufferManager::BWrite(BufferCtrl* p_buffer) 
{
    p_buffer->b_flags_ &= ~(BufferCtrl::B_DEL_WRI);  // ȥ���ӳ�д���
    gDeviceManager.Write(p_buffer->b_addr_, BUFFER_SIZE, p_buffer->b_block_no_ * BUFFER_SIZE, SEEK_SET);
    BRelease(p_buffer);
}

/* �ӳ�д���̿� */
void BufferManager::BDWrite(BufferCtrl* p_buffer) 
{
    p_buffer->b_flags_ |= (BufferCtrl::B_DEL_WRI);  // ��ӱ��
    BRelease(p_buffer);
}

/* ��ջ��������� */
void BufferManager::BClear(BufferCtrl* p_buffer) 
{
    memset(p_buffer->b_addr_, 0, BUFFER_SIZE);
}

/* ���������ӳ�д�Ļ���ȫ����������� */
void BufferManager::BFlush() 
{
    int i;
    BufferCtrl* p_buffer;

    p_buffer = NULL;

    for (i = 0; i < BUF_NUM; i++) {
        p_buffer = buf_ctrls_ + i;

        if ((p_buffer->b_flags_ & BufferCtrl::B_DEL_WRI)) {
            p_buffer->b_flags_ &= ~(BufferCtrl::B_DEL_WRI);  // ȥ���ӳ�д���
            gDeviceManager.Write(p_buffer->b_addr_, BUFFER_SIZE, p_buffer->b_block_no_ * BUFFER_SIZE, SEEK_SET);
        }
    }
}
