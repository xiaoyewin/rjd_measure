#include "mainwindow.h"
#include "ui_mainwindow.h"


#include <QDesktopWidget>
#include <QApplication>


#include <QIcon>
#include <QDockWidget>
#include <QMessageBox>
#include <QPainter>
#include <QFileDialog>
#include "excelengine.h"
#include  <qmath.h>
#include <QDateTime>
#include <QItemDelegate>

//4-1  做个模拟程序网络程序来控制
extern T_Item_Data gt_item_data;
extern T_Item_Data open_item_data;


extern short g_titlebar_height;
extern short g_toolbar_height;



QAction * createAction(QString name,QString object_name,QString icon_path){
   QAction *action = new QAction(name);
   //QIcon icon(":/images/new.png");

   action->setIcon(QIcon(icon_path));//图片路劲
   action->setObjectName(object_name);//应该和action名字一样
   return action;
}


//0X6  表示测距的数据 ？？？
int MainWindow::anlay_rjdd_header(unsigned char *data,int len){
    if(len!=6){
        return 0;
    }
    if(data[0]==0x77){
        if(data[1]==0x61){
            if(data[2]==0x6b){
                if(data[3]==0x73){
                    if(data[4]==0x71){
                        if(data[5]==0x06){
                            return 0x06;
                        }
                    }
                }
            }
        }
    }
    return 0;
}




//其实我觉得子线程应该是实时采集比较好，可以查看是否断开

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
   // this->setWindowState(Qt::WindowMaximized);//初始设置最大化
    //this->setWindowTitle("智能收缩膨胀试验监测系统");


    this->setWindowFlags(Qt::FramelessWindowHint);
        // 设置窗口背景透明;
    setAttribute(Qt::WA_TranslucentBackground);

    // 注意！！！如果是主窗口不要设置WA_DeleteOnClose属性;
    // 关闭窗口时释放资源;
    setAttribute(Qt::WA_DeleteOnClose);
    // 初始化标题栏;
    initTitleBar();

    // this->setStyleSheet("background-color:#33CCFF;");
    //软件的图标

    this->setWindowIcon(QIcon(":logo.ico"));  //设置LOGO


    this->setAcceptDrops(true);  //接收拖拽事件  这个一定要加
    drag_filepath1 ="";


    for(int i=0;i<6;i++){
        probe_data_zero[i]=0;
    }


    init();
    configure=new Configure();


    modebus_rtu =new Modbus_rtu();

   // modebus_rtu->moveToThread(serialThread);


    QObject::connect(modebus_rtu,SIGNAL(data_received(uint8_t *,int)),this,SLOT(get_data_from_serial_slot(uint8_t *,int)),Qt::QueuedConnection);


    is_monitor_sample=false;
    is_temperature_humi_sample=false;
    is_auto_sample=false;
    is_zero_sample=false;

    is_start=false;
    is_monitor_start=false;

    is_presskey=false;
    createMeauBar();
    createStatBar();

    rjdd=new RjdData();

    //之前的可以不用管
    //安装LABEL的监听器
    //应该打开默认的程序，算本地

    //默认是上次采集的
    //播放打开软件声音
    user_Sound=new SoundMedia();
    //add sub thread

    qDebug()<<"main thread id = "<<QThread::currentThreadId();
    //升级之类的以后再加 //启动网络重连问题


    timer=new QTimer(this);
    QObject::connect(timer,SIGNAL(timeout()),this,SLOT(timerout_slot()));

    monitor_timer=new QTimer(this);
    QObject::connect(monitor_timer,SIGNAL(timeout()),this,SLOT(monitor_timerout_slot()));
    monitor_timer->stop();

    timer->stop(); //5S一次  //可以作为调试 3秒发一次即可  //

    dist_plot=new Dist_Plot(this);

    serial_timer=new QTimer(this);
    QObject::connect(serial_timer,SIGNAL(timeout()),this,SLOT(serial_timerout_slot()));

    serial_timer->start(1000); //5S一次  //可以作为调试 3秒发一次即可


    ui->le_standard->setEnabled(false);
    ui->le_item_name->setEnabled(false);
    ui->le_item_tester->setEnabled(false);
    ui->le_item_time->setEnabled(false);


    ui->le_test_len->setEnabled(false);
    ui->le_sample_interval->setEnabled(false);

    ui->le_total_hour->setEnabled(false);
    ui->le_rest_time->setEnabled(false);
    ui->le_sample_time->setEnabled(false);


    ui->le_ch1->setEnabled(false);
    ui->le_ch2->setEnabled(false);
    ui->le_ch3->setEnabled(false);
    ui->le_ch4->setEnabled(false);
    ui->le_ch5->setEnabled(false);
    ui->le_ch6->setEnabled(false);

    ui->le_ch1_auto->setEnabled(false);
    ui->le_ch2_auto->setEnabled(false);
    ui->le_ch3_auto->setEnabled(false);
    ui->le_ch4_auto->setEnabled(false);
    ui->le_ch5_auto->setEnabled(false);
    ui->le_ch6_auto->setEnabled(false);

    ui->le_temperature->setEnabled(false);
    ui->le_humi->setEnabled(false);
    ui->le_shrink->setEnabled(false);
}

//bool MainWindow::eventFilter(QObject *watched, QEvent  *event)
//{
//    if(watched == ui->label_step && event->type() == QEvent::Paint)
//    {
//        //updateStep();
//        printf("12112");
//    }
//    return MainWindow::eventFilter(watched,event);
//}



