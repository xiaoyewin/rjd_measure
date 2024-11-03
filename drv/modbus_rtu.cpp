#include "modbus_rtu.h"
#include <QTime>
#include <QtDebug>
#include <QCoreApplication>

#include <QThread>
Modbus_rtu::Modbus_rtu(){
    target_addr=-1;// 表示没有设备地址

    send_addr=-1;
    is_busy=false;

    serial=new Serial_Dev();

    seral_dev.dev_init=NULL;
    seral_dev.dev_close=NULL;
    seral_dev.dev_read = std::bind(&Serial_Dev::serial_read,serial,std::placeholders::_1, std::placeholders::_2);
    seral_dev.dev_write = std::bind(&Serial_Dev::serial_write,serial,std::placeholders::_1, std::placeholders::_2);

    bool is_connecf= QObject::connect(serial,SIGNAL(data_received_serial(int)),this,SLOT(serial_data_received_slot(int)));


    if(is_connecf){
        qDebug("sucefully");
    }
    else{
        qDebug("failed");
    }



   QObject::connect(this,SIGNAL(debug_signal(QString)),this,SLOT(debug_show_slot(QString)));

    before_send_len=0;

}

//这里面做个注册函数
void Modbus_rtu::modbus_init(){


}


int Modbus_rtu::serial_close(){
    serial->serial_close();
}

int Modbus_rtu::dev_open(QString port){


    if(serial->is_serial_open){
        return 0;
    }

    T_Serial_Conf conf;
    conf.port=port;
    conf.baudrate=9600;
    conf.databit=8;
    conf.checkbit=1;
    conf.stopbit=0;

    serial->serial_open(&conf);

    //return 0;  //调试的，需要删除

    if(serial->is_serial_open){

         scan_target();
         //需要等待3S

         QThread::msleep(1000);

         if(target_addr>=0){
             return 0;
         }
         else{
             return 1;
         }
    }
    else{
        return 2;
    }
}


//后期支持多设备接口

static const uint16_t crctalbeabs[] = {
    0x0000, 0xCC01, 0xD801, 0x1400, 0xF001, 0x3C00, 0x2800, 0xE401,
    0xA001, 0x6C00, 0x7800, 0xB401, 0x5000, 0x9C01, 0x8801, 0x4400
};

static uint16_t crc16tablefast(uint8_t *ptr, uint16_t len)
{
    uint16_t crc = 0xffff;
    uint16_t i;
    uint8_t ch;

    for (i = 0; i < len; i++) {
        ch = *ptr++;
        crc = crctalbeabs[(ch ^ crc) & 15] ^ (crc >> 4);
        crc = crctalbeabs[((ch >> 4) ^ crc) & 15] ^ (crc >> 4);
    }

    return crc;
}


static void delaymsec(int msec)
{
    QTime dieTime = QTime::currentTime().addMSecs(msec);

    while( QTime::currentTime() < dieTime )
        QCoreApplication::processEvents(QEventLoop::AllEvents, 1);
}



void Modbus_rtu::scan_target(){
    //需要等待
     read_data(0xFA,0x00,0x1);
}
bool Modbus_rtu::read_reg(uint16_t reg_addr, uint16_t data_len){
    is_busy=true;

    qDebug("t=%x,ref=%d,%d",target_addr,reg_addr,data_len);
    bool temp_bool=read_data(target_addr,reg_addr,data_len);
    is_busy=false;
    return temp_bool;
}





