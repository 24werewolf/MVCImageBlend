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

    bool operator == (const Vector2d& v) const
    {
        if (abs(x - v.x) < EPSILON && abs(y - v.y) < EPSILON)
        {
            return true;
        }
        return false;
    }

    bool operator < (const Vector2d& v) const
    {
        if (abs(x - v.x) < EPSILON)
        {
            return y < v.y ? true : false;
        }
        return x<v.x ? true : false;
    }

    double x, y;
};

//��άdoubleʸ��
struct Vector3d
{
    double x, y, z;

    Vector3d()
    {
        x = 0.0;
        y = 0.0;
        z = 0.0;
    }
    Vector3d(double dx, double dy, double dz)
    {
        x = dx;
        y = dy;
        z = dz;
    }
    Vector3d(Vector2d &v)
    {
        x = v.x;
        y = v.y;
        z = 0;
    }

    // ʸ����ֵ
    void Set(double dx, double dy, double dz)
    {
        x = dx;
        y = dy;
        z = dz;
    }

    // ʸ�����
    Vector3d operator + (const Vector3d& v) const
    {
        return Vector3d(x + v.x, y + v.y, z + v.z);
    }

    // ʸ�����
    Vector3d operator - (const Vector3d& v) const
    {
        return Vector3d(x - v.x, y - v.y, z - v.z);
    }

    //ʸ������
    Vector3d Scalar(double c) const
    {
        return Vector3d(c*x, c*y, c*z);
    }

    // ʸ�����
    double Dot(const Vector3d& v) const
    {
        return x * v.x + y * v.y + z * v.z;
    }

    // ʸ�����
    Vector3d Cross(const Vector3d& v) const
    {
        return Vector3d(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
    }
};

//�жϵ��ڶ������
bool Point_In_Polygon_2D(double x, double y, const std::vector<Vector2d> &POL);

//�жϵ�������������
bool Point_In_Triangle_2D(const Vector2d & A, const Vector2d & B, const Vector2d & C, const Vector2d & P);

//��֪�ռ�������ɵ����������ĳ���Zֵ
void CalPlanePointZ(const Vector3d& v1, const Vector3d& v2, const Vector3d& v3, Vector3d& vp);

// �жϵ�P�Ƿ���������ABC��(ͬ��)
bool PointinTriangle(Vector3d A, Vector3d B, Vector3d C, Vector3d P);

class GeometryAlg
{
public:
    GeometryAlg();
};

#endif // GEOMETRYALG_H