void MainWindow::resizeEvent(QResizeEvent *){

    screenWidth_cst =this->width()/10*10 ;  //获取当前窗口的高度和宽度
    screenHeight_cst = this->height()/10*10-10;

    g_toolbar_height=this->height()/8;
    int tabWidget_height=g_toolbar_height;
    int other_info_width=0;

    other_info_width=screenWidth_cst/5;

    int groupBox_item_width=(screenWidth_cst-other_info_width)/3;

    int half_height=screenHeight_cst/2;

    g_titlebar_height=qSqrt(screenHeight_cst);
    int groupBox_item_height=half_height-tabWidget_height-g_titlebar_height-30;

    tool_tabWidget->setGeometry(0,g_titlebar_height,screenWidth_cst,tabWidget_height);


   // ui->treeView_item->setStyleSheet("QTreeView::item { height: 50px}");


    dist_plot->setGeometry(0,half_height,screenWidth_cst,half_height);
    qDebug("width2=%d,height2=%d",screenWidth_cst,screenHeight_cst);

    ui->groupBox_item->setGeometry(0,tabWidget_height+g_titlebar_height,groupBox_item_width,groupBox_item_height);
    ui->groupBox_testarea->setGeometry(groupBox_item_width,tabWidget_height+g_titlebar_height,groupBox_item_width,groupBox_item_height);
    ui->groupBox_monitor->setGeometry(groupBox_item_width*2,tabWidget_height+g_titlebar_height,groupBox_item_width,groupBox_item_height);

    ui->groupBox_other_info->setGeometry(groupBox_item_width*3,tabWidget_height+g_titlebar_height,other_info_width,groupBox_item_height);

    int start_height=30;
    int start_width=10;
    groupBox_item_height-=start_height;
    int group_label_height=groupBox_item_height/6;
    int group_label_width=groupBox_item_width/5 *2-start_width;

    int group_le_height=groupBox_item_height/6;
    int group_le_width=groupBox_item_width-group_label_width-start_width;
    int group_le_height_cha=8;


    short temp_font_size=qSqrt(group_label_height*6);

    //里面的也要重新布局

    ui->label_item_name->setGeometry(start_width,start_height,group_label_width,group_label_height);
    ui->label_standard->setGeometry(start_width,start_height+group_label_height,group_label_width,group_label_height);
    ui->label_item_tester->setGeometry(start_width,start_height+group_label_height*2,group_label_width,group_label_height);
    ui->label_item_time->setGeometry(start_width,start_height+group_label_height*3,group_label_width,group_label_height);
    ui->label_backup->setGeometry(start_width,start_height+group_label_height*4,group_label_width,group_label_height);

    ui->label_serial->setGeometry(start_width,start_height+group_label_height*5,group_label_width,group_label_height);

    ui->le_item_name->setGeometry(group_label_width,start_height,group_le_width,group_le_height-group_le_height_cha);
    ui->le_standard->setGeometry(group_label_width,start_height+group_le_height,group_le_width,group_le_height-group_le_height_cha);
    ui->le_item_tester->setGeometry(group_label_width,start_height+group_le_height*2,group_le_width,group_le_height-group_le_height_cha);
    ui->le_item_time->setGeometry(group_label_width,start_height+group_le_height*3,group_le_width,group_le_height-group_le_height_cha);
    ui->label_backup_text->setGeometry(group_label_width,start_height+group_le_height*4,group_le_width,group_le_height-group_le_height_cha);
    ui->cb_port->setGeometry(group_label_width,start_height+group_le_height*5,group_le_width,group_le_height-group_le_height_cha);




    ui->label_test_len->setGeometry(start_width,start_height,group_label_width,group_label_height);
    ui->label_sample_interval->setGeometry(start_width,start_height+group_label_height,group_label_width,group_label_height);


    ui->label_total_hour->setGeometry(start_width,start_height+group_label_height*3,group_label_width,group_label_height);
    ui->label_sample_time->setGeometry(start_width,start_height+group_label_height*4,group_label_width,group_label_height);
    ui->label_rest_time->setGeometry(start_width,start_height+group_label_height*5,group_label_width,group_label_height);

    ui->le_test_len->setGeometry(group_label_width,start_height,group_le_width,group_le_height-group_le_height_cha);
    ui->le_sample_interval->setGeometry(group_label_width,start_height+group_le_height,group_le_width,group_le_height-group_le_height_cha);



    ui->le_total_hour->setGeometry(group_label_width,start_height+group_label_height*3,group_le_width,group_le_height-group_le_height_cha);
    ui->le_sample_time->setGeometry(group_label_width,start_height+group_le_height*4,group_le_width,group_le_height-group_le_height_cha);
    ui->le_rest_time->setGeometry(group_label_width,start_height+group_le_height*5,group_le_width,group_le_height-group_le_height_cha);


    int group_ch_height=groupBox_item_height/7;

    group_label_width=groupBox_item_width/5-start_width;

    group_le_width=(groupBox_item_width-group_label_width)/2;

    ui->label_test_type->setGeometry(group_label_width,start_height,group_label_width*3,group_ch_height);

    ui->label_ch1->setGeometry(start_width,start_height+group_ch_height,group_label_width,group_ch_height);
    ui->label_ch2->setGeometry(start_width,start_height+group_ch_height*2,group_label_width,group_ch_height);
    ui->label_ch3->setGeometry(start_width,start_height+group_ch_height*3,group_label_width,group_ch_height);
    ui->label_ch4->setGeometry(start_width,start_height+group_ch_height*4,group_label_width,group_ch_height);
    ui->label_ch5->setGeometry(start_width,start_height+group_ch_height*5,group_label_width,group_label_height);
    ui->label_ch6->setGeometry(start_width,start_height+group_ch_height*6,group_label_width,group_ch_height);

    ui->le_ch1->setGeometry(group_label_width,start_height+group_ch_height,group_le_width,group_ch_height-group_le_height_cha);
    ui->le_ch2->setGeometry(group_label_width,start_height+group_ch_height*2,group_le_width,group_ch_height-group_le_height_cha);
    ui->le_ch3->setGeometry(group_label_width,start_height+group_ch_height*3,group_le_width,group_ch_height-group_le_height_cha);
    ui->le_ch4->setGeometry(group_label_width,start_height+group_ch_height*4,group_le_width,group_ch_height-group_le_height_cha);
    ui->le_ch5->setGeometry(group_label_width,start_height+group_ch_height*5,group_le_width,group_ch_height-group_le_height_cha);
    ui->le_ch6->setGeometry(group_label_width,start_height+group_ch_height*6,group_le_width,group_ch_height-group_le_height_cha);


    ui->le_ch1_auto->setGeometry(group_label_width+group_le_width,start_height+group_ch_height,group_le_width,group_ch_height-group_le_height_cha);
    ui->le_ch2_auto->setGeometry(group_label_width+group_le_width,start_height+group_ch_height*2,group_le_width,group_ch_height-group_le_height_cha);
    ui->le_ch3_auto->setGeometry(group_label_width+group_le_width,start_height+group_ch_height*3,group_le_width,group_ch_height-group_le_height_cha);
    ui->le_ch4_auto->setGeometry(group_label_width+group_le_width,start_height+group_ch_height*4,group_le_width,group_ch_height-group_le_height_cha);
    ui->le_ch5_auto->setGeometry(group_label_width+group_le_width,start_height+group_ch_height*5,group_le_width,group_ch_height-group_le_height_cha);
    ui->le_ch6_auto->setGeometry(group_label_width+group_le_width,start_height+group_ch_height*6,group_le_width,group_ch_height-group_le_height_cha);




    group_ch_height=groupBox_item_height/6;
    group_label_width=other_info_width/5 *2-start_width;
    group_le_width=other_info_width-group_label_width-start_width;


    ui->label_temperature->setGeometry(start_width,start_height,group_label_width,group_ch_height);
    ui->label_humi->setGeometry(start_width,start_height+group_ch_height,group_label_width,group_ch_height);
    ui->label_shrink->setGeometry(start_width,start_height+group_ch_height*2,group_label_width,group_ch_height);

    ui->le_temperature->setGeometry(group_label_width,start_height,group_le_width,group_ch_height);
    ui->le_humi->setGeometry(group_label_width,start_height+group_ch_height,group_le_width,group_ch_height);
    ui->le_shrink->setGeometry(group_label_width,start_height+group_ch_height*2,group_le_width,group_ch_height);






    //1 地平平整度测量仪 字体

   //需要调整字体大小

    //
    QFont font,group_font;


    font.setPointSize(temp_font_size/2);

    tool_tabWidget->setFont(font);
    tool_tabWidget->tabBar()->setFont(font);

    font.setPointSize(temp_font_size*2/3);
    group_font.setPointSize(temp_font_size);
    //font.setPointSize(temp_font_size);



    //my_titlebar->setTitleContent(QStringLiteral("Floor flatness measuring instrument"),temp_font_size);
    my_titlebar->setTitleContent(title_content,temp_font_size*2/3);

    my_titlebar->setTitleWidth(this->width());
    my_titlebar->setTitleHeight(g_titlebar_height);


    ui->groupBox_item->setFont(group_font);
    ui->groupBox_testarea->setFont(group_font);
    ui->groupBox_monitor->setFont(group_font);
    ui->groupBox_other_info->setFont(group_font);

    ui->label_test_type->setFont(group_font);

    //里面的也要重新布局
    ui->label_standard->setFont(font);
    ui->label_item_name->setFont(font);
    ui->label_item_tester->setFont(font);
    ui->label_item_time->setFont(font);
    ui->label_backup->setFont(font);
    ui->label_serial->setFont(font);

    ui->label_test_len->setFont(font);
    ui->label_sample_interval->setFont(font);
    ui->label_total_hour->setFont(font);
    ui->label_sample_time->setFont(font);
    ui->label_rest_time->setFont(font);


    ui->le_standard->setFont(font);
    ui->le_item_name->setFont(font);
    ui->le_item_tester->setFont(font);
    ui->le_item_time->setFont(font);
    ui->label_backup_text->setFont(font);
    ui->cb_port->setFont(font);

    ui->label_ch1->setFont(font);
    ui->label_ch2->setFont(font);
    ui->label_ch3->setFont(font);
    ui->label_ch4->setFont(font);
    ui->label_ch5->setFont(font);
    ui->label_ch6->setFont(font);

    ui->label_temperature->setFont(font);
    ui->label_humi->setFont(font);
    ui->label_shrink->setFont(font);

    ui->le_test_len->setFont(font);
    ui->le_sample_interval->setFont(font);
    ui->le_total_hour->setFont(font);
    ui->le_sample_time->setFont(font);
    ui->le_rest_time->setFont(font);

    ui->le_ch1->setFont(font);
    ui->le_ch2->setFont(font);
    ui->le_ch3->setFont(font);
    ui->le_ch4->setFont(font);
    ui->le_ch5->setFont(font);
    ui->le_ch6->setFont(font);

    ui->le_ch1_auto->setFont(font);
    ui->le_ch2_auto->setFont(font);
    ui->le_ch3_auto->setFont(font);
    ui->le_ch4_auto->setFont(font);
    ui->le_ch5_auto->setFont(font);
    ui->le_ch6_auto->setFont(font);

    ui->le_temperature->setFont(font);
    ui->le_humi->setFont(font);
    ui->le_shrink->setFont(font);
}


double MainWindow::calc_shrink(T_Item_Data *item_data){
    int data_front[6];
    double shrink_rate=0;
    if(item_data->ch_data[0].size()<=0){
        return shrink_rate;
    }
    int vaild_ch_num=0;

    if(item_data->test_type==1){
        //先看有记住有效数据
        for(int i=0;i<3;i++){
            data_front[i]=item_data->ch_data[i].at(0);
        }

        int cur_time_index=item_data->ch_data[0].size()-1;
        int total_diff_value=0;
        for(int i=0;i<3;i++){
            if((data_front[i]!=ZERO_DEFAULT_VALUE)&&(item_data->ch_data[i].at(cur_time_index)!=ZERO_DEFAULT_VALUE)){
                vaild_ch_num++;
                total_diff_value+=(item_data->ch_data[i].at(cur_time_index)-data_front[i]);
            }
        }

        shrink_rate=(double)total_diff_value/item_data->test_len/vaild_ch_num/1000;
        return shrink_rate;
    }
    else if(item_data->test_type==0){

        for(int i=0;i<6;i++){
            data_front[i]=item_data->ch_data[i].at(0);
        }

        int cur_time_index=item_data->ch_data[0].size()-1;
        int total_diff_value=0;
        for(int i=0;i<6;i++){
            if((data_front[i]!=ZERO_DEFAULT_VALUE)&&(item_data->ch_data[i].at(cur_time_index)!=ZERO_DEFAULT_VALUE)){
                vaild_ch_num++;
                total_diff_value+=(item_data->ch_data[i].at(cur_time_index)-data_front[i]);
            }
        }

        shrink_rate=(double)total_diff_value/item_data->test_len/vaild_ch_num/1000;
        return shrink_rate;
    }
}
void MainWindow::sample_data_add_table(int16_t *data){
   for(int i=0;i<6;i++){
      gt_item_data.ch_data[i].push_back(data[i]);
   }

   gt_item_data.temperature.push_back(data[6]);
   gt_item_data.humi.push_back(data[7]);

   qDebug("test1");

   update_opendata_table(&gt_item_data);
   qDebug("test2");
   save_rjdd(gt_item_data,false,true);
   qDebug("test3");

    //然后更新曲线
    dist_plot->update_data();

    qDebug("test4");

}


void MainWindow::update_opendata_table(T_Item_Data *item_data){

    //可以从gradient 中或的索引
    int size=item_data->ch_data[0].size();
    if(size<=0){
        dist_plot->clear_curve();
        return;
    }



    int row_count;
    if(item_data->test_type==1){
        row_count=size*3;
    }
    else {
        row_count=size*6;
    }

    //先获得当前的数据



}

void MainWindow::clear_update_info(){
    QString empty_string="";
    ui->le_standard->setText(empty_string);
    ui->le_item_name->setText(empty_string);

    ui->le_item_tester->setText(empty_string);

    //获得当前时间
    ui->le_item_time->setText(empty_string);
    ui->le_standard->setText(empty_string);
    ui->le_item_tester->setText(empty_string);
    ui->le_item_time->setText(empty_string);
    ui->label_backup_text->setText(empty_string);

    ui->le_test_len->setText(empty_string);
    ui->le_sample_interval->setText(empty_string);

    ui->le_total_hour->setText(empty_string);
    ui->le_sample_time->setText(empty_string);
    ui->le_rest_time->setText(empty_string);


    double temp_float=0;
    ui->le_ch1->setText(QString::number(temp_float,'f',3));
    ui->le_ch2->setText(QString::number(temp_float,'f',3));
    ui->le_ch3->setText(QString::number(temp_float,'f',3));
    ui->le_ch4->setText(QString::number(temp_float,'f',3));
    ui->le_ch5->setText(QString::number(temp_float,'f',3));
    ui->le_ch6->setText(QString::number(temp_float,'f',3));

    ui->le_ch1_auto->setText(QString::number(temp_float,'f',3));
    ui->le_ch2_auto->setText(QString::number(temp_float,'f',3));
    ui->le_ch3_auto->setText(QString::number(temp_float,'f',3));
    ui->le_ch4_auto->setText(QString::number(temp_float,'f',3));
    ui->le_ch5_auto->setText(QString::number(temp_float,'f',3));
    ui->le_ch6_auto->setText(QString::number(temp_float,'f',3));


    ui->le_temperature->setText(empty_string);
    ui->le_humi->setText(empty_string);
    ui->le_shrink->setText(empty_string);


    dist_plot->clear_curve();

}


