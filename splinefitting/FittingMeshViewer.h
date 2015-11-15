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
/** To use the IO facility of OpenMesh make sure that the include MeshIO.hh is included first.*/
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

/** FitSubWindow central widget.
    It support OpenGL,include 2D drawing and 3D drawing.
*/
class FittingMeshViewer:public QGLViewer
{
	Q_OBJECT           /** \note ֻ�ж������������ʹ���ź�-�ۻ���. */
public:
	FittingMeshViewer(void);
	~FittingMeshViewer(void);
	void set_surface_data(CSurfaceData *data);
	void update_fitting_view();
	void clear_data();
	void set_fitted_mesh_view(bool fv);
	void set_control_point(Array2 point);   /**< ���ÿ��ƶ���*/

    void set_error_show(bool ev);            /**< ������������Ƿ���ʾ */  
	void set_max_error_point_show(bool mv);  /**< �������������Ƿ���ʾ*/
	void set_controledge_show(bool cv);      /**< ���ÿ��������Ƿ���ʾ */

	bool write_mesh(QString &fileName, Mesh_Type type);
	
signals:
	
protected:
	virtual void draw();
	virtual void init();
	virtual void paintGL() { update(); };
	virtual void paintEvent(QPaintEvent *event);

    void draw_error_colorbar(QPainter *painter); /**< ������������ɫ�� */
	

private:
	void setDefaultMaterial();
	void draw_fitted_mesh();             /**< ����������� */
	void draw_control_mesh_orginal();    /**< ���ƿ�������*/
	void draw_error_mesh();              /**< ����������� */
	void draw_max_error_point();         /**< ��������������*/
	
	void set_scene(Bbox_3 &box);         /**< ���ó����뾶������*/
	

	CSurfaceData *surfacedata;
	Array2 ControlPoint;               /**< ���ƶ������ */
	
	 double max_err;                    /**< ���������*/

	bool bfitted_mesh_view;             /**< ��������Ƿ����*/

	  bool berror_show;                  /**< �����������Ƿ����*/

	  bool bcontroledge_show;             /**< ��Ͽ��������Ƿ����*/

	  bool bmax_error_point_show;         /**< �����������Ƿ����*/

	
	 double radius;    /**< �����뾶*/           
	 
	  Bbox_3 bBox;     /**< ������Χ��*/
};

//--------------------------------------------
#endif