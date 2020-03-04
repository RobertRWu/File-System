// 1752149 吴子睿
#ifndef  User_H
#define User_H

#include "file_manager.h"
#include <string>
using namespace std;

/*  */
class User
{
public:
    static const int EAX = 0;	/* u.ar0_[EAX]；访问现场保护区中EAX寄存器的偏移量 */

    enum ErrorCode {
        U_NO_ERROR = 0,  // No error
        U_ERR_NO_ENT = 2,  // No such file or directory
        //U_EIO = 5,	    /* I/O u_error */
        //U_E2BIG = 7,	    /* Arg list too long */
        //U_ENOEXEC = 8,	/* Exec format u_error */
        U_ERR_BAD_F = 9,  // Bad file number
        U_ERR_ACCES = 13,  // Permission denied
        //U_EEXIST = 17,	/* File exists */
        U_ERR_NOT_DIR = 20,  // Not a directory
        //U_EISDIR = 21,	/* Is a directory */
        U_ERR_NFILE = 23,  // File table overflow
        U_ERR_M_FILE = 24,  // Too many open files
        //U_ETXTBSY = 26,	/* Text file busy */
        U_ERR_F_BIG = 27,  // File too large
        U_ERR_NO_SPC = 28,  // No space left on device
        //U_ESPIPE = 29,	/* Illegal seek */
        //U_EROFS = 30,	    /* Read-only file system */
        //U_EMLINK = 31,	/* Too many links */
        //U_EPIPE = 32,	    /* Broken pipe */
    };

public:
    /* 指向当前目录的Inode指针 */
    INode* cur_dir_node_;

    /* 指向父目录的Inode指针 */
    INode* p_dir_node_;

    /* 当前目录的目录项 */
    DirectoryEntry dir_entry_;

    /* 当前路径分量 */
    char dir_buf_[DirectoryEntry::DIR_MAX_SIZE];

    /* 当前工作目录完整路径 */
    string cur_dir_path_;

    /* 系统调用参数(一般用于Pathname)的指针，记录目标路径，
     * 若目标路径是绝对路径或者包含父目录的路径，均存为绝对路径形式 */
    string dirp_;

    /* 存放当前系统调用参数 */
    long arg_[5];	
    
    /* 指向核心栈现场保护区中EAX寄存器
       存放的栈单元，本字段存放该栈单元的地址。
       在V6中r0存放系统调用的返回值给用户程序，
       x86平台上使用EAX存放返回值，替代u.ar0_[R0] */
    unsigned int ar0_[5];	    

    /* 存放错误码 */
    ErrorCode u_error_;

    OpenFiles ofiles_;		    /* 进程打开文件描述符表对象 */

    /* 记录当前读、写文件的偏移量，用户目标区域和剩余字节数参数 */
    IOParameter io_param_;

public:
    User();
    void ChangeDir(string dir_name);
    void List();
    void MakeDir(string dir_name);
    void CreateFile(string file_name);
    void OpenFile(string file_name, string mode);
    void CloseFile(string fd);
    void ReadFile(string fd, string str_read_size, string option, string file_name);
    void WriteFile(string fd, string str_write_size, string option, string file_name);
    void Seek(string str_fd, string offset, string whence);
    void DeleteFile(string file_name);

private:
    /* 打印错误信息 */
    void PrintError();

    /* 获取打开文件模式 */
    int GetFileMode(string str_mode);

    /* 检查路径名 */
    bool CheckPath(string path);

    /* 检查是否缺少参数 */
    bool CheckMissingParam(string param);

    /* 检查给定字符串是否只包含数字字符 */
    bool CheckInt(string param1, string param2="1", string param3="1");
};

#endif