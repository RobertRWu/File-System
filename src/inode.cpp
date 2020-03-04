// 1752149 吴子睿
#include "include/utility.h"
#include "include/inode.h"
#include "include/file_system.h"
#include "include/user.h"
#include "include/main.h"

extern BufferManager gBufferManager;
extern FileSystem gFileSystem;
extern User gUser;

INode::INode() 
{
	i_flag_ = 0;
	i_mode_ = I_FREE;
	i_count_ = 0;
	i_nlink_ = 0;
	i_number_ = -1;
	i_uid_ = -1;
	i_gid_ = -1;
	i_size_ = 0;
	memset(i_addr_, 0, sizeof(i_addr_));
}


/* 根据Inode对象中的物理磁盘块索引表，读取相应的文件数据 */
void INode::ReadI() 
{
	int logic_blk_num;  // 文件逻辑块号
	int blk_num;  // logic_blk_num对应的物理盘块号
	int offset;
	int remain;
	int temp_size;
	unsigned char* start;
	BufferCtrl* p_buffer;

	/* 需要读字节数为零，则返回 */
	if (0 == gUser.io_param_.count_)
		return;
	
	i_flag_ |= INode::I_ACCESS;

	/* 一次一个数据块地读入所需全部数据，直至遇到文件尾 */
	while (User::U_NO_ERROR == gUser.u_error_ && gUser.io_param_.count_) {
		logic_blk_num = gUser.io_param_.offset_ / INode::BLOCK_SIZE;
		offset = gUser.io_param_.offset_ % INode::BLOCK_SIZE;

		/* 传送到用户区的字节数量，取读请求的剩余字节数与当前字符块内有效字节数较小值 */
		temp_size = Utility::Min(INode::BLOCK_SIZE - offset, gUser.io_param_.count_);
		remain = i_size_ - gUser.io_param_.offset_;

		/* 如果已读到超过文件结尾，则返回 */
		if (remain <= 0) 
			return;
		
		/* 传送的字节数量还取决于剩余文件的长度 */
		temp_size = Utility::Min(temp_size, remain);

		/* 将逻辑块号转换成物理盘块号 */
		if ((blk_num = BMap(logic_blk_num)) == 0) 
			return;
		
		p_buffer = gBufferManager.BRead(blk_num);

		/* 缓存中数据起始读位置 */
		start = p_buffer->b_addr_ + offset;
		memcpy(gUser.io_param_.base_, start, temp_size);

		/* 用传送字节数temp_size更新读写位置 */
		gUser.io_param_.base_ += temp_size;
		gUser.io_param_.offset_ += temp_size;
		gUser.io_param_.count_ -= temp_size;

		gBufferManager.BRelease(p_buffer);
	}
}

/* 根据Inode对象中的物理磁盘块索引表，将数据写入文件 */
void INode::WriteI() 
{
	int logic_blk_num;
	int blk_num;
	int offset;
	int temp_size;
	unsigned char* start;
	BufferCtrl* p_buffer;

	i_flag_ |= (INode::I_ACCESS | INode::I_UPDATE);

	/* 需要写字节数为零，则返回 */
	if (0 == gUser.io_param_.count_) 
		return;
	
	while (User::U_NO_ERROR == gUser.u_error_ && gUser.io_param_.count_) {
		logic_blk_num = gUser.io_param_.offset_ / INode::BLOCK_SIZE;
		offset = gUser.io_param_.offset_ % INode::BLOCK_SIZE;
		temp_size = Utility::Min(INode::BLOCK_SIZE - offset, gUser.io_param_.count_);
		if ((blk_num = BMap(logic_blk_num)) == 0) 
			return;
		
		/* 如果写入数据正好满一个字符块，则为其分配缓存 */
		if (INode::BLOCK_SIZE == temp_size) 	
			p_buffer = gBufferManager.GetBlk(blk_num);

		/* 写入数据不满一个字符块，先读后写（读出该字符块以保护不需要重写的数据） */
		else 		
			p_buffer = gBufferManager.BRead(blk_num);

		/* 缓存中数据的起始写位置 写操作: 从用户目标区拷贝数据到缓冲区 */
		start = p_buffer->b_addr_ + offset;
		memcpy(start, gUser.io_param_.base_, temp_size);

		/* 用传送字节数temp_size更新读写位置 */
		gUser.io_param_.base_ += temp_size;
		gUser.io_param_.offset_ += temp_size;
		gUser.io_param_.count_ -= temp_size;

		/* 写过程中出错 */
		if (gUser.u_error_ != User::U_NO_ERROR)
			gBufferManager.BRelease(p_buffer);
		
		/* 将缓存标记为延迟写，不急于进行I/O操作将字符块输出到磁盘上 */
		gBufferManager.BDWrite(p_buffer);

		/* 普通文件长度增加 */
		if (i_size_ < gUser.io_param_.offset_)
			i_size_ = gUser.io_param_.offset_;

		i_flag_ |= INode::I_UPDATE;
	}
}

