// 1752149 吴子睿

#include <iostream>

#include "include/buffer_manager.h"
#include "include/utility.h"
#include "include/main.h"

using namespace std;

extern DeviceManager gDeviceManager;


/*
 *	BufferCtrl只用到了两个标志，B_DONE和B_DEL_WRI，分别表示已经完成IO和延迟写的标志。
 *	空闲BufferCtrl无任何标志
*/
BufferManager::BufferManager()
{
    Initialize();
}

BufferManager::~BufferManager() 
{
    BFlush();
}

/* 格式化所有BufferCtrl */
void BufferManager::FormatBuffer() 
{
    int i;
    BufferCtrl temp;

    for (i = 0; i < BUF_NUM; i++) 
        memcpy(buffers_ + i, &temp, sizeof(BufferCtrl));
    
    Initialize();
}

/* 初始化 */
void BufferManager::Initialize() 
{
    int i;
    BufferCtrl* p_buffer;

    buf_free_list_.b_p_forw_ = &buf_free_list_;
    buf_free_list_.b_p_back_ = &buf_free_list_;

    for (i = 0; i < BUF_NUM; i++) {
        p_buffer = &buf_ctrls_[i];
        p_buffer->b_addr_ = buffers_[i];

        /* 初始化自由队列 */
        BRelease(p_buffer);
    }

    return;
}

/* LRU，取出缓存块 */
void BufferManager::NotAvail(BufferCtrl* p_buffer)
{
    p_buffer->b_p_back_->b_p_forw_ = p_buffer->b_p_forw_;
    p_buffer->b_p_forw_->b_p_back_ = p_buffer->b_p_back_;
}

/* 申请一块缓存，从缓存队列中取出，用于读写设备上的块block_no。*/
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
        cout << "无BufferCtrl可供使用" << endl;
        return NULL;
    }

    NotAvail(p_buffer);
    buffer_map_.erase(p_buffer->b_block_no_);

    /* 若有延迟写标记，则写回 */
    if (p_buffer->b_flags_ & BufferCtrl::B_DEL_WRI) 
        gDeviceManager.Write(p_buffer->b_addr_, BUFFER_SIZE, p_buffer->b_block_no_ * BUFFER_SIZE);
    
    p_buffer->b_flags_ &= ~BufferCtrl::B_DEL_WRI;
    p_buffer->b_block_no_ = block_no;
    buffer_map_[block_no] = p_buffer;

    return p_buffer;
}

/* 释放缓存控制块buf，将其插入到首部 */
void BufferManager::BRelease(BufferCtrl* p_buffer) 
{
    buf_free_list_.b_p_back_->b_p_forw_ = p_buffer;
    p_buffer->b_p_back_ = buf_free_list_.b_p_back_;
    p_buffer->b_p_forw_ = &buf_free_list_;
    buf_free_list_.b_p_back_ = p_buffer;
}

/* 读一个磁盘块，block_no为目标磁盘块逻辑块号。 */
BufferCtrl* BufferManager::BRead(int block_no) 
{
    BufferCtrl* p_buffer;

    p_buffer = GetBlk(block_no);

    /* 若有延迟写标记，则写回 */
    if (p_buffer->b_flags_ & BufferCtrl::B_DEL_WRI) 
        return p_buffer;
    
    gDeviceManager.Read(p_buffer->b_addr_, BUFFER_SIZE, p_buffer->b_block_no_ * BUFFER_SIZE);

    return p_buffer;
}

/* 写一个磁盘块 */
void BufferManager::BWrite(BufferCtrl* p_buffer) 
{
    p_buffer->b_flags_ &= ~(BufferCtrl::B_DEL_WRI);  // 去除延迟写标记
    gDeviceManager.Write(p_buffer->b_addr_, BUFFER_SIZE, p_buffer->b_block_no_ * BUFFER_SIZE, SEEK_SET);
    BRelease(p_buffer);
}

/* 延迟写磁盘块 */
void BufferManager::BDWrite(BufferCtrl* p_buffer) 
{
    p_buffer->b_flags_ |= (BufferCtrl::B_DEL_WRI);  // 添加标记
    BRelease(p_buffer);
}

/* 清空缓冲区内容 */
void BufferManager::BClear(BufferCtrl* p_buffer) 
{
    memset(p_buffer->b_addr_, 0, BUFFER_SIZE);
}

/* 将队列中延迟写的缓存全部输出到磁盘 */
void BufferManager::BFlush() 
{
    int i;
    BufferCtrl* p_buffer;

    p_buffer = NULL;

    for (i = 0; i < BUF_NUM; i++) {
        p_buffer = buf_ctrls_ + i;

        if ((p_buffer->b_flags_ & BufferCtrl::B_DEL_WRI)) {
            p_buffer->b_flags_ &= ~(BufferCtrl::B_DEL_WRI);  // 去掉延迟写标记
            gDeviceManager.Write(p_buffer->b_addr_, BUFFER_SIZE, p_buffer->b_block_no_ * BUFFER_SIZE, SEEK_SET);
        }
    }
}
