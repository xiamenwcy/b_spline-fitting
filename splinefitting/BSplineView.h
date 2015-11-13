/*************************************************
* Copyright (c) 2014-����,���Ŵ�ѧ�����ͼ��ѧʵ����Graphics@XMU
* All rights reserved.
*
* �ļ����ƣ�BSplineView.h 
* ժҪ������������ں���
*
* ��ǰ�汾��2.0
* ���ߣ�������
* ������ڣ�2015��11��10��
*
* ȡ���汾��1.9
* ԭ���ߣ�������
* ������ڣ�2015��4��1��
*/
#ifndef _BSplineView_H_
#define _BSplineView_H_


//Eigen
#include <Eigen/Eigen>

#include "B_parameter.h"

//OpenMesh
/************************************************************************/
/* OpenMeshҪ��"Include MeshIO.hh before including a mesh type!"������
   ����MeshIO.hh������TriMesh_ArrayKernelT.hh����PolyMesh_ArrayKernelT.hh
   �ο���D:\Program Files (x86)\OpenMesh-3.2\include\OpenMesh\Core\IO\MeshIO.hh
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
typedef Mesh::TexCoord2D                                    TexCoord;/** \note{"����д��TexCoord2D"} */

/** ����һ��������0�ļ���ֵ  */
const double  EPSILON3 = 10e-16;

/**  ��ά�ռ��Point3D��.  */
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

/** Point3D���͵Ķ�ά���� */
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
     Array2  ControlPoint;  /**< ������ƶ������ */
	 B_parameter *parameter; /**< �������    */
	 Mesh  *m_pFittedMesh;   /**< ��Ϻõ����� */
	
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
	   * ���ù�ʽ�����B-spline surface points.
       * @param u u knot.
	   * @param v v knot.
       * @see solvecontrolpoint()
	   * @see Base()
	   * @see getControlPoint()
       * @return B-spline surface point.
       */
	Point3D BSpline(double u,double v);

     /** ����B���������� \f$N_{i,k}(u)\f$ k=p+1,kΪ������pΪ���� */    
	double Base(int i,int k,double knot[],int num,double u); 

	//double  filter(double t);//���˵�0.0000001���������

	void  setBparameter(B_parameter *pa);
	
};
//---------------------------------------------------------
#endif