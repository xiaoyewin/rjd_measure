#include "dist_plot.h"

#include <qwt_plot_curve.h>
#include <qwt_point_data.h>


#include<qwt_scale_draw.h>
#include <qwt_symbol.h>
#include <qwt_math.h>
#include<QMessageBox>
#include <complex>


extern struct program_setting gt_program_setting;   //全局结构体
extern T_Item_Data gt_item_data;
extern T_Item_Data open_item_data;
using namespace std;



Dist_Plot::Dist_Plot(QWidget *parent)
    : QwtPlot(parent) {

    //setAutoFillBackground(true);
    //setPalette(QPalette(QColor(0x33, 0xcc, 0xff)));
    //setAutoReplot(false);


    cur_tick=0;
    /* X 轴  刚开始是0-24小时 */
    setAxisTitle(xBottom, QObject::tr("时间(分)"));
    //先单位分钟  最小为60分种
    cur_max_time=60;
    setAxisScale(xBottom,0,cur_max_time,cur_max_time/10);

    cur_min_height=0;
    cur_max_height=0;
    height_tol=0.5;  //这个精度向上移动0.5mm
//    /* Y 轴 */
    setAxisTitle(yLeft, tr("位移(mm)"));
    //setAxisTitle(yLeft, tr("height(mm)"));

    setAxisScale(yLeft,cur_min_height,cur_max_height+height_tol,(cur_max_height-cur_min_height+height_tol)/10);
    canvas = new QwtPlotCanvas();
    canvas->setLineWidth(1);
    canvas->setFrameStyle(QFrame::Box | QFrame::Plain);
    canvas->setBorderRadius(1);

    QPalette canvasPalette(Qt::gray);
    canvasPalette.setColor(QPalette::Foreground, QColor(133, 190, 232));
    canvas->setPalette(canvasPalette);

    setCanvas(canvas);

    td_pen=new QPen(Qt::red);
    td_pen->setWidth(3);


    fix_color[0]=QColor(Qt::red);
    fix_color[1]=QColor(Qt::green);
    fix_color[2]=QColor(Qt::blue);
    fix_color[3]=QColor(Qt::cyan);
    fix_color[4]=QColor(Qt::magenta);
    fix_color[5]=QColor(Qt::yellow);
    fix_color[6]=QColor(Qt::darkRed);
    fix_color[7]=QColor(Qt::darkGreen);
    fix_color[8]=QColor(Qt::darkBlue);
    fix_color[9]=QColor(Qt::darkCyan);
    fix_color[10]=QColor(Qt::darkMagenta);
    fix_color[11]=QColor(Qt::darkYellow);
    fix_color[12]=QColor(Qt::white);
    fix_color[13]=QColor(Qt::darkGray);
    fix_color[14]=QColor(Qt::gray);
    fix_color[15]=QColor(Qt::lightGray);
    fix_color[16]=QColor(Qt::color0);
    fix_color[17]=QColor(Qt::color1);

    //这个要突出方门



    td_pen=new QPen(Qt::red);
    td_pen->setWidth(3);


    legend = new QwtLegend(this);
    legend->setDefaultItemMode( QwtLegendData::Checkable);//图例可被点击

    insertLegend( legend, QwtPlot::RightLegend );


     QObject::connect(legend, SIGNAL(checked( const QVariant & , bool , int )), this,SLOT(legendClicked(const QVariant & , bool , int )));


    for(int i=0;i<6;i++){
        QwtPlotCurve * temp_curve = new QwtPlotCurve(QString::number(i+1));

        temp_curve->setRenderHint( QwtPlotItem::RenderAntialiased );
        temp_curve->setLegendAttribute( QwtPlotCurve::LegendShowSymbol, true );
        temp_curve->setStyle(QwtPlotCurve::Lines);
        td_pen->setColor(fix_color[i%18]);
        temp_curve->setPen(*td_pen);
        temp_curve->attach( this );
        temp_curve->setLegendAttribute(temp_curve->LegendShowLine);
        m_curve.push_back(temp_curve);

    }

    cur_stat=0;
}





void Dist_Plot::legendClicked( const QVariant &itemInfo, bool on, int index){
    qDebug() << "Legend item clicked, enabled:" << itemInfo.type() <<"index"<<index<<on;

    //获取曲线
    QwtPlotItem *plotItem = this->infoToItem( itemInfo );
    //根据图例选择状态，设置曲线隐藏还是显示
    if ( plotItem )
        plotItem->setVisible(!on);
    //重要，下面这句没有不起作用
    replot();

    // 在这里添加你的代码来处理点击事件
}

//获取最大和最小值  有用
void Dist_Plot::calc_origin_data(QVector<double> &data,double &min_value, double &max_value){

    int size=data.size();
    for(int i=0;i<size;i++){
        if(data[i]==(double)ZERO_DEFAULT_VALUE){
            continue;
        }
        if(data[i]>max_value){
            max_value=data[i];
        }
        if(data[i]<min_value){
            min_value=data[i];
        }
    }

    //需要向上和向下取整
    min_value= (double)((int)(min_value)/1);
    if((max_value-min_value)<1){
        max_value=min_value+1;
    }
}



