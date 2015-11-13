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

class SplineKnotsViewer : public QGLViewer
{
	Q_OBJECT
public:
	SplineKnotsViewer(void);
	~SplineKnotsViewer(void);
	void set_surface_data(CSurfaceData *data);
	void clear_data();
	void set_knots_view(bool kv);
	void set_curvature_show(bool kv); 
	void error_fitting_view(bool kv);
	//	void set_domain_view(bool dv);
	//	void set_centroid_view(bool cv);
	//	void set_basis_view(bool bv);
	//	void set_basis_sum_view(bool sv);
	// 	void set_basis_index(int &index);
	//	void set_basis_knots_view(bool bv);
	void update_view();
	//	void save_data();
	//	void set_basis_support_view(bool bv);
	//	void set_triangulation_view(bool tv);
	//	void set_delaunay_view(bool dv);
	//	void set_ddt_view(bool dv);
	//	void set_feature_blocks(bool fv);
	//	void set_density_threshold();
	void set_domain_mesh_view(bool dv);
	void set_error_curvature_view(bool ev);
	//	void set_density_domain_view(bool ev);
	//	void set_thintriangles_view(bool tv);
	//	void set_thintriangles_angle_view(bool av);
	//	void set_principal_direction_view(bool pv);
	//public slots:
	//	void triangular_test();
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
	//	void draw_centroidtrianles();
	//void draw_domain();
	//	void draw_basis_sum();
	void draw_domain_mesh();
	void draw_curvature_mesh();
	void draw_curvature_error_domain();
	void draw_fitting_error_domain();
	//	void draw_basis();
	//	void draw_basis_knots();
	void set_scene(Bbox_3 &box);
	//	int basis_index;
	bool bknotsview;
	bool bdomainview;
	bool berror_show;
	double max_err;
	//	bool bcentroidview;
	//	bool bbasisview;
	//	bool bbasissumview;
	//	bool bbasis_knots;
	//	bool bbasis_support;
	//	void draw_basis_suport();
	//	void draw_triangulation();
	//	bool btriangulation_view;
	//	bool bdelaunayview;
	//	bool bddtview;
	//	bool bfeatureview;
	bool bmeshdomainview;
	bool bcurvature_show;
	bool berrordomainview;
	//	bool bdensitydomainview;
	//	void draw_delaunay();
	//	void draw_ddt();
	//	void draw_feature_hull();
	//	int index;
	//	bool bthintrianglesview;
	//	void draw_thin_triangles();
	//	void draw_thin_triangles_angle();
	//	bool thintrianglesangle_view;
	//	bool bprincipaldirection_view;
	//	void draw_principal_directions();
};

//---------------------------------------------------------------
#endif

