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

/************************************************************************/
/* OpenMeshҪ��"Include MeshIO.hh before including a mesh type!"������
   ����MeshIO.hh������TriMesh_ArrayKernelT.hh����PolyMesh_ArrayKernelT.hh
   �ο���D:\Program Files (x86)\OpenMesh-3.2\include\OpenMesh\Core\IO\MeshIO.hh
*/
/************************************************************************/

// To use the IO facility of OpenMesh make sure that the include MeshIO.hh is included first.
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

/** MeshSubWindow central widget.
    It support OpenGL,include 2D drawing and 3D drawing.
*/
class MeshViewer:public QGLViewer
{
	Q_OBJECT        /** \note ֻ�ж������������ʹ���ź�-�ۻ���. */
public:
	MeshViewer();
	~MeshViewer(void);
	void set_surface_data(CSurfaceData *data);      /**< ����CSurfaceData */

	void update_view();                             /**< ����MeshSubWindow�Ĵ��� */

	void clear_data();                              /**< ������� */

	void set_origin_mesh_view(bool ov);             /**<����ԭʼ�����Ƿ���� */

	void set_curvature_show(bool cv);                /**<�������������Ƿ���� */

	bool write_mesh(QString &fileName, Mesh_Type type);/**< ������д���ļ� */
	
signals:
	
protected:
	virtual void draw();                            /**< ��ά��ͼ���� */
	virtual void init();                            /**< ��ͼ��ʼ������  */
	virtual void paintGL() { update(); };           /**< ���»�ͼ����*/
    virtual void paintEvent(QPaintEvent *event);    /**< ������ͼ������������ά����ά��ͼ*/
	void draw_curvature_colorbar(QPainter *painter); /**<����������ɫ�� */


private:
	void setDefaultMaterial();                        /**< ���ò��ʣ���������� */   

	void draw_original_mesh();                        /**< ����ԭʼ���� */
	
	void draw_curvature_mesh();                        /**< ������������ */
	
	void set_scene(Bbox_3 &box);                       /**< Set scene center and scene radius*/
	

	CSurfaceData *surfacedata;                         /**< Core surface data */     

	bool borigina_mesh_view;                           /**< ���ԭʼ�����Ƿ���� */

	bool bcurvature_show;                              /**< ������������Ƿ���� */
	
	double radius;                                     /**< scene radius */

	Bbox_3 bBox;                                       /**< scene bounding box */




};

//-----------------------------------------------------
#endif