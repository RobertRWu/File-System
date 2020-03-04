// 1752149 �����
#ifndef  User_H
#define User_H

#include "file_manager.h"
#include <string>
using namespace std;

/*  */
class User
{
public:
    static const int EAX = 0;	/* u.ar0_[EAX]�������ֳ���������EAX�Ĵ�����ƫ���� */

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
    /* ָ��ǰĿ¼��Inodeָ�� */
    INode* cur_dir_node_;

    /* ָ��Ŀ¼��Inodeָ�� */
    INode* p_dir_node_;

    /* ��ǰĿ¼��Ŀ¼�� */
    DirectoryEntry dir_entry_;

    /* ��ǰ·������ */
    char dir_buf_[DirectoryEntry::DIR_MAX_SIZE];

    /* ��ǰ����Ŀ¼����·�� */
    string cur_dir_path_;

    /* ϵͳ���ò���(һ������Pathname)��ָ�룬��¼Ŀ��·����
     * ��Ŀ��·���Ǿ���·�����߰�����Ŀ¼��·��������Ϊ����·����ʽ */
    string dirp_;

    /* ��ŵ�ǰϵͳ���ò��� */
    long arg_[5];	
    
    /* ָ�����ջ�ֳ���������EAX�Ĵ���
       ��ŵ�ջ��Ԫ�����ֶδ�Ÿ�ջ��Ԫ�ĵ�ַ��
       ��V6��r0���ϵͳ���õķ���ֵ���û�����
       x86ƽ̨��ʹ��EAX��ŷ���ֵ�����u.ar0_[R0] */
    unsigned int ar0_[5];	    

    /* ��Ŵ����� */
    ErrorCode u_error_;

    OpenFiles ofiles_;		    /* ���̴��ļ������������ */

    /* ��¼��ǰ����д�ļ���ƫ�������û�Ŀ�������ʣ���ֽ������� */
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
    /* ��ӡ������Ϣ */
    void PrintError();

    /* ��ȡ���ļ�ģʽ */
    int GetFileMode(string str_mode);

    /* ���·���� */
    bool CheckPath(string path);

    /* ����Ƿ�ȱ�ٲ��� */
    bool CheckMissingParam(string param);

    /* �������ַ����Ƿ�ֻ���������ַ� */
    bool CheckInt(string param1, string param2="1", string param3="1");
};

#endif