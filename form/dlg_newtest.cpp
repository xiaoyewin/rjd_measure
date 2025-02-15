#include "dlg_newtest.h"
#include "ui_dlg_newtest.h"
#include <QMessageBox>
#include <QStyleFactory>
extern T_Item_Data gt_item_data;

Dlg_NewTest::Dlg_NewTest(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dlg_NewTest)
{
    ui->setupUi(this);
    setAutoFillBackground(true);

   // this->setStyle("plastique");
    //QApplication::setStyle("plastique");
    this->setWindowTitle(tr("新建试验"));

    ui->label_item_name->setEnabled(true);
    ui->label_standard->setEnabled(true);
    ui->label_tester->setEnabled(true);
    ui->label_testarea_sq->setEnabled(true);
    init();
    is_save=false;
}

void Dlg_NewTest::set_style(QStyle *s){
    //setStyle(QStyleFactory::create("plastique"));
    this->setStyle(s);
}

void Dlg_NewTest::accept() //重载他的OK接收事件
{
    qDebug("456");
    // 保存文件
    //先检查有没有填写 的




    if(ui->lineEdit_item_name->text()==NULL){
        QMessageBox::information(NULL, tr("提示"), tr("项目名称不能为空！"));
        return;
    }


//        if(ui->te->text()==NULL){
//            QMessageBox::information(NULL, tr("提示"), tr("测线编号不能为空！"));
//            return;
//        }
    if(ui->lineEdit_tester->text()==NULL){
        QMessageBox::information(NULL, tr("提示"), tr("测线人不能为空！"));
        return;
    }

    if(ui->doubleSpinBox_time_sq->value()<=0){
        QMessageBox::information(NULL, tr("提示"), tr("时间不能为0！"));
        return;
    }

    if(ui->doubleSpinBox_sample_interval->value()<=0){
        QMessageBox::information(NULL, tr("提示"), tr("采样间隔不能为0！"));
        return;
    }

    if(ui->doubleSpinBox_test_len->value()<=0){
        QMessageBox::information(NULL, tr("提示"), tr("试件长度不能为0！"));
        return;
    }

    int td_select_data[6];

    if(ui->cb_test_type->currentIndex()==1){
        //开始判断有没有重复的
        td_select_data[0]=ui->cb_arr1_1->currentIndex();
        td_select_data[1]=ui->cb_arr1_2->currentIndex();
        td_select_data[2]=ui->cb_arr2_1->currentIndex();
        td_select_data[3]=ui->cb_arr2_2->currentIndex();
        td_select_data[4]=ui->cb_arr3_1->currentIndex();
        td_select_data[5]=ui->cb_arr3_2->currentIndex();

        for(int i=0;i<6;i++){
            for(int j=i+1;j<6;j++){
                if(td_select_data[i]==td_select_data[j]){
                    return;  //直接跳出
                }
            }
        }
    }

    //执行到这一步开始保存信息
    QDateTime cur_datetime=QDateTime::currentDateTime();
    gt_item_data.tester=ui->lineEdit_tester->text();
    gt_item_data.item_time=cur_datetime;
    gt_item_data.item_name=ui->lineEdit_item_name->text();
    gt_item_data.test_standard=ui->cb_standard->currentIndex();
    gt_item_data.backup=ui->te_backup->toPlainText();
    //最后折换成分钟
    gt_item_data.total_time=ui->doubleSpinBox_time_sq->value()*60; //总共需要测试的时间

    gt_item_data.sample_interval=ui->doubleSpinBox_sample_interval->value();

    gt_item_data.test_len=ui->doubleSpinBox_test_len->value();

    gt_item_data.sample_time=0;

    gt_item_data.test_type=ui->cb_test_type->currentIndex();

    if(gt_item_data.test_type==1){
        for(int i=0;i<6;i++){
            gt_item_data.probe_set[i]=td_select_data[i];
        }
    }

    for(int i=0;i<6;i++){
        gt_item_data.ch_data[i].erase(gt_item_data.ch_data[i].begin(),gt_item_data.ch_data[i].end());
        QVector<int>  temp=gt_item_data.ch_data[i];
        gt_item_data.ch_data[i].swap(temp);
    }

    for(int i=0;i<6;i++){
        gt_item_data.ch_point[i].erase(gt_item_data.ch_point[i].begin(),gt_item_data.ch_point[i].end());
        QVector<QPointF>  temp=gt_item_data.ch_point[i];
        gt_item_data.ch_point[i].swap(temp);
    }

    gt_item_data.temperature.erase(gt_item_data.temperature.begin(),gt_item_data.temperature.end());
    QVector<int>  temp=gt_item_data.temperature;
    gt_item_data.temperature.swap(temp);

    gt_item_data.humi.erase(gt_item_data.humi.begin(),gt_item_data.humi.end());
    temp=gt_item_data.humi;
    gt_item_data.humi.swap(temp);


    is_save=true;
    emit closeSignal();//发出关闭信号
}

void Dlg_NewTest::reject(){ //重载他的cancel接收事件

    emit closeSignal();
}