//表示默认默认打开的文件
void MainWindow::update_info(T_Item_Data *item_data, bool is_default){



    clear_update_info();



    ui->le_item_name->setText(item_data->item_name);
    QString temp_string;

    switch(item_data->test_standard){
        case 0:
            temp_string=tr("GB50082-2019:非接触法");
            break;
        case 1:
            temp_string=tr("GB50082-2019:接触法");
            break;
        case 2:
            temp_string=tr("JGJT70-2009");
            break;
        case 3:
            temp_string=tr("JCJT603-2004");
            break;
        case 4:
            temp_string=tr("JTGE30-2005");
            break;
        case 5:
            temp_string=tr("JC/T2551-2019");
            break;
        case 6:
            temp_string=tr("GBT2542-201");
            break;
        case 7:
            temp_string=tr("通用收缩膨胀试验");
            break;
        default:
            temp_string=tr("未知");
            break;
    }
    ui->le_standard->setText(temp_string);


    ui->label_backup_text->setText(item_data->backup);
    ui->le_item_tester->setText(item_data->tester);
    //获得当前时间
    ui->le_item_time->setText(item_data->item_time.toString("yyyy-MM-dd hh:mm:ss"));
    ui->le_total_hour->setText(QString::number(item_data->total_time));
    ui->le_sample_time->setText(QString::number(item_data->sample_time));
    ui->le_rest_time->setText(QString::number(item_data->total_time-item_data->sample_time));

    ui->le_test_len->setText(QString::number(item_data->test_len));
    ui->le_sample_interval->setText(QString::number(item_data->sample_interval));




    if(item_data->test_type==1){
        ui->label_test_type->setText(tr("双面测量"));

        ui->label_ch1->setText("组1");
        ui->label_ch2->setText("组2");
        ui->label_ch3->setText("组3");

        ui->label_ch4->hide();
        ui->label_ch5->hide();
        ui->label_ch6->hide();

        ui->le_ch4->hide();
        ui->le_ch5->hide();
        ui->le_ch6->hide();
    }
    else{
        ui->label_test_type->setText(tr("单面测量"));
        ui->label_ch1->setText("CH1");
        ui->label_ch2->setText("CH2");
        ui->label_ch3->setText("CH3");
        ui->label_ch4->setText("CH4");
        ui->label_ch5->setText("CH5");
        ui->label_ch6->setText("CH6");

        ui->label_ch4->show();
        ui->label_ch5->show();
        ui->label_ch6->show();

        ui->le_ch4->show();
        ui->le_ch5->show();
        ui->le_ch6->show();
    }
}




//标题栏的高度为60px
void MainWindow::initTitleBar(){

    my_titlebar = new MyTitleBar(this);
    my_titlebar->move(0, 0);  //标题栏所占用的位置

    connect(my_titlebar, SIGNAL(signalButtonMinClicked()), this, SLOT(onButtonMinClicked()));
    connect(my_titlebar, SIGNAL(signalButtonRestoreClicked()), this, SLOT(onButtonRestoreClicked()));
    connect(my_titlebar, SIGNAL(signalButtonMaxClicked()), this, SLOT(onButtonMaxClicked()));
    connect(my_titlebar, SIGNAL(signalButtonCloseClicked()), this, SLOT(onButtonCloseClicked()));


    title_content=tr("智能收缩膨胀试验监测系统(V1.2)");
    // 设置标题栏跑马灯效果，可以不设置;
    my_titlebar->setTitleRoll();
    my_titlebar->setBackgroundColor(0x33, 0xCC, 0xff , false);


    my_titlebar->setTitleIcon(":images/logo.png",QSize(g_titlebar_height,g_titlebar_height));
    my_titlebar->setTitleContent(title_content,16);


    my_titlebar->setButtonType(MIN_MAX_BUTTON);
    my_titlebar->setTitleWidth(this->width());
    my_titlebar->show();
}



//ff 和fl 的计算公式
void MainWindow::init()
{
    tPer_Frm_State.is_open_new_test_dlg=false;
    tPer_Frm_State.is_open_sys_setting=false;





        ui->groupBox_item->setStyleSheet(" QGroupBox {  \
                                         border: 1px solid gray;\
                                         margin-top:3ex;  \
                                        border-radius:10px;\
                                    } \
                                    QGroupBox::title {  \
                                         subcontrol-origin: margin;  \
                                         position: relative;  \
                                         left: 10px; \
                                    }");

        ui->groupBox_testarea->setStyleSheet(" QGroupBox {  \
                                         border: 1px solid gray;\
                                         margin-top:3ex;  \
                                        border-radius:10px;\
                                    } \
                                    QGroupBox::title {  \
                                         subcontrol-origin: margin;  \
                                         position: relative;  \
                                         left: 10px; \
                                    }");


            ui->groupBox_monitor->setStyleSheet(" QGroupBox {  \
                                             border: 1px solid gray;\
                                             margin-top:3ex;  \
                                            border-radius:10px;\
                                        } \
                                        QGroupBox::title {  \
                                             subcontrol-origin: margin;  \
                                             position: relative;  \
                                             left: 10px; \
                                        }");

            ui->groupBox_other_info->setStyleSheet(" QGroupBox {  \
                                             border: 1px solid gray;\
                                             margin-top:3ex;  \
                                            border-radius:10px;\
                                        } \
                                        QGroupBox::title {  \
                                             subcontrol-origin: margin;  \
                                             position: relative;  \
                                             left: 10px; \
                                        }");




//     ui->tw_opendata->setStyleSheet(" QGroupBox {  \
//                                    border: 1px solid gray;\
//                                    margin-top:3ex;  \
//                                   border-radius:10px;\
//                               } \
//                               QGroupBox::title {  \
//                                    subcontrol-origin: margin;  \
//                                    position: relative;  \
//                                    left: 10px; \
//                               }");
    QFont font16;
    font16.setPointSize(14);
}

void MainWindow::showWifiStat(){

}

void MainWindow::createMeauBar()
{
    tool_tabWidget=new QTabWidget(this);



    tool_tabWidget->setStyleSheet("QTabWidget::pane{ \
                                  border-width:1px ; \
                                  border-style: outset;\
                              }\
                              QTabWidget::tab-bar{ \
                                      alignment:left; \
                              } \
                              QTabBar::tab{ \
                                  background:rgb(33, cc, ff,1);\
                                  color:rgb(33, cc, ff,1);\
                                  min-width:30ex;\
                                  min-height:9ex;\
                              }\
                              QTabBar::tab:hover{\
                                  background:#ccffcc;\
                              }\
                              QTabBar::tab:selected{\
                                  border-color:#33ccff;\
                                  background:#33ccff;\
                                  color:black;\
                              }\
                             ");


    tool_tabWidget->setGeometry(0,g_titlebar_height,this->width(),g_toolbar_height);
    QFont font;
    font.setPixelSize(20);
    tool_tabWidget->tabBar()->setFont(font);

    //menuBar = new QMenuBar(this);
   // menuBar->setGeometry(QRect(0,0,700,50));//24
   // menu_collect_site=menuBar->addMenu(tr("现场采集"));
   // menu_data_analy=menuBar->addMenu(tr("数据采集"));


   toolbar_test_item = addToolBar(tr("测试项目"));
   toolbar_control = addToolBar(tr("采集控制"));

   toolbar_test_data = addToolBar(tr("测试数据"));
   toolbar_operate = addToolBar(tr("操作"));
   toolbar_function = addToolBar(tr("功能"));

   toolbar_setting=addToolBar(tr("设置"));

   toolbar_test_item->setMovable(false);  //不可移动
   toolbar_test_item->show();

   act_new_test = createAction(tr("新建试验"),"act_new_test",":/images/new_test.png");
   toolbar_test_item->addAction(act_new_test);
   toolbar_test_item->widgetForAction(act_new_test)->setObjectName(act_new_test->objectName());

   act_continue_test = createAction(tr("继续测试"),"act_continue_test",":/images/continue_test.png");
   toolbar_test_item->addAction(act_continue_test);
   toolbar_test_item->widgetForAction(act_continue_test)->setObjectName(act_continue_test->objectName());


   act_saveas = createAction(tr("另存为..."),"act_saveas",":/images/saveas.png");
   toolbar_test_item->addAction(act_saveas);
   toolbar_test_item->widgetForAction(act_saveas)->setObjectName(act_saveas->objectName());


   act_start = createAction(tr("开始"),"act_start",":/images/start.png");
   toolbar_control->addAction(act_start);
   toolbar_control->widgetForAction(act_start)->setObjectName(act_start->objectName());

   act_stop = createAction(tr("停止"),"act_start",":/images/stop.png");
   toolbar_control->addAction(act_stop);
   toolbar_control->widgetForAction(act_stop)->setObjectName(act_stop->objectName());


   act_manual_sample = createAction(tr("监控采集"),"act_manual_sample",":/images/monitor_sample_start.png");
   toolbar_control->addAction(act_manual_sample);
   toolbar_control->widgetForAction(act_manual_sample)->setObjectName(act_manual_sample->objectName());

   act_clear_all = createAction(tr("归零"),"act_clear_all",":/images/stop.png");
   toolbar_control->addAction(act_clear_all);
   toolbar_control->widgetForAction(act_clear_all)->setObjectName(act_clear_all->objectName());




    act_open_data = createAction(tr("打开数据"),"act_open_data",":/images/open_data.png");
    toolbar_test_data->addAction(act_open_data);
    toolbar_test_data->widgetForAction(act_open_data)->setObjectName(act_open_data->objectName());




    act_close_data = createAction(tr("关闭数据"),"act_close_data",":/images/close_data.png");
    toolbar_test_data->addAction(act_close_data);
    toolbar_test_data->widgetForAction(act_close_data)->setObjectName(act_close_data->objectName());



    act_export_data = createAction(tr("数据导出"),"act_export_data",":/images/export_data.png");
    toolbar_function->addAction(act_export_data);
    toolbar_function->widgetForAction(act_export_data)->setObjectName(act_export_data->objectName());


    act_system_setting = createAction(tr("系统设置"),"act_system_setting",":/images/setting.png");
    toolbar_setting->addAction(act_system_setting);
    toolbar_setting->widgetForAction(act_system_setting)->setObjectName(act_system_setting->objectName());




//    ui->mainToolBar->setStyleSheet
//        (
//          "QAction#act_open_data { background:red }"
//          "QAction#act_empty_list { background:blue }"
//          "QAction#act_export_data { background:green }"
//);

    //图标和文字一起显示


    toolbar_test_item->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    toolbar_control->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    toolbar_test_data->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    toolbar_operate->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    toolbar_function->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    toolbar_setting->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);



    widget_collect_site=new QMainWindow(this);
    widget_data_analy=new QMainWindow(this);
    widget_system_setting=new QMainWindow(this);

    widget_collect_site->addToolBar(Qt::TopToolBarArea, toolbar_test_item);
    widget_collect_site->addToolBar(Qt::TopToolBarArea, toolbar_control);

    widget_data_analy->addToolBar(Qt::TopToolBarArea, toolbar_test_data);
    widget_data_analy->addToolBar(Qt::TopToolBarArea, toolbar_operate);
    widget_data_analy->addToolBar(Qt::TopToolBarArea, toolbar_function);

    widget_system_setting->addToolBar(Qt::TopToolBarArea, toolbar_setting);
    tool_tabWidget->addTab(widget_collect_site,tr("现场采集"));
    tool_tabWidget->addTab(widget_data_analy,tr("数据分析"));
    tool_tabWidget->addTab(widget_system_setting,tr("设置"));
    tool_tabWidget->setFont(font);


