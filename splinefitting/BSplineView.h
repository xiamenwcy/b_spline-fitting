/*************************************************
* Copyright (c) 2014-至今,厦门大学计算机图形学实验室Graphics@XMU
* All rights reserved.
*
* 文件名称：BSplineView.h 
* 摘要：主函数，入口函数
*
* 当前版本：2.0
* 作者：王财勇
* 完成日期：2015年11月10日
*
* 取代版本：1.9
* 原作者：王财勇
* 完成日期：2015年4月1日
*/
#ifndef _BSplineView_H_
#define _BSplineView_H_


//Eigen
#include <Eigen/Eigen>

#include "B_parameter.h"

//OpenMesh
/************************************************************************/
/* OpenMesh要求"Include MeshIO.hh before including a mesh type!"，所以
   先有MeshIO.hh，后有TriMesh_ArrayKernelT.hh或者PolyMesh_ArrayKernelT.hh
   参考：D:\Program Files (x86)\OpenMesh-3.2\include\OpenMesh\Core\IO\MeshIO.hh
*/
/************************************************************************/
#include <OpenMesh/Core/IO/MeshIO.hh>   
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>
#include <OpenMesh/Core/Mesh/PolyMesh_ArrayKernelT.hh>
#include <OpenMesh/Core/Geometry/VectorT.hh>

#include <vector>

using std::vector;
using namespace Eigen;

#ifndef DOXY_IGNORE_THIS
// Define my personal traits
struct MyTraits : OpenMesh::DefaultTraits
{
	// Let Point and Normal be a vector of doubles
	typedef OpenMesh::Vec3d Point;
	typedef OpenMesh::Vec3d Normal;
	typedef OpenMesh::Vec2d TexCoord2D;
};
#endif
// Define my mesh with the new traits!
typedef OpenMesh::TriMesh_ArrayKernelT<MyTraits>            Mesh;
typedef OpenMesh::PolyMesh_ArrayKernelT<MyTraits>           MyMesh;
typedef Mesh::TexCoord2D                                    TexCoord;//不能写成TexCoord2D 

class Point3D//三维空间点Point3D类
{
public:
	double x,y,z;
	Point3D(){x=0;y=0;z=0;};
	Point3D(double _x,double _y,double _z){x=_x;y=_y;z=_z;};
	Point3D(double s){x=s;y=s;z=s;};
	Point3D operator =(Point3D p1){x=p1.x;y=p1.y;z=p1.z;return (*this);};
	Point3D operator +=(Point3D p1){x+=p1.x;y+=p1.y;z+=p1.z;return(*this);};
	Point3D operator -=(Point3D p1){x-=p1.x;y-=p1.y;z-=p1.z;return(*this);};
	Point3D operator *=(double s){x*=s;y*=s;z*=s;return(*this);};
	Point3D operator /=(double s){x/=s;y/=s;z/=s;return(*this);};
	Point3D operator +(Point3D p1){Point3D t;t.x=x+p1.x;t.y=y+p1.y;t.z=z+p1.z;return(t);};
	Point3D operator -(Point3D p1){Point3D t;t.x=x-p1.x;t.y=y-p1.y;t.z=z-p1.z;return(t);};
	Point3D operator *(double s){Point3D t;t.x=x*s;t.y=y*s;t.z=z*s;return(t);};
	Point3D operator /(double s){Point3D t;t.x=x/s;t.y=y/s;t.z=z/s;return(t);};
	friend Point3D operator *(double s,Point3D p1){Point3D t;t.x=p1.x*s;t.y=p1.y*s;t.z=p1.z*s;return(t);};
	Mesh::Point toPoint(){return(Mesh::Point(x,y,z));};

};

typedef vector<vector<Point3D> >                            Array2;

class CBSplineSurfaceView 
{
public:
	CBSplineSurfaceView();

private:
     Array2  ControlPoint;//曲面角点的信息矩阵
	 B_parameter *parameter;
	 Mesh  *m_pFittedMesh;
	
public:
	bool fitting_bspline( Mesh *mesh);//mesh为原始网格,先计算控制顶点，再计算拟合网格m_pFittedMesh
	Mesh* getFittingMesh(); 
	bool solvecontrolpoint( Mesh *mesh);
	Array2 getControlPoint();

public:
	Point3D BSpline(double u,double v);//采用公式法
	double Base(int i,int k,double knot[],int num,double u);//计算B样条基函数k=p+1      
	//double  filter(double t);//过滤掉0.0000001后面的数字
	void  setBparameter(B_parameter *pa);
	


public:
	virtual ~CBSplineSurfaceView();
};
//---------------------------------------------------------
#endif