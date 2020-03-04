// 1752149 �����

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


/* ����������ļ�ʱ���ڴ��ļ����������з���һ�����б��� */
int OpenFiles::AllocFreeSlot() 
{
	int i;

	for (i = 0; i < OpenFiles::MAX_FILES; i++) {

		/* ���̴��ļ������������ҵ�������򷵻� */
		if (process_open_file_table[i] == NULL) {
			gUser.ar0_[User::EAX] = i;
			return i;
		}
	}

	/* Open1����Ҫһ����־�������ļ��ṹ����ʧ��ʱ�����Ի���ϵͳ��Դ */
	gUser.ar0_[User::EAX] = NO_OPEN_FILE_SLOT;   
	gUser.u_error_ = User::U_ERR_M_FILE;

	return -1;
}

/* �����û�ϵͳ�����ṩ���ļ�����������fd���ҵ���Ӧ�Ĵ��ļ����ƿ�File�ṹ */
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

/* Ϊ�ѷ��䵽�Ŀ���������fd���ѷ���Ĵ��ļ����п���File������������ϵ */
void OpenFiles::SetF(int fd, File* p_file) {
	if (fd < 0 || fd >= OpenFiles::MAX_FILES) 
		return;
	
	process_open_file_table[fd] = p_file;
}