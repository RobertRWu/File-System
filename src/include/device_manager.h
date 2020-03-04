// 1752149 �����
#ifndef  DeviceManager_H
#define DeviceManager_H

#include <stdio.h>

#include "buffer.h"


class DeviceManager 
{
public:
    /* ���̾����ļ��� */
    static const char* DISK_NAME;

private:
    /* �����ļ�ָ�� */
    FILE* p_file_;

public:
    DeviceManager();
    ~DeviceManager();

    /* �жϴ����ļ��Ƿ���ڣ�p_file��private�� */
    bool Exist();

    /* �������� */
    void Build();

    /* ������д����� */
    void Write(const void* buf, unsigned int size, int offset = 0, unsigned int whence = SEEK_CUR);
    
    /* ��ȡ�������� */
    void Read(void* buf, unsigned int size, int offset = 0, unsigned int whence = SEEK_SET);
};

#endif