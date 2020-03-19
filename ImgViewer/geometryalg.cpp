#include "geometryalg.h"

#include <algorithm>

//�жϵ����߶���
bool IsPointOnLine(double px0, double py0, double px1, double py1, double px2, double py2)
{
    bool flag = false;
    double d1 = (px1 - px0) * (py2 - py0) - (px2 - px0) * (py1 - py0);
    if ((abs(d1) < EPSILON) && ((px0 - px1) * (px0 - px2) <= 0) && ((py0 - py1) * (py0 - py2) <= 0))
    {
        flag = true;
    }
    return flag;
}

//�ж����߶��ཻ
bool IsIntersect(double px1, double py1, double px2, double py2, double px3, double py3, double px4, double py4)
{
    bool flag = false;
    double d = (px2 - px1) * (py4 - py3) - (py2 - py1) * (px4 - px3);
    if (d != 0)
    {
        double r = ((py1 - py3) * (px4 - px3) - (px1 - px3) * (py4 - py3)) / d;
        double s = ((py1 - py3) * (px2 - px1) - (px1 - px3) * (py2 - py1)) / d;
        if ((r >= 0) && (r <= 1) && (s >= 0) && (s <= 1))
        {
            flag = true;
        }
    }
    return flag;
}

//�жϵ��ڶ������
bool Point_In_Polygon_2D(double x, double y, const std::vector<Vector2d> &POL)
{
    bool isInside = false;
    int count = 0;

    //
    double minX = DBL_MAX;
    for (int i = 0; i < POL.size(); i++)
    {
        minX = std::min(minX, POL[i].x);
    }

    //
    double px = x;
    double py = y;
    double linePoint1x = x;
    double linePoint1y = y;
    double linePoint2x = minX -10;			//ȡ��С��Xֵ��С��ֵ��Ϊ���ߵ��յ�
    double linePoint2y = y;

    //����ÿһ����
    for (int i = 0; i < POL.size() - 1; i++)
    {
        double cx1 = POL[i].x;
        double cy1 = POL[i].y;
        double cx2 = POL[i + 1].x;
        double cy2 = POL[i + 1].y;

        if (IsPointOnLine(px, py, cx1, cy1, cx2, cy2))
        {
            return true;
        }

        if (fabs(cy2 - cy1) < EPSILON)   //ƽ�����ཻ
        {
            continue;
        }

        if (IsPointOnLine(cx1, cy1, linePoint1x, linePoint1y, linePoint2x, linePoint2y))
        {
            if (cy1 > cy2)			//ֻ��֤�϶˵�+1
            {
                count++;
            }
        }
        else if (IsPointOnLine(cx2, cy2, linePoint1x, linePoint1y, linePoint2x, linePoint2y))
        {
            if (cy2 > cy1)			//ֻ��֤�϶˵�+1
            {
                count++;
            }
        }
        else if (IsIntersect(cx1, cy1, cx2, cy2, linePoint1x, linePoint1y, linePoint2x, linePoint2y))   //�Ѿ��ų�ƽ�е����
        {
            count++;
        }
    }

    if (count % 2 == 1)
    {
        isInside = true;
    }

    return isInside;
}

// v1 = Cross(AB, AC)
// v2 = Cross(AB, AP)
// �ж�ʸ��v1��v2�Ƿ�ͬ��
bool SameSide(Vector3d A, Vector3d B, Vector3d C, Vector3d P)
{
    Vector3d AB = B - A;
    Vector3d AC = C - A;
    Vector3d AP = P - A;

    Vector3d v1 = AB.Cross(AC);
    Vector3d v2 = AB.Cross(AP);

    // v1 and v2 should point to the same direction
    return v1.Dot(v2) >= 0 ;
    //return v1.Dot(v2) > 0;
}

// �жϵ�P�Ƿ���������ABC��(ͬ��)
bool PointinTriangle(Vector3d A, Vector3d B, Vector3d C, Vector3d P)
{
    return SameSide(A, B, C, P) && SameSide(B, C, A, P) && SameSide(C, A, B, P);
}

bool Point_In_Triangle_2D(const Vector2d & A, const Vector2d & B, const Vector2d & C, const Vector2d & P)
{
    return  (B.x - A.x) * (P.y - A.y) > (B.y - A.y) * (P.x - A.x) &&
            (C.x - B.x) * (P.y - B.y) > (C.y - B.y) * (P.x - B.x) &&
            (A.x - C.x) * (P.y - C.y) > (A.y - C.y) * (P.x - C.x) ? false : true;
}


//�����������ķ�����
void Cal_Normal_3D(const Vector3d& v1, const Vector3d& v2, const Vector3d& v3, Vector3d &vn)
{
    //v1(n1,n2,n3);
    //ƽ�淽��: na * (x �C n1) + nb * (y �C n2) + nc * (z �C n3) = 0 ;
    double na = (v2.y - v1.y)*(v3.z - v1.z) - (v2.z - v1.z)*(v3.y - v1.y);
    double nb = (v2.z - v1.z)*(v3.x - v1.x) - (v2.x - v1.x)*(v3.z - v1.z);
    double nc = (v2.x - v1.x)*(v3.y - v1.y) - (v2.y - v1.y)*(v3.x - v1.x);

    //ƽ�淨����
    vn.Set(na, nb, nc);
}

//��֪�ռ�������ɵ����������ĳ���Zֵ
void CalPlanePointZ(const Vector3d& v1, const Vector3d& v2, const Vector3d& v3, Vector3d& vp)
{
    Vector3d vn;
    Cal_Normal_3D(v1, v2, v3, vn);

    if (vn.z != 0)				//���ƽ��ƽ��Z��
    {
        vp.z = v1.z - (vn.x * (vp.x - v1.x) + vn.y * (vp.y - v1.y)) / vn.z;			//�㷨ʽ���
    }
}

GeometryAlg::GeometryAlg()
{

}
