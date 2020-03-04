// 1752149 �����

#ifndef Buffer_H
#define Buffer_H

using namespace std;

#define BUFFER_SIZE 512  // ��������С�� ���ֽ�Ϊ��λ

typedef unsigned char Buffer[512];
typedef unsigned char* BufferPointer;

/* ������ƿ飬���ǻ���� */
class BufferCtrl 
{
public:
	/* flags�б�־λ */
	enum BufferFlag {
		B_DEL_WRI = 0x80  // �ӳ�д������Ӧ����Ҫ��������ʱ���ٽ�������д����Ӧ���豸��
	};

public:
	/* ������ƿ��־λ */
	unsigned int b_flags_;

	/* ǰһ�������ָ�� */
	BufferCtrl* b_p_forw_;

	/* ��һ�������ָ�� */
	BufferCtrl* b_p_back_;

	/* �贫�͵��ֽ��� */
	int	b_wcount_;

	/* ָ��û�����ƿ�������Ļ��������׵�ַ */
	BufferPointer b_addr_;

	/* �����߼���� */
	int	b_block_no_;

	/* I/O����ʱ��Ϣ */
	int	b_u_error_;

	/* I/O����ʱ��δ���͵�ʣ���ֽ��� */
	int	b_resid_;

public:
	BufferCtrl();
};


#endif 
