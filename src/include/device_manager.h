// 1752149 吴子睿
#ifndef  DeviceManager_H
#define DeviceManager_H

#include <stdio.h>

#include "buffer.h"


class DeviceManager 
{
public:
    /* 磁盘镜像文件名 */
    static const char* DISK_NAME;

private:
    /* 磁盘文件指针 */
    FILE* p_file_;

public:
    DeviceManager();
    ~DeviceManager();

    /* 判断磁盘文件是否存在（p_file是private） */
    bool Exist();

    /* 创建磁盘 */
    void Build();

    /* 将数据写入磁盘 */
    void Write(const void* buf, unsigned int size, int offset = 0, unsigned int whence = SEEK_CUR);
    
    /* 读取磁盘数据 */
    void Read(void* buf, unsigned int size, int offset = 0, unsigned int whence = SEEK_SET);
};

#endif