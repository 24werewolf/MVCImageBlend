#include "qimageshowwidget.h"

#include <QPainter>
#include <QDebug>
#include <QMouseEvent>
#include <QTime>
#include <iostream>
#include <vector>

using namespace cv;
using namespace std;

QImageShowWidget::QImageShowWidget(QWidget *parent) : QWidget(parent)
{
    //��䱳��ɫ
    setAutoFillBackground(true);
    setBackgroundRole(QPalette::Base);

    winBuf = nullptr;
    winWidth = rect().width();
    winHeight = rect().height();
    winBandNum = 3;

    bDraw = false;
    bLeftClick = false;
    bMove = false;
    pointList.clear();
    setMouseTracking(true);
}

QImageShowWidget::~QImageShowWidget()
{
    if(winBuf)
    {
        delete[] winBuf;
        winBuf = nullptr;
    }
}

bool QImageShowWidget::LoadImage(const char* imagePath)
{
    //���ļ��ж�ȡ�ɻҶ�ͼ��
    srcImg = imread(imagePath);
    if (srcImg.empty())
    {
        fprintf(stderr, "Can not load image %s\n", imagePath);
        return false;
    }

    Release();

    winWidth = rect().width();
    winHeight = rect().height();
    size_t winBufNum = (size_t) winWidth * winHeight * winBandNum;
    winBuf = new uchar[winBufNum];
    memset(winBuf, 255, winBufNum*sizeof(uchar));

    for (int ri = 0; ri < srcImg.rows; ++ri)
    {
        for (int ci = 0; ci < srcImg.cols; ++ci)
        {
            for(int bi = 0; bi < winBandNum; bi++)
            {
                size_t m = (size_t) winWidth * winBandNum * ri + winBandNum * ci + bi;
                size_t n = (size_t) srcImg.cols * winBandNum * ri + winBandNum * ci + (winBandNum - 1 - bi);
                winBuf[m] = srcImg.data[n];
            }
        }
    }

    update();

    return true;
}

void QImageShowWidget::ImageBlend(const char* dstImgPath, int posX, int posY)
{
    dstImg = imread(dstImgPath);
    if (dstImg.empty())
    {
        fprintf(stderr, "Can not load image %s\n", dstImgPath);
        return;
    }

    MVCBlend(posX, posY);

    //imwrite("D:\\dst.jpg", dstImg);

    Release();

    winWidth = rect().width();
    winHeight = rect().height();
    size_t winBufNum = (size_t) winWidth * winHeight * winBandNum;
    winBuf = new uchar[winBufNum];
    memset(winBuf, 255, winBufNum*sizeof(uchar));

    for (int ri = 0; ri < dstImg.rows; ++ri)
    {
        for (int ci = 0; ci < dstImg.cols; ++ci)
        {
            for(int bi = 0; bi < winBandNum; bi++)
            {
                size_t m = (size_t) winWidth * winBandNum * ri + winBandNum * ci + bi;
                size_t n = (size_t) dstImg.cols * winBandNum * ri + winBandNum * ci + (winBandNum - 1 - bi);
                winBuf[m] = dstImg.data[n];
            }
        }
    }

    bDraw = false;
    update();
}

void QImageShowWidget::MVCBlend(int posX, int posY)
{
    QTime startTime = QTime::currentTime();

    //Step1:�ҵ��߽������е����ص�
    vector<Vector2d> ROIBoundPointList;
    CalBoundPoint(ROIBoundPointList);

    //Step2:���㷶Χ��ÿ����� mean-value coordinates
    size_t srcImgBufNum = static_cast<size_t>(srcImg.cols) * static_cast<size_t>(srcImg.rows);
    vector<vector<double>> MVC(srcImgBufNum);
    for(size_t i = 0; i < srcImgBufNum; i++)
    {
        MVC[i].resize(ROIBoundPointList.size()-1, 0);
    }
    vector<bool> clipMap(srcImgBufNum, true);           //��ʶ��Χ�ڵĵ�

    cout<<"��ʼ���� mean-value coordinates..." << endl;
    #pragma omp parallel for        //����OpenMP���м���
    for (int ri = 0; ri < srcImg.rows; ++ri)
    {
        for (int ci = 0; ci < srcImg.cols; ++ci)
        {
            //���Ƿ��ڶ������
            size_t m = static_cast<size_t>(srcImg.cols) * ri + ci;
            if(!Point_In_Polygon_2D(ci, ri, ROIBoundPointList))
            {
                clipMap[m] = false;
                continue;
            }

            //������MVC
            Vector2d P(ci, ri);
            vector<double> alphaAngle(ROIBoundPointList.size());
            for(size_t pi = 1; pi < ROIBoundPointList.size(); pi++)
            {
                alphaAngle[pi] = threePointCalAngle(ROIBoundPointList[pi-1], P, ROIBoundPointList[pi]);
            }
            alphaAngle[0] = alphaAngle[ROIBoundPointList.size()-1];


            for(size_t pi = 1; pi < ROIBoundPointList.size(); pi++)
            {
                double w_a = tan(alphaAngle[pi-1]/2) + tan(alphaAngle[pi]/2);
                double w_b = (ROIBoundPointList[pi-1] - P).Mod();
                MVC[m][pi-1] = w_a / w_b;
                if(_isnan(MVC[m][pi-1])==1)
                {
                    MVC[m][pi-1] = 0;
                }
            }

            double sum = 0;
            for(size_t pi = 0; pi < MVC[m].size(); pi++)
            {
                sum = sum + MVC[m][pi];
            }

            for(size_t pi = 0; pi < MVC[m].size(); pi++)
            {
                MVC[m][pi] = MVC[m][pi] / sum;
            }
        }
    }
    cout<<"������ɣ�" << endl;

    //Step3:����߽�����ز�ֵ
    vector<int> diff;
    for(size_t i = 0; i < ROIBoundPointList.size()-1; i++)
    {
        size_t l = (size_t) srcImg.cols * ROIBoundPointList[i].y + ROIBoundPointList[i].x;
        for(int bi = 0; bi < winBandNum; bi++)
        {
            size_t m = (size_t) dstImg.cols * winBandNum * (ROIBoundPointList[i].y + posY)+ winBandNum * (ROIBoundPointList[i].x + posX) + bi;
            size_t n = (size_t) srcImg.cols * winBandNum * ROIBoundPointList[i].y + winBandNum * ROIBoundPointList[i].x + bi;
            int d = (int)(dstImg.data[m]) - (int)(srcImg.data[n]);
            diff.push_back(d);
        }
        clipMap[l] = false;         //�ڶ���α��ϵĵ�û������MVC
    }

    //Step4:��ֵ����
    cout<<"��ʼ��ֵ����..." << endl;
    //Mat rMat(srcImg.rows, srcImg.cols, CV_64FC3);
    #pragma omp parallel for
    for (int ri = 0; ri < srcImg.rows; ++ri)
    {
        for (int ci = 0; ci < srcImg.cols; ++ci)
        {
            size_t l = (size_t) srcImg.cols * ri + ci;
            if(!clipMap[l])
            {
                continue;
            }

            vector<double> r(winBandNum, 0);

            for(size_t pi = 0; pi < MVC[l].size(); pi++)
            {
                for(int bi = 0; bi < winBandNum; bi++)
                {
                    r[bi] = r[bi] + MVC[l][pi] * diff[pi * winBandNum + bi];
                }
            }

            for(int bi = 0; bi < winBandNum; bi++)
            {
                size_t n = (size_t) srcImg.cols * winBandNum * ri + winBandNum * ci + bi;
                size_t m = (size_t) dstImg.cols * winBandNum * (ri + posY)+ winBandNum * (ci + posX) + bi;

                dstImg.data[m] = min(max(srcImg.data[n] + r[bi], 0.0), 255.0);
            }
        }
    }
    cout<<"��ֵ��ɣ�" << endl;

    QTime stopTime = QTime::currentTime();
    int elapsed = startTime.msecsTo(stopTime);
    cout<<"�ܽ������ʱ"<<elapsed<<"����";
}

void QImageShowWidget::CalBoundPoint(std::vector<Vector2d>& ROIBoundPointList)
{
    vector<pair<Vector2d, Vector2d>> lineList;
    for(int i = 0; i<pointList.size()-1; i++)
    {
        lineList.push_back(make_pair(pointList[i], pointList[i+1]));
    }

    //�������бߣ���դ��
    vector<Vector2d> tmpPointList;
    for(size_t i = 0; i < lineList.size(); i++)
    {
        std::vector<Vector2d> linePointList;
        RasterLine(lineList[i], linePointList);
        std::copy(linePointList.begin(), linePointList.end(), std::back_inserter(tmpPointList));
    }

    ROIBoundPointList.clear();
    ROIBoundPointList.push_back(pointList[0]);
    for(size_t i = 0; i< tmpPointList.size(); i++)
    {
        //�����һ��ֵ�Ƚϣ�ȥ��
        if(!tmpPointList[i].Equel(ROIBoundPointList[ROIBoundPointList.size()-1]))
        {
            ROIBoundPointList.push_back(tmpPointList[i]);
        }
    }
    if(!ROIBoundPointList[0].Equel(ROIBoundPointList[ROIBoundPointList.size()-1]))
    {
        ROIBoundPointList.push_back(ROIBoundPointList[0]);
    }
}

//դ��һ���߶�
void QImageShowWidget::RasterLine(std::pair<Vector2d, Vector2d> line, std::vector<Vector2d>& linePointList)
{
    Vector2d vecLine = line.second-line.first;
    double lineLength = vecLine.Mod();
    double step = 1.0;

    vector<Vector2d> tmpPointList;
    double curLength = 0;
    while(curLength<lineLength)
    {
        curLength = curLength + step;
        Vector2d P = line.first + vecLine.Scalar(curLength/lineLength);
        P.x = (int)(P.x + 0.5);
        P.y = (int)(P.y + 0.5);
        tmpPointList.push_back(P);
    }

    //������㣬�������յ�
    linePointList.push_back(line.first);
    for(size_t i = 0; i< tmpPointList.size(); i++)
    {
        //�����һ��ֵ�Ƚϣ�ȥ��
        if(!tmpPointList[i].Equel(linePointList[linePointList.size()-1]))
        {
            linePointList.push_back(tmpPointList[i]);
        }
    }
}

double QImageShowWidget::threePointCalAngle(const Vector2d &P1, const Vector2d &O, const Vector2d &P2)
{
    Vector2d OP1 = P1 - O;
    Vector2d OP2 = P2 - O;

    double cosAng = OP1.Dot(OP2) / OP1.Mod() / OP2.Mod();
    double ang = acos(cosAng);
    if( _isnan(ang)==1)
    {
        ang=0;
    }

    return ang;
}

void QImageShowWidget::SetDraw(bool bDraw)
{
    this->bDraw = bDraw;
    pointList.clear();
}

//����ʵ��paintEvent
void QImageShowWidget::paintEvent(QPaintEvent *)
{
    if(!winBuf)
    {
        return;
    }

    QImage::Format imgFomat = QImage::Format_RGB888;

    QPainter painter(this);
    QImage qImg(winBuf, winWidth, winHeight, winWidth*winBandNum, imgFomat);
    painter.drawPixmap(0, 0, QPixmap::fromImage(qImg));

    if(bDraw)
    {
       painter.setPen(QColor(255,0,0));
       QVector<QLineF> lines;
       for(int i = 0; i<pointList.size()-1; i++)
       {
           QLineF line(QPointF(pointList[i].x, pointList[i].y), QPointF(pointList[i+1].x, pointList[i+1].y));
           lines.push_back(line);
       }
       if(bMove&&pointList.size()>0)
       {
           QLineF line(QPointF(pointList[pointList.size()-1].x, pointList[pointList.size()-1].y), movePoint);
           lines.push_back(line);
       }
       painter.drawLines(lines);
    }
}

//����
void QImageShowWidget::mousePressEvent(QMouseEvent *e)
{
    if(bDraw)
    {
        if(!bLeftClick)
        {
            pointList.clear();
            bLeftClick = true;
        }
    }
    //qDebug()<<"Press";
}

//�ƶ�
void QImageShowWidget::mouseMoveEvent(QMouseEvent *e)
{
    if(bDraw&&bLeftClick)
    {
        movePoint = e->pos();
        bMove = true;
        this->update();
    }
    //qDebug()<<"Move";
}

//�ɿ�
void QImageShowWidget::mouseReleaseEvent(QMouseEvent *e)
{
    if(bDraw&&bLeftClick)
    {
        pointList.push_back(Vector2d(e->x(), e->y()));
        bMove = false;
        this->update();
    }
    //qDebug()<<"Release";
}

//˫��
void QImageShowWidget::mouseDoubleClickEvent(QMouseEvent *event)
{
    if(bDraw)
    {
        bLeftClick = false;
        pointList.push_back(pointList[0]);
        this->update();
        //singalDrawOver();
    }
    //qDebug()<<"DoubleClick";
}

void QImageShowWidget::Release()
{
    if(winBuf)
    {
        delete[] winBuf;
        winBuf = nullptr;
    }
}
