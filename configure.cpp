#include "configure.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QMessageBox>
//这个还包含对应的数据格式
T_Item_Data gt_item_data;

T_Item_Data open_item_data;  //这个表示已经打开的项目 因为会有多个的情况先暂时只有一个的情况
Configure::Configure(){
    init_item_data();
   // read_conf();
}
void Configure::init_item_data(){
    gt_item_data.is_exsit=false;  //表示里面为空
}

void Configure::read_conf()
{

    QFile file("./conf/setting.txt");
    if(!file.open(QIODevice::ReadOnly)){
        QMessageBox::warning(NULL,QObject::tr("警告"),QObject::tr("配置文件setting.txt"));
        return;
    }


    QTextStream *file_in;

    file_in=new QTextStream(&file);// 只读 输入数据流
    file_in->seek(0); //开始

    gt_item_data.shrink_rate=0;
    QString temp_string = file_in->readLine();//每次读取一行

    QStringList temp_stringlist = temp_string.split("?=");




    delete file_in;
}



