// 1752149 吴子睿
#ifndef  BufferManager_H
#define BufferManager_H

#include <unordered_map>

#include "buffer.h"
#include "device_manager.h"

using namespace std;

class BufferManager 
{
public:
    /* 缓存控制块、缓冲区的数量 */
    static const int BUF_NUM = 100;

private:
    /* 自由缓存队列控制块 */
    BufferCtrl buf_free_list_;

    /* 缓存控制块数组 */
    BufferCtrl buf_ctrls_[BUF_NUM];

    /* 缓冲区数组 */
    Buffer buffers_[BUF_NUM];
    unordered_map<int, BufferCtrl*> buffer_map_;

public:
    BufferManager();
    ~BufferManager();

    /* 申请一块缓存，用于读写设备上的块block_no。*/
    BufferCtrl* GetBlk(int block_no);

    /* 释放缓存控制块buf */
    void BRelease(BufferCtrl* p_buffer);

    /* 读一个磁盘块，block_no为目标磁盘块逻辑块号。 */
    BufferCtrl* BRead(int block_no);

    /* 写一个磁盘块 */
    void BWrite(BufferCtrl* p_buffer);

    /* 延迟写磁盘块 */
    void BDWrite(BufferCtrl* p_buffer);

    /* 清空缓冲区内容 */
    void BClear(BufferCtrl* p_buffer);

    /* 将队列中延迟写的缓存全部输出到磁盘 */
    void BFlush();

    /* 获取空闲控制块Buf对象引用 */
    //BufferCtrl& GetFreeBuffer();				

    void FormatBuffer();

private:
    /* 初始化 */
    void Initialize();
    void NotAvail(BufferCtrl* p_buffer);
};

#endif