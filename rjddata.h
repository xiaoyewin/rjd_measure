#ifndef RJDDATA_H
#define RJDDATA_H

#include <QString>
#include<QVector>
#include<QDateTime>
#include <QFile>
#include <QTextStream>
#include "configure.h"

#define  RJDD_HEADER "RJDDATE!"
#define  RJDD_SEPAERATED "，。"
#define  RJDD_LINE_SWPAERATED "\n"

#define  RJDD_ITEM_HEADER "ITEM!"

#define  RJDD_DATA_HEADER "DATA!"

#define FILENAME_DEFAULT "./default_data.rjdd"
class RjdData
{
public:
    RjdData();
    ~RjdData();
    void new_default_file(T_Item_Data &item_data);
    void ouput_header(T_Item_Data &item_data,bool is_default);
    void write_data_rjdd(T_Item_Data &item_data, bool is_default);  //项目的保存
    void write_data_rjdd_all(T_Item_Data &item_data, bool is_default);


    void read_rjdd(T_Item_Data *itm_data, QString file_name,bool is_default);

    void close_rjdd(bool is_default, bool is_saveas);
    int open_rjdd(QString file_name, bool is_default, bool is_savas);



private:

    QFile *default_file;
    QTextStream *default_file_in,*default_file_out;

    QFile *opened_file;
    QTextStream *opened_file_in,*opened_file_out;

    QFile *saveas_file;
    QTextStream *saveas_file_out,*saveas_file_in;



    bool is_defaultfile_opened;
    bool is_openedfile_opened;

    int cur_index;

};


#endif
