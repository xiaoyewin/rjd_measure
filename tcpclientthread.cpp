#include "tcpclientthread.h"

#include<QDataStream>
#include <QMessageBox>
#include <QTimer>

T_Login_Info gt_login_info;


TCPClientThread::TCPClientThread(int version)
{
    qDebug("main%d",QThread::currentThreadId());

    qDebug("TCP thread start!");

    //开辟一个大的内存池，用来接收数据,最大为1M  这个1M 的就是专门下载文件和其他的用途
    net_info_send.data=(uint8_t*)malloc(1024*1024*1);
    net_info_recv.data=(uint8_t*)malloc(1024*1024*1);
    is_connected=false;


    tcpClient = new QTcpSocket(this);  //这个是客户端

    version_no=version;

    //tcpClient->connectToHost("139.196.90.47",3000,QTcpSocket::ReadWrite);

     set_ip_port();
     connect(tcpClient,SIGNAL(connected()),this,SLOT(connect_slot()));
     connect(tcpClient,SIGNAL(disconnected()),this,SLOT(disconnect_slot()));



     timer=new QTimer(this);
     QObject::connect(timer,SIGNAL(timeout()),this,SLOT(timerout_slot()));

     timer->start(3000); //5S一次  //可以作为调试 3秒发一次即可






}

//进行网络的重连
void TCPClientThread::reconnect_net()
{
    set_ip_port();
}

//应该每隔一定时间发送一个更新信息才对
int TCPClientThread::sen_instr_data(T_NET_Info *info)
{

    //在需要设置缓存。
    uint8_t buf[100];
    //qDebug("info->instr=%d",info->instr);
    buf[0]=( uint8_t)((info->instr)&0xff);
    buf[1]=( uint8_t)((info->instr>>8)&0xff);

    buf[2]=( uint8_t)((info->return_instr)&0xff);
    buf[3]=( uint8_t)((info->return_instr>>8)&0xff);

    buf[4]=( uint8_t)((info->len)&0xff);
    buf[5]=( uint8_t)((info->len>>8)&0xff);


    //代表数据的长度
    info->crc=0;

    for(int i=0;i<6;i++){
        info->crc+=buf[i];
    }

    if(info->len>0)
    {
        for(int i=0;i<info->len;i++)
        {
           buf[i+8]=info->data[i];
           info->crc+=info->data[i];
        }
    }
    //后面还有个纠错码;
    buf[6]=(uint8_t)((info->crc)&0xff);
    buf[7]=(uint8_t)((info->crc>>8)&0xff);

    send_data_to_server((void*)(buf),info->len+8);

}


void TCPClientThread::timerout_slot()
{
    //5秒1下

    //发送信息指令,表示已经连接上了。
    //先发送当前的版本号。

    if(is_connected){
        qDebug("send!");
        //4个字节
//       net_info_send.len=8;
//       net_info_send.instr=NET_INSTR_TIME;
//       net_info_send.return_instr=NET_INSTR_TIME;

//   //    //第一个字节 年 只保存大于2000的值
//   //    //第二个字节 月  //第三个字节 日

//       QDateTime dateTime = QDateTime::currentDateTime();
//       net_info_send.data[0]=(dateTime.date().year()-2000)&0xff;
//       net_info_send.data[1]=(dateTime.date().month())&0xff;
//       net_info_send.data[2]=(dateTime.date().day())&0xff;
//       net_info_send.data[4]=(dateTime.time().hour())&0xff;
//       net_info_send.data[5]=(dateTime.time().minute())&0xff;
//       net_info_send.data[6]=(dateTime.time().second())&0xff;



//       net_info_send.data[5]=(dateTime.time().minute())&0xff;
//       net_info_send.data[6]=(dateTime.time().second())&0xff;

//       sen_instr_data(&net_info_send);

    }
    else{
        //先大致进行重连
        reconnect_net();


    }


   //直接发送空包就可以了。

}

