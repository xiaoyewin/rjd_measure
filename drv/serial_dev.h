#ifndef SERIAL_DEV_H
#define SERIAL_DEV_H


#include "stdlib.h"
#include "fifo_buf.h"
#include <QVector>
#include <QSerialPort>        //提供访问串口的功能
#include <QSerialPortInfo>    //提供系统中存在的串口的信息

#define MAX_BUF_SIZE 100


typedef struct
{
    /* 03H 06H 读写保持寄存器 */
    int  baudrate;
    QString port; //端口号
    uint8_t  databit;
    uint8_t  checkbit;
    uint8_t  stopbit;
}T_Serial_Conf;



class Serial_Dev : public QObject
{
    Q_OBJECT
public:
    Serial_Dev();
    ~Serial_Dev();
    QSerialPort serial;
    bool is_serial_open;


    //如果超出怎么处理  做个封装的FIFO 把
    Fifo_Buf *read_buf;
    Fifo_Buf *write_buf;

    int serial_read(uint8_t* buf,int len);
    int serial_write(uint8_t* buf,int len);

    bool serial_open(T_Serial_Conf *conf);
    void serial_close();
    QVector<QString> serial_scan(); //扫描可用串口


private slots:
    void serialPort_readyRead();

signals:
    void data_received_serial(int len);


};

#endif // SERIAL_DEV_H
