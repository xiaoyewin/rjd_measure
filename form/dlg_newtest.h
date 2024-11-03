#ifndef DLG_NEWTEST_H
#define DLG_NEWTEST_H

#include <QDialog>
#include <QCloseEvent>
#include <QPushButton>
#include <QRegExpValidator>
#include "configure.h"


namespace Ui {
class Dlg_NewTest;
}

class Dlg_NewTest : public QDialog{
    Q_OBJECT
public:

    explicit Dlg_NewTest(QWidget *parent = 0);
    ~Dlg_NewTest();
    void set_style(QStyle *s);
   bool is_save;
protected:
    void closeEvent(QCloseEvent *);
    void keyPressEvent(QKeyEvent *evt);
private:
    Ui::Dlg_NewTest *ui;
    void init();
    QRegExpValidator* validator;
    void accept();
    void reject();
private slots:
    void test_type_select_slot(int index);


signals:
    void closeSignal();

};

#endif // DLG_NEWTEST_H
