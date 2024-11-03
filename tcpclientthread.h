#ifndef TCPCLIENTTHREAD_H
#define TCPCLIENTTHREAD_H

#include<QObject>
#include<QTcpServer>
#include<QTcpSocket>
#include<QObject>
#include <configure.h>
#include <QThread>
#include <QFile>
#include <QDataStream>
#include <QTimer>
#include "net_common.h"

class TCPClientThread : public QObject
{
    Q_OBJECT

    typedef struct _Net_Return_Code
    {
        int instr;  //指令
        int step;   //表示进行到哪一步了;
    }T_Net_Return_Code,*PT_Net_Return_Code;

    //每秒需要发送的数据
    typedef struct _Net_Send_Data_PerSecond
    {
        unsigned char year;
        unsigned char month;
        unsigned char day;
        unsigned char hour;
        unsigned char minute;
        unsigned char second;
        unsigned short cur_step; //当前步数

    }T_Net_Send_Data_PerSecond,*PT_Net_Send_Data_PerSecond;


public:
    TCPClientThread(int version);
    ~TCPClientThread();
    bool is_connected;  //是否连接好

    //需要写个重连的函数
    void reconnect_net();

    int sen_instr_data(T_NET_Info *info);  //这个是外部的接口
    int version_no;

private:
    QTcpSocket *tcpClient;
    QTimer *timer;
    void set_ip_port();

    int send_data_to_server(void *s, int length);

    T_Net_Return_Code net_return_code_t;  //用于数据返回码的。

    T_NET_Info  net_info_send;
    T_NET_Info  net_info_recv;

    T_Net_Send_Data_PerSecond  net_sendData_persecond;
    int cur_trans_state; //当前的状态

    void proc_send_gradient(int data);




public slots:
    void recv_instr_slot(int instr);

    void sen_instr_data_slot(T_NET_Info *info);  //这个是外部的接口



private slots:
    void connect_slot();
    void disconnect_slot();
    void client_readMessage_slot();
    void timerout_slot();

signals:
    void  send_data_signal(int len,int options);
    void  gradient_singal(int data);
    void  wifi_connect_singal(bool connected);

};

#endif // TCPCLIENTTHREAD_H