bool Modbus_rtu::read_write_data_custom(uint8_t dev_addr, uint8_t func_no, uint16_t reg_addr, uint16_t data_len,int16_t write_data,uint8_t *recv_data, uint32_t time_out){
    uint8_t data_buf[8];
    uint16_t crc_val = 0;

    memset(data_buf,0,sizeof (data_buf));

    data_buf[0] = dev_addr;
    data_buf[1] = func_no;     //read
    data_buf[2] = (reg_addr >> 8) & 0xFF;
    data_buf[3] = reg_addr & 0xFF;

    if(func_no==0x6){
        data_buf[4] = (write_data >> 8) & 0xFF;
        data_buf[5] = write_data & 0xFF;
    }
    else{
        data_buf[4] = (data_len >> 8) & 0xFF;
        data_buf[5] = data_len & 0xFF;
    }


    crc_val = crc16tablefast(data_buf,6);

    data_buf[6] = crc_val & 0xFF;
    data_buf[7] = (crc_val >> 8) & 0xFF;


    seral_dev.dev_write(data_buf,8);
    //这里发送完了，应该发个信号，表示数据发送完成
    debug_string="tx:";
    debug_string.append(package_debugInfo(data_buf,8,16));

    emit debug_signal(debug_string);


    int recv_len=0;

    //然后在这里接收数据
    int err_cnt=0;

    uint8_t recv_data_buf[5 + data_len * 2+10];//多10个防止数据溢出
    while(1){
        recv_len=seral_dev.dev_read(recv_data_buf,5 + data_len * 2);

        if(recv_len >= (int)(5 + data_len * 2))
            break;
        delaymsec(20);
        err_cnt++;
        if(err_cnt >= 5){
            return false;
        }
    }

    //recv_data

    //有2种情况，0XFA 还有一种设备地址

    if((recv_data_buf[0]!= dev_addr) && (dev_addr != 0xFA))
        return false;

    if((recv_data_buf[1]!= func_no))
        return false;




    crc_val = crc16tablefast(recv_data_buf,3 + data_len * 2);

    if(crc_val != (uint16_t)(recv_data_buf[3 + data_len * 2] + (uint16_t)recv_data_buf[3 + data_len * 2 + 1] * 256))
        return false;

    if(dev_addr == 0xFA)
        target_addr = recv_data_buf[0];


    debug_string="rx:";
    debug_string.append(package_debugInfo(recv_data_buf,recv_len,16));
    emit debug_signal(debug_string);
    //然后复制有用的数据到recv_data
    for(int i=0;i<(data_len * 2);i++){
        recv_data[i]=recv_data_buf[i+3];
    }
    return true;
}

bool Modbus_rtu::read_data(uint8_t dev_addr, uint16_t reg_addr, uint16_t data_len)
{
    uint8_t data_buf[8];
    uint16_t crc_val = 0;
    memset(data_buf,0,sizeof (data_buf));

    send_addr=dev_addr;

    data_buf[0] = dev_addr;
    data_buf[1] = 0x03;     //read
    data_buf[2] = (reg_addr >> 8) & 0xFF;
    data_buf[3] = reg_addr & 0xFF;
    data_buf[4] = (data_len >> 8) & 0xFF;
    data_buf[5] = data_len & 0xFF;

    crc_val = crc16tablefast(data_buf,6);

    data_buf[6] = crc_val & 0xFF;
    data_buf[7] = (crc_val >> 8) & 0xFF;

    seral_dev.dev_write(data_buf,8);
    //这里发送完了，应该发个信号，表示数据发送完成
    debug_string="tx:";
    debug_string.append(package_debugInfo(data_buf,8,16));
    qDebug("manual 002");
    emit debug_signal(debug_string);



    before_send_len=data_len;

    return true;



    //然后在这里接收数据
    //int err_cnt=0;

    //uint8_t recv_data_buf[5 + data_len * 2+10];//多10个防止数据溢出


    //这里应该加个延时。
   // qDebug("size=%s",package_debugInfo(recv_data_buf,recv_len,16));
    //qDebug(package_debugInfo(recv_data_buf,recv_len,16));

    //recv_data


    //有2种情况，0XFA 还有一种设备地址



}





void Modbus_rtu::read_data_debug()
{
    int data_len=2;
    int recv_len;
    uint8_t recv_data_buf[5 + data_len * 2+10];//多10个防止数据溢出
    while(1){
        recv_len=seral_dev.dev_read(recv_data_buf,5 + data_len * 2);
        if(recv_len>0){
           qDebug("recv_len=%d",recv_len);
        }

        if(recv_len >= (int)(5 + data_len * 2))
            break;
        delaymsec(100);
    }



}