//    addToolBar(Qt::TopToolBarArea, toolbar_test_item);
//    addToolBar(Qt::TopToolBarArea, toolbar_control);
//    addToolBar(Qt::TopToolBarArea, toolbar_test_data);
//    addToolBar(Qt::TopToolBarArea, toolbar_operate);
//    addToolBar(Qt::TopToolBarArea, toolbar_function);



 //制作浮窗
    /*
    QDockWidget *dock = new QDockWidget(tr("案例三十六拉伸"), this);
    dock->setFeatures(QDockWidget::NoDockWidgetFeatures);
      //dock->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
     // dock->setAllowedAreas(Qt::NoDockWidgetArea);
     dock->setWidget(toolbar_test_item);
     addDockWidget(Qt::RightDockWidgetArea, dock);
      //addDockWidget(Qt::NoDockWidgetArea, dock);
      */


    act_start->setDisabled(true);



    QObject::connect(act_new_test,SIGNAL(triggered()),this,SLOT(on_click_new_test()));
    QObject::connect(act_system_setting,SIGNAL(triggered()),this,SLOT(on_click_sys_settting()));
    QObject::connect(act_continue_test,SIGNAL(triggered()),this,SLOT(on_click_continue_test()));//数据帅选的信号槽

    QObject::connect(act_start,SIGNAL(triggered()),this,SLOT(on_click_start()));//数据帅选的信号槽
    QObject::connect(act_stop,SIGNAL(triggered()),this,SLOT(on_click_stop()));

    QObject::connect(act_manual_sample,SIGNAL(triggered()),this,SLOT(on_click_manual_sample()));//手动采集
    QObject::connect(act_clear_all,SIGNAL(triggered()),this,SLOT(on_click_clear_all()));


    QObject::connect(act_open_data,SIGNAL(triggered()),this,SLOT(on_click_open_data()));
    QObject::connect(act_saveas,SIGNAL(triggered()),this,SLOT(on_click_saveas()));
    QObject::connect(act_close_data,SIGNAL(triggered()),this,SLOT(on_click_close_data()));

    QObject::connect(act_export_data,SIGNAL(triggered()),this,SLOT(on_click_export_data()));


    QObject::connect(tool_tabWidget,SIGNAL(currentChanged(int)),this,SLOT(on_click_toolbar_selected(int)));
   // QObject::connect(toolbar_control,SIGNAL(actionTriggered(QAction*)),this,SLOT(on_click_export_data(QAction*)));




}
void MainWindow::deleteMeauBar(){

    delete act_new_test;   //新建测试
    delete act_continue_test;   //继续测试

    delete act_start;   //开始
    delete act_stop;    //停止

    delete act_open_data;  //打开 文件1
    delete act_saveas;   // 另存为
    delete act_close_data;   // 关闭数据

    delete act_export_data;   // 数据导出

    delete tool_tabWidget;

}

void MainWindow::createStatBar(){
    statBar = new QStatusBar(this);
    setStatusBar(statBar);

    // 创建label控件
    wifiStatLabel = new QLabel(this);
    QFont font;
    font.setPointSize(12);
    wifiStatLabel->setFont(font);
    // 添加label控件到状态栏（从左到右添加）
    statBar->addWidget(wifiStatLabel);
}

void MainWindow::deleteStatBar()
{
    delete wifiStatLabel;
    delete statBar;
}


void MainWindow::opened_data_fresh(){

    //最后末尾的表示最新的
    //需要更新图标和表格





    //下面有个表格，存放所有显示的数据
    //需要得到当前选中行
}


void MainWindow::on_click_new_test(){

    if(!tPer_Frm_State.is_open_new_test_dlg){
        tPer_Frm_State.is_open_new_test_dlg=true;
        dlg_newTest=new Dlg_NewTest(this);
        dlg_newTest->set_style(this->style());
        //这个是操作的关闭信号
        QObject::connect(dlg_newTest,SIGNAL(closeSignal()),this,SLOT(close_new_test()));

        dlg_newTest->show();
    }
}

void MainWindow::on_click_sys_settting(){

    if(!tPer_Frm_State.is_open_sys_setting){
        tPer_Frm_State.is_open_sys_setting=true;

        frm_sys_setting=new Dlg_Sys_Setting(this);

        //这个是操作的关闭信号
        QObject::connect(frm_sys_setting,SIGNAL(closeSignal()),this,SLOT(close_sys_settting()));
        QObject::connect(frm_sys_setting,SIGNAL(temp_humi_signal()),this,SLOT(sample_temp_humi_correct_slot()));

        frm_sys_setting->show();

        is_temperature_humi_sample=false;

        //需要在这里打开端口一下
        int err_num;
        if(err_num=modebus_rtu->dev_open(ui->cb_port->currentText())){

            if(err_num==1){
                QMessageBox::information(NULL, "错误", "未识别串口的地址！");
            }
            else if(err_num==2){
                QMessageBox::information(NULL, "错误", "串口打开失败，请更换端口！");
            }
            return ;
        }
    }
}





//
void MainWindow::save_rjdd(T_Item_Data &item_data,bool is_header, bool is_default){//文本框不能输入逗号

        if(is_header){
            rjdd->ouput_header(item_data,is_default);
        }
        //输出头部内容
        rjdd->write_data_rjdd(item_data,is_default);

}

void MainWindow::close_new_test(){

    tPer_Frm_State.is_open_new_test_dlg=false;
    if(dlg_newTest->is_save){

        is_start=false;
        if(is_start){
            act_start->setDisabled(true);
            act_stop->setDisabled(false);
        }
        else{
            act_start->setDisabled(false);
            act_stop->setDisabled(true);
        }

        act_clear_all->setDisabled(false);

        qDebug("10");
        if(gt_item_data.is_exsit){
           rjdd->close_rjdd(true,false);
        }
        qDebug("11");

        gt_item_data.is_exsit=true;
        //这个获取最新的
        update_info(&gt_item_data,true);
        //然后需要保存一帧的数据
        qDebug("12");

        rjdd->new_default_file(gt_item_data);
        qDebug("13");
        save_rjdd(gt_item_data,true,true);

        qDebug("14");
        int err_num;
        if(err_num=modebus_rtu->dev_open(ui->cb_port->currentText())){
            if(err_num==1){
                QMessageBox::information(NULL, "错误", "未识别串口的地址！");
            }
            else if(err_num==2){
                QMessageBox::information(NULL, "错误", "串口打开失败，请更换端口！");
            }
        }
    }

    delete dlg_newTest;
    dlg_newTest=NULL;
}

void MainWindow::close_sys_settting(){
    tPer_Frm_State.is_open_sys_setting=false;
    delete frm_sys_setting;
    frm_sys_setting=NULL;
    configure->write_conf();
}

void MainWindow::on_click_continue_test()
{
    QString temp_file_name;
    QSettings path_setting("./conf/path_setting.ini", QSettings::IniFormat);

    QString lastPath = path_setting.value("LastFilePath").toString();

     temp_file_name = QFileDialog::getOpenFileName(this,
        tr("Open File"),
        lastPath,
        tr("数据文件(*.rjdd)"),
        0);

     if (!temp_file_name.isNull()){
        path_setting.setValue("LastFilePath",temp_file_name);

        //这里到时候考虑，有肯恶搞

        clear_update_info();
        is_start=false;
        act_start->setDisabled(false);
        act_stop->setDisabled(true);

        timer->stop();

        if(gt_item_data.is_exsit){
           rjdd->close_rjdd(true,false);
        }




       //直接先读取
        T_Item_Data item_data;
        if(rjdd->open_rjdd(temp_file_name,true,false)>0){
            return;
        }
        rjdd->read_rjdd(&item_data,temp_file_name,true);
        if(item_data.is_exsit){
            gt_item_data=item_data;
            update_info(&gt_item_data,true);
            update_opendata_table(&gt_item_data);
            dist_plot->update_data_default_data();
        }
     }
}

void MainWindow::on_click_serial_connect(){
    if(!tPer_Frm_State.is_open_serial_connect){
        tPer_Frm_State.is_open_serial_connect=true;
    }
}