void TCPClientThread::client_readMessage_slot()
{
     //在这里接收信息

   QByteArray datagram = tcpClient->readAll();
   int i=0,j=0;
   int recv_data_size=datagram.size();

   uint16_t instr=0;
   int data;

   qDebug("recv=%d",recv_data_size);

   //根据接收的指令类型来判断

   if(recv_data_size<8)
   {
       return ;
   }



   net_info_recv.instr=(((uchar)datagram.at(1)<<8)|((uchar)datagram.at(0)));
   net_info_recv.return_instr=(((uchar)datagram.at(3)<<8)|((uchar)datagram.at(2)));
   net_info_recv.len=(((uchar)datagram.at(5)<<8)|((uchar)datagram.at(4)));
   net_info_recv.crc=(((uchar)datagram.at(7)<<8)|((uchar)datagram.at(6)));
   //因为有可能多帧数据
   uint16_t crc=0;
   if(net_info_recv.len+8>recv_data_size){
        return;
   }
   //进行CRC 校验
   for(int i=0;i<6;i++){
       crc+=(uchar)datagram.at(i);
   }
   for(int i=8;i<recv_data_size;i++){
       crc+=(uchar)datagram.at(i);
   }

   if(net_info_recv.crc!=crc){
       return;
   }

   //根据指令，来完成
   int temp_data;
   switch(net_info_recv.instr)
   {
   case NET_INSTR_VERSION_INFO:   //接收到版本信息
       version_no=((uchar)datagram.at(11)<<24)|((uchar)datagram.at(10)<<16)|((uchar)datagram.at(9)<<8)|((uchar)datagram.at(8));
       break;
       //这个按照道理讲先收到文件信息
   case NET_SEND_DATA_INFO:
       //首先接收文件信息
       if(recv_data_size==100)  //这里面的数据肯定是100个字节
       {
           //先算出文件名的长度
       }
       break;
   case NET_SEND_GRADIENT:
       temp_data=((uchar)datagram.at(11)<<24)|((uchar)datagram.at(10)<<16)|((uchar)datagram.at(9)<<8)|((uchar)datagram.at(8));

       proc_send_gradient(temp_data);
       break;

   case NET_SEND_DATA:
       //用来接收数据
   case NET_INSTR_TIME:
       //用来接收时间的回复信息

       break;
   default:
       break;
   }
}

//表明要发送的指令
void TCPClientThread::recv_instr_slot(int instr)
{
    //如果处于正在发送中，还有阻塞的问题  一开始需要发送一个登陆信息
   // sen_instr_data(instr,NULL,4);
}


void TCPClientThread::sen_instr_data_slot(T_NET_Info *info)
{
   //qDebug("2321=%x",info->instr);
    sen_instr_data(info);
}

void TCPClientThread::set_ip_port()
{
     int port = 8081;
     QHostAddress address;
     address.setAddress("192.168.1.104");
     tcpClient->abort();
     tcpClient->connectToHost(address,port);
}

int TCPClientThread::send_data_to_server(void *s,int length)
{
    if(!is_connected)
    {
        return -1;
    }
    char *send_buf=(char*)s;
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);

    out.setVersion(QDataStream::Qt_5_7);

    //length  最长为256个字节，防止字节过长。
    if(length>0)
    {
        out.writeRawData(send_buf,length);
        //这个要判断下返回值
        if(tcpClient->write(block,block.size())==length)
        {
            return 0;  //这个表示发送成功
        }
    }

    return 2;


}

void TCPClientThread::proc_send_gradient(int data)
{
    emit gradient_singal(data);
}

void TCPClientThread::connect_slot()  //这个表示已经连接好
{
    is_connected=true;
    qDebug("have connected!");
    QObject::connect(tcpClient,SIGNAL(readyRead()),this,SLOT(client_readMessage_slot()));



    //最好在这里做一个连接网络提示


    //发送信息指令,表示已经连接上了。
    //先发送当前的版本号。
     //4个字节
    net_info_send.len=4;
    net_info_send.instr=NET_INSTR_VERSION_INFO;
    net_info_send.return_instr=NET_INSTR_VERSION_INFO;

    net_info_send.data[0]=( char)((CUR_VERSION)&0xff);
    net_info_send.data[1]=( char)((CUR_VERSION>>8)&0xff);
    net_info_send.data[2]=( char)((CUR_VERSION>>16)&0xff);
    net_info_send.data[3]=( char)((CUR_VERSION>>24)&0xff);

    sen_instr_data(&net_info_send);

    //在这里需要收应答码
}


void TCPClientThread::disconnect_slot()
{
    is_connected=false;
    qDebug("have disconnected!");
}



TCPClientThread::~TCPClientThread()
{
    qDebug("TCP DELETE!");

    delete tcpClient;
}
