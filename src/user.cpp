// 1752149 吴子睿

#include <iostream>
#include <fstream>

#include "main.h"
#include "user.h"
#include "utility.h"

using namespace std;

extern FileManager gFileManager;


User::User()
{
    u_error_ = U_NO_ERROR;
    dirp_ = "/";
    cur_dir_path_ = "/";
    cur_dir_node_ = gFileManager.root_dir_node_;
    p_dir_node_ = NULL;
    memset(arg_, 0, sizeof(arg_));
}

void User::ChangeDir(string dir) 
{
    if (CheckPath(dir)) {
        gFileManager.Cd();
        if (u_error_ != U_NO_ERROR)
            PrintError();
    }   
}

void User::List() 
{
    gFileManager.Ls();
    if (u_error_ != U_NO_ERROR)
        PrintError();
}

void User::MakeDir(string dir) 
{
    if (CheckPath(dir)) {
        arg_[1] = INode::I_DIR;  // 传递参数
        gFileManager.Fcreat();

        if (u_error_ != U_NO_ERROR)
            PrintError();
    }
}

void User::CreateFile(string file_name) 
{
    if(CheckPath(file_name)) {
        arg_[1] = INode::I_READ | INode::I_WRITE;  // 传递参数
        gFileManager.Fcreat();

        if (u_error_ != U_NO_ERROR)
            PrintError();
    }
}

void User::DeleteFile(string file_name)
{
    if (CheckPath(file_name)) {
        gFileManager.UnLink();

        if (u_error_ != U_NO_ERROR)
            PrintError();
        else
            cout << "Delete file succesfully!" << endl;
    }
}

void User::OpenFile(string file_name, string str_mode) 
{
    int mode;

	if (CheckPath(file_name)) {
        mode = GetFileMode(str_mode);
        if (mode == 0) {
            cout << "The mode is undefined ! \n";
            return;
        }

        arg_[1] = mode;
        gFileManager.Fopen();

        if (u_error_ != U_NO_ERROR)
            PrintError();

        /* 输出成功信息 */
        cout << "Open file successfully, return fd=" << ar0_[EAX] << endl;
	}
}

void User::CloseFile(string str_fd) 
{
    if (CheckMissingParam(str_fd))
        return;
    if (!CheckInt(str_fd))
        return;

    arg_[0] = stoi(str_fd);
    gFileManager.Fclose();

    if (u_error_ != U_NO_ERROR)
        PrintError();
}

void User::Seek(string str_fd, string offset, string whence) 
{
    /* 缺少参数 */
    if (CheckMissingParam(whence))
        return;
    
    if (!CheckInt(str_fd, offset, whence))
        return;

    /* 通过arg_传递参数 */
    arg_[0] = stoi(str_fd);
    arg_[1] = stoi(offset);
    arg_[2] = stoi(whence);

    /* whence参数错误 */
    if (arg_[2] < 0 || arg_[2] > 2) {
        cout << " whence " << arg_[2] << " is undefined!" << endl;
        return;
    }

    gFileManager.Fseek();

    if (u_error_ != U_NO_ERROR)
        PrintError();
    else
        cout << "Seek Successfully!" << endl;
}

void User::WriteFile(string str_fd, string str_write_size, string option, string file_name)
{
    int fd;
    int write_size;
    char* content;

    if (CheckMissingParam(str_write_size))
        return;
    if (!CheckInt(str_fd, str_write_size))
        return;

    fd = stoi(str_fd);
    write_size = stoi(str_write_size);

    if ("-f" == option) {
        fstream file;
        
        file.open(file_name, ios::binary | ios::in);
        if (-1 == write_size) {
            file.seekg(0, file.end);
            write_size = file.tellg();
            file.seekg(0, file.beg);
        }

        content = new char[write_size];
        if (!content)
            exit(-1);
        file.read(content, write_size);
        file.close();
        arg_[1] = (long)content;
    }
    else {
        if (write_size == -1)
            write_size = option.size();
        arg_[1] = (long)option.c_str();
    }
    
    arg_[0] = fd;
    arg_[2] = write_size;
    gFileManager.Fwrite();

    if (u_error_ != U_NO_ERROR)
        PrintError();
    cout << "Write " << ar0_[User::EAX] << "bytes successfully !" << endl;
}

