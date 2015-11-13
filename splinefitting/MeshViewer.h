/*************************************************
* Copyright (c) 2014-����,���Ŵ�ѧ�����ͼ��ѧʵ����Graphics@XMU
* All rights reserved.
*
* �ļ����ƣ�MeshViewer.h 
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

#ifndef _MESHVIEWER_H_
#define _MESHVIEWER_H_

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

#include "SurfaceData.h"

using namespace qglviewer;


class MeshViewer:public QGLViewer
{
	Q_OBJECT        //ֻ�ж������������ʹ���ź�-�ۻ���
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