void Dist_Plot::clear_curve(){

//    int curve_size=m_curve.size();
//    if(1){
//        for(int i=0;i<curve_size;i++){
//            delete m_curve[i];
//        }
//        m_curve.erase(m_curve.begin(),m_curve.end());
//        QVector<QwtPlotCurve *>  temp=m_curve;
//        m_curve.swap(temp);
//    }



    cur_max_time=60;
    setAxisScale(xBottom,0,cur_max_time,cur_max_time/10);

    cur_min_height=0;
    cur_max_height=0;
//    /* Y 轴 */
    setAxisTitle(yLeft, tr("距离(mm)"));
    //setAxisTitle(yLeft, tr("height(mm)"));

    setAxisScale(yLeft,cur_min_height,cur_max_height+height_tol,(cur_max_height-cur_min_height+height_tol)/10);
    QPolygonF m_point;
    m_point<< QPointF(0,0);

    for(int i=0;i<6;i++){
         m_curve[i]->setSamples(m_point);
    }
    replot();
}

//在这里需要添加标注
void Dist_Plot::paint_curve(QVector<double> &data){
    //获得最新的索引
    int curve_index=m_curve.size()-1;

    //进行数据的更新
    QPolygonF m_point;
    double temp_dist;
    double temp_double;
    int size=data.size();
    for(int i=0;i<size;i++){
        temp_dist=((double)step_len*i)/1000;
        temp_double=data.at(i);
        m_point<< QPointF(temp_dist,temp_double);
    }

    m_curve[curve_index]->setSamples(m_point);


    QwtLegend *legend = new QwtLegend(this);
     legend->setDefaultItemMode( QwtLegendData::Checkable );//图例可被点击
     insertLegend( legend, QwtPlot::RightLegend );
     m_curve[curve_index]->setLegendAttribute(m_curve[curve_index]->LegendShowLine);

    replot();
}

//始终最大为3米的窗口值
//那就是最大为11个点
//可以将这可以将这写数据放到一个循环队列中

void Dist_Plot::update_data(){

    //这个表示新的6组数据

    cur_tick+=step_len;

    QVector<double> temp_data_vector;

    int size=gt_item_data.ch_data[0].size();

    if(size<=0){
        return ;
    }
    int temp_value;
    if(gt_item_data.test_type==1){
        for(int i=0;i<3;i++){
            temp_value=gt_item_data.ch_data[i].at(size-1);
            if(temp_value!=ZERO_DEFAULT_VALUE){
                temp_data_vector.push_back((double)temp_value/1000);
            }
        }
        //找出对应的格式方式
        calc_origin_data(temp_data_vector,cur_min_height,cur_max_height);
    }
    else{
        for(int i=0;i<6;i++){
            temp_value=gt_item_data.ch_data[i].at(size-1);
            if(temp_value!=ZERO_DEFAULT_VALUE){
               temp_data_vector.push_back((double)temp_value/1000);
            }
        }
        calc_origin_data(temp_data_vector,cur_min_height,cur_max_height);
    }

    setAxisScale(yLeft,cur_min_height,cur_max_height+height_tol,(cur_max_height-cur_min_height+height_tol)/10);

     cur_tick=size;
     temp_value=cur_tick/60;
     cur_max_time=temp_value*60+60;
     setAxisScale(xBottom,0,cur_max_time,cur_max_time/10);




    double temp_dist;
    double temp_double;

    for(int j=0;j<6;j++){
        temp_dist=size-1;
        if(gt_item_data.ch_data[j].at(size-1)!=ZERO_DEFAULT_VALUE){
            temp_double=(double)gt_item_data.ch_data[j].at(size-1)/1000;
            gt_item_data.ch_point[j].push_back(QPointF(temp_dist,temp_double));
        }
    }


    for(int i=0;i<6;i++){
         m_curve[i]->setSamples(gt_item_data.ch_point[i]);
    }




    //将数据保存到vector 中
//    if(gt_item_data.test_type==0){
//        for(int i=0;i<size;i++){
//            for(int j=3;j<6;j++){
//                temp_dist=i;
//                temp_double=gt_item_data.ch_data[j].at(i);
//                m_point[j]<< QPointF(temp_dist,temp_double);
//            }
//        }

//        for(int i=3;i<6;i++){
//             m_curve[i]->setSamples(m_point[i]);
//        }
//    }
     replot();
}