/* 将包含外存Inode字符块中信息拷贝到内存Inode中 */
void INode::ICopy(BufferCtrl* p_buffer, int inode_num)
{
	DiskINode& d_inode = *(DiskINode*)(p_buffer->b_addr_ + (inode_num % FileSystem::INODE_NUMBER_PER_SECTOR) * sizeof(DiskINode));
	i_mode_ = d_inode.d_mode_;
	i_nlink_ = d_inode.d_nlink_;
	i_uid_ = d_inode.d_uid_;
	i_gid_ = d_inode.d_gid_;
	i_size_ = d_inode.d_size_;
	memcpy(i_addr_, d_inode.d_addr_, sizeof(i_addr_));
}

/* 将文件的逻辑块号转换成对应的物理盘块号 */
int INode::BMap(int logic_blk_num) 
{
	/*
	* Unix V6++的文件索引结构：(小型、大型和巨型文件)
	* (1) i_addr[0] - i_addr[5]为直接索引表，文件长度范围是0 - 6个盘块；
	*
	* (2) i_addr[6] - i_addr[7]存放一次间接索引表所在磁盘块号，每磁盘块
	* 上存放128个文件数据盘块号，此类文件长度范围是7 - (128 * 2 + 6)个盘块；
	*
	* (3) i_addr[8] - i_addr[9]存放二次间接索引表所在磁盘块号，每个二次间接
	* 索引表记录128个一次间接索引表所在磁盘块号，此类文件长度范围是
	* (128 * 2 + 6 ) < size <= (128 * 128 * 2 + 128 * 2 + 6)
	*/
	BufferCtrl* p_first_buffer;
	BufferCtrl* p_second_buffer;
	int phy_block_no;
	int	index;
	int* table;  // 用于访问索引盘块中一次间接、两次间接索引表

	/* 逻辑块号超出最大限制，报错 */
	if (logic_blk_num >= INode::HUGE_FILE_BLOCK) {
		gUser.u_error_ = User::U_ERR_F_BIG;
		return 0;
	}

	/* 如果小于6，从基本索引表i_addr[0-5]中获得物理盘块号即可 */
	if (logic_blk_num < 6) {
		phy_block_no = i_addr_[logic_blk_num];

		/* 如果该逻辑块号还没有相应的物理盘块号与之对应，则分配一个物理块。
		 * 这通常发生在对文件的写入，当写入位置超出文件大小，即对当前
		 * 文件进行扩充写入，就需要分配额外的磁盘块，并为之建立逻辑块号
		 * 与物理盘块号之间的映射。*/
		if (phy_block_no == 0 && (p_first_buffer = gFileSystem.Alloc()) != NULL) {
			phy_block_no = p_first_buffer->b_block_no_;

			/* 因为后面很可能马上还要用到此处新分配的数据块，所以不急于立刻输出到
			 * 磁盘上；而是将缓存标记为延迟写方式，这样可以减少系统的I/O操作。*/
			gBufferManager.BDWrite(p_first_buffer);
			i_addr_[logic_blk_num] = phy_block_no;
			i_flag_ |= INode::I_UPDATE;
		}

		return phy_block_no;
	}

	/* logic_blk_num >= 6 */
	if (logic_blk_num < INode::LARGE_FILE_BLOCK) 
		index = (logic_blk_num - INode::SMALL_FILE_BLOCK) / INode::ADDRESS_PER_INDEX_BLOCK + 6;
	
	/* 长度介于263 - (128 * 128 * 2 + 128 * 2 + 6)个盘块之间 */
	else 
		index = (logic_blk_num - INode::LARGE_FILE_BLOCK) / (INode::ADDRESS_PER_INDEX_BLOCK * INode::ADDRESS_PER_INDEX_BLOCK) + 8;
	

	phy_block_no = i_addr_[index];

	/* 若该项为零，则表示不存在相应的间接索引表块 */
	if (0 == phy_block_no) {
		i_flag_ |= INode::I_UPDATE;

		/* 分配一空闲盘块存放间接索引表 */
		if ((p_first_buffer = gFileSystem.Alloc()) == 0)
			return 0;

		i_addr_[index] = p_first_buffer->b_block_no_;
	}
	/* 存在相应的简介索引表块，读出存储间接索引表的字符块 */
	else 
		p_first_buffer = gBufferManager.BRead(phy_block_no);
	
	table = (int*)p_first_buffer->b_addr_;  // 获取缓冲区首址

	/* 对于巨型文件的情况，pFirstBuf中是二次间接索引表，
	 * 还需根据逻辑块号，经由二次间接索引表找到一次间接索引表 */
	if (index >= 8) {
		index = ((logic_blk_num - INode::LARGE_FILE_BLOCK) / INode::ADDRESS_PER_INDEX_BLOCK) % INode::ADDRESS_PER_INDEX_BLOCK;
		phy_block_no = table[index];

		/* table指向缓存中的二次间接索引表。该项为零，不存在二次间接索引表 */
		if (0 == phy_block_no) {

			/* 分配一次间接索引表磁盘块失败，释放缓存中的一次间接索引表，然后返回 */
			if ((p_second_buffer = gFileSystem.Alloc()) == NULL) {
				gBufferManager.BRelease(p_first_buffer);
				return 0;
			}
			/* 将新分配的二次间接索引表磁盘块号，记入一次间接索引表相应项 */
			table[index] = p_second_buffer->b_block_no_;
			gBufferManager.BDWrite(p_first_buffer);
		}
		/* 若存在 */
		else {
			/* 释放一次间接索引表占用的缓存，并读入二次间接索引表 */
			gBufferManager.BRelease(p_first_buffer);
			p_second_buffer = gBufferManager.BRead(phy_block_no);
		}

		p_first_buffer = p_second_buffer;
		table = (int*)p_second_buffer->b_addr_;
	}

	/* 计算逻辑块号lbn最终位于一次间接索引表中的表项序号index */
	if (logic_blk_num < INode::LARGE_FILE_BLOCK) 
		index = (logic_blk_num - INode::SMALL_FILE_BLOCK) % INode::ADDRESS_PER_INDEX_BLOCK;
	else 
		index = (logic_blk_num - INode::LARGE_FILE_BLOCK) % INode::ADDRESS_PER_INDEX_BLOCK;
	
	/* 逻辑块号还没有相应的物理盘块号与之对应，分配一个物理块成功 */
	if ((phy_block_no = table[index]) == 0 && (p_second_buffer = gFileSystem.Alloc()) != NULL) {
		phy_block_no = p_second_buffer->b_block_no_;
		table[index] = phy_block_no;

		/* 将数据盘块、更改后的一次间接索引表用延迟写方式输出到磁盘 */
		gBufferManager.BDWrite(p_second_buffer);
		gBufferManager.BDWrite(p_first_buffer);
	}
	else 
		gBufferManager.BRelease(p_first_buffer);

	return phy_block_no;
}

