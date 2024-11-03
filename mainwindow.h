#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtGui>
#include <QResizeEvent>
#include <QTableWidget>
#include <QKeyEvent>

#include "dlg_newtest.h"
#include<QThread>
#include "tcpclientthread.h"  //用于网络的多线程
#include "soundmedia.h"

#include "dist_plot.h"
#include "rjddata.h"
#include "mytitlebar.h"


#include "drv/modbus_rtu.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
   // bool eventFilter(QObject *watched, QEvent  *event);

protected:
    void paintEvent(QPaintEvent* event);
    void resizeEvent(QResizeEvent *);

    void keyReleaseEvent(QKeyEvent *event);
    void keyPressEvent(QKeyEvent *event);

private:
    void init();
    void initTitleBar();
    void createMeauBar();// 创建菜单栏
    void deleteMeauBar();// 删除菜单栏
    void createStatBar();
    void deleteStatBar();
    void showWifiStat();

    void get_itemList();
    void update_info(T_Item_Data *item_data,bool is_default);
    void clear_update_info();

    void update_opendata_table(T_Item_Data *item_data);

    void sample_data_add_table(int16_t* data);

    double calc_shrink(T_Item_Data *item_data);

    int anlay_rjdd_header(unsigned char *data,int len);

    Dist_Plot *dist_plot;
    RjdData *rjdd;
    Configure *configure;

    void save_rjdd(T_Item_Data &item_data, bool is_header, bool is_default);

    void show_wifi_stat();

    void opened_data_fresh();

    int16_t probe_data_zero[6];  //归零0值：


private:
    Ui::MainWindow *ui;


    SoundMedia *user_Sound;

    //各个窗口的打开状态
     typedef struct Per_Frm_State {
        bool is_open_new_test_dlg;  //包含测试，测区，测线
        bool is_open_serial_connect;
   }T_Per_Frm_State;

    bool is_presskey;
    MyTitleBar *my_titlebar;
    T_Per_Frm_State tPer_Frm_State;
    QString drag_filepath1;


    QMainWindow *widget_collect_site;
    QMainWindow *widget_data_analy;
    QMainWindow *widget_system_setting;


   // QMenuBar *menu_data_analy; //数据采集

    QTabWidget *tool_tabWidget;
    QStatusBar *statBar;//状态栏
    QLabel *wifiStatLabel;

    QToolBar *toolbar_test_item;
    QToolBar *toolbar_control;

    QToolBar *toolbar_test_data;
    QToolBar *toolbar_operate;
    QToolBar *toolbar_function;

    QToolBar *toolbar_setting;

    QAction *act_new_test;   //新建测试
    QAction *act_continue_test;   //继续测试

    QAction *act_start;   //自动采集开始
    QAction *act_stop;    //停止
    QAction *act_manual_sample;    //停止
    QAction *act_clear_all;    //停止

    QAction *act_open_data;  //打开 文件1
    QAction *act_saveas;   // 另存为
    QAction *act_close_data;   // 关闭数据
    QAction *act_export_data;   // 数据导出

    QAction *act_system_setting;   //继续测试

    Dlg_NewTest *dlg_newTest;//新建测试的类


    int screenWidth_cst;
    int screenHeight_cst;

    bool is_start;

    bool is_monitor_start;


    QTimer *timer;
    QTimer *serial_timer;
    QTimer *monitor_timer;

    Modbus_rtu *modebus_rtu;
    void logPrintf(QString s);

    void loadStyleSheet(const QString &sheetName);

    bool is_monitor_sample;

    bool is_zero_sample;

    bool is_auto_sample;


private slots:
    void on_click_new_test();
    void close_new_test();
    void on_click_continue_test();
    void on_click_serial_connect();
    void close_serial_connect();

    void on_click_start();
    void on_click_stop();
    void on_click_manual_sample();
    void on_click_clear_all();


    void on_click_open_data();
    void on_click_saveas();
    void on_click_close_data();

    void on_click_export_data();

    void on_click_toolbar_selected(int index);

    void timerout_slot();

    void monitor_timerout_slot();
    void serial_timerout_slot();

    void get_data_from_serial_slot(uint8_t *data,int len);

private slots:
    // 按钮触发的槽;
    void onButtonMinClicked();
    void onButtonRestoreClicked();
    void onButtonMaxClicked();
    void onButtonCloseClicked();

signals:
    void instr_data_signal(T_NET_Info *info);

};

#endif // MAINWINDOW_H
