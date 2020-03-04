// 1752149 �����
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

/* �жϴ����ļ��Ƿ���ڣ�p_file��private�� */
bool DeviceManager::Exist()
{
    return p_file_ != NULL;
}

/* �������� */
void DeviceManager::Build()
{
    p_file_ = fopen(DISK_NAME, "wb+");

    if (p_file_ == NULL) {
        cout << "���ļ�" << DISK_NAME << "ʧ�ܣ�" << endl;
        exit(-1);
    }
}

/* ������д����� */
void DeviceManager::Write(const void* buf, unsigned int size, int offset, unsigned int whence)
{
    if (offset >= 0) 
        fseek(p_file_, offset, whence);
    
    fwrite(buf, size, 1, p_file_);
    fflush(p_file_);
}

/* ��ȡ�������� */
void DeviceManager::Read(void* buf, unsigned int size, int offset, unsigned int whence)
{
    if (offset >= 0) 
        fseek(p_file_, offset, whence);
    
    fread(buf, size, 1, p_file_);
}