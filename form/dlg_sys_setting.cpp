#include "dlg_sys_setting.h"
#include "ui_dlg_sys_setting.h"
#include "configure.h"
extern T_Program_Setting gt_program_setting;
Dlg_Sys_Setting::Dlg_Sys_Setting(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dlg_Sys_Setting){

    ui->setupUi(this);
    this->setWindowTitle("系统设置");
    init();
}



void Dlg_Sys_Setting::init(){

    doubleValidator = new QDoubleValidator(-1.0, 1.0, 2, this);

    doubleValidator->setNotation(QDoubleValidator::StandardNotation);

    ui->le_n40->setValidator(doubleValidator);
    ui->le_n20->setValidator(doubleValidator);
    ui->le_p0->setValidator(doubleValidator);
    ui->le_p20->setValidator(doubleValidator);
    ui->le_p40->setValidator(doubleValidator);
    ui->le_p60->setValidator(doubleValidator);
    ui->le_p80->setValidator(doubleValidator);
    ui->le_p100->setValidator(doubleValidator);
    ui->le_p120->setValidator(doubleValidator);


   //进行显示
    ui->le_n40->setText(QString::number((double)gt_program_setting.compensate_temp_n40/100,'f',2));
    ui->le_n20->setText(QString::number((double)gt_program_setting.compensate_temp_n20/100,'f',2));
    ui->le_p0->setText(QString::number((double)gt_program_setting.compensate_temp_p0/100,'f',2));
    ui->le_p20->setText(QString::number((double)gt_program_setting.compensate_temp_p20/100,'f',2));
    ui->le_p40->setText(QString::number((double)gt_program_setting.compensate_temp_p40/100,'f',2));
    ui->le_p60->setText(QString::number((double)gt_program_setting.compensate_temp_p60/100,'f',2));
    ui->le_p80->setText(QString::number((double)gt_program_setting.compensate_temp_p80/100,'f',2));
    ui->le_p100->setText(QString::number((double)gt_program_setting.compensate_temp_p100/100,'f',2));
    ui->le_p120->setText(QString::number((double)gt_program_setting.compensate_temp_p120/100,'f',2));


    ui->le_rate0->setValidator(doubleValidator);
    ui->le_rate20->setValidator(doubleValidator);
    ui->le_rate40->setValidator(doubleValidator);
    ui->le_rate60->setValidator(doubleValidator);
    ui->le_rate80->setValidator(doubleValidator);
    ui->le_rate100->setValidator(doubleValidator);

    ui->le_rate0->setText(QString::number((double)gt_program_setting.compensate_humi_0/100,'f',2));
    ui->le_rate20->setText(QString::number((double)gt_program_setting.compensate_humi_20/100,'f',2));
    ui->le_rate40->setText(QString::number((double)gt_program_setting.compensate_humi_40/100,'f',2));
    ui->le_rate60->setText(QString::number((double)gt_program_setting.compensate_humi_60/100,'f',2));
    ui->le_rate80->setText(QString::number((double)gt_program_setting.compensate_humi_80/100,'f',2));
    ui->le_rate100->setText(QString::number((double)gt_program_setting.compensate_humi_100/100,'f',2));


    ui->label_real_humi_value->setText("未读到");
    ui->label_real_temp_value->setText("未读到");
    ui->label_correct_temp_value->setText("未读到");
    ui->label_correct_humi_value->setText("未读到");

    timer=new QTimer(this);
    QObject::connect(timer,SIGNAL(timeout()),this,SLOT(timerout_slot()));

    timer->start(2000);  //2S


}

void Dlg_Sys_Setting::accept(){ //重载他的OK接收事件
    emit closeSignal();//发出关闭信号
}

void Dlg_Sys_Setting::reject(){ //重载他的cancel接收事件
    emit closeSignal();
}



