// 1752149 吴子睿

#include "include/utility.h"
#include "include/file.h"
#include "include/user.h"
#include "include/main.h"

extern User gUser;

File::File() 
{
	flag_ = 0;
	count_ = 0;
	inode_ = NULL;
	offset_ = 0;
}

OpenFiles::OpenFiles() 
{
	memset(process_open_file_table, NULL, sizeof(process_open_file_table));
}


/* 进程请求打开文件时，在打开文件描述符表中分配一个空闲表项 */
int OpenFiles::AllocFreeSlot() 
{
	int i;

	for (i = 0; i < OpenFiles::MAX_FILES; i++) {

		/* 进程打开文件描述符表中找到空闲项，则返回 */
		if (process_open_file_table[i] == NULL) {
			gUser.ar0_[User::EAX] = i;
			return i;
		}
	}

	/* Open1，需要一个标志。当打开文件结构创建失败时，可以回收系统资源 */
	gUser.ar0_[User::EAX] = NO_OPEN_FILE_SLOT;   
	gUser.u_error_ = User::U_ERR_M_FILE;

	return -1;
}

/* 根据用户系统调用提供的文件描述符参数fd，找到对应的打开文件控制块File结构 */
File* OpenFiles::GetF(int fd) 
{
	File* p_file;

	if (fd < 0 || fd >= OpenFiles::MAX_FILES) {
		gUser.u_error_ = User::U_ERR_BAD_F;
		return NULL;
	}

	p_file = process_open_file_table[fd];
	if (p_file == NULL) 
		gUser.u_error_ = User::U_ERR_BAD_F;
	
	return p_file;
}

/* 为已分配到的空闲描述符fd和已分配的打开文件表中空闲File对象建立勾连关系 */
void OpenFiles::SetF(int fd, File* p_file) {
	if (fd < 0 || fd >= OpenFiles::MAX_FILES) 
		return;
	
	process_open_file_table[fd] = p_file;
}