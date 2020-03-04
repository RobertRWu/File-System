// 1752149 吴子睿

#include "include/utility.h"
#include "include/file_system.h"
#include "include/user.h"
#include "include/main.h"

extern DeviceManager gDeviceManager;
extern BufferManager gBufferManager;
extern SuperBlock gSuperBlock;
extern FileSystem gFileSystem;
extern INodeTable gINodeTable;
extern User gUser;

FileSystem::FileSystem() 
{
    /* 不存在会重新建磁盘文件，因此要格式化 */
    if (!gDeviceManager.Exist()) 
        FormatDevice();
    else 
        LoadSuperBlock(); 
}

FileSystem::~FileSystem() 
{
    Update();
}

/* 格式化SuperBlock */
void FileSystem::FormatSuperBlock() 
{
    gSuperBlock.s_isize_ = FileSystem::INODE_ZONE_SIZE;
    gSuperBlock.s_fsize_ = FileSystem::DISK_SIZE;
    gSuperBlock.s_nfree_ = 0;
    gSuperBlock.s_free_[0] = -1;
    gSuperBlock.s_ninode_ = 0;
    gSuperBlock.s_flock_ = 0;
    gSuperBlock.s_ilock_ = 0;
    gSuperBlock.s_fmod_ = 0;
    gSuperBlock.s_ronly_ = 0;
    time((time_t*)&gSuperBlock.s_time_);

    //空文件，先写入superblock占据空间
    gDeviceManager.Write(&gSuperBlock, sizeof(SuperBlock), 0, SEEK_SET);
}

/* 格式化DiskINode区 */
void FileSystem::FormatDiskINodes()
{
    int i;

    DiskINode root_d_inode;
    DiskINode temp_d_inode;

    //根目录DiskNode
    root_d_inode.d_mode_ |= INode::I_ALLOC | INode::I_DIR;
    root_d_inode.d_nlink_ = 1;
    gSuperBlock.s_inode_[0] = 0;
    gDeviceManager.Write(&root_d_inode, sizeof(root_d_inode));
    gSuperBlock.s_ninode_++;

    //从第1个DiskINode初始化，第0个固定用于根目录"/"，不可改变
    for (i = 1; i < FileSystem::INode_NUMBERS; i++) {

        /* SuperBlock添加空闲DiskInode */
        if (gSuperBlock.s_ninode_ < SuperBlock::MAX_INODE_NUM)
            gSuperBlock.s_inode_[gSuperBlock.s_ninode_++] = i;

        gDeviceManager.Write(&temp_d_inode, sizeof(temp_d_inode));
    }
}

/* 格式化盘块区 */
void FileSystem::FormatDiskBlocks()
{
    int i;

    //空闲盘块初始化
    char temp_block[BLOCK_SIZE];  // 初始化数据盘块
    char temp_index_block[BLOCK_SIZE];  // 初始化索引盘块

    memset(temp_block, 0, BLOCK_SIZE);
    memset(temp_index_block, 0, BLOCK_SIZE);

    for (i = 0; i < FileSystem::DATA_ZONE_SIZE; i++) {

        /* SuperBlock直接管理的空闲盘块数量超过最大数量 */
        if (gSuperBlock.s_nfree_ >= SuperBlock::MAX_FREE_NUM) {

            /* 写入索引信息(404字节)，free在nfree后边 */
            memcpy(temp_index_block, &gSuperBlock.s_nfree_, sizeof(int) + sizeof(gSuperBlock.s_free_));
            gDeviceManager.Write(&temp_index_block, BLOCK_SIZE);
            gSuperBlock.s_nfree_ = 0;
        }
        else 
            gDeviceManager.Write(temp_block, BLOCK_SIZE);
        
        gSuperBlock.s_free_[gSuperBlock.s_nfree_++] = i + DATA_ZONE_START_SECTOR;
    }
}

/* 格式化整个文件系统 */
void FileSystem::FormatDevice() 
{
    gDeviceManager.Build();
    FormatSuperBlock();
    FormatDiskINodes();
    FormatDiskBlocks();

    time((time_t*)&gSuperBlock.s_time_);

    /* 再次写入superblock */
    gDeviceManager.Write(&gSuperBlock, sizeof(SuperBlock), 0, SEEK_SET);
}