void Dlg_Sys_Setting::closeEvent(QCloseEvent *){
    gt_program_setting.compensate_temp_n40=(ui->le_n40->text().toDouble()*100);
    gt_program_setting.compensate_temp_n20=(ui->le_n20->text().toDouble()*100);
    gt_program_setting.compensate_temp_p0=(ui->le_p0->text().toDouble()*100);
    gt_program_setting.compensate_temp_p20=(ui->le_p20->text().toDouble()*100);
    gt_program_setting.compensate_temp_p40=(ui->le_p40->text().toDouble()*100);
    gt_program_setting.compensate_temp_p60=(ui->le_p60->text().toDouble()*100);
    gt_program_setting.compensate_temp_p80=(ui->le_p80->text().toDouble()*100);
    gt_program_setting.compensate_temp_p100=(ui->le_p100->text().toDouble()*100);
    gt_program_setting.compensate_temp_p120=(ui->le_p120->text().toDouble()*100);

    gt_program_setting.compensate_humi_0=(ui->le_rate0->text().toDouble()*100);
    gt_program_setting.compensate_humi_20=(ui->le_rate20->text().toDouble()*100);
    gt_program_setting.compensate_humi_40=(ui->le_rate40->text().toDouble()*100);
    gt_program_setting.compensate_humi_60=(ui->le_rate60->text().toDouble()*100);
    gt_program_setting.compensate_humi_80=(ui->le_rate80->text().toDouble()*100);
    gt_program_setting.compensate_humi_100=(ui->le_rate100->text().toDouble()*100);
    emit closeSignal();//发出关闭信号
}



void Dlg_Sys_Setting::show_temp_humi(int temp,int humi){
   //
    int real_humi,real_temp;
    int correct_humi,correct_temp;

    real_humi=humi;
    real_temp=temp;

    ui->label_real_temp_value->setText(QString::number((double)real_temp/100,'f',2));
    ui->label_real_humi_value->setText(QString::number((double)real_humi/100,'f',2));

    //在下面写计算函数

    //获得校对值ui->le_n40
    gt_program_setting.compensate_temp_n40=(ui->le_n40->text().toDouble()*100);
    gt_program_setting.compensate_temp_n20=(ui->le_n20->text().toDouble()*100);
    gt_program_setting.compensate_temp_p0=(ui->le_p0->text().toDouble()*100);
    gt_program_setting.compensate_temp_p20=(ui->le_p20->text().toDouble()*100);
    gt_program_setting.compensate_temp_p40=(ui->le_p40->text().toDouble()*100);
    gt_program_setting.compensate_temp_p60=(ui->le_p60->text().toDouble()*100);
    gt_program_setting.compensate_temp_p80=(ui->le_p80->text().toDouble()*100);
    gt_program_setting.compensate_temp_p100=(ui->le_p100->text().toDouble()*100);
    gt_program_setting.compensate_temp_p120=(ui->le_p120->text().toDouble()*100);

    gt_program_setting.compensate_humi_0=(ui->le_rate0->text().toDouble()*100);
    gt_program_setting.compensate_humi_20=(ui->le_rate20->text().toDouble()*100);
    gt_program_setting.compensate_humi_40=(ui->le_rate40->text().toDouble()*100);
    gt_program_setting.compensate_humi_60=(ui->le_rate60->text().toDouble()*100);
    gt_program_setting.compensate_humi_80=(ui->le_rate80->text().toDouble()*100);
    gt_program_setting.compensate_humi_100=(ui->le_rate100->text().toDouble()*100);

    correct_temp=Configure::calc_temperature(temp);
    correct_humi=Configure::calc_humi(humi);

    ui->label_real_humi_value->setText(QString::number((float)real_humi/100,'f',2).append("%"));
    ui->label_real_temp_value->setText(QString::number((float)real_temp/100,'f',2).append("℃"));

    ui->label_correct_temp_value->setText(QString::number((float)correct_temp/100,'f',2).append("℃"));
    ui->label_correct_humi_value->setText(QString::number((float)correct_humi/100,'f',2).append("%"));

}

void Dlg_Sys_Setting::timerout_slot(){
    emit temp_humi_signal();
}




Dlg_Sys_Setting::~Dlg_Sys_Setting(){
    delete ui;
    delete doubleValidator;
    delete timer;
}
