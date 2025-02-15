#include "configure.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QMessageBox>
//这个还包含对应的数据格式
T_Item_Data gt_item_data;

T_Item_Data open_item_data;  //这个表示已经打开的项目 因为会有多个的情况先暂时只有一个的情况

T_Program_Setting gt_program_setting;
Configure::Configure(){
    init_item_data();
    read_conf();
}

Configure::~Configure(){

}
void Configure::init_item_data(){
    gt_item_data.is_exsit=false;  //表示里面为空
}

void Configure::read_conf(){

    QFile file("./conf/setting.ini");
    if(!file.open(QIODevice::ReadOnly)){
        QMessageBox::warning(NULL,QObject::tr("警告"),QObject::tr("配置文件setting.txt"));
        return;
    }


    QDataStream *file_in;

    file_in=new QDataStream(&file);// 只读 输入数据流

    if(file.size()<(15*4)){
        //直接赋值为0; 不需要读了

        gt_program_setting.compensate_temp_n40=0;
        gt_program_setting.compensate_temp_n20=0;
        gt_program_setting.compensate_temp_p0=0;
        gt_program_setting.compensate_temp_p20=0;
        gt_program_setting.compensate_temp_p40=0;
        gt_program_setting.compensate_temp_p60=0;
        gt_program_setting.compensate_temp_p80=0;
        gt_program_setting.compensate_temp_p100=0;
        gt_program_setting.compensate_temp_p120=0;

        gt_program_setting.compensate_humi_0=0;
        gt_program_setting.compensate_humi_20=0;
        gt_program_setting.compensate_humi_40=0;
        gt_program_setting.compensate_humi_60=0;
        gt_program_setting.compensate_humi_80=0;
        gt_program_setting.compensate_humi_100=0;

        delete file_in;
        return ;

    }

    *file_in>>gt_program_setting.compensate_temp_n40;
    *file_in>>gt_program_setting.compensate_temp_n20;
    *file_in>>gt_program_setting.compensate_temp_p0;
    *file_in>>gt_program_setting.compensate_temp_p20;
    *file_in>>gt_program_setting.compensate_temp_p40;
    *file_in>>gt_program_setting.compensate_temp_p60;
    *file_in>>gt_program_setting.compensate_temp_p80;
    *file_in>>gt_program_setting.compensate_temp_p100;
    *file_in>>gt_program_setting.compensate_temp_p120;

    *file_in>>gt_program_setting.compensate_humi_0;
    *file_in>>gt_program_setting.compensate_humi_20;
    *file_in>>gt_program_setting.compensate_humi_40;
    *file_in>>gt_program_setting.compensate_humi_60;
    *file_in>>gt_program_setting.compensate_humi_80;
    *file_in>>gt_program_setting.compensate_humi_100;

    delete file_in;
}



void Configure::write_conf()
{

    QFile file("./conf/setting.ini");
    if(!file.open(QIODevice::WriteOnly)){
        QMessageBox::warning(NULL,QObject::tr("警告"),QObject::tr("配置文件setting.txt"));
        return;
    }


    QDataStream *file_out;

    file_out=new QDataStream(&file);// 只读 输入数据流

    *file_out<<gt_program_setting.compensate_temp_n40;
    *file_out<<gt_program_setting.compensate_temp_n20;
    *file_out<<gt_program_setting.compensate_temp_p0;
    *file_out<<gt_program_setting.compensate_temp_p20;
    *file_out<<gt_program_setting.compensate_temp_p40;
    *file_out<<gt_program_setting.compensate_temp_p60;
    *file_out<<gt_program_setting.compensate_temp_p80;
    *file_out<<gt_program_setting.compensate_temp_p100;
    *file_out<<gt_program_setting.compensate_temp_p120;

    *file_out<<gt_program_setting.compensate_humi_0;
    *file_out<<gt_program_setting.compensate_humi_20;
    *file_out<<gt_program_setting.compensate_humi_40;
    *file_out<<gt_program_setting.compensate_humi_60;
    *file_out<<gt_program_setting.compensate_humi_80;
    *file_out<<gt_program_setting.compensate_humi_100;

    delete file_out;
}


