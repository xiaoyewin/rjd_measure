#-------------------------------------------------
#
# Project created by QtCreator 2022-02-20T09:11:14
#
#-------------------------------------------------

QT       += core gui
QT       += network
QT       += multimedia
QT       += serialport

QT       += axcontainer   # QT5
CONFIG += c++11  #QT5


VPATH       +=  form
INCLUDEPATH +=  form


qtHaveModule(printsupport): QT += printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
INCLUDEPATH += D:\work\Qt\qwt-6.1.2\src

TARGET = rjd_floorleveling
TEMPLATE = app

LIBS+= -lqwt


#用于屏蔽debug输出��DEFINES += QT_NO_WARNING_OUTPUT
DEFINES += QT_NO_DEBUG_OUTPUT

TRANSLATIONS += en_tr.ts zh_tr.ts

SOURCES += main.cpp\
    dist_plot.cpp \
    drv/fifo_buf.cpp \
    drv/modbus_rtu.cpp \
    drv/serial_dev.cpp \
    form/dlg_sys_setting.cpp \
        mainwindow.cpp \
    form/dlg_newtest.cpp \
    configure.cpp \
    tcpclientthread.cpp \
    soundmedia.cpp \
    rjddata.cpp \
    excelengine.cpp \
    mytitlebar.cpp

HEADERS  += mainwindow.h \
    dist_plot.h \
    drv/fifo_buf.h \
    drv/modbus_rtu.h \
    drv/serial_dev.h \
    form/dlg_newtest.h \
    configure.h \
    form/dlg_sys_setting.h \
    tcpclientthread.h \
    net_common.h \
    soundmedia.h \
    gradient_plot.h \
    rjddata.h \
    excelengine.h \
    mytitlebar.h

FORMS    += mainwindow.ui \
    form/dlg_newtest.ui \
    form/dlg_sys_setting.ui

RESOURCES += \
    resource.qrc   \
    language.qrc \
    sound.qrc

RC_ICONS = logo.ico
RC_FILE = logo.rc
