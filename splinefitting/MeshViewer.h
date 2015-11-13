/*************************************************
* Copyright (c) 2014-至今,厦门大学计算机图形学实验室Graphics@XMU
* All rights reserved.
*
* 文件名称：MeshViewer.h 
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

#ifndef _MESHVIEWER_H_
#define _MESHVIEWER_H_

//OpenMesh
/************************************************************************/
/* OpenMesh要求"Include MeshIO.hh before including a mesh type!"，所以
   先有MeshIO.hh，后有TriMesh_ArrayKernelT.hh或者PolyMesh_ArrayKernelT.hh
   参考：D:\Program Files (x86)\OpenMesh-3.2\include\OpenMesh\Core\IO\MeshIO.hh
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

#include "SurfaceData.h"

using namespace qglviewer;


class MeshViewer:public QGLViewer
{
	Q_OBJECT        //只有定义了这个才能使用信号-槽机制
public:
	MeshViewer();
	~MeshViewer(void);
	void set_surface_data(CSurfaceData *data);
	void update_view();

	void clear_data();
	void set_origin_mesh_view(bool ov);

	void set_curvature_show(bool cv);

	void set_adjustpoints_enabled(bool av);
	void set_adjustedmesh_enabled(bool av);
	bool write_mesh(QString &fileName, Mesh_Type type);
	
signals:
	void position_changed();
protected:
	virtual void draw();
	virtual void init();
	virtual void paintGL() { update(); };
    virtual void paintEvent(QPaintEvent *event);
	void draw_curvature_colorbar(QPainter *painter);
	/*
	void mousePressEvent(QMouseEvent *event);
	void mouseMoveEvent(QMouseEvent *event);
	void mouseReleaseEvent(QMouseEvent *event);*/

private:
	void setDefaultMaterial();
	void draw_original_mesh();
	
	void draw_curvature_mesh();
	
	void set_scene(Bbox_3 &box);
	

	CSurfaceData *surfacedata;

	bool borigina_mesh_view;

	bool bcurvature_show;
	
	  bool badjustedmesh_show;
	  bool badjust;
	
	double radius;

	Bbox_3 bBox;













};

//-----------------------------------------------------
#endif