/* 系统初始化时读入SuperBlock */
void FileSystem::LoadSuperBlock() 
{
    gDeviceManager.Read(&gSuperBlock, sizeof(SuperBlock), SUPERBLOCK_START_SECTOR * BLOCK_SIZE);
}

/* 将SuperBlock对象的内存副本更新到存储设备的SuperBlock中去 */
void FileSystem::Update() 
{
    BufferCtrl* p_buffer;
    int i;
    int* p;

    gSuperBlock.s_fmod_ = 0;  // 清空修改标志
    gSuperBlock.s_time_ = (int)time(NULL);
    
    /* 为将要写回到磁盘上去的SuperBlock申请一块缓存，由于缓存块大小为512字节，
     * SuperBlock大小为1024字节，占据2个连续的扇区，所以需要2次写入操作。*/
    for (i = 0; i < 2; i++) {

        /* 第一次p指向SuperBlock的第0字节，第二次p指向第512字节 */
        p = (int*)&gSuperBlock + i * 128;

        /* SuperBlock占据前两个扇区 */
        p_buffer = gBufferManager.GetBlk(FileSystem::SUPERBLOCK_START_SECTOR + i);
        memcpy(p_buffer->b_addr_, p, BLOCK_SIZE);

        /* 将缓冲区中的数据写到磁盘上 */
        gBufferManager.BWrite(p_buffer);
    }

    /* 同步修改过的内存Inode到对应外存Inode */
    gINodeTable.UpdateINodeTable();

    /* 将延迟写的缓存块写到磁盘上 */
    gBufferManager.BFlush();
}

/* 在存储设备上分配空闲磁盘块 */
BufferCtrl* FileSystem::Alloc() 
{
    int block_no;
    int* p;
    BufferCtrl* p_buffer;

    /* 从索引表“栈顶”获取空闲磁盘块编号 */
    block_no = gSuperBlock.s_free_[--gSuperBlock.s_nfree_];

    /* 若获取磁盘块编号为零，则表示已分配尽所有的空闲磁盘块。*/
    if (block_no <= 0) {
        gSuperBlock.s_nfree_ = 0;
        gUser.u_error_ = User::U_ERR_NO_SPC;

        return NULL;
    }

    /*
    * 栈已空，新分配到空闲磁盘块中记录了下一组空闲磁盘块的编号
    * 将下一组空闲磁盘块的编号读入SuperBlock的空闲磁盘块索引表s_free[100]中。
    */
    if (gSuperBlock.s_nfree_ <= 0) {

        /* 读入该空闲磁盘块 */
        p_buffer = gBufferManager.BRead(block_no);

        /* 从该磁盘块的0字节开始记录，共占据4(s_nfree)+400(s_free[100])个字节 */
        p = (int*)p_buffer->b_addr_;

        /* 首先读出空闲盘块数s_nfree */
        gSuperBlock.s_nfree_ = *p++;

        /* 读取缓存中后续位置的数据，写入到SuperBlock空闲盘块索引表s_free[100]中 */
        memcpy(gSuperBlock.s_free_, p, sizeof(gSuperBlock.s_free_));

        /* 缓存使用完毕，释放以便被其它进程使用 */
        gBufferManager.BRelease(p_buffer);
    }

    /* 普通情况下成功分配到一空闲磁盘块 */
    p_buffer = gBufferManager.GetBlk(block_no);  // 为该磁盘块申请缓存
    if (p_buffer) 
        gBufferManager.BClear(p_buffer);  // 清空缓存中的数据

    /* 设置SuperBlock被修改标志 */
    gSuperBlock.s_fmod_ = 1;

    return p_buffer;
}

