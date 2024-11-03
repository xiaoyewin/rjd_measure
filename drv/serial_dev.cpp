#include "serial_dev.h"



Serial_Dev::Serial_Dev(){
    is_serial_open=false;
    read_buf=new Fifo_Buf(1000);
    write_buf=new Fifo_Buf(1000);
}
Serial_Dev::~Serial_Dev(){
    delete read_buf;
    delete write_buf;
}



void Serial_Dev::serialPort_readyRead(){
    //从接收缓冲区中读取数据
    QByteArray buffer = serial.readAll();

    uint8_t *buf;//只是一个指针
    int len;//buf的长度
    buf = (uint8_t *)buffer.data();
    len = buffer.size();

    read_buf->fifo_buf_write(buf,len);

    qDebug("manual 003");

    emit data_received_serial(len);

    //因该发送一个信号给上面，告知已经得到数据

    //表明读取到了数据
}


void Serial_Dev::serial_close(){
    if(is_serial_open){
        is_serial_open=false;
        serial.close();
    }

}

bool Serial_Dev::serial_open(T_Serial_Conf *conf){
    //设置串口名
    serial.setPortName(conf->port);
    //设置波特率
    serial.setBaudRate(conf->baudrate);
    //设置数据位数
    switch(conf->databit){
        case 5: serial.setDataBits(QSerialPort::Data5); break;
        case 6: serial.setDataBits(QSerialPort::Data6); break;
        case 7: serial.setDataBits(QSerialPort::Data7); break;
        case 8: serial.setDataBits(QSerialPort::Data8); break;
        default: break;
    }
    //设置奇偶校验
    switch(conf->checkbit){
        case 0: serial.setParity(QSerialPort::NoParity); break;
        case 1: serial.setParity(QSerialPort::OddParity); break;
        case 2: serial.setParity(QSerialPort::EvenParity); break;
        default: break;
    }
    //设置停止位
    switch(conf->stopbit){
        case 0: serial.setStopBits(QSerialPort::OneStop); break;
        case 1: serial.setStopBits(QSerialPort::OneAndHalfStop); break;
        case 2: serial.setStopBits(QSerialPort::TwoStop); break;
        default: break;
    }
    //设置流控制  无流量控制
    serial.setFlowControl(QSerialPort::NoFlowControl);
    //打开串口
    if(!serial.open(QIODevice::ReadWrite)){
        return false;
    }
    QObject::connect(&serial,SIGNAL(readyRead()),this,SLOT(serialPort_readyRead()));
    is_serial_open=true;
    return true;
}

QVector<QString> Serial_Dev::serial_scan()
{
    QVector<QString>  resturn_string;
    foreach(const QSerialPortInfo &info, QSerialPortInfo::availablePorts()){
        resturn_string.push_back(info.portName());
    }
}


int Serial_Dev::serial_read(uint8_t* buf,int len){
    if(is_serial_open){
       return read_buf->fifo_buf_read(buf,len);
    }
    else{
        return -1;
    }
}

int Serial_Dev::serial_write(uint8_t* buf,int len){
    QByteArray temp_array;
    temp_array = QByteArray((char *)buf, len);
    int size=serial.write(temp_array);
    serial.flush();
    return size;
}