void MainWindow::close_serial_connect(){
    tPer_Frm_State.is_open_new_test_dlg=false;
    //需要判断有没有按确定键
    if(dlg_newTest->is_save){
        is_start=false;
        if(is_start){
            act_start->setDisabled(true);
            act_stop->setDisabled(false);
        }
        else{
            act_start->setDisabled(false);
            act_stop->setDisabled(true);
        }

        //这个获取最新的



        update_info(&gt_item_data,true);
        save_rjdd(gt_item_data,false,true);

    }
    delete dlg_newTest;

}

T_NET_Info s_timenet_info;
uint8_t s_timeData[10];
void MainWindow::on_click_start(){
    //先打开串口


//    test_timer->start(5000);//先没5S种去一次数据

//    return;
    //日志
    int err_num;
    if(err_num=modebus_rtu->dev_open(ui->cb_port->currentText())){

        if(err_num==1){
            QMessageBox::information(NULL, "错误", "未识别串口的地址！");
        }
        else if(err_num==2){
            QMessageBox::information(NULL, "错误", "串口打开失败，请更换端口！");
        }
        return ;
    }
    is_start=true;

    act_start->setDisabled(true);
    act_stop->setDisabled(false);
    act_clear_all->setDisabled(true);
    act_new_test->setDisabled(true);

    tool_tabWidget->setTabEnabled(1,false);
    tool_tabWidget->setTabEnabled(2,false);
    timer->start(gt_item_data.sample_interval*60000);//先没5S种去一次数据  60000


    if(!is_monitor_start){
       modebus_rtu->read_reg((reinterpret_cast< int >(&((T_MODBUS_REG *)0)->temperature_reg))>>1,8);
    }
    is_auto_sample=true;
    //一开始应该有一帧
}


void MainWindow::on_click_stop(){

    if(is_start){
        QMessageBox::StandardButton box;
        box = QMessageBox::question(this, "提示", "确实要停止采样吗?", QMessageBox::Yes|QMessageBox::No);
        if(box==QMessageBox::No)
           return;


        is_start=false;
        is_auto_sample=false;
        timer->stop();
        modebus_rtu->serial_close();
        act_start->setDisabled(false);
        act_clear_all->setDisabled(false);
        act_new_test->setDisabled(false);
    }
    tool_tabWidget->setTabEnabled(1,true);
    tool_tabWidget->setTabEnabled(2,true);
}



void MainWindow::get_data_from_serial_slot(uint8_t *data,int len){

    float temp_float;
    int16_t probe_data[16];
    int16_t temperature;
    int16_t humi;

    uint8_t recv_data[20];

    int16_t temp_probe_data[6];

    if(len>16){
        //接收到的收据发生失败
        return ;
    }
    for(int i=0;i<16;i++){
        recv_data[i]=data[i];
    }

    if(is_zero_sample){
        is_zero_sample=false;
        probe_data[0]=(int)recv_data[4]*256+recv_data[5];
        probe_data[1]=(int)recv_data[6]*256+recv_data[7];
        probe_data[2]=(int)recv_data[8]*256+recv_data[9];
        probe_data[3]=(int)recv_data[10]*256+recv_data[11];
        probe_data[4]=(int)recv_data[12]*256+recv_data[13];
        probe_data[5]=(int)recv_data[14]*256+recv_data[15];


        QString  temp_string;
        temp_string=tr("已归零的通道有:");

        for(int i=0;i<6;i++){
            if(probe_data[i]!=ZERO_DEFAULT_VALUE){
                temp_string.append("CH:").append(QString::number(i+1)).append(",");
            }
        }

        if(gt_item_data.test_type==1){

            if((probe_data[gt_item_data.probe_set[0]]!=ZERO_DEFAULT_VALUE)&&(probe_data[gt_item_data.probe_set[1]])!=ZERO_DEFAULT_VALUE){
                //说明有效
                 probe_data_zero[0]=probe_data[gt_item_data.probe_set[0]]+probe_data[gt_item_data.probe_set[1]];
            }

            if((probe_data[gt_item_data.probe_set[2]]!=ZERO_DEFAULT_VALUE)&&(probe_data[gt_item_data.probe_set[3]])!=ZERO_DEFAULT_VALUE){
                //说明有效
                 probe_data_zero[1]=probe_data[gt_item_data.probe_set[2]]+probe_data[gt_item_data.probe_set[3]];
            }

            if((probe_data[gt_item_data.probe_set[4]]!=ZERO_DEFAULT_VALUE)&&(probe_data[gt_item_data.probe_set[5]])!=ZERO_DEFAULT_VALUE){
                //说明有效
                 probe_data_zero[2]=probe_data[gt_item_data.probe_set[4]]+probe_data[gt_item_data.probe_set[5]];
            }

            probe_data_zero[3]=ZERO_DEFAULT_VALUE;
            probe_data_zero[4]=ZERO_DEFAULT_VALUE;
            probe_data_zero[5]=ZERO_DEFAULT_VALUE;

        }
        else{

            for(int i=0;i<6;i++){
                probe_data_zero[i]=probe_data[i];
                qDebug("%d....=%d",i,probe_data_zero[i]);
            }
            QMessageBox::information(NULL, "提示", temp_string);
        }

        double temp_float=0;
        ui->le_ch1->setText(QString::number(temp_float,'f',3));
        ui->le_ch2->setText(QString::number(temp_float,'f',3));
        ui->le_ch3->setText(QString::number(temp_float,'f',3));
        ui->le_ch4->setText(QString::number(temp_float,'f',3));
        ui->le_ch5->setText(QString::number(temp_float,'f',3));
        ui->le_ch6->setText(QString::number(temp_float,'f',3));

        //然后自动的也要为零
        ui->le_ch1_auto->setText(QString::number(temp_float,'f',3));
        ui->le_ch2_auto->setText(QString::number(temp_float,'f',3));
        ui->le_ch3_auto->setText(QString::number(temp_float,'f',3));
        ui->le_ch4_auto->setText(QString::number(temp_float,'f',3));
        ui->le_ch5_auto->setText(QString::number(temp_float,'f',3));
        ui->le_ch6_auto->setText(QString::number(temp_float,'f',3));
        return;
    }

    if(is_temperature_humi_sample){
        is_temperature_humi_sample=false;
        temperature=(int)recv_data[0]*256+recv_data[1];
        humi=(int)recv_data[2]*256+recv_data[3];

        if(tPer_Frm_State.is_open_sys_setting){
            frm_sys_setting->show_temp_humi(temperature,humi);
        }
        return;
     }




    if(is_monitor_sample){// 如果是监控采集，只更新自动窗口

        temperature=(int)recv_data[0]*256+recv_data[1];
        humi=(int)recv_data[2]*256+recv_data[3];

        temperature=Configure::calc_temperature(temperature);
        humi=Configure::calc_humi(humi);


        probe_data[0]=(int)recv_data[4]*256+recv_data[5];
        probe_data[1]=(int)recv_data[6]*256+recv_data[7];
        probe_data[2]=(int)recv_data[8]*256+recv_data[9];
        probe_data[3]=(int)recv_data[10]*256+recv_data[11];
        probe_data[4]=(int)recv_data[12]*256+recv_data[13];
        probe_data[5]=(int)recv_data[14]*256+recv_data[15];


        if(gt_item_data.test_type==1){
            if((probe_data[gt_item_data.probe_set[0]]!=ZERO_DEFAULT_VALUE)&&(probe_data[gt_item_data.probe_set[1]]!=ZERO_DEFAULT_VALUE)){
                temp_float=(float)(probe_data[gt_item_data.probe_set[0]]+probe_data[gt_item_data.probe_set[1]]-probe_data_zero[0])/1000;


                ui->le_ch1_auto->setText(QString::number(temp_float,'f',3));
            }
            else{
                ui->le_ch1_auto->setText(tr("未连接"));
            }


            if((probe_data[gt_item_data.probe_set[2]]!=ZERO_DEFAULT_VALUE)&&(probe_data[gt_item_data.probe_set[3]]!=ZERO_DEFAULT_VALUE)){
                temp_float=(float)(probe_data[gt_item_data.probe_set[2]]+probe_data[gt_item_data.probe_set[3]]-probe_data_zero[1])/1000;
                ui->le_ch2_auto->setText(QString::number(temp_float,'f',3));
            }
            else{
                ui->le_ch2_auto->setText(tr("未连接"));
            }

            if((probe_data[gt_item_data.probe_set[4]]!=ZERO_DEFAULT_VALUE)&&(probe_data[gt_item_data.probe_set[5]]!=ZERO_DEFAULT_VALUE)){
                temp_float=(float)(probe_data[gt_item_data.probe_set[4]]+probe_data[gt_item_data.probe_set[5]]-probe_data_zero[2])/1000;
                ui->le_ch3_auto->setText(QString::number(temp_float,'f',3));
            }
            else{
                ui->le_ch3_auto->setText(tr("未连接"));
            }
        }
        else{
             if(probe_data[0]!=ZERO_DEFAULT_VALUE){
                 temp_float=(float)(probe_data[0]-probe_data_zero[0])/1000;
                 ui->le_ch1_auto->setText(QString::number(temp_float,'f',3));
             }
             else{
                 ui->le_ch1_auto->setText(tr("未连接"));
             }

             if(probe_data[1]!=ZERO_DEFAULT_VALUE){
                 temp_float=(float)(probe_data[1]-probe_data_zero[1])/1000;
                 ui->le_ch2_auto->setText(QString::number(temp_float,'f',3));
             }
             else{
                 ui->le_ch2_auto->setText(tr("未连接"));
             }


             if(probe_data[2]!=ZERO_DEFAULT_VALUE){
                 temp_float=(float)(probe_data[2]-probe_data_zero[2])/1000;
                 ui->le_ch3_auto->setText(QString::number(temp_float,'f',3));
             }
             else{
                 ui->le_ch3_auto->setText(tr("未连接"));
             }

             if(probe_data[3]!=ZERO_DEFAULT_VALUE){
                 temp_float=(float)(probe_data[3]-probe_data_zero[3])/1000;
                 ui->le_ch4_auto->setText(QString::number(temp_float,'f',3));
             }
             else{
                 ui->le_ch4_auto->setText(tr("未连接"));
             }

             if(probe_data[4]!=ZERO_DEFAULT_VALUE){
                 temp_float=(float)(probe_data[4]-probe_data_zero[4])/1000;
                 ui->le_ch5_auto->setText(QString::number(temp_float,'f',3));
             }
             else{
                 ui->le_ch5_auto->setText(tr("未连接"));
             }

             if(probe_data[5]!=ZERO_DEFAULT_VALUE){
                 temp_float=(float)(probe_data[5]-probe_data_zero[5])/1000;
                 ui->le_ch6_auto->setText(QString::number(temp_float,'f',3));
             }
             else{
                 ui->le_ch6_auto->setText(tr("未连接"));
             }
        }
        ui->le_temperature->setText(QString::number(((float)temperature/100),'f',1).append("℃"));
        ui->le_humi->setText(QString::number(((float)humi/100),'f',1).append("%"));
    }
    if(is_auto_sample){

        for(int i=0;i<6;i++){
            temp_probe_data[i]=ZERO_DEFAULT_VALUE;
            probe_data[i]=0;
        }
        gt_item_data.sample_time+=gt_item_data.sample_interval;

        if(gt_item_data.sample_time>=gt_item_data.total_time){
            gt_item_data.sample_time=gt_item_data.total_time;
            timer->stop();
            is_start=false;
            act_start->setDisabled(false);
            act_stop->setDisabled(true);
            act_new_test->setDisabled(false);

            act_clear_all->setDisabled(false);
            tool_tabWidget->setTabEnabled(1,true);
            tool_tabWidget->setTabEnabled(2,true);

            //serial->serial_close();
            QMessageBox::information(this, tr("提示"), tr("试验已结束！"));
        }

            ui->le_sample_time->setText(QString::number(gt_item_data.sample_time));
            ui->le_rest_time->setText(QString::number(gt_item_data.total_time-gt_item_data.sample_time));


            temperature=(int)recv_data[0]*256+recv_data[1];
            humi=(int)recv_data[2]*256+recv_data[3];

            temperature=Configure::calc_temperature(temperature);
            humi=Configure::calc_humi(humi);

            probe_data[0]=(int)recv_data[4]*256+recv_data[5];
            probe_data[1]=(int)recv_data[6]*256+recv_data[7];
            probe_data[2]=(int)recv_data[8]*256+recv_data[9];
            probe_data[3]=(int)recv_data[10]*256+recv_data[11];
            probe_data[4]=(int)recv_data[12]*256+recv_data[13];
            probe_data[5]=(int)recv_data[14]*256+recv_data[15];

            for(int i=0;i<6;i++){
                qDebug("%d------%d",i,probe_data[i]);
            }

            if(gt_item_data.test_type==1){
                if((probe_data[gt_item_data.probe_set[0]]!=ZERO_DEFAULT_VALUE)&&(probe_data[gt_item_data.probe_set[1]])!=ZERO_DEFAULT_VALUE){
                    //说明有效
                     temp_probe_data[0]=probe_data[gt_item_data.probe_set[0]]+probe_data[gt_item_data.probe_set[1]];
                     temp_probe_data[0]=temp_probe_data[0]-probe_data_zero[0];
                }
                else{
                    temp_probe_data[0]=ZERO_DEFAULT_VALUE;
                }


                if((probe_data[gt_item_data.probe_set[2]]!=ZERO_DEFAULT_VALUE)&&(probe_data[gt_item_data.probe_set[3]])!=ZERO_DEFAULT_VALUE){
                    //说明有效
                     temp_probe_data[1]=probe_data[gt_item_data.probe_set[2]]+probe_data[gt_item_data.probe_set[3]];
                     temp_probe_data[1]=temp_probe_data[1]-probe_data_zero[1];

                }
                else{
                    temp_probe_data[1]=ZERO_DEFAULT_VALUE;
                }

                if((probe_data[gt_item_data.probe_set[4]]!=ZERO_DEFAULT_VALUE)&&(probe_data[gt_item_data.probe_set[5]])!=ZERO_DEFAULT_VALUE){
                    //说明有效
                     temp_probe_data[2]=probe_data[gt_item_data.probe_set[4]]+probe_data[gt_item_data.probe_set[5]];
                     temp_probe_data[2]=temp_probe_data[2]-probe_data_zero[2];
                }
                else{
                    temp_probe_data[2]=ZERO_DEFAULT_VALUE;
                }
            }
            else{
                for(int i=0;i<6;i++){
                    if(probe_data[i]!=ZERO_DEFAULT_VALUE){
                       if(probe_data_zero[i]!=ZERO_DEFAULT_VALUE){
                           temp_probe_data[i]=probe_data[i]-probe_data_zero[i];
                       }
                    }
                    qDebug("%d,=%d,=%d",i,probe_data_zero[i],temp_probe_data[i]);
                }
            }

            for(int i=0;i<6;i++){
                probe_data[i]=temp_probe_data[i];
            }

            if(probe_data[0]!=ZERO_DEFAULT_VALUE){
                temp_float=(float)probe_data[0]/1000;
                ui->le_ch1->setText(QString::number(temp_float,'f',3));
            }
            else{
                ui->le_ch1->setText("未连接");
            }


            if(probe_data[1]!=ZERO_DEFAULT_VALUE){
                temp_float=(float)probe_data[1]/1000;
                ui->le_ch2->setText(QString::number(temp_float,'f',3));
            }
            else{
                ui->le_ch2->setText("未连接");
            }


            if(probe_data[2]!=ZERO_DEFAULT_VALUE){
                temp_float=(float)probe_data[2]/1000;
                ui->le_ch3->setText(QString::number(temp_float,'f',3));
            }
            else{
                ui->le_ch3->setText("未连接");
            }

            if(probe_data[3]!=ZERO_DEFAULT_VALUE){
                temp_float=(float)probe_data[3]/1000;
                ui->le_ch4->setText(QString::number(temp_float,'f',3));
            }
            else{
                ui->le_ch4->setText("未连接");
            }


            if(probe_data[4]!=ZERO_DEFAULT_VALUE){
                temp_float=(float)probe_data[4]/1000;
                ui->le_ch5->setText(QString::number(temp_float,'f',3));
            }
            else{
                ui->le_ch5->setText("未连接");
            }

            if(probe_data[5]!=ZERO_DEFAULT_VALUE){
                temp_float=(float)probe_data[5]/1000;
                ui->le_ch6->setText(QString::number(temp_float,'f',3));
            }
            else{
                ui->le_ch6->setText("未连接");
            }

            ui->le_temperature->setText(QString::number(((float)temperature/100),'f',1).append("℃"));
            ui->le_humi->setText(QString::number(((float)humi/100),'f',1).append("%"));
            //采集到了数据，需要将数据，放到固定容器中
            probe_data[6]=temperature;
            probe_data[7]=humi;
            sample_data_add_table(probe_data);

           gt_item_data.shrink_rate=calc_shrink(&gt_item_data);
           ui->le_shrink->setText(QString::number(gt_item_data.shrink_rate,'f',9));
    }

    is_monitor_sample=false;
    is_auto_sample=false;
}