int Configure::calc_humi(int humi){
    int real_rate0,real_rate20,real_rate40,real_rate60,real_rate80,real_rate100;
    real_rate0=0+gt_program_setting.compensate_humi_0;
    real_rate20=2000+gt_program_setting.compensate_humi_20;
    real_rate40=4000+gt_program_setting.compensate_humi_40;
    real_rate60=6000+gt_program_setting.compensate_humi_60;
    real_rate80=8000+gt_program_setting.compensate_humi_80;
    real_rate100=10000+gt_program_setting.compensate_humi_100;
    double rate;
    int output_humi=0;

    if((humi>=0)&&(humi<2000)){
        //
        rate=(double)(real_rate20-real_rate0)/2000;
        output_humi=real_rate0+(humi-0)*rate;
    }
    else if((humi>=2000)&&(humi<4000)){
        rate=(double)(real_rate40-real_rate20)/2000;
        output_humi=real_rate20+(humi-2000)*rate;
    }
    else if((humi>=4000)&&(humi<6000)){
        rate=(double)(real_rate60-real_rate40)/2000;
        output_humi=real_rate40+(humi-4000)*rate;
    }
    else if((humi>=6000)&&(humi<8000)){
        rate=(double)(real_rate80-real_rate60)/2000;
        output_humi=real_rate60+(humi-6000)*rate;
    }
    else if((humi>=8000)&&(humi<10000)){
        rate=(double)(real_rate100-real_rate80)/2000;
        output_humi=real_rate80+(humi-8000)*rate;
    }
    else{
        output_humi=-20000;
    }

    return output_humi;


}

int Configure::calc_temperature(int temp){

    int real_temp_n40,real_temp_n20,real_temp_p0,real_temp_p20,real_temp_p40,real_temp_p60,real_temp_p80,real_temp_p100,real_temp_p120;

    real_temp_n40=-4000+gt_program_setting.compensate_temp_n40;
    real_temp_n20=-2000+gt_program_setting.compensate_temp_n20;
    real_temp_p0=0+gt_program_setting.compensate_temp_p0;
    real_temp_p20=2000+gt_program_setting.compensate_temp_p20;
    real_temp_p40=4000+gt_program_setting.compensate_temp_p40;
    real_temp_p60=6000+gt_program_setting.compensate_temp_p60;
    real_temp_p80=8000+gt_program_setting.compensate_temp_p80;
    real_temp_p100=10000+gt_program_setting.compensate_temp_p100;
    real_temp_p120=12000+gt_program_setting.compensate_temp_p120;

     double rate;

     int output_temp=0;
     if((temp>=-4000)&&(temp<-2000)){
         //
         rate=(double)(real_temp_n20-real_temp_n40)/2000;
         output_temp=real_temp_n40+(temp+4000)*rate;
     }
     else if((temp>=-2000)&&(temp<0)){
         rate=(double)(real_temp_p0-real_temp_n20)/2000;
         output_temp=real_temp_n20+(temp+2000)*rate;
     }
     else if((temp>=0)&&(temp<2000)){
         rate=(double)(real_temp_p20-real_temp_p0)/2000;
         output_temp=real_temp_p0+(temp+0)*rate;
     }
     else if((temp>=2000)&&(temp<4000)){
         rate=(double)(real_temp_p40-real_temp_p20)/2000;
         output_temp=real_temp_p20+(temp-2000)*rate;
     }
     else if((temp>=4000)&&(temp<6000)){
         rate=(double)(real_temp_p60-real_temp_p40)/2000;
         output_temp=real_temp_p40+(temp-4000)*rate;
     }
     else if((temp>=6000)&&(temp<8000)){
         rate=(double)(real_temp_p80-real_temp_p60)/2000;
         output_temp=real_temp_p60+(temp-6000)*rate;
     }
     else if((temp>=8000)&&(temp<10000)){
         rate=(double)(real_temp_p100-real_temp_p80)/2000;
         output_temp=real_temp_p80+(temp-8000)*rate;

     }
     else if((temp>=10000)&&(temp<=12000)){
         rate=(double)(real_temp_p120-real_temp_p100)/2000;
         output_temp=real_temp_p120+(temp-10000)*rate;
     }
     else{
         output_temp=-20000; //表示未连接
     }

     return output_temp;


}

