#include "rjddata.h"
#include <QMessageBox>
#include <QtDebug>
#include <unistd.h>
RjdData::RjdData()
{

    default_file=NULL;
    default_file_in=NULL;
    default_file_out=NULL;
    is_defaultfile_opened=false;
    is_openedfile_opened=false;

}
//好吧


void RjdData::new_default_file(T_Item_Data &item_data){
    //文本读写模块 //这个新建默认的项目文件应该在新建项目的时候生成

    QString temp_filenname="./";
    temp_filenname.append(item_data.item_time.toString("yyyy_MM_dd_hh_mm_ss-"));
    temp_filenname.append(item_data.item_name);
    temp_filenname.append(".rjdd");
    qDebug()<<temp_filenname;

    qDebug("121");


    if(default_file_in){
        default_file->close();
        qDebug("1211");
        delete default_file_in;
        qDebug("1212");
        default_file_in=NULL;
        qDebug("1213");
    }


    if(default_file){
        delete default_file;
        default_file=NULL;
    }


    qDebug("122");
    if(default_file_out){
        delete default_file_out;
        default_file_out=NULL;
    }

    qDebug("123");
    cur_index=0;
    default_file=new QFile(temp_filenname);//打开的默认的文件
    //在操作过程中，会生成一个文件，和源一样，用复制操作
    if(!default_file->open(QIODevice::ReadWrite| QIODevice::Text)){
        QMessageBox::warning(NULL,QObject::tr("文件警告"),QObject::tr("打开文件失败，请重新打开文件"));
        delete default_file;
        default_file=NULL;
        return;
    }
    default_file_in=new QTextStream(default_file);// 只读 输入数据流
    default_file_out=new QTextStream(default_file);

    qDebug("122=%x",default_file);
    default_file->seek(cur_index); //开始
    default_file->resize(cur_index);

}


//主要是识别码
void RjdData::ouput_header(T_Item_Data &item_data,bool is_default){

    QFile *file;
    QTextStream *file_in,*file_out;

    if(is_default){
        file=default_file;
        file_in=default_file_in;
        file_out=default_file_out;
    }
    else{
        file=saveas_file;
        file_in=saveas_file_in;
        file_out=saveas_file_out;
    }

    cur_index=0;
    file->seek(cur_index); //开始
    file->resize(cur_index);
    QString start_string=RJDD_HEADER;
    start_string.append(RJDD_LINE_SWPAERATED);

    *file_out<<start_string;



    QString item_string;
    item_string=RJDD_ITEM_HEADER;
    item_string.append(RJDD_SEPAERATED).append(item_data.item_name);

    item_string.append(RJDD_SEPAERATED).append(item_data.tester).append(RJDD_SEPAERATED).append(item_data.item_time.toString("yyyy-MM-dd hh:mm:ss"))  \
            .append(RJDD_SEPAERATED).append(QString::number(item_data.test_standard)).append(RJDD_SEPAERATED).append(QString::number(item_data.total_time)).append(RJDD_SEPAERATED) \
            .append(QString::number(item_data.sample_interval)).append(RJDD_SEPAERATED).append(QString::number(item_data.test_len)).append(RJDD_SEPAERATED)   \
            .append(item_data.backup).append(RJDD_SEPAERATED).append(QString::number(item_data.test_type)) ;

    for(int i=0;i<6;i++){
        item_string.append(RJDD_SEPAERATED).append(QString::number(item_data.probe_set[i]));
    }

    item_string.append(RJDD_LINE_SWPAERATED);

    *file_out<<item_string;
    file_out->flush();
}


