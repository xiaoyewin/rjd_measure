#include "fifo_buf.h"
#include "stdlib.h"
#include "string.h"


Fifo_Buf::Fifo_Buf(int size)
{
   if(size<=0){
       return ;
   }
   if(size>1000){
       size=1000;
   }

   fifo.max_size=size;

   fifo.data=(uint8_t*)malloc(fifo.max_size);
   if(fifo.data==NULL){ //直接退出程序
       exit(-1);
   }
   fifo.read_index=0;
   fifo.write_index=0;
   fifo.occupy=0;
   memset(fifo.data,0,fifo.max_size);

}

Fifo_Buf::~Fifo_Buf(){
    if(fifo.data!=NULL){
       free(fifo.data);
    }
}

int Fifo_Buf::fifo_buf_write(uint8_t *buf,int size)
{
    //如果太大就写不进了
    if(size>(fifo.max_size-fifo.occupy)){
        return -1;
    }
    if(size<0){
        return -2;
    }

    if(size==0){
        size=fifo.occupy;
    }



    int i=0;
    while(i<size){
            if(fifo.occupy==fifo.max_size){
                  break;
            }
            fifo.data[fifo.write_index++]=buf[i++];
            if(fifo.write_index>=fifo.max_size){
                    fifo.write_index=0;
            }
            fifo.occupy++;
    }

    return size;
}

//如果为size=0 表示读全部
int Fifo_Buf::fifo_buf_read(uint8_t *buf,int size)
{
    if(size<0){
        return -1;
    }
    int read_size;
    if(size==0){
        read_size=fifo.occupy;
    }
    else{
        read_size=size;
    }

    if(fifo.occupy<size){
        read_size=fifo.occupy;
    }
    uint32_t i=0;

    while(i<read_size){
        buf[i]=fifo.data[fifo.read_index++];

        if(fifo.read_index>=fifo.max_size){
                fifo.read_index=0;
        }
        fifo.occupy--;
        i++;
    }
    return read_size;
}