/* 在存储设备上分配一个空闲外存INode，一般用于创建新的文件。*/
INode* FileSystem::IAlloc() 
{
    BufferCtrl* p_buffer_ctrl;
    INode* p_inode;
    int inode_no;
    int mode;
    int i, j;
    int* p_buffer;

    /* SuperBlock直接管理的空闲Inode索引表已空，必须到磁盘上搜索空闲Inode。*/
    if (gSuperBlock.s_ninode_ <= 0) {
        inode_no = -1;

        /* 依次读入磁盘Inode区中的磁盘块，搜索其中空闲外存Inode，记入空闲Inode索引表 */
        for (i = 0; i < gSuperBlock.s_isize_; i++) {
            p_buffer_ctrl = gBufferManager.BRead(FileSystem::INODE_ZONE_START_SECTOR + i);

            /* 获取缓冲区首址 */
            p_buffer = (int*)p_buffer_ctrl->b_addr_;

            /* 检查该缓冲区中每个外存Inode的i_mode != 0，表示已经被占用 */
            for (j = 0; j < FileSystem::INODE_NUMBER_PER_SECTOR; j++) {
                inode_no++;

                /* d_mode位于DiskINode开头 */
                mode = *(p_buffer + j * sizeof(DiskINode) / sizeof(int));

                /* 该外存Inode已被占用，不能记入空闲Inode索引表 */
                if (mode) 
                    continue;
                
                /* 如果外存inode的i_mode==0，此时并不能确定该inode是空闲的，
                 * 因为有可能是内存inode没有写到磁盘上,所以要继续搜索内存inode中是否有相应的项 */
                if (gINodeTable.IsLoaded(inode_no) == INODE_NOT_LOADED) {

                    /* 该外存Inode没有对应的内存拷贝，将其记入空闲Inode索引表 */
                    gSuperBlock.s_inode_[gSuperBlock.s_ninode_++] = inode_no;

                    /* 如果空闲索引表已经装满，则不继续搜索 */
                    if (gSuperBlock.s_ninode_ >= SuperBlock::MAX_INODE_NUM) 
                        break;  
                }
            }

            /* 至此已读完当前磁盘块，释放相应的缓存 */
            gBufferManager.BRelease(p_buffer_ctrl);

            /* 如果空闲索引表已经装满，则不继续搜索 */
            if (gSuperBlock.s_ninode_ >= SuperBlock::MAX_INODE_NUM) 
                break;
        }

        /* 如果在磁盘上没有搜索到任何可用外存Inode，返回NULL */
        if (gSuperBlock.s_ninode_ <= 0) {
            gUser.u_error_ = User::U_ERR_NO_SPC;
            return NULL;
        }
    }

    inode_no = gSuperBlock.s_inode_[--gSuperBlock.s_ninode_];
    p_inode = gINodeTable.IGet(inode_no);
    if (NULL == p_inode) {
        cout << "No Free INode" << endl;
        return NULL;
    }

    p_inode->Clean();
    gSuperBlock.s_fmod_ = 1;

    return p_inode;
}

/* 释放编号为number的外存INode，一般用于删除文件。*/
void FileSystem::IFree(int number) 
{
    /* 直接管理的空闲外存结点数量已满 */
    if (gSuperBlock.s_ninode_ >= SuperBlock::MAX_INODE_NUM) 
        return;

    gSuperBlock.s_inode_[gSuperBlock.s_ninode_++] = number;
    gSuperBlock.s_fmod_ = 1;
}

/* 释放存储设备上编号为block_no的磁盘块 */
void FileSystem::Free(int block_no) 
{
    BufferCtrl* p_buffer;
    int* p;

    /* 直接管理的空闲磁盘块数量已满 */
    if (gSuperBlock.s_nfree_ >= SuperBlock::MAX_FREE_NUM) {
        /* 为当前正要释放的磁盘块分配缓存 */
        p_buffer = gBufferManager.GetBlk(block_no);

        /* 从该磁盘块的0字节开始记录，共占据4(s_nfree)+400(s_free[100])个字节 */
        p = (int*)p_buffer->b_addr_;

        /* 首先写入空闲盘块数，除了第一组为99块，后续每组都是100块 */
        *p++ = gSuperBlock.s_nfree_;

        /* 将SuperBlock的空闲盘块索引表s_free[100]写入缓存中后续位置 */
        memcpy(p, gSuperBlock.s_free_, sizeof(int) * SuperBlock::MAX_FREE_NUM);
        gSuperBlock.s_nfree_ = 0;  // 下面会++
        gBufferManager.BWrite(p_buffer);
    }

    gSuperBlock.s_free_[gSuperBlock.s_nfree_++] = block_no;
    gSuperBlock.s_fmod_ = 1;
}