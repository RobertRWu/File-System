// 1752149 吴子睿
#ifndef  INODE_H
#define INODE_H

#include "buffer.h"

class INode {
public:
    /* INodeFlag中标志位 */
    enum INodeFlag {
        I_UPDATE = 0x2,		// 内存INode被修改过，需要更新相应外存INode
        I_ACCESS = 0x4,		// 内存INode被访问过，需要修改最近一次访问时间
    };
    /* 文件空闲 */
    static const unsigned int I_FREE = 0x0;

    /* 文件被使用 */
    static const unsigned int I_ALLOC = 0x8000;

    /* 文件类型掩码 */
    static const unsigned int I_MASK = 0x6000;

    /* 文件类型：目录文件 */
    static const unsigned int I_DIR = 0x4000;

    /* 块设备特殊类型文件，为0表示常规数据文件 */
    static const unsigned int I_BLK = 0x6000;

    /* 文件长度类型：大型或巨型文件 */
    static const unsigned int I_LARGE = 0x1000;

    /* 对文件的读权限 */
    static const unsigned int I_READ = 0x100;

    /* 对文件的写权限 */
    static const unsigned int I_WRITE = 0x80;

    /* 对文件的执行权限 */
    static const unsigned int I_EXEC = 0x40;	

    /* 文件主对文件的读、写、执行权限 */
    static const unsigned int I_RWX_U = (I_READ | I_WRITE | I_EXEC);	

    /* 文件主同组用户对文件的读、写、执行权限 */
    static const unsigned int I_RWX_G = ((I_RWX_U) >> 3);

    /* 其他用户对文件的读、写、执行权限 */
    static const unsigned int I_RWX_O = ((I_RWX_U) >> 6);

    /* 文件逻辑块大小: 512字节 */
    static const int BLOCK_SIZE = 512;

    /* 每个间接索引表(或索引块)包含的物理盘块号 */
    static const int ADDRESS_PER_INDEX_BLOCK = BLOCK_SIZE / sizeof(int);

    /* 小型文件：直接索引表最多可寻址的逻辑块号 */
    static const int SMALL_FILE_BLOCK = 6;

    /* 大型文件：经一次间接索引表最多可寻址的逻辑块号 */
    static const int LARGE_FILE_BLOCK = 128 * 2 + 6;

    /* 巨型文件：经二次间接索引最大可寻址文件逻辑块号 */
    static const int HUGE_FILE_BLOCK = 128 * 128 * 2 + 128 * 2 + 6;

public:
    /* 状态的标志位，定义见enum INodeFlag */
    unsigned int i_flag_;

    /* 文件工作方式信息 */
    unsigned int i_mode_;

    /* 引用计数 */
    int		i_count_;

    /* 文件联结计数，即该文件在目录树中不同路径名的数量 */
    int		i_nlink_;
    
    /* 外存INode区中的编号 */
    int		i_number_;

    /* 文件所有者的用户标识数 */
    short	i_uid_;

    /* 文件所有者的组标识数 */
    short	i_gid_;

    /* 文件大小，字节为单位 */
    int		i_size_;

    /* 用于文件逻辑块号和物理块号转换的基本索引表 */
    int		i_addr_[10];

public:
    INode();

    /* 根据Inode对象中的物理磁盘块索引表，读取相应的文件数据 */
    void ReadI();

    /* 根据Inode对象中的物理磁盘块索引表，将数据写入文件 */
    void WriteI();

    /* 将文件的逻辑块号转换成对应的物理盘块号 */
    int BMap(int logic_block_num);

    /* 更新外存Inode的最后的访问时间、修改时间 */
    void IUpdate(int time);

    /* 释放INode对应文件在磁盘上的所有内容 */
    void ITrunc();

    /* 清空Inode对象中的数据 */
    void Clean();

    /* 将包含外存Inode字符块中信息拷贝到内存Inode中 */
    void ICopy(BufferCtrl* p_buffer, int inode_num);
};

/* 外存索引节点 */
class DiskINode {
public:
    /* 状态的标志位，定义见INode一堆静态常量 */
    unsigned int d_mode_;

    /* 该文件在目录树中不同路径名的数量 */
    int	d_nlink_;

    /* 文件所有者的用户标识数 */
    short d_uid_;	
    
    /* 文件所有者的组标识数 */
    short d_gid_;
    
    /* 文件大小，字节为单位 */
    int	d_size_;
    
    /* 文件逻辑块号和物理块号转换的基本索引表 */
    int	d_addr_[10];
    
    /* 最后访问时间，不能删，得凑够64kb */
    int	d_atime_;
    
    /* 最后修改时间 */
    int	d_mtime_;

public:
    DiskINode();
};

#endif