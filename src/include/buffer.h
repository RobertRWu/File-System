// 1752149 吴子睿

#ifndef Buffer_H
#define Buffer_H

using namespace std;

#define BUFFER_SIZE 512  // 缓冲区大小。 以字节为单位

typedef unsigned char Buffer[512];
typedef unsigned char* BufferPointer;

/* 缓存控制块，不是缓存块 */
class BufferCtrl 
{
public:
	/* flags中标志位 */
	enum BufferFlag {
		B_DEL_WRI = 0x80  // 延迟写，在相应缓存要移做他用时，再将其内容写到相应块设备上
	};

public:
	/* 缓存控制块标志位 */
	unsigned int b_flags_;

	/* 前一个缓存块指针 */
	BufferCtrl* b_p_forw_;

	/* 后一个缓存块指针 */
	BufferCtrl* b_p_back_;

	/* 需传送的字节数 */
	int	b_wcount_;

	/* 指向该缓存控制块所管理的缓冲区的首地址 */
	BufferPointer b_addr_;

	/* 磁盘逻辑块号 */
	int	b_block_no_;

	/* I/O出错时信息 */
	int	b_u_error_;

	/* I/O出错时尚未传送的剩余字节数 */
	int	b_resid_;

public:
	BufferCtrl();
};


#endif 
