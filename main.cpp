#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

//    QTranslator* translator = new QTranslator();
//    translator->load(":en_tr.qm");
//    a.installTranslator(translator);


    QApplication::setStyle("plastique");
    MainWindow w;
    w.show();


    return a.exec();
}