void MainWindow::sample_temp_humi_correct_slot(){
    modebus_rtu->read_reg((reinterpret_cast< int >(&((T_MODBUS_REG *)0)->temperature_reg))>>1,8);
    is_temperature_humi_sample=true;

}
void MainWindow::on_click_manual_sample(){

    if(is_monitor_start){
        QMessageBox::StandardButton box;
        box = QMessageBox::question(this, "提示", "确实要停止监控采样吗?", QMessageBox::Yes|QMessageBox::No);
        if(box==QMessageBox::No)
           return;

        is_monitor_start=false;
        is_monitor_sample=false;
        monitor_timer->stop();
        if(!is_start){
            modebus_rtu->serial_close();
            tool_tabWidget->setTabEnabled(1,true);
            tool_tabWidget->setTabEnabled(2,true);
        }
        QIcon icon(":/images/monitor_sample_start.png");
        act_manual_sample->setIcon(icon);
    }
    else{
        is_monitor_sample=false;
        monitor_timer->stop();

        //需要在这里打开端口一下
        int err_num;
        if(err_num=modebus_rtu->dev_open(ui->cb_port->currentText())){

            if(err_num==1){
                QMessageBox::information(NULL, "错误", "未识别串口的地址！");
            }
            else if(err_num==2){
                QMessageBox::information(NULL, "错误", "串口打开失败，请更换端口！");
            }
            return ;
        }
        //我觉的可以让监控采集一直运行
        monitor_timer->start(2000);  //监控采集，2秒停止一下
        is_monitor_start=true;

        QIcon icon(":/images/monitor_sample_end.png");
        act_manual_sample->setIcon(icon);


        tool_tabWidget->setTabEnabled(1,false);
        tool_tabWidget->setTabEnabled(2,false);
    }

}

void MainWindow::logPrintf(QString s){
    QDateTime cur_time = QDateTime::currentDateTime();
    QString  string;
    string.append("[" + cur_time.toString ("hh:mm:ss") + "] " + s);
    qDebug()<<string;
}


void MainWindow::on_click_open_data()
{
    //先选择一个目录
    QString temp_file_name;
    QSettings path_setting("./conf/path_setting.ini", QSettings::IniFormat);

    QString lastPath = path_setting.value("LastFilePath").toString();

     temp_file_name = QFileDialog::getOpenFileName(this,
        tr("Open File"),
        lastPath,
        tr("数据文件(*.rjdd)"),
        0);

     if (!temp_file_name.isNull()){
        path_setting.setValue("LastFilePath",temp_file_name);

        T_Item_Data  temp_item_data;
        qDebug("has running!");

        rjdd->open_rjdd(temp_file_name,false,false);
        rjdd->read_rjdd(&temp_item_data,temp_file_name,false);

        temp_item_data.sample_time=temp_item_data.sample_interval*temp_item_data.ch_data[0].size();

        //将读出来的数据显示出来
        if(temp_item_data.is_exsit){
            open_item_data=temp_item_data;
            update_info(&open_item_data,true);

            update_opendata_table(&open_item_data);
            dist_plot->update_data_open_data();
            int temperature=open_item_data.temperature.at(open_item_data.temperature.size()-1);
            int humi=open_item_data.humi.at(open_item_data.humi.size()-1);
            ui->le_temperature->setText(QString::number(((float)temperature/100),'f',1).append("℃"));
            ui->le_humi->setText(QString::number(((float)humi/100),'f',1).append("%"));

           open_item_data.shrink_rate=calc_shrink(&open_item_data);
           ui->le_shrink->setText(QString::number(open_item_data.shrink_rate,'f',9));


        }
        opened_data_fresh();
     }
}