void User::ReadFile(string str_fd, string str_read_size, string option, string file_name)
{
    unsigned int i;
    int fd;
    int read_size;
    char* content;

    if (CheckMissingParam(str_read_size))
        return;
    if (!option.empty()) {
        if (file_name.empty()) {
            cout << "Missing Parameter!" << endl;
            return;
        }
    }
    if (!CheckInt(str_fd, str_read_size))
        return;

    fd = stoi(str_fd);
    read_size = stoi(str_read_size);

    content = new char[read_size];
    memset(content, 0, read_size * sizeof(char));
    if (!content)
        exit(-1);

    arg_[0] = fd;
    arg_[1] = (long)content;
    arg_[2] = read_size;

    gFileManager.Fread();
    if (u_error_ != U_NO_ERROR)
        PrintError();

    cout << "read " << ar0_[User::EAX] << " bytes successfully : \n";

    if ("-f" == option) {
        fstream file;

        file.open(file_name, ios::binary | ios::out);
        file.write(content, ar0_[User::EAX]);
        file.close();
    }
    else {
        /* 输出实际读取到的内容 */
        for (i = 0; i < ar0_[User::EAX]; i++)
            cout << (char)content[i];
        cout << endl;
    }
    
    delete[] content;
}

int User::GetFileMode(string str_mode) 
{
    int mode;

    mode = 0;

    if (str_mode == "-r")
        mode |= File::FREAD;
    else if (str_mode == "-w")
        mode |= File::FWRITE;
    else if (str_mode == "-rw")
        mode |= (File::FREAD | File::FWRITE);
    else
        cout << "Wrong file open mode!" << endl;
    
    return mode;
}

/* 检查路径名正确性，为dirp_赋值 */
bool User::CheckPath(string path) 
{
    string temp_dir;
    unsigned int pos, end_pos;

    /* 如果路径为空 */
    if (path.empty()) {
        cout << "Error: Path is empty!" << endl;
        return false;
    }

    /* 检查路径名长度 */
    for (pos = 0, end_pos = 0; pos < path.length(); pos = end_pos + 1) {
        end_pos = path.find('/', pos);  // 找到本路径名结尾

        /* 如果没找到，则说明到结尾了 */
        if (end_pos == int(path.npos))
            end_pos = path.length();

        if (end_pos - pos > DirectoryEntry::DIR_MAX_SIZE) {
            cout << "The directory name can't be longer than 28 letters!" << endl;
            return false;
        }
    }

    /* 如果路径不包含父目录，直接赋值 */
    if (path.substr(0, 2) != "..") 
        dirp_ = path;
    /* 包含父目录，将其转化为绝对路径形式 */
    else {
        temp_dir = cur_dir_path_;
        pos = 0;

        /* 处理含父目录情况，可以多重父目录，../../ */
        while (temp_dir.length() > 3 && pos < path.length() && path.substr(pos, 2) == "..") {
            temp_dir.pop_back();  // 防止最后有/
            temp_dir.erase(temp_dir.find_last_of('/') + 1);  // 返回父目录
            pos += 2;  // 本次..已处理完
            pos += pos < path.length() && path[pos] == '/';  //若有/，则跳过
        }

        dirp_ = temp_dir + path.substr(pos);
    }

    /* 删除结尾多余的/ */
    if (dirp_.length() > 1 && dirp_.back() == '/') 
        dirp_.pop_back();
    
    return true;
}

/* 打印错误信息 */
void User::PrintError() 
{
    string error_msg;

	switch (u_error_) {
	case User::U_ERR_NO_ENT:
		error_msg = "There is No such file or directory";
		break;
	case User::U_ERR_BAD_F:
        error_msg = " Bad file number ";
		break;
	case User::U_ERR_ACCES:
        error_msg = " Permission denied ";
		break;
	case User::U_ERR_NOT_DIR:
        error_msg = " Not a directory ";
		break;
	case User::U_ERR_NFILE:
        error_msg = " File table overflow ";
		break;
	case User::U_ERR_M_FILE:
        error_msg = " Too many open files ";
		break;
	case User::U_ERR_F_BIG:
        error_msg = " File too large ";
		break;
	case User::U_ERR_NO_SPC:
        error_msg = " No space left on device ";
		break;
	default:
		break;
	}

	cout << error_msg << endl;
}

/* 检查是否缺少参数，使用时只需将最后一个参数传入该函数，
 * 若该参数为空，说明缺少参数，返回true */
bool User::CheckMissingParam(string param)
{
    if (param.empty()) {
        cout << "Missing parameter!" << endl;
        return true;
    }

    return false;
}

/* 检查参数是否都为整数，是的话返回true */
bool User::CheckInt(string param1, string param2, string param3)
{
    if (!Utility::IsInt(param1) || !Utility::IsInt(param2) || !Utility::IsInt(param3)) {
        cout << "Paramter can't be non-integer!" << endl;
        return false;
    }

    return true;
}