void Modbus_rtu::debug_show_slot(QString s){
    qDebug("info");
    qDebug()<<s;
}
void Modbus_rtu::serial_data_received_slot(int len){


    if(before_send_len==0){
        return ;
    }


    uint8_t  temp_recv_data[30];
    serial->serial_read(temp_recv_data,len);



     qDebug("info004");
    if((temp_recv_data[0]!= target_addr) && (send_addr != 0xFA))
        return ;

    if(temp_recv_data[1]!= 0x03)
        return ;
    if(temp_recv_data[2] != before_send_len*2) //还有一种是采集到地址
        return ;


    uint16_t crc_val = crc16tablefast(temp_recv_data,3 + before_send_len * 2);

    if(crc_val != (uint16_t)(temp_recv_data[3 + before_send_len* 2] + (uint16_t)temp_recv_data[3 + before_send_len * 2 + 1] * 256))
        return ;




    debug_string="rx:";
    debug_string.append(package_debugInfo(temp_recv_data,len,16));
    emit debug_signal(debug_string);
    //然后复制有用的数据到recv_data
    if(send_addr == 0xFA){
        target_addr=temp_recv_data[0];
        return ;
    }

    for(int i=0;i<(before_send_len * 2);i++){
        recv_data_buf[i]=temp_recv_data[i+3];
    }

    //在这里得到数据
    //就是每5秒种就更新下
    emit data_received(recv_data_buf,before_send_len*2);
}

bool Modbus_rtu::write_reg(uint16_t reg_addr,uint8_t* send_data)
{
    return write_data(target_addr,reg_addr,1,send_data,0);
}


bool Modbus_rtu::write_data(uint8_t dev_addr, uint16_t reg_addr, uint16_t data_len, uint8_t *send_data, uint32_t time_out)
{
    uint8_t data_buf[6+data_len*2+10];
    uint16_t crc_val = 0;

    uint8_t err_cnt = 0;

    data_buf[0] = dev_addr;
    data_buf[1] = 0x06;     //write
    data_buf[2] = (reg_addr >> 8) & 0xFF;
    data_buf[3] = reg_addr & 0xFF;
    for(int i=0;i<(data_len*2);i++){
        data_buf[i+4] = send_data[i];
    }

    crc_val = crc16tablefast(data_buf,4+data_len*2);

    data_buf[data_len*2+4] = crc_val & 0xFF;
    data_buf[data_len*2+5] = (crc_val >> 8) & 0xFF;

    seral_dev.dev_write(data_buf,data_len*2+6);

    //这里发送完了，应该发个信号，表示数据发送完成
    debug_string="tx:";
    debug_string.append(package_debugInfo(data_buf,data_len*2+6,16));

    emit debug_signal(debug_string);

    uint8_t recv_data_buf[5 + data_len * 2+10];//多10个防止数据溢出

    int recv_len=0;
    while(1)
    {
        recv_len=seral_dev.dev_read(recv_data_buf,6 + data_len * 2);

        if(recv_len >= 8)
            break;
        delaymsec(20);
        err_cnt++;
        if(err_cnt >= 5){
            return false;
        }
    }


    if(recv_data_buf[1] != 0x06)
        return false;
    if(recv_data_buf[2] != (uint8_t)((reg_addr >> 8) & 0xFF))
        return false;
    if(recv_data_buf[3] != (uint8_t)((reg_addr) & 0xFF))
        return false;

    crc_val = crc16tablefast(recv_data_buf,data_len*2+4);
    if(crc_val != recv_data_buf[data_len*2+4] + recv_data_buf[data_len*2+5] * 256)
        return false;

    debug_string="rx:";
    debug_string.append(package_debugInfo(recv_data_buf,recv_len,16));
    emit debug_signal(debug_string);

    return true;
}



QString Modbus_rtu::package_debugInfo(uint8_t *buf, int len, int radix)
{
    QString output_string;
    int i=0;
    for(;i<(len-1);i++){
        if(radix==16){
           output_string.append("0x");
        }
        output_string.append(QString::number(buf[i],radix)).append("、");
    }
    if(radix==16){
       output_string.append("0x");
    }
    output_string.append(QString::number(buf[i],radix));
    return output_string;

}

Modbus_rtu::~Modbus_rtu(){
    delete serial;

}
