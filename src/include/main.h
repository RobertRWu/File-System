// 1752149 吴子睿
#ifndef MAIN_H
#define MAIN_H

#include <time.h>

#include "user.h"

/* 打开文件描述符表无多余空间 */
#define NO_OPEN_FILE_SLOT -1

/* 外存DiskINode为装入内存中，InodeTable没有对应的Inode */
#define INODE_NOT_LOADED -1

/* 最大参数数量 */
#define MAX_ARG_NUM  5

#endif