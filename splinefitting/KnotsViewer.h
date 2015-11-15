/*************************************************
* Copyright (c) 2014-����,���Ŵ�ѧ�����ͼ��ѧʵ����Graphics@XMU
* All rights reserved.
*
* �ļ����ƣ�splineknotsviewer.h 
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

#ifndef _SPLINEKNOTSVIEWER_H_
#define _SPLINEKNOTSVIEWER_H_

//CGAL
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Bbox_3.h>

//libqglviewer
#include <QGLViewer/qglviewer.h>

//Qt
#include <QMenu>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QMap>
#include <QCursor>

//STL
#include<iostream>
#include <vector>

#include "surfacedata.h"


using namespace qglviewer;
using namespace std;

typedef CGAL::Bbox_3                                 Bbox_3;


/** KnotSubWindow central widget.
    It support OpenGL,include 2D drawing and 3D drawing.
*/
class KnotsViewer : public QGLViewer
{
	Q_OBJECT               /** \note ֻ�ж������������ʹ���ź�-�ۻ���. */
public:
	KnotsViewer(void);
	~KnotsViewer(void);
	void set_surface_data(CSurfaceData *data);   /**< ����CSurfaceData */

	void clear_data();                            /**< ������� */

	void set_knots_view(bool kv);                  /**< ���ýڵ��ߴ����Ƿ���� */

	void set_curvature_show(bool kv);               /**< �������������Ƿ����  */

	void error_fitting_view(bool kv);               /**< ���������������Ƿ���� */
	
	void update_view();                             /**< ����KnotSubWindow���� */
	
	void set_domain_mesh_view(bool dv);              /**< ���ò�������ԭʼ�����Ƿ���� */
	     
	void set_error_curvature_view(bool ev);           /**< �������ʻ�������Ƿ����  */
	
	vector<double> uknot;                             /**< u��ڵ�����*/
	vector<double> vknot;                             /**< v��ڵ�����*/
protected: 
	virtual void draw();                               /**< ��ά��ͼ���� */
	virtual void init();                                /**< ��ͼ��ʼ������  */
	virtual void paintGL() { update(); };               /**< ���»�ͼ����*/
	virtual void paintEvent(QPaintEvent *event);        /**< ������ͼ������������ά����ά��ͼ*/

	void draw_curvature_colorbar(QPainter *painter);      /**<����������ɫ�� */

	void draw_curvature_error_colorbar(QPainter *painter); /**<�������ʻ��������ɫ�� */

	void draw_fitting_error_colorbar(QPainter *painter);    /**<������������ɫ�� */
private:
	CSurfaceData *surfacedata;                           /**< Core surface data */   

	B_parameter *parameter2;                              /**< B spline parameter:p,q,m,n ... */     

	void draw_knots();                          /**< ���ƽڵ��� */     
	
	void draw_domain_mesh();                    /**< ���Ʋ�������ԭʼ���� */     

	void draw_curvature_mesh();                 /**< ������������ */     

	void draw_curvature_error_domain();         /**< ������������������ */     

	void draw_fitting_error_domain();            /**< �������������� */     

	void set_scene(Bbox_3 &box);                 /**< Set scene center and scene radius*/
	
	bool bknotsview;     /**< ��ǽڵ��ߴ����Ƿ���� */

	bool berror_show;      /**< ����������Ƿ���� */

	bool bmeshdomainview;  /**< ��ǲ�������ԭʼ�����Ƿ���� */

	bool bcurvature_show;  /**< ������������Ƿ���� */

	bool berrordomainview; /**< ������ʻ�����������Ƿ���� */

	double max_err;        /**< ��������� */
};

//---------------------------------------------------------------
#endif