//另存为应该是在此基础上修改的文件


//这个是另存为
void MainWindow::on_click_saveas()
{
   //直接复制对应的文件

    QString filename = QFileDialog::getSaveFileName(this, tr("另存为"),
                  QObject::tr("0.rjdd"),
                QObject::tr("(*.rjdd)"));
    if(filename!=""){


        rjdd->open_rjdd(filename,false,true);

        rjdd->ouput_header(gt_item_data,false);

        rjdd->write_data_rjdd_all(gt_item_data,false);
        QMessageBox::information(NULL,QObject::tr("信息"),QObject::tr("另存为...成功！"));

        rjdd->close_rjdd(false,true);
    }
}

// 关闭数据是关闭整个项目
void MainWindow::on_click_close_data(){
    if(open_item_data.is_exsit){
        //那么需要更新信息
        clear_update_info();
        rjdd->close_rjdd(false,false);
        open_item_data.is_exsit=false;
    }
}

//归零操作
void MainWindow::on_click_clear_all(){

    int err_num;
    if(err_num=modebus_rtu->dev_open(ui->cb_port->currentText())){

        if(err_num==1){
            QMessageBox::information(NULL, "错误", "未识别串口的地址！");
        }
        else if(err_num==2){
            QMessageBox::information(NULL, "错误", "串口打开失败，请更换端口！");
        }
        return ;
    }

    modebus_rtu->read_reg((reinterpret_cast< int >(&((T_MODBUS_REG *)0)->temperature_reg))>>1,8);
    is_zero_sample=true;
}

void MainWindow::on_click_toolbar_selected(int index){
   //先清除掉全部信息
   clear_update_info();
   is_start=false;
   act_start->setDisabled(false);
   act_stop->setDisabled(true);
   //serial->serial_close();
   timer->stop();
   logPrintf("串口已关闭！");

   if(index==1){
        if(open_item_data.is_exsit){
            //那么需要更新信息
            update_info(&open_item_data,true);
            update_opendata_table(&open_item_data);
            dist_plot->update_data_open_data();
        }
   }
   else if(index==0){
       if(gt_item_data.is_exsit){
           //那么需要更新信息
           update_info(&gt_item_data,true);
           update_opendata_table(&gt_item_data);
           dist_plot->update_data_default_data();
       }
   }
}


void MainWindow::on_click_export_data()
{

    QTableWidget *excel_table=new QTableWidget(this);


    //导出直接导出当前测区


    int now_excel_index=0;

    QString temp_string;



        //那就说明是需要导出所有的测线数据

        //那么就需要数据放到里面


        now_excel_index=10;

        int data_size=open_item_data.ch_data[0].size();

       int row_count=data_size+now_excel_index;

       //那么就需要数据放到里面


       excel_table->setRowCount(row_count);
       excel_table->setColumnCount(9);



        excel_table->setItem(0,0,new QTableWidgetItem(tr("项目名称")));
        excel_table->setItem(0,1,new QTableWidgetItem(open_item_data.item_num));
        excel_table->setItem(0,2,new QTableWidgetItem(tr("测试人")));
        excel_table->setItem(0,3,new QTableWidgetItem(open_item_data.tester));

        excel_table->setItem(1,0,new QTableWidgetItem(tr("标准")));

        switch(open_item_data.test_standard){
            case 0:
                temp_string=tr("GB50082-2019:非接触法");
                break;
            case 1:
                temp_string=tr("GB50082-2019:接触法");
                break;
            case 2:
                temp_string=tr("JGJT70-2009");
                break;
            case 3:
                temp_string=tr("JCJT603-2004");
                break;
            case 4:
                temp_string=tr("JTGE30-2005");
                break;
            case 5:
                temp_string=tr("JC/T2551-2019");
                break;
            case 6:
                temp_string=tr("GBT2542-201");
                break;
            case 7:
                temp_string=tr("通用收缩膨胀试验");
                break;
            default:
                temp_string=tr("未知");
                break;
        }

        excel_table->setItem(1,1,new QTableWidgetItem(temp_string));
        excel_table->setItem(1,2,new QTableWidgetItem(tr("项目时间")));
        excel_table->setItem(1,3,new QTableWidgetItem(open_item_data.item_time.toString("yyyy-MM-dd hh:mm:ss")));

        excel_table->setItem(1,4,new QTableWidgetItem(tr("收缩率")));
        excel_table->setItem(1,5,new QTableWidgetItem(QString::number(open_item_data.shrink_rate,'f',9)));


        excel_table->setItem(2,0,new QTableWidgetItem(tr("备注")));
        excel_table->setItem(2,1,new QTableWidgetItem(open_item_data.backup));

        excel_table->setItem(3,0,new QTableWidgetItem(tr("")));
        excel_table->setItem(3,1,new QTableWidgetItem(tr("")));
        excel_table->setItem(3,2,new QTableWidgetItem(tr("")));
        excel_table->setItem(3,3,new QTableWidgetItem(tr("")));

        //3  empty

        excel_table->setItem(4,0,new QTableWidgetItem(tr("试件长度(mm)")));
        excel_table->setItem(4,1,new QTableWidgetItem(QString::number(open_item_data.test_len).append(tr("mm"))));
        excel_table->setItem(4,2,new QTableWidgetItem(tr("采样间隔(分)")));
        excel_table->setItem(4,3,new QTableWidgetItem(QString::number(open_item_data.sample_interval).append(tr("分"))));




        excel_table->setItem(5,0,new QTableWidgetItem(tr("总时间")));
        excel_table->setItem(5,1,new QTableWidgetItem(QString::number(open_item_data.total_time).append(tr("分"))));
        excel_table->setItem(5,2,new QTableWidgetItem(tr("采样时间")));
        excel_table->setItem(5,3,new QTableWidgetItem(QString::number(open_item_data.sample_time).append(tr("分"))));

        excel_table->setItem(6,0,new QTableWidgetItem(tr("测量方式")));
        if(open_item_data.test_type==1){
            excel_table->setItem(6,1,new QTableWidgetItem(tr("双面测量")));
        }
        else{
            excel_table->setItem(6,1,new QTableWidgetItem(tr("单面测量")));
        }

        if(open_item_data.test_type==1){
            excel_table->setItem(6,2,new QTableWidgetItem(tr("第一组")));
            excel_table->setItem(6,3,new QTableWidgetItem(QString::number(open_item_data.probe_set[0]).append(tr("和")).append(QString::number(open_item_data.probe_set[1]))));
            excel_table->setItem(7,0,new QTableWidgetItem(tr("第二组")));
            excel_table->setItem(7,1,new QTableWidgetItem(QString::number(open_item_data.probe_set[2]).append(tr("和")).append(QString::number(open_item_data.probe_set[3]))));
            excel_table->setItem(7,2,new QTableWidgetItem(tr("第三组")));
            excel_table->setItem(7,3,new QTableWidgetItem(QString::number(open_item_data.probe_set[4]).append(tr("和")).append(QString::number(open_item_data.probe_set[5]))));
        }

        excel_table->setItem(8,0,new QTableWidgetItem(tr("")));
        excel_table->setItem(8,1,new QTableWidgetItem(tr("")));
        excel_table->setItem(8,2,new QTableWidgetItem(tr("")));
        excel_table->setItem(8,3,new QTableWidgetItem(tr("")));

        excel_table->setItem(9,0,new QTableWidgetItem(tr("时间")));
        excel_table->setItem(9,1,new QTableWidgetItem(tr("传感器1")));
        excel_table->setItem(9,2,new QTableWidgetItem(tr("传感器2")));
        excel_table->setItem(9,3,new QTableWidgetItem(tr("传感器3")));

        if(open_item_data.test_type==0){
            excel_table->setItem(9,4,new QTableWidgetItem(tr("传感器4")));
            excel_table->setItem(9,5,new QTableWidgetItem(tr("传感器5")));
            excel_table->setItem(9,6,new QTableWidgetItem(tr("传感器6")));

            excel_table->setItem(9,7,new QTableWidgetItem(tr("温度")));
            excel_table->setItem(9,8,new QTableWidgetItem(tr("湿度")));
        }
        else{
            excel_table->setItem(9,4,new QTableWidgetItem(tr("温度")));
            excel_table->setItem(9,5,new QTableWidgetItem(tr("湿度")));
        }




        //先获得当前的数据

        for(int j=0;j<data_size;j++){
            QString temp_string;

            temp_string=QString::number(open_item_data.sample_interval*j/60).append("时");
            temp_string.append(QString::number(open_item_data.sample_interval*j%60).append("分"));

            excel_table->setItem(now_excel_index+j,0,new QTableWidgetItem(temp_string));
            if(open_item_data.test_type==1){
                for(int i=0;i<3;i++){
                    if(open_item_data.ch_data[i].at(j)==ZERO_DEFAULT_VALUE){
                        excel_table->setItem(now_excel_index+j,i+1,new QTableWidgetItem(QString(tr("未连接！"))));
                    }
                    else{
                        excel_table->setItem(now_excel_index+j,i+1,new QTableWidgetItem(QString::number((float)open_item_data.ch_data[i].at(j)/1000,'f',3)));
                    }
                }

                excel_table->setItem(now_excel_index+j,4,new QTableWidgetItem(QString::number((float)open_item_data.temperature.at(j)/100,'f',1).append("℃")));
                excel_table->setItem(now_excel_index+j,5,new QTableWidgetItem(QString::number((float)open_item_data.humi.at(j)/100,'f',1).append("%")));

            }
            else{
                for(int i=0;i<6;i++){
                    if(open_item_data.ch_data[i].at(j)==ZERO_DEFAULT_VALUE){
                        excel_table->setItem(now_excel_index+j,i+1,new QTableWidgetItem(QString(tr("未连接！"))));
                    }
                    else{
                        excel_table->setItem(now_excel_index+j,i+1,new QTableWidgetItem(QString::number((float)open_item_data.ch_data[i].at(j)/1000,'f',3)));
                    }
                }


                excel_table->setItem(now_excel_index+j,7,new QTableWidgetItem(QString::number((float)open_item_data.temperature.at(j)/100,'f',1).append("℃")));
                excel_table->setItem(now_excel_index+j,8,new QTableWidgetItem(QString::number((float)open_item_data.humi.at(j)/100,'f',1).append("%")));
            }
        }



    QString filename = QFileDialog::getSaveFileName(excel_table, tr("保存"),
                  QObject::tr("data.xls"),
                QObject::tr("Excel 文件(*.xls *.xlsx)"));
    if(filename!="")
    {
        //经试验有效
           ExcelEngine excel(filename);
           excel.Open();
           excel.SaveDataFrTable(excel_table); //导出报表
           excel.Close();
    }

    delete excel_table;
}