void Dlg_NewTest::init(){
    //
    ui->doubleSpinBox_time_sq->setMinimum(0);
    ui->doubleSpinBox_time_sq->setMaximum(9999);
    ui->doubleSpinBox_time_sq->setSingleStep(1);


    ui->doubleSpinBox_sample_interval->setMinimum(0);
    ui->doubleSpinBox_sample_interval->setMaximum(9999);
    ui->doubleSpinBox_sample_interval->setSingleStep(1);

    ui->doubleSpinBox_test_len->setMinimum(0);
    ui->doubleSpinBox_test_len->setMaximum(9999);
    ui->doubleSpinBox_test_len->setSingleStep(1);

    ui->cb_standard->addItem("GB50082-2019:非接触法");
    ui->cb_standard->addItem("GB50082-2019:接触法");
    ui->cb_standard->addItem("JGJT70-2009");
    ui->cb_standard->addItem("JCJT603-2004");
    ui->cb_standard->addItem("JTGE30-2005");
    ui->cb_standard->addItem("JC/T2551-2019");
    ui->cb_standard->addItem("GBT2542-201");
    ui->cb_standard->addItem("通用收缩膨胀试验");


    ui->cb_test_type->addItem("单面测量");
    ui->cb_test_type->addItem("双面测量");

   connect(ui->cb_test_type,SIGNAL(currentIndexChanged(int)),this,SLOT(test_type_select_slot(int)));

    ui->cb_arr1_1->addItem("通道一");
    ui->cb_arr1_1->addItem("通道二");
    ui->cb_arr1_1->addItem("通道三");
    ui->cb_arr1_1->addItem("通道四");
    ui->cb_arr1_1->addItem("通道五");
    ui->cb_arr1_1->addItem("通道六");

    ui->cb_arr1_2->addItem("通道一");
    ui->cb_arr1_2->addItem("通道二");
    ui->cb_arr1_2->addItem("通道三");
    ui->cb_arr1_2->addItem("通道四");
    ui->cb_arr1_2->addItem("通道五");
    ui->cb_arr1_2->addItem("通道六");

    ui->cb_arr2_1->addItem("通道一");
    ui->cb_arr2_1->addItem("通道二");
    ui->cb_arr2_1->addItem("通道三");
    ui->cb_arr2_1->addItem("通道四");
    ui->cb_arr2_1->addItem("通道五");
    ui->cb_arr2_1->addItem("通道六");

    ui->cb_arr2_2->addItem("通道一");
    ui->cb_arr2_2->addItem("通道二");
    ui->cb_arr2_2->addItem("通道三");
    ui->cb_arr2_2->addItem("通道四");
    ui->cb_arr2_2->addItem("通道五");
    ui->cb_arr2_2->addItem("通道六");

    ui->cb_arr3_1->addItem("通道一");
    ui->cb_arr3_1->addItem("通道二");
    ui->cb_arr3_1->addItem("通道三");
    ui->cb_arr3_1->addItem("通道四");
    ui->cb_arr3_1->addItem("通道五");
    ui->cb_arr3_1->addItem("通道六");

    ui->cb_arr3_2->addItem("通道一");
    ui->cb_arr3_2->addItem("通道二");
    ui->cb_arr3_2->addItem("通道三");
    ui->cb_arr3_2->addItem("通道四");
    ui->cb_arr3_2->addItem("通道五");
    ui->cb_arr3_2->addItem("通道六");

    //默认是关闭的
    ui->cb_arr1_1->hide();
    ui->cb_arr1_2->hide();
    ui->cb_arr2_1->hide();
    ui->cb_arr2_2->hide();
    ui->cb_arr3_1->hide();
    ui->cb_arr3_2->hide();


    ui->label_test_arr1->hide();
    ui->label_test_arr2->hide();
    ui->label_test_arr3->hide();

    ui->buttonBox->button(QDialogButtonBox::Ok)->setText(tr("确定"));//将buttonbox中的ok 变成汉化
    ui->buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("取消"));
    QFont font;
    font.setPointSize(16);
    ui->buttonBox->setFont(font);


    //需要在这里添加输入限制
    QRegExp rx = QRegExp("^[\u4E00-\u9FA5A-Za-z0-9_]+$");

    validator = new QRegExpValidator(rx);
    ui->lineEdit_item_name->setValidator(validator);
    ui->lineEdit_tester->setValidator(validator);

}


void Dlg_NewTest::test_type_select_slot(int index){

    qDebug("index changed");


    if(index==1){
        ui->cb_arr1_1->show();
        ui->cb_arr1_2->show();
        ui->cb_arr2_1->show();
        ui->cb_arr2_2->show();
        ui->cb_arr3_1->show();
        ui->cb_arr3_2->show();

        ui->label_test_arr1->show();
        ui->label_test_arr2->show();
        ui->label_test_arr3->show();

        ui->cb_arr1_1->setCurrentIndex(0);
        ui->cb_arr1_2->setCurrentIndex(1);
        ui->cb_arr2_1->setCurrentIndex(2);
        ui->cb_arr2_2->setCurrentIndex(3);
        ui->cb_arr3_1->setCurrentIndex(4);
        ui->cb_arr3_2->setCurrentIndex(5);

    }
    else{
        ui->cb_arr1_1->hide();
        ui->cb_arr1_2->hide();
        ui->cb_arr2_1->hide();
        ui->cb_arr2_2->hide();
        ui->cb_arr3_1->hide();
        ui->cb_arr3_2->hide();

        ui->label_test_arr1->hide();
        ui->label_test_arr2->hide();
        ui->label_test_arr3->hide();
    }


}
void Dlg_NewTest::keyPressEvent(QKeyEvent *evt)
{
    if(evt->key() == Qt::Key_Escape)
        return;
}





void Dlg_NewTest::closeEvent(QCloseEvent *)
{
    emit closeSignal();//发出关闭信号
}

Dlg_NewTest::~Dlg_NewTest()
{
    delete validator;
    delete ui;
}
