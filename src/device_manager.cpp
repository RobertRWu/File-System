// 1752149 吴子睿
#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include "include/device_manager.h"

using namespace std;

const char* DeviceManager::DISK_NAME = "test.img";

DeviceManager::DeviceManager() 
{
    p_file_ = fopen(DISK_NAME, "rb+");
}

DeviceManager::~DeviceManager() 
{
    if (p_file_) {
        fflush(p_file_);
        fclose(p_file_);
    }
}

/* 判断磁盘文件是否存在（p_file是private） */
bool DeviceManager::Exist()
{
    return p_file_ != NULL;
}

/* 创建磁盘 */
void DeviceManager::Build()
{
    p_file_ = fopen(DISK_NAME, "wb+");

    if (p_file_ == NULL) {
        cout << "打开文件" << DISK_NAME << "失败！" << endl;
        exit(-1);
    }
}

/* 将数据写入磁盘 */
void DeviceManager::Write(const void* buf, unsigned int size, int offset, unsigned int whence)
{
    if (offset >= 0) 
        fseek(p_file_, offset, whence);
    
    fwrite(buf, size, 1, p_file_);
    fflush(p_file_);
}

/* 读取磁盘数据 */
void DeviceManager::Read(void* buf, unsigned int size, int offset, unsigned int whence)
{
    if (offset >= 0) 
        fseek(p_file_, offset, whence);
    
    fread(buf, size, 1, p_file_);
}