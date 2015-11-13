/*************************************************
* Copyright (c) 2014-至今,厦门大学计算机图形学实验室Graphics@XMU
* All rights reserved.
*
* 文件名称：splineknotsviewer.h 
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

class KnotsViewer : public QGLViewer
{
	Q_OBJECT
public:
	KnotsViewer(void);
	~KnotsViewer(void);
	void set_surface_data(CSurfaceData *data);
	void clear_data();
	void set_knots_view(bool kv);
	void set_curvature_show(bool kv); 
	void error_fitting_view(bool kv);
	
	void update_view();
	
	void set_domain_mesh_view(bool dv);
	void set_error_curvature_view(bool ev);
	
	vector<double> uknot;
	vector<double> vknot;
protected:
	virtual void draw();
	virtual void init();
	virtual void paintGL() { update(); };
	virtual void paintEvent(QPaintEvent *event);
	void draw_curvature_colorbar(QPainter *painter);
	void draw_curvature_error_colorbar(QPainter *painter);
	void draw_fitting_error_colorbar(QPainter *painter);
private:
	CSurfaceData *surfacedata;
	B_parameter *parameter2;
	void draw_knots();
	
	void draw_domain_mesh();
	void draw_curvature_mesh();
	void draw_curvature_error_domain();
	void draw_fitting_error_domain();
	
	void set_scene(Bbox_3 &box);
	
	bool bknotsview;
	bool bdomainview;
	bool berror_show;
	double max_err;


	bool bmeshdomainview;
	bool bcurvature_show;
	bool berrordomainview;
	
};

//---------------------------------------------------------------
#endif