void RjdData::write_data_rjdd(T_Item_Data &item_data, bool is_default){

    QTextStream *file_in,*file_out;

    if(is_default){
        file_in=default_file_in;
        file_out=default_file_out;
    }
    else{
        file_in=saveas_file_in;
        file_out=saveas_file_out;
    }


    int data_size=item_data.ch_data[0].size();
    if(data_size<=0){
        return ;
    }


    QString data_string;

    data_string=RJDD_DATA_HEADER;

    if(item_data.test_type==1){

        for(int i=0;i<3;i++){
            data_string.append(RJDD_SEPAERATED).append(QString::number(item_data.ch_data[i].at(data_size-1)));
        }

        data_string.append(RJDD_SEPAERATED).append(QString::number(item_data.temperature.at(data_size-1)))   \
                .append(RJDD_SEPAERATED).append(QString::number(item_data.humi.at(data_size-1)));

        data_string.append(RJDD_LINE_SWPAERATED);

    }
    else{
        for(int i=0;i<6;i++){
            data_string.append(RJDD_SEPAERATED).append(QString::number(item_data.ch_data[i].at(data_size-1)));
        }
        data_string.append(RJDD_SEPAERATED).append(QString::number(item_data.temperature.at(data_size-1)))   \
                .append(RJDD_SEPAERATED).append(QString::number(item_data.humi.at(data_size-1)));
        data_string.append(RJDD_LINE_SWPAERATED);
    }

    *file_out<<data_string;
    file_out->flush();
}


void RjdData::write_data_rjdd_all(T_Item_Data &item_data, bool is_default){

    QTextStream *file_in,*file_out;

    if(is_default){
        file_in=default_file_in;
        file_out=default_file_out;
    }
    else{
        file_in=saveas_file_in;
        file_out=saveas_file_out;
    }


    int data_size=item_data.ch_data[0].size();
    if(data_size<=0){
        return ;
    }


    QString data_string;



    if(item_data.test_type==1){
        for(int j=0;j<data_size;j++){
            data_string=RJDD_DATA_HEADER;
            for(int i=0;i<3;i++){
                data_string.append(RJDD_SEPAERATED).append(QString::number(item_data.ch_data[i].at(j)));
            }
            data_string.append(RJDD_SEPAERATED).append(QString::number(item_data.temperature.at(data_size-1)))   \
                    .append(RJDD_SEPAERATED).append(QString::number(item_data.humi.at(data_size-1)));

            data_string.append(RJDD_LINE_SWPAERATED);
            *file_out<<data_string;
        }

    }
    else{
        for(int j=0;j<data_size;j++){
            data_string=RJDD_DATA_HEADER;
            for(int i=0;i<6;i++){
                data_string.append(RJDD_SEPAERATED).append(QString::number(item_data.ch_data[i].at(j)));
            }
            data_string.append(RJDD_SEPAERATED).append(QString::number(item_data.temperature.at(data_size-1)))   \
                    .append(RJDD_SEPAERATED).append(QString::number(item_data.humi.at(data_size-1)));
            data_string.append(RJDD_LINE_SWPAERATED);
            *file_out<<data_string;
        }

    }


    file_out->flush();
}






//打开是不是本地的
int RjdData::open_rjdd(QString file_name,bool is_default , bool is_savas){


    if(is_default){
        default_file=new QFile(file_name);//打开的默认的文件
        if(!default_file->open(QIODevice::ReadWrite| QIODevice::Text)){
            delete default_file;
            default_file=NULL;
            QMessageBox::warning(NULL,QObject::tr("文件警告"),QObject::tr("打开文件失败，请重新打开文件"));
            return 1;
        }

        default_file_in=new QTextStream(default_file);
        default_file_out=new QTextStream(default_file);
        default_file->seek(0); //开始

    }
    else {
        if(is_savas){
            saveas_file=new QFile(file_name);//打开的默认的文件
            if(!saveas_file->open(QIODevice::ReadWrite| QIODevice::Text)){
                delete saveas_file;
                QMessageBox::warning(NULL,QObject::tr("文件警告"),QObject::tr("打开文件失败，请重新打开文件"));
                return 1;
            }

            saveas_file_in=new QTextStream(saveas_file);
            saveas_file_out=new QTextStream(saveas_file);

            saveas_file->seek(0); //开始
        }
        else{
            opened_file=new QFile(file_name);//打开的默认的文件
            if(!opened_file->open(QIODevice::ReadOnly| QIODevice::Text)){
                delete saveas_file;
                QMessageBox::warning(NULL,QObject::tr("文件警告"),QObject::tr("打开文件失败，请重新打开文件"));
                return 1;
            }

            opened_file_in=new QTextStream(opened_file);
            opened_file_out=new QTextStream(opened_file);

            opened_file->seek(0); //开始
            is_openedfile_opened=true;
        }

    }

    return 0;

}

