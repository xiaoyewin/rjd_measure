#ifndef MODBUS_RTU_H
#define MODBUS_RTU_H
#include <stdint.h>
#include <QString>
#include <QObject>
#include <functional>
#include <QTimer>
#include "drv/serial_dev.h"


//此协议包含了设备的物理接口
typedef struct
{
    /* 03H 06H 读写保持寄存器 */
    int16_t temperature_reg; //0x0   //第一个值是识别好  为0x1234  //32768
    int16_t humi_reg;
    int16_t dist_1; //0x1   //   0-10000
    int16_t dist_2; //0x2    ¼ÌµçÆ÷µÄ×´Ì¬£¬Ö»¶Á
    int16_t dist_3; //0x3      0-65535
    int16_t dist_4; //0x3      0-65535
    int16_t dist_5; //0x3      0-65535
    int16_t dist_6; //0x3      0-65535
    int16_t reserve_reg[92];  //0x2

    int16_t  model_number_reg;//0x64
    int16_t  testPoint_number_reg;//0x65
    int16_t  device_addr_reg;//0x66
    int16_t  baudrate_reg;//0x67  //1:2400  2:4800 3:9600 4:19200  5:38400  6:115200
    int16_t  comm_mode_reg;//0x68    //0x1:RS485   OX2:主动上传 其他参数参数没有用
    int16_t  protocol_reg;//0x69
    int16_t  upload_time_reg;//0x6a   //1-3600  秒       、、
    int16_t  checkbit_reg;//0x6b
    int16_t  stopbit_reg;//0x6c
    int16_t  temperature_correct_reg;//0x6d
    int16_t  humidity_correct_reg;//0x6e
    int16_t  smoke_correct_reg;//0x6f

    int16_t  pm1_0_correct_reg; //0x70  sunLight_correct_reg   （-1000~1000）
    int16_t  nc_correct_reg; //0x71
    int16_t  pm1_0_alarm_threshold_reg; //0x72   （0~5000）
    int16_t  ppm_threshold_reg; //0x73           （0~4095）
    int16_t  smoke_da_cur_reg; //0x74            （0~4095）
    int16_t  smoke_da_max_reg; //0x75  固定      （0~4095）
    int16_t  smoke_da_min_reg; //0x76            （0~4095）
    int16_t  smoke_range_max_reg; //0x77         （0~5000）
    int16_t  smoke_range_min_reg; //0x78         （0~5000）
    int16_t  alarm_stop_reset_time_reg; //0x79   （0~3600）   单位分   最大 1440
    int16_t  alarm_stop_reg; //07a

    //uint16_t  reserve_reg_1;  //reserve 6f

//    uint16_t  temperature_correct__80_reg;
//    uint16_t  temperature_correct__40_reg;
//    uint16_t  temperature_correct_0_reg;
//    uint16_t  temperature_correct_40_reg;
//    uint16_t  temperature_correct_80_reg;
//    uint16_t  temperature_correct_120_reg;
//    uint16_t  temperature_correct_160_reg;
//    uint16_t  temperature_correct_200_reg;
//    uint16_t  temperature_correct_240_reg;
//    uint16_t  temperature_correct_280_reg;
//    uint16_t  temperature_correct_320_reg;
//    uint16_t  temperature_correct_360_reg;
//    uint16_t  temperature_correct_400_reg;
    //.... 设置最大显示的温度
    int16_t  temperature_min_reg;
    int16_t  temperature_max_reg;

    //T_TEMP_CORRECT_VALUE temp_value[MAX_CORRECT_SIZE];//一般最大需要20组数据，进行从0到大排序即可

}T_MODBUS_REG;



typedef struct{
   QString dev_name;
   int (*dev_init)(void);
   std::function<int(uint8_t* ,int)> dev_read;
   //int (*dev_read)(uint8_t* buf,int len);
   //int (*dev_write)(uint8_t* buf,int len);
   std::function<int(uint8_t* ,int)> dev_write;
   int (*dev_close)(void);
}T_Modbus_Device;


class Modbus_rtu : public QObject
{
    Q_OBJECT
public:
    Modbus_rtu();
    ~Modbus_rtu();


    void modbus_init();
    int dev_open(QString port);

    int serial_close();
    void scan_target();
    bool read_reg(uint16_t reg_addr, uint16_t data_len);
    bool read_data(uint8_t dev_addr, uint16_t reg_addr, uint16_t data_len);

    void read_data_debug();
    bool read_write_data_custom(uint8_t dev_addr, uint8_t func_no, uint16_t reg_addr, uint16_t data_len,int16_t write_data,uint8_t *recv_data, uint32_t time_out);


    bool write_data(uint8_t dev_addr, uint16_t reg_addr, uint16_t data_len, uint8_t *send_data, uint32_t time_out);
    bool write_reg(uint16_t reg_addr,uint8_t* send_data);


    QString debug_string;
    QString package_debugInfo(uint8_t *buf,int len,int radix);
    bool is_busy;




private:
    T_Modbus_Device seral_dev;

    uint8_t target_addr;

    uint8_t send_addr;

    Serial_Dev * serial;

    uint8_t  recv_data_buf[30];

    int before_send_len;


private slots:
   void serial_data_received_slot(int len);
   void debug_show_slot(QString s);


signals:
    void debug_signal(QString s);
    void no_data_read();  //表示数据读不到

    void data_received(uint8_t *data,int len);
};

#endif // MODBUS_RTU_H
