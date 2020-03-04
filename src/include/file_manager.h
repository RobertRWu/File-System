// 1752149 吴子睿
#ifndef  FILE_MANAGER_H
#define FILE_MANAGER_H

#include "file_system.h"
#include "open_file_manager.h"

/*
* 文件管理类(FileManager)
* 封装了文件系统的各种系统调用在核心态下处理过程，
* 如对文件的Open()、Close()、Read()、Write()等等
* 封装了对文件系统访问的具体细节。
*/
class FileManager
{
public:
    /* 目录搜索模式，用于NameI()函数 */
    enum DirectorySearchMode
    {
        FM_OPEN = 0,  // 以打开文件方式搜索目录
        FM_CREATE = 1,  // 以新建文件方式搜索目录
        FM_DELETE = 2  // 以删除文件方式搜索目录
    };

public:
    /* 根目录内存INode */
    INode* root_dir_node_;

public:
    FileManager();

    /* 切换当前所在目录 */
    void Cd();

    /* 列出当前INode节点的文件项 */
    void Ls();

    /* Open()系统调用处理过程 */
    void Fopen();

    /* Creat()系统调用处理过程 */
    void Fcreat();

    /* Open()、Creat()系统调用的公共部分 */
    void Open1(INode* p_inode, int mode, int trf);

    /* Close()系统调用处理过程 */
    void Fclose();

    /* Seek()系统调用处理过程 */
    void Fseek();

    /* Read()系统调用处理过程 */
    void Fread();

    /* Write()系统调用处理过程 */
    void Fwrite();

    /* 读写系统调用公共部分代码 */
    void Rdwr(enum File::FileFlags mode);

    /* 目录搜索，将路径转化为相应的INode，返回上锁后的INode */
    INode* NameI(enum DirectorySearchMode mode);

    /* 被Creat()系统调用使用，用于为创建新文件分配内核资源 */
    INode* MakNode(unsigned int mode);

    /* 取消文件 */
    void UnLink();

    /* 向父目录的目录文件写入一个目录项 */
    void WriteDir(INode* p_inode);
};

/*  */
class DirectoryEntry 
{
public:
    /* 目录项中路径部分的最大字符串长度 */
    static const int DIR_MAX_SIZE = 28;

public:
    /* 目录项中INode编号部分 */
    int inode_no_;

    /* 目录项中路径名部分 */
    char dir_name_[DIR_MAX_SIZE];
};

#endif