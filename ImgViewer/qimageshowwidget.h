#ifndef QIMAGESHOWWIDGET_H
#define QIMAGESHOWWIDGET_H

#include <QWidget>
#include <opencv2\opencv.hpp>
#include <vector>
#include "geometryalg.h"

class QImageShowWidget : public QWidget
{
    Q_OBJECT
public:
    explicit QImageShowWidget(QWidget *parent = nullptr);
    ~QImageShowWidget();

    bool LoadImage(const char* imagePath);

    void ImageBlend(const char* dstImgPath, int posX, int posY);

    void SetDraw(bool bDraw);

signals:

public slots:

protected:
    void paintEvent(QPaintEvent *);     //����
    void mousePressEvent(QMouseEvent *e);       //����
    void mouseMoveEvent(QMouseEvent *e);        //�ƶ�
    void mouseReleaseEvent(QMouseEvent *e);     //�ɿ�
    void mouseDoubleClickEvent(QMouseEvent *event);        //˫��

    void Release();

    void MVCBlend(int posX, int posY);
    void MVCBlendOptimize(int posX, int posY); //�������Ż�

    void CalBoundPoint(std::vector<Vector2d>& ROIBoundPointList);           //դ��һ���߶�
    void RasterLine(std::pair<Vector2d, Vector2d> line, std::vector<Vector2d>& linePointList);

    double threePointCalAngle(const Vector2d &P1, const Vector2d &O, const Vector2d &P2);

private:
    uchar* winBuf;      //�������buf
    int winWidth;      //�������ؿ�
    int winHeight;      //�������ظ�
    int winBandNum;      //������

    cv::Mat srcImg;
    cv::Mat dstImg;


    bool bDraw;             //�Ƿ��ڻ���״̬
    bool bLeftClick;            //�Ƿ��Ѿ���ʼ��������ͬʱ��ʶ�Ƿ�ʼ���л���
    bool bMove;             //�Ƿ��ڻ���ʱ������ƶ�״̬

    QVector<Vector2d> pointList;
    QPointF movePoint;
};

#endif // QIMAGESHOWWIDGET_H