//进行数据的的解析
//is_deafault 表示是否打开默认的数据
void RjdData::read_rjdd(T_Item_Data *itm_data, QString file_name, bool is_default){

    QTextStream *file_in,*file_out;
    QFile *file;

    if(is_default){
        file=default_file;
        file_in=default_file_in;
        file_out=default_file_out;
    }
    else{
        file=opened_file;
        file_in=opened_file_in;
        file_out=opened_file_out;
    }
    file_in->seek(0); //开始
    //进行一行一行的读取
    while(!file_in->atEnd()){
        QString temp_string = file_in->readLine();//每次读取一行

        //然后进行分割
        QStringList temp_stringlist = temp_string.split(RJDD_SEPAERATED);


        if(temp_stringlist.at(0)==RJDD_HEADER){
            itm_data->filename=file_name;
            itm_data->is_exsit=true;
        }
        else if(temp_stringlist.at(0)==RJDD_ITEM_HEADER){
            itm_data->item_name=temp_stringlist.at(1);
            itm_data->tester=temp_stringlist.at(2);
            itm_data->item_time=QDateTime::fromString(temp_stringlist.at(3), "yyyy-MM-dd hh:mm:ss");
            itm_data->test_standard=temp_stringlist.at(4).toInt();
            itm_data->total_time=temp_stringlist.at(5).toInt();
            itm_data->sample_interval=temp_stringlist.at(6).toInt();
            itm_data->test_len=temp_stringlist.at(7).toInt();
            itm_data->backup=temp_stringlist.at(8);
            itm_data->test_type=temp_stringlist.at(9).toInt();

            for(int i=0;i<6;i++){
                itm_data->probe_set[i]=temp_stringlist.at(10+i).toInt();
            }
        }

        else if(temp_stringlist.at(0)==RJDD_DATA_HEADER){

            if(itm_data->test_type==1){
                int temp_data=0;
                for(int i=0;i<3;i++){
                    temp_data=temp_stringlist.at(1+i).toInt();
                    itm_data->ch_data[i].push_back(temp_data);
                }
                temp_data=temp_stringlist.at(4).toInt();
                itm_data->temperature.push_back(temp_data);

                temp_data=temp_stringlist.at(5).toInt();
                itm_data->humi.push_back(temp_data);
            }
            else{
                int temp_data=0;
                for(int i=0;i<6;i++){
                    temp_data=temp_stringlist.at(1+i).toInt();
                    itm_data->ch_data[i].push_back(temp_data);
                }

                temp_data=temp_stringlist.at(7).toInt();
                itm_data->temperature.push_back(temp_data);

                temp_data=temp_stringlist.at(8).toInt();
                itm_data->humi.push_back(temp_data);
            }
        }
    }
}

void RjdData::close_rjdd(bool is_default,bool is_saveas){


    if(is_default){
        default_file_out->flush();
        default_file->close();

        delete default_file;
        delete default_file_in;
        delete default_file_out;

        default_file=NULL;
        default_file_in=NULL;
        default_file_out=NULL;
        is_defaultfile_opened=false;

    }
    else{
        if(is_saveas){

            saveas_file_out->flush();
            saveas_file->close();

            delete saveas_file;
            delete saveas_file_in;
            delete saveas_file_out;

            saveas_file=NULL;
            saveas_file_in=NULL;
            saveas_file_out=NULL;

        }
        else{

            opened_file_out->flush();
            opened_file->close();

            delete opened_file;
            delete opened_file_in;
            delete opened_file_out;

            opened_file=NULL;
            opened_file_in=NULL;
            opened_file_out=NULL;

            is_openedfile_opened=false;
        }
    }
}



RjdData::~RjdData(){
    if(is_defaultfile_opened){
        default_file->close();
        delete default_file_in;
        delete default_file_out;
        delete default_file;
         default_file_in=NULL;
         default_file_out=NULL;
         default_file=NULL;

    }

    if(is_openedfile_opened){
        opened_file->close();
        delete opened_file_in;
        delete opened_file_out;
        delete opened_file;
    }
}
