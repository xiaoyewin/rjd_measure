#ifndef DIST_PLOT_H
#define DIST_PLOT_H


#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_marker.h>
#include <qwt_plot.h>
#include <qwt_plot_curve.h>
#include <qwt_plot_canvas.h>
#include <qwt_legend_label.h>
#include <qwt_legend.h>
#include "configure.h"
#define ORIGIN_DATA_SIZE 11

#define ANGLE_VS_RATE  0.000002129


#define MAX_COLOR_NUM  18
class Dist_Plot : public QwtPlot {


    typedef struct _gradient_data{
          uint32_t  dist;
          double    height;
    } T_Gradient_Data;


    typedef struct _origin_dataBuf{
          T_Gradient_Data data[ORIGIN_DATA_SIZE];
          int8_t write_index;
          int8_t read_index;
          int8_t  occupy;  //表示已经占用多少个
    } T_Origin_DataBuf;


     Q_OBJECT
public:
    Dist_Plot( QWidget *parent = NULL);
    ~Dist_Plot();

    void update_data();
    void update_data_open_data();
    void update_data_default_data();
    void set_per_curve_color(short num);
    //设置每个线的颜色

    void paint_curve(QVector<double> &data);
    void clear_curve();


    int default_item_index;  //这个对应最新的索引

    QVector<double> clac_height(QVector<double> &src);


    double clac_curvity(QVector<double> &src);
    double clac_levelness(QVector<double> &src);



    int cur_stat;
    int cur_max_time;
    double cur_min_height;
    double cur_max_height;

    double height_tol; //高度容忍的差值


private:

    QVector<int> grad_height;
    int step_len;  //表示每步的步长,默认是30MM,
    int cur_tick;  //表示当前时间
    int curstep;

    QColor fix_color[MAX_COLOR_NUM];
    QVector<QwtPlotCurve *> m_curve;
    QPen  * td_pen;

    QwtLegend *legend;


    QwtPlotCanvas *canvas;

    QVector<QPointF> m_point[6];

    //初始化颜色
    void calc_origin_data(QVector<double> &data,double &min_value,double &max_value);


private slots:
    void legendClicked( const QVariant &itemInfo, bool on, int index);
};


#endif // A_PLOT_H