void Dist_Plot::update_data_default_data(){

    //这个表示新的6组数据

    QVector<double> temp_data_vector;

    int size=gt_item_data.ch_data[0].size();



    if(size<=0){
        return ;
    }

    int temp_value;
    if(gt_item_data.test_type==1){
        for(int j=0;j<size;j++){
            for(int i=0;i<3;i++){
                temp_value=gt_item_data.ch_data[i].at(j);
                temp_data_vector.push_back((double)temp_value/1000);
            }
        }

        //找出对应的格式方式
        calc_origin_data(temp_data_vector,cur_min_height,cur_max_height);

    }
    else{
        for(int j=0;j<size;j++){
            for(int i=0;i<6;i++){
                temp_value=gt_item_data.ch_data[i].at(j);
                temp_data_vector.push_back((double)temp_value/1000);
            }
        }
        calc_origin_data(temp_data_vector,cur_min_height,cur_max_height);
    }

     setAxisScale(yLeft,cur_min_height,cur_max_height+height_tol,(cur_max_height-cur_min_height+height_tol)/10);

     cur_tick=size;
     temp_value=cur_tick/60;
     cur_max_time=temp_value*60+60;
     setAxisScale(xBottom,0,cur_max_time,cur_max_time/10);


    QPolygonF m_point[6];
    double temp_dist;
    double temp_double;


    for(int i=0;i<size;i++){
        for(int j=0;j<3;j++){
            temp_dist=i;
            temp_double=gt_item_data.ch_data[j].at(i);
            temp_double=temp_double/1000;
            m_point[j]<< QPointF(temp_dist,temp_double);
        }
    }

    for(int i=0;i<3;i++){
         m_curve[i]->setSamples(m_point[i]);
    }



    //将数据保存到vector 中
    if(gt_item_data.test_type==0){
        for(int i=0;i<size;i++){
            for(int j=3;j<6;j++){
                temp_dist=i;
                temp_double=gt_item_data.ch_data[j].at(i);
                temp_double=temp_double/1000;
                m_point[j]<< QPointF(temp_dist,temp_double);
            }
        }

        for(int i=3;i<6;i++){
             m_curve[i]->setSamples(m_point[i]);
        }
    }
     replot();
}




void Dist_Plot::update_data_open_data(){
    //这个表示新的6组数据

    QVector<double> temp_data_vector;

    int size=open_item_data.ch_data[0].size();

    double cur_opened_min_height;
    double cur_opened_max_height;

    if(size<=0){
        return ;
    }

    int temp_value;
    cur_opened_min_height=cur_opened_max_height=(double)open_item_data.ch_data[0].at(0)/1000;
    if(open_item_data.test_type==1){
        for(int j=0;j<size;j++){
            for(int i=0;i<3;i++){
                temp_value=open_item_data.ch_data[i].at(j);
                if(temp_value!=ZERO_DEFAULT_VALUE){
                    temp_data_vector.push_back((double)temp_value/1000);
                }

            }
        }

        //找出对应的格式方式
        calc_origin_data(temp_data_vector,cur_opened_min_height,cur_opened_max_height);

    }
    else{
        for(int j=0;j<size;j++){
            for(int i=0;i<6;i++){

                temp_value=open_item_data.ch_data[i].at(j);
                if(temp_value!=ZERO_DEFAULT_VALUE){
                    temp_data_vector.push_back((double)temp_value/1000);
                }
            }
        }
        calc_origin_data(temp_data_vector,cur_opened_min_height,cur_opened_max_height);
    }

     setAxisScale(yLeft,cur_opened_min_height,cur_opened_max_height+height_tol,(cur_opened_max_height-cur_opened_min_height+height_tol)/10);

     cur_tick=size;
     temp_value=cur_tick/60;
     cur_max_time=temp_value*60+60;
     setAxisScale(xBottom,0,cur_max_time,cur_max_time/10);


    QPolygonF m_point[6];
    double temp_dist;
    double temp_double;


    for(int i=0;i<size;i++){
        for(int j=0;j<3;j++){
            temp_dist=i;
            if(open_item_data.ch_data[j].at(i)!=ZERO_DEFAULT_VALUE){
                temp_double=open_item_data.ch_data[j].at(i);
                temp_double=temp_double/1000;
                m_point[j]<< QPointF(temp_dist,temp_double);
            }
        }
    }

    for(int i=0;i<3;i++){
         m_curve[i]->setSamples(m_point[i]);
    }



    //将数据保存到vector 中
    if(open_item_data.test_type==0){
        for(int i=0;i<size;i++){
            for(int j=3;j<6;j++){
                if(open_item_data.ch_data[j].at(i)!=ZERO_DEFAULT_VALUE){
                    temp_dist=i;
                    temp_double=open_item_data.ch_data[j].at(i);
                    temp_double=temp_double/1000;
                    m_point[j]<< QPointF(temp_dist,temp_double);
                }

            }
        }

        for(int i=3;i<6;i++){
             m_curve[i]->setSamples(m_point[i]);
        }
    }
     replot();
}


//是否需要清除之前的




Dist_Plot::~Dist_Plot(){

    delete td_pen;
    for(int i=0;i<m_curve.size();i++){
        delete m_curve[i];
    }
    delete legend;
    delete canvas;
}