void MainWindow::monitor_timerout_slot(){
    modebus_rtu->read_reg((reinterpret_cast< int >(&((T_MODBUS_REG *)0)->temperature_reg))>>1,8);
    is_monitor_sample=true;
}




void MainWindow::timerout_slot(){

    is_auto_sample=true;
    if(!is_monitor_start){
       modebus_rtu->read_reg((reinterpret_cast< int >(&((T_MODBUS_REG *)0)->temperature_reg))>>1,8);
    }


//        gt_item_data.sample_time+=1;

//        if(gt_item_data.sample_time>=gt_item_data.total_time){
//            is_start=false;

//            act_start->setDisabled(false);
//            act_manual_sample->setDisabled(false);
//            act_clear_all->setDisabled(false);

//            act_stop->setDisabled(true);

//            //serial->serial_close();
//            timer->stop();
//        }

//        ui->le_sample_time->setText(QString::number(gt_item_data.sample_time));
//        ui->le_rest_time->setText(QString::number(gt_item_data.total_time-gt_item_data.sample_time));


//        probe_data[0]=(int)recv_data[0]*256+recv_data[1];
//        probe_data[1]=(int)recv_data[2]*256+recv_data[3];
//        probe_data[2]=(int)recv_data[4]*256+recv_data[5];
//        probe_data[3]=(int)recv_data[6]*256+recv_data[7];
//        probe_data[4]=(int)recv_data[8]*256+recv_data[9];
//        probe_data[5]=(int)recv_data[10]*256+recv_data[11];

//        for(int i=0;i<6;i++){
//            qDebug("%d------%d",i,probe_data[i]);
//        }

//        if(gt_item_data.test_type==1){

//            if((probe_data[gt_item_data.probe_set[0]]!=ZERO_DEFAULT_VALUE)&&(probe_data[gt_item_data.probe_set[1]])!=ZERO_DEFAULT_VALUE){
//                //说明有效
//                 temp_probe_data[0]=probe_data[gt_item_data.probe_set[0]]+probe_data[gt_item_data.probe_set[1]];

//                 if(probe_data_zero[0]!=ZERO_DEFAULT_VALUE){
//                    temp_probe_data[0]=temp_probe_data[0]-probe_data_zero[0];
//                 }
//            }
//            else{
//                temp_probe_data[0]=ZERO_DEFAULT_VALUE;
//            }


//            if((probe_data[gt_item_data.probe_set[2]]!=ZERO_DEFAULT_VALUE)&&(probe_data[gt_item_data.probe_set[3]])!=ZERO_DEFAULT_VALUE){
//                //说明有效
//                 temp_probe_data[1]=probe_data[gt_item_data.probe_set[2]]+probe_data[gt_item_data.probe_set[3]];

//                 if(probe_data_zero[1]!=ZERO_DEFAULT_VALUE){
//                    temp_probe_data[1]=temp_probe_data[1]-probe_data_zero[1];
//                 }
//            }
//            else{
//                temp_probe_data[1]=ZERO_DEFAULT_VALUE;
//            }

//            if((probe_data[gt_item_data.probe_set[4]]!=ZERO_DEFAULT_VALUE)&&(probe_data[gt_item_data.probe_set[5]])!=ZERO_DEFAULT_VALUE){
//                //说明有效
//                 temp_probe_data[2]=probe_data[gt_item_data.probe_set[4]]+probe_data[gt_item_data.probe_set[5]];

//                 if(probe_data_zero[2]!=ZERO_DEFAULT_VALUE){
//                    temp_probe_data[2]=temp_probe_data[2]-probe_data_zero[2];
//                 }
//            }
//            else{
//                temp_probe_data[2]=ZERO_DEFAULT_VALUE;
//            }


//        }
//        else{

//            for(int i=0;i<6;i++){

//                if(probe_data[i]!=ZERO_DEFAULT_VALUE){
//                   if(probe_data_zero[i]!=ZERO_DEFAULT_VALUE){
//                       temp_probe_data[i]=probe_data[i]-probe_data_zero[i];
//                   }
//                }
//                qDebug("%d,=%d,=%d",i,probe_data_zero[i],temp_probe_data[i]);
//            }
//        }



//        for(int i=0;i<6;i++){
//            probe_data[i]=temp_probe_data[i];
//        }



//        if(probe_data[0]!=ZERO_DEFAULT_VALUE){
//            temp_float=(float)probe_data[0]/1000;
//            ui->le_ch1->setText(QString::number(temp_float,'f',3));
//        }
//        else{
//            ui->le_ch1->setText("未连接");
//        }


//        if(probe_data[1]!=ZERO_DEFAULT_VALUE){
//            temp_float=(float)probe_data[1]/1000;
//            ui->le_ch2->setText(QString::number(temp_float,'f',3));
//        }
//        else{
//            ui->le_ch2->setText("未连接");
//        }


//        if(probe_data[2]!=ZERO_DEFAULT_VALUE){
//            temp_float=(float)probe_data[2]/1000;
//            ui->le_ch3->setText(QString::number(temp_float,'f',3));
//        }
//        else{
//            ui->le_ch3->setText("未连接");
//        }

//        if(probe_data[3]!=ZERO_DEFAULT_VALUE){
//            temp_float=(float)probe_data[3]/1000;
//            ui->le_ch4->setText(QString::number(temp_float,'f',3));
//        }
//        else{
//            ui->le_ch4->setText("未连接");
//        }


//        if(probe_data[4]!=ZERO_DEFAULT_VALUE){
//            temp_float=(float)probe_data[4]/1000;
//            ui->le_ch5->setText(QString::number(temp_float,'f',3));
//        }
//        else{
//            ui->le_ch5->setText("未连接");
//        }

//        if(probe_data[5]!=ZERO_DEFAULT_VALUE){
//            temp_float=(float)probe_data[5]/1000;
//            ui->le_ch6->setText(QString::number(temp_float,'f',3));
//        }
//        else{
//            ui->le_ch6->setText("未连接");
//        }

//        //采集到了数据，需要将数据，放到固定容器中
//        sample_data_add_table(probe_data);
//        //在这里进行数据的实时保存
//        qDebug("sample data!");

//    }
}


void MainWindow::serial_timerout_slot(){
    //搜索串口，如果不一样就更新
    int serial_size=ui->cb_port->count();
    //通过QSerialPortInfo查找可用串口

    foreach(const QSerialPortInfo &info, QSerialPortInfo::availablePorts()){
        bool is_equal =false;
        for(int i=0;i<serial_size;i++){
            if(ui->cb_port->itemText(i)==info.portName()){
                is_equal=true;
            }
        }
        if(!is_equal){
          ui->cb_port->addItem(info.portName());
        }
    }
}



void MainWindow::keyReleaseEvent(QKeyEvent *event)
{
    is_presskey=false;
}


//按键事件
void MainWindow::keyPressEvent(QKeyEvent *event)
{
    //左右键控制步长 ，相当于左溢有意按钮

     if(event->key()==Qt::Key_Space){
        if(!is_presskey){
            qDebug("space");
            is_presskey=true;
        }

    }

}



void MainWindow::paintEvent(QPaintEvent* event)
{
    //设置背景色;
    QPainter painter(this);
    QPainterPath pathBack;
    pathBack.setFillRule(Qt::WindingFill);
    pathBack.addRoundedRect(QRect(0, 0, this->width(), this->height()), 3, 3);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    painter.fillPath(pathBack, QBrush(QColor(238, 223, 204)));

    return QWidget::paintEvent(event);
}

void MainWindow::loadStyleSheet(const QString &sheetName)
{
    QFile file(":/Resources/" + sheetName + ".css");
    file.open(QFile::ReadOnly);
    if (file.isOpen())
    {
        QString styleSheet = this->styleSheet();
        styleSheet += QLatin1String(file.readAll());
        this->setStyleSheet(styleSheet);
    }
}

void MainWindow::onButtonMinClicked()
{
    showMinimized();
}

void MainWindow::onButtonRestoreClicked()
{
    QPoint windowPos;
    QSize windowSize;
    my_titlebar->getRestoreInfo(windowPos, windowSize);
    this->setGeometry(QRect(windowPos, windowSize));
}

void MainWindow::onButtonMaxClicked(){
    qDebug("max windows!");
    my_titlebar->saveRestoreInfo(this->pos(), QSize(this->width(), this->height()));
    QRect desktopRect = QApplication::desktop()->availableGeometry();
    QRect FactRect = QRect(desktopRect.x() - 3, desktopRect.y() - 3, desktopRect.width() + 6, desktopRect.height() + 6);
    setGeometry(FactRect);
}

void MainWindow::onButtonCloseClicked(){
    qDebug("close!");
    close();
}




MainWindow::~MainWindow(){

    delete ui;
    delete my_titlebar;
    delete configure;
    modebus_rtu->serial_close();
    delete modebus_rtu;
    deleteMeauBar();

    deleteStatBar();
    qDebug("delete");

    if(gt_item_data.is_exsit){
       rjdd->close_rjdd(true,false);
    }



    delete timer;
    delete serial_timer;
    delete monitor_timer;
    qDebug("delete1");
    delete rjdd;
    delete dist_plot;
    delete  user_Sound;
    qDebug("delete2");

}
