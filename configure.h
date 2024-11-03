#ifndef CONFIGURE_H
#define CONFIGURE_H

#include <QString>
#include<QVector>
#include<QDateTime>
#define CUR_VERSION 1 //当前版本号
#define PI_180  3.14159265

#define STEP_LENGTH  300


#define ZERO_DEFAULT_VALUE  -1000

#define ZERO_DEFAULT_VALUE_DOUBLE  -2000

typedef struct _sample_data{
    int timestamp;
    int dist;//表示计算的间距
}T_Sample_Data;

//这个新建的只会有一个 ，但是打开的话会有好几个 ，相同的也不会合并
typedef struct _item_data{
    QString filename; //表示打开的路径
    QString item_num;
    QString item_name;
    QDateTime item_time;//创建时间
    QString tester; //测试人
    QString backup;  // 备注信息
    int  test_len; // 试件长度
    int  total_time;  //总时间
    int  sample_interval; //采样间隔
    int  sample_time; //采样的时间
    int  test_standard;// 测试的标准
    int test_type;  //0:单面测量，双面测量
    int probe_set[6];  // 6种通道所对应的探头

    QVector<int>ch_data[6];
    QVector<int> temperature;
    QVector<int> humi;

    // 当前的始终是最新的
    bool is_exsit;
    double shrink_rate; //收缩率
}T_Item_Data;




#define CUR_ITEM item_data

#define DEFAULT_ITEM gt_item_data


class Configure
{
public:
    Configure();

private:
    void init_item_data();
    void read_conf();
};

#endif // CONFIGURE_H
