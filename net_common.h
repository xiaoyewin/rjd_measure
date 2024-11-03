#ifndef NET_COMMON_H
#define NET_COMMON_H

#include "stdint.h"
//这些都是前四个字节

#define NET_INSTR_NO  0x0
#define NET_INSTR_DOWNLOAD_MANUAL  0x1
#define NET_INSTR_DOWNLOAD_LGTSY_HF_APP  0x2

#define NET_INSTR_LOGIN_INFO  0x3  //发送登录信息

#define NET_INSTR_VERSION_INFO  0x4  //发送版本验证信息

#define NET_INSTR_START  0x5  //发送开始的指令
#define NET_INSTR_STOP   0x6  //发送停止的指令

#define NET_INSTR_TIME    0x7  //发送时间和当前的步数



#define NET_INSTR_DOWNLOAD_SPECIFIC_APP 0xF   //下载指定的文件

 //下载特定的文件需要输入文件名。
 #define NET_INSTR_NEED_DOWNLOAD_FILENAME 0x10

//指令基本从客户端向服务器发送的
 #define NET_INSTR_FILEINFO 0x20    //包括实际的文件名，文件大小。

 #define NET_INSTR_DOWNLOADING 0x30    //表示正在下载，知道下载完成。

#define NET_INSTR_CHECKSUM 0x80
//下载完成之后，需要产生一个累加和，用于校验，一般为4个字节


#define NET_INSTR_ACK_SUCCESS_SERVER 0x1000  //用服务器发送的应答
#define NET_INSTR_ACK_SUCCESS_CLIENT 0x1001  //用与客户端发送的应答


#define NET_RECV_INSTR 0x2000  //用于接收指令


#define NET_SEND_DATA_INFO 0x4000  //用于发送数据
#define NET_SEND_DATA 0x8000  //用于发送数据

#define NET_SEND_GRADIENT 0x100  //用于发送数据

#define NET_MINI_SIZE 12  //最少发送的数据大小

typedef struct _NET_Info
{
    uint16_t instr;  //指令
    uint16_t return_instr;  //需要返回的指令
    uint16_t len;   //数据的长度
    uint16_t crc;
    uint8_t* data;
}T_NET_Info,*PT_NET_Info;




//下面是登录信息的结构体

typedef struct _Login_info
{
    int machine_id;  //机器的ID 信息
    int play_name[4];                 //回放人姓名 最多16个字节
    int play_date;                //回放时间    年月日 四个字节
    int play_time;


}T_Login_Info,*PT_Login_Info;








#endif // NET_COMMON_H
