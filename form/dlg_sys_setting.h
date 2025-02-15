#ifndef DLG_SET_SETTING_H
#define DLG_SET_SETTING_H

#include <QDialog>
#include <QCloseEvent>
#include <QDoubleValidator>
#include <QTimer>

namespace Ui {
    class Dlg_Sys_Setting;
}

class Dlg_Sys_Setting : public QDialog
{
    Q_OBJECT

public:
    explicit Dlg_Sys_Setting(QWidget *parent = nullptr);
    ~Dlg_Sys_Setting();

    void show_temp_humi(int temp,int humi);
protected:
    void closeEvent(QCloseEvent *);
private:
    Ui::Dlg_Sys_Setting *ui;
    QDoubleValidator *doubleValidator;
    QTimer *timer;
    void init(void);
    void accept();
    void reject();
    int calc_temperature(int temp);
    int calc_humi(int humi);
private slots:
    void timerout_slot();

signals:
    void closeSignal();
    void temp_humi_signal();

};

#endif // DLG_SET_SETTING_H
