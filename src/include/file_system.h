// 1752149 吴子睿

#ifndef  FILE_SYSTEM_H
#define FILE_SYSTEM_H

#include "inode.h"
#include "device_manager.h"
#include "buffer_manager.h"

/*
 * 文件系统存储资源管理块(Super Block)的定义。
 */
class SuperBlock 
{
public:
    const static int MAX_FREE_NUM = 100;
    const static int MAX_INODE_NUM = 100;

public:
    /* 外存INode区占用的盘块数 */
    int	s_isize_;

    /* 盘块总数 */
    int	s_fsize_;

    /* 直接管理的空闲盘块数量 */
    int	s_nfree_;

    /* 直接管理的空闲盘块索引表 */
    int	s_free_[MAX_FREE_NUM];

    /* 直接管理的空闲外存INode数量 */
    int	s_ninode_;

    /* 直接管理的空闲外存INode索引表 */
    int	s_inode_[MAX_INODE_NUM];

    /* 封锁空闲盘块索引表标志，不能删，不然不够1024 */
    int	s_flock_;

    /* 封锁空闲INode表标志 */
    int	s_ilock_;

    /* 内存中super block副本被修改标志，意味着需要更新外存对应的Super Block */
    int	s_fmod_;

    /* 本文件系统只能读出 */
    int	s_ronly_;

    /* 最近一次更新时间 */
    int	s_time_;

    /* 填充使SuperBlock块大小等于1024字节，占据2个扇区 */
    int	padding_[47];
};



class FileSystem 
{
public:
    // Block块大小
    static const int BLOCK_SIZE = 512;

    // 磁盘所有扇区数量
    static const int DISK_SIZE = 16384;

    // 定义SuperBlock位于磁盘上的扇区号，占据两个扇区
    static const int SUPERBLOCK_START_SECTOR = 0;

    // 外存INode区位于磁盘上的起始扇区号
    static const int INODE_ZONE_START_SECTOR = 2;

    // 磁盘上外存INode区占据的扇区数
    static const int INODE_ZONE_SIZE = 1022;

    // 外存INode对象长度为64字节，每个磁盘块可以存放512/64 = 8个外存INode
    static const int INODE_NUMBER_PER_SECTOR = BLOCK_SIZE / sizeof(DiskINode);

    // 文件系统根目录外存INode编号
    static const int ROOT_INODE_NO = 0;

    // 外存INode的总个数
    static const int INode_NUMBERS = INODE_ZONE_SIZE * INODE_NUMBER_PER_SECTOR;

    // 数据区的起始扇区号
    static const int DATA_ZONE_START_SECTOR = INODE_ZONE_START_SECTOR + INODE_ZONE_SIZE;

    // 数据区的最后扇区号
    static const int DATA_ZONE_END_SECTOR = DISK_SIZE - 1;

    // 数据区占据的扇区数量
    static const int DATA_ZONE_SIZE = DISK_SIZE - DATA_ZONE_START_SECTOR;

public:
    FileSystem();
    ~FileSystem();

    /* 格式化SuperBlock */
    void FormatSuperBlock();

    /* 格式化DiskINode区 */
    void FormatDiskINodes();

    /* 格式化盘块区 */
    void FormatDiskBlocks();

    /* 格式化整个文件系统 */
    void FormatDevice();

    /* 系统初始化时读入SuperBlock */
    void LoadSuperBlock();

    /* 将SuperBlock对象的内存副本更新到存储设备的SuperBlock中去 */
    void Update();

    /* 在存储设备dev上分配一个空闲外存INode，一般用于创建新的文件。*/
    INode* IAlloc();

    /* 释放编号为number的外存INode，一般用于删除文件。*/
    void IFree(int number);

    /* 在存储设备上分配空闲磁盘块 */
    BufferCtrl* Alloc();

    /* 释放存储设备dev上编号为block_no的磁盘块 */
    void Free(int block_no);

};

#endif