/* 清空Inode对象中的数据 */
void INode::Clean() {
	/*
	* Inode::Clean()特定用于IAlloc()中清空新分配DiskInode的原有数据，
	* 即旧文件信息。Clean()函数中不应当清除i_dev, i_number, i_flag, i_count,
	* 这是属于内存Inode而非DiskInode包含的旧文件信息，而Inode类构造函数需要
	* 将其初始化为无效值。
	*/

	// i_flag = 0;
	i_mode_ = 0;
	//i_count = 0;
	i_nlink_ = 0;
	//i_dev = -1;
	//i_number = -1;
	i_uid_ = -1;
	i_gid_ = -1;
	i_size_ = 0;
	memset(i_addr_, 0, sizeof(i_addr_));
}

/* 更新外存Inode的最后的访问时间、修改时间 */
void INode::IUpdate(int time) 
{
	BufferCtrl* p_buffer;
	DiskINode d_inode;
	DiskINode* p_node;
	unsigned char* p;

	/* 当IUPD和IACC标志之一被设置，才需要更新相应DiskInode */
	if (i_flag_ & (INode::I_UPDATE | INode::I_ACCESS)) {

		/* 在缓存池中找到包含本i节点（this->i_number）的缓存块 */
		p_buffer = gBufferManager.BRead(FileSystem::INODE_ZONE_START_SECTOR + i_number_ / FileSystem::INODE_NUMBER_PER_SECTOR);
		
		/* 将内存Inode副本中的信息复制到dInode中，然后将dInode覆盖缓存中旧的外存Inode */
		d_inode.d_mode_ = i_mode_;
		d_inode.d_nlink_ = i_nlink_;
		d_inode.d_uid_ = i_uid_;
		d_inode.d_gid_ = i_gid_;
		d_inode.d_size_ = i_size_;
		memcpy(d_inode.d_addr_, i_addr_, sizeof(d_inode.d_addr_));

		if (i_flag_ & INode::I_ACCESS) {
			d_inode.d_atime_ = time;
		}
		if (i_flag_ & INode::I_UPDATE) {
			d_inode.d_mtime_ = time;
		}

		/* 将p指向缓存区中旧外存Inode的偏移位置 */
		p = p_buffer->b_addr_ + (i_number_ % FileSystem::INODE_NUMBER_PER_SECTOR) * sizeof(DiskINode);
		p_node = &d_inode;

		/* 用d_inode中的新数据覆盖缓存中的旧外存Inode */
		memcpy(p, p_node, sizeof(DiskINode));

		/* 将缓存写回至磁盘，达到更新旧外存Inode的目的 */
		gBufferManager.BWrite(p_buffer);
	}
}

/* 释放INode对应文件在磁盘上的所有内容 */
void INode::ITrunc() 
{
	int i, j, k;
	int* p_first;
	int* p_second;
	BufferCtrl* p_first_buffer;
	BufferCtrl* p_second_buffer;

	/* 从i_addr[9]到i_addr[0] */
	for (i = 9; i >= 0; --i) {

		/* 如果i_addr[]中第i项存在索引 */
		if (i_addr_[i]) {

			/* 如果是i_addr[]中的一次间接、两次间接索引项 */
			if (i >= 6) {

				/* 将间接索引表读入缓存 */
				p_first_buffer = gBufferManager.BRead(i_addr_[i]);

				/* 获取缓冲区首址 */
				p_first = (int*)p_first_buffer->b_addr_;

				/* 每张间接索引表记录 512/sizeof(int) = 128个磁盘块号，遍历这全部128个磁盘块 */
				for (j = BLOCK_SIZE / sizeof(int) - 1; j >= 0; --j) {

					/* 如果该项存在索引 */
					if (p_first[j]) {

						/* 如果是两次间接索引表，i_addr[8]或i_addr[9]项，
						 * 那么该字符块记录的是128个一次间接索引表存放的磁盘块号 */
						if (i >= 8) {
							p_second_buffer = gBufferManager.BRead(p_first[j]);
							p_second = (int*)p_second_buffer->b_addr_;

							for (k = BLOCK_SIZE / sizeof(int) - 1; k >= 0; --k) {
								/* 释放指定的磁盘块 */
								if (p_second[k]) 
									gFileSystem.Free(p_second[k]);
							}

							/* 缓存使用完毕，释放以便被其它进程使用 */
							gBufferManager.BRelease(p_second_buffer);
						}
						gFileSystem.Free(p_first[j]);
					}
				}
				gBufferManager.BRelease(p_first_buffer);
			}
			/* 释放索引表本身占用的磁盘块 */
			gFileSystem.Free(i_addr_[i]);

			/* 0表示该项不包含索引 */
			i_addr_[i] = 0;
		}
	}

	/* 盘块释放完毕，文件大小清零 */
	i_size_ = 0;

	/* 增设IUPD标志位，表示此内存Inode需要同步到相应外存Inode */
	i_flag_ |= INode::I_UPDATE;

	/* 清大文件标志 和原来的RWXRWXRWX比特*/
	i_mode_ &= ~(INode::I_LARGE);
	i_nlink_ = 1;
}

DiskINode::DiskINode() 
{
	d_mode_ = INode::I_FREE;
	d_nlink_ = 0;
	d_uid_ = -1;
	d_gid_ = -1;
	d_size_ = 0;
	d_atime_ = 0;
	d_mtime_ = 0;
	memset(d_addr_, 0, sizeof(d_addr_));
}
