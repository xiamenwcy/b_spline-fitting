/*************************************************
* Copyright (c) 2014-����,���Ŵ�ѧ�����ͼ��ѧʵ����Graphics@XMU
* All rights reserved.
*
* �ļ����ƣ�FittingMeshViewer.h 
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

#ifndef _FITTINGMESHVIEWER_H_
#define _FITTINGMESHVIEWER_H_

//OpenMesh
/************************************************************************/
/* OpenMeshҪ��"Include MeshIO.hh before including a mesh type!"������
   ����MeshIO.hh������TriMesh_ArrayKernelT.hh����PolyMesh_ArrayKernelT.hh
   �ο���D:\Program Files (x86)\OpenMesh-3.2\include\OpenMesh\Core\IO\MeshIO.hh
*/
/************************************************************************/
#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>

//libqglviewer
#include <QGLViewer/vec.h>
#include <QGLViewer/qglviewer.h>

//Qt
#include <QtGui/QPaintEvent>
#include <QtGui/QVector3D>

//CGAL
#include <CGAL/Bbox_2.h>
#include <CGAL/Bbox_3.h>

#include "SurfaceData.h"

using namespace qglviewer;
typedef CGAL::Bbox_3  Bbox_3;

class QPaintEvent;
class QPainter;

class FittingMeshViewer:public QGLViewer
{
	Q_OBJECT        //ֻ�ж������������ʹ���ź�-�ۻ���
public:
	FittingMeshViewer(void);
	~FittingMeshViewer(void);
	void set_surface_data(CSurfaceData *data);
	void update_fitting_view();
	void clear_data();
	void set_fitted_mesh_view(bool fv);
	void set_control_point(Array2 point);
     void set_error_show(bool ev);
	
	void set_max_error_point_show(bool mv);
	void set_controledge_show(bool cv);
	
	void set_adjustpoints_enabled(bool av);
	void set_adjustedmesh_enabled(bool av);
	bool write_mesh(QString &fileName, Mesh_Type type);
	
signals:
	void position_changed();
protected:
	virtual void draw();
	virtual void init();
	void draw_error_colorbar(QPainter *painter);
	virtual void paintGL() { update(); };
	virtual void paintEvent(QPaintEvent *event);
	

private:
	void setDefaultMaterial();
	void draw_original_mesh();
	void draw_fitted_mesh();
	void draw_control_mesh_orginal();
	void draw_error_mesh();
	void draw_max_error_point();
	
	void set_scene(Bbox_3 &box);
	

	CSurfaceData *surfacedata;
	Array2 ControlPoint;
	
	 double max_err;

	bool bfitted_mesh_view;

	  bool berror_show;

	  bool bcontroledge_show;
	  bool bmax_error_point_show;
	  bool badjustedmesh_show;
	  bool badjust;
	
	double radius;
	
	Bbox_3 bBox;
};

//--------------------------------------------
#endif