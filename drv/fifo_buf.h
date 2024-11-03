#ifndef FIFO_BUF_H
#define FIFO_BUF_H
#include "stdint.h"


typedef struct _fifo_buf{
      uint8_t * data;
      int write_index;
      int read_index;
      int  occupy;  //表示已经占用多少个
      int  max_size;

} T_Fifo_buf;

class Fifo_Buf
{
public:
    Fifo_Buf(int size);
    ~Fifo_Buf();
    int fifo_buf_write(uint8_t *buf,int size);
    int fifo_buf_read(uint8_t *buf,int size);
public:
    T_Fifo_buf fifo;
};

#endif // FIFO_BUF_H
