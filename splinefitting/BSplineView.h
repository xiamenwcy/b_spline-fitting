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
#include <exception>
#include <cmath>

using std::vector;
using namespace Eigen;

#ifndef DOXY_IGNORE_THIS
/**  Define my personal traits */
struct MyTraits : OpenMesh::DefaultTraits
{
	//  Let Point and Normal be a vector of doubles 
	typedef OpenMesh::Vec3d Point;  
	typedef OpenMesh::Vec3d Normal; 
	typedef OpenMesh::Vec2d TexCoord2D; 
};
#endif
/** Define my mesh with the new traits! */
typedef OpenMesh::TriMesh_ArrayKernelT<MyTraits>            Mesh;
typedef OpenMesh::PolyMesh_ArrayKernelT<MyTraits>           MyMesh;
typedef Mesh::TexCoord2D                                    TexCoord;/** \note{"不能写成TexCoord2D"} */

/** 定义一个近似于0的极限值  */
const double  EPSILON3 = 10e-16;

/**  三维空间点Point3D类.  */
class Point3D
{
public:
	double x,y,z;
public:
	Point3D()
	{
		x=0.;
		y=0.;
		z=0.;
	};
	Point3D(double _x,double _y,double _z)
	{
		x=_x;
		y=_y;
		z=_z;
	};
	Point3D(double s)
	{
		x=s;
		y=s;
		z=s;
	};
	Point3D& operator =(Point3D p1)
	{
		x=p1.x;
		y=p1.y;
		z=p1.z;
		return (*this);
	};
	Point3D& operator +=(Point3D p1)
	{
		x+=p1.x;
		y+=p1.y;
		z+=p1.z;
		return(*this);
	};
	Point3D& operator -=(Point3D p1)
	{
		x-=p1.x;
		y-=p1.y;
		z-=p1.z;
		return(*this);
	};
	Point3D& operator *=(double s)
	{
		x*=s;
		y*=s;
		z*=s;
		return(*this);
	};
	Point3D& operator /=(double s)
	{
		if(abs(s)<=EPSILON3)
			throw std::exception("The divisor should be  not 0");
		x/=s;
		y/=s;
		z/=s;
		return(*this);
	};
	Point3D operator +(Point3D p1)
	{
		Point3D t;
		t.x=x+p1.x;
		t.y=y+p1.y;
		t.z=z+p1.z;
		return(t);
	};
	Point3D operator -(Point3D p1)
	{
		Point3D t;
		t.x=x-p1.x;
		t.y=y-p1.y;
		t.z=z-p1.z;
		return(t);
	};
	Point3D operator *(double s)
	{
		Point3D t;
		t.x=x*s;
		t.y=y*s;
		t.z=z*s;
		return(t);
	};
	Point3D operator /(double s)
	{
		if(abs(s)<=EPSILON3)
			throw std::exception("The divisor should be  not 0");
		Point3D t;
		t.x=x/s;
		t.y=y/s;
		t.z=z/s;
		return(t);
	};
	friend Point3D operator *(double s,Point3D p1)
	{
		Point3D t;
		t.x=p1.x*s;
		t.y=p1.y*s;
		t.z=p1.z*s;
		return(t);
	};
	Mesh::Point toPoint()
	{
		return(Mesh::Point(x,y,z));
	};

};

/** Point3D类型的二维数组 */
typedef vector<vector<Point3D> >                            Array2;

/** B spline surface.
    contains:
	- compute control points.
	- fit mesh.
 */
class CBSplineSurfaceView 
{
public:
	CBSplineSurfaceView();
    ~CBSplineSurfaceView();

private:
     Array2  ControlPoint;  /**< 曲面控制顶点矩阵 */
	 B_parameter *parameter; /**< 曲面参数    */
	 Mesh  *m_pFittedMesh;   /**< 拟合好的曲面 */
	
public:
     /**
       * a normal member taking an argument and returning an bool value.
	   * It's used to be fitting b-spline,contains two procedure below:
	   *- First,compute control points.
	   *- Second,compute fitting mesh.
       * @param mesh original tri_mesh.
       * @see solvecontrolpoint()
       * @see BSpline()
	   * @see Base()
	   * @see getControlPoint()
	   * @see getFittingMesh()
       * @return The fitting results:True or False.
       */
	bool fitting_bspline( Mesh *mesh);
	/**  compute control points */
	bool solvecontrolpoint( Mesh *mesh);
    /**  get solved control points */
	Array2 getControlPoint();
	/**  get fitted mesh   */
	Mesh* getFittingMesh(); 
  
	  /**
       * a normal member taking two arguments and returning an Point3D .
	   * 采用公式法求解B-spline surface points.
       * @param u u knot.
	   * @param v v knot.
       * @see solvecontrolpoint()
	   * @see Base()
	   * @see getControlPoint()
       * @return B-spline surface point.
       */
	Point3D BSpline(double u,double v);

     /** 计算B样条基函数 \f$N_{i,k}(u)\f$ k=p+1,k为阶数，p为次数 */    
	double Base(int i,int k,double knot[],int num,double u); 

	//double  filter(double t);//过滤掉0.0000001后面的数字

	void  setBparameter(B_parameter *pa);
	
};
//---------------------------------------------------------
#endif