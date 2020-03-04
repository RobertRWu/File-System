// 1752149 Îâ×Óî£

#include "include/buffer_manager.h"
#include "include/buffer.h"

BufferCtrl::BufferCtrl()
{
    b_flags_ = 0;
    b_p_forw_ = NULL;
    b_p_back_ = NULL;
    b_wcount_ = 0;
    b_addr_ = NULL;
    b_block_no_ = -1;
    b_u_error_ = -1;
    b_resid_ = 0;
}