#ifndef GEOMETRYALG_H
#define GEOMETRYALG_H

#include <cmath>
#include <vector>

const double EPSILON  = 0.000001;

// 2D Point
struct Vector2d
{
public:
    Vector2d()
    {
    }

    Vector2d(double dx, double dy)
    {
        x = dx;
        y = dy;
    }

    // ʸ����ֵ
    void set(double dx, double dy)
    {
        x = dx;
        y = dy;
    }

    // ʸ�����
    Vector2d operator + (const Vector2d& v) const
    {
        return Vector2d(x + v.x, y + v.y);
    }

    // ʸ�����
    Vector2d operator - (const Vector2d& v) const
    {
        return Vector2d(x - v.x, y - v.y);
    }

    //ʸ������
    Vector2d Scalar(double c) const
    {
        return Vector2d(c*x, c*y);
    }

    // ʸ�����
    double Dot(const Vector2d& v) const
    {
        return x * v.x + y * v.y;
    }

    //������ģ
    double Mod() const
    {
        return sqrt(x * x + y * y);
    }

    bool Equel(const Vector2d& v) const
    {
        if(abs(x-v.x) < EPSILON && abs(y-v.y)< EPSILON)
        {
            return true;
        }
        return false;
    }

    double x, y;
};

//�жϵ��ڶ������
bool Point_In_Polygon_2D(double x, double y, const std::vector<Vector2d> &POL);

class GeometryAlg
{
public:
    GeometryAlg();
};

#endif // GEOMETRYALG_H
