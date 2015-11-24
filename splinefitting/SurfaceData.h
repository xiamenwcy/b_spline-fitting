/*************************************************
* Copyright (c) 2014-����,���Ŵ�ѧ�����ͼ��ѧʵ����Graphics@XMU
* All rights reserved.
*
* �ļ����ƣ�SurfaceData.h 
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
#ifndef _SURFACEDATA_H_
#define _SURFACEDATA_H_


//Qt
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QObject>


#include "BSplineView.h"
#include "B_parameter.h"
#include "data_types.h"
#include <vector>
#include <cmath>
#include <iostream>
#include <algorithm>
#include <assert.h> //����debugģʽʹ��
#include <set>

//CGAL
#include <CGAL/Bbox_2.h>
#include <CGAL/Bbox_3.h>
#include <CGAL/Segment_2.h>
#include <CGAL/Triangle_2.h>
#include <CGAL/Iso_rectangle_2.h>
#include <CGAL/Triangle_2_Iso_rectangle_2_intersection.h>
#include <CGAL/Segment_2_Triangle_2_intersection.h>
#include <CGAL/Cartesian.h>
#include <CGAL/Point_2.h>
#include <CGAL/Polygon_2.h>

using namespace std;

const double  EPSILON = 10e-16;

typedef CGAL::Cartesian<double>                           Kernel;
typedef CGAL::Point_2<Kernel>                             Point_2; 
typedef CGAL::Segment_2<Kernel>                           Segment_2;
typedef  CGAL::Triangle_2<Kernel>                         Triangle_2;
typedef  CGAL::Iso_rectangle_2<Kernel>                    Iso_rectangle_2;
typedef CGAL::Polygon_2<Kernel>                           Polygon_2;
typedef CGAL::Bbox_2                                      Bbox_2;
/** Bbox_3(double x_min, double y_min, double z_min, double x_max, double y_max, double z_max) */																	
typedef CGAL::Bbox_3                                      Bbox_3; 

class CSurfaceData //:public  QObject
{
	//Q_OBJECT
public:
	CSurfaceData(void);
	~CSurfaceData(void);

    //original mesh
	bool read_mesh(const QString &fileName);  /**< �������� */
	int get_mesh_vertex_num();               /**< ���������ԭʼ���񶥵��� */
	int get_mesh_face_num();                 /**< ���������ԭʼ���������� */
	QString get_mesh_name();                  /**< ���������ԭʼ�����ļ��� */
	Mesh *get_original_mesh();               /**< ����ԭʼ���� */

    void   clear_data();                   /**<  ��������  */
	void setparameter(B_parameter *parameter_2); /**< ����B��������Ĳ���p,q,m,n */

	//���ӽڵ���
	void set_knots_adding_num(int num);    /**< ÿ��Ӧ�����ӵ���ֱ�ڵ����������û�ָ�� */
	void set_horizon_knots_addnum(int num);/**< ÿ��Ӧ�����ӵ�ˮƽ�ڵ����������û�ָ��*/
	bool add_vertical_knots();             /**< ������ֱ�ڵ��ߣ���u�� */
    bool add_horizon_knots();              /**< ����ˮƽ�ڵ��ߣ���v�� */
	void  vertical_range_query();           /**< ��ѯ��ֱ���εĶ��� */
	void  horizon_range_query();            /**< ��ѯˮƽ���εĶ��� */
	bool  rangequery_vertical;             /**< �궨vertical_range_query()����v_index�������ӵ�ִ�����,\see vertical_range_query(),horizon_range_query() */
	bool  rangequery_horizon;              /**< �궨horizon_range_query()����v_index�������ӵ�ִ�����,\see vertical_range_query(),horizon_range_query() */
	
   
	/** uknots��vknots��ˮƽ����ֱ�Ľڵ��ߣ����������ɸ�С���Σ�pΪ����һ�㣬�ҵ���������ڵľ���������� */
	int location(vector<double>& uknots,vector<double>& vknots,TexCoord& p);

	void   set_max_curvature(double curvature); /**< Ϊ�˷�ֹ������ʵ���ڹ����������������� */
	double get_max_meancurvature();             /**< ��������ƽ������  */
    double get_min_meancurvature();             /**< ������С��ƽ������  */
	B_parameter *getparameter();                /**< ����B��������Ĳ���p,q,m,n */
	Mesh *get_fitted_mesh();                    /**< ����������� */

	bool mesh_fitting();                       /**< ���ԭʼ��������������� */
    CBSplineSurfaceView* getbspline();          /**< ������������࣬���ھ���ļ�����Ϲ���  */
	vector<TexCoord>  get_domain();            /**< ����ԭʼ����������������� */

	Bbox_2 &get_box();                        /**< ����ԭʼ������������İ�Χ��,\see box*/
	Bbox_3 get_original_box();                 /**< ����ԭʼ����İ�Χ�� \see originalbox */
	Bbox_3 get_fitting_box();                  /**<  �����������İ�Χ�� \see fittingbox  */
    Bbox_3 get_error_box();                    /**< �����������İ�Χ�� \see errorbox */
	void   set_original_box(Bbox_3 box);       /**< ����ԭʼ����İ�Χ�� \see originalbox  */
    void   set_fitting_box(Bbox_3 box);        /**< �����������İ�Χ�� \see fittingbox */

	bool mesh_parameterization();            /**< ��������� */
	void compute_error();                    /**< ���������� */
	bool curvature_loading();                /**< �����Ƿ�����ɹ�? */
	bool fitting_completed();                /**< ����Ƿ����? */
	bool curvature_error_completed();        /**< ���ʻ�������Ƿ������� \see CSurfaceData::update_polymesh()*/
	void set_fitting_completed(bool state);     /**< ����������״̬ */
	void set_curvature_loadingstate(bool state); /**< ������������״̬ */
	bool compute_curvature(QStringList files);   /**< ��������*/
  

   /**
       * Triangle and  line segments intersect whether or not ?
	   * @param[out] length  intersecting line length.
	   * @param[in]  tri   A planar triangle.
	   * @param[in]  seg   A planar line segment.
       * @return The intersection results: True or False.
       */
	 bool intersection_st( Triangle_2 tri,Segment_2 seg,double *length); 

	void  update_curvature_color();

	/** \note ����������� */
	 void  update_knots(int k);
	
	/** �������ʵ����ڵ��� */
	bool  adjust_knots_by_curvature();
	/** ��������������ڵ��� */
	bool  adjust_knots_by_fitting_error();

	/**  ���¾�������polymesh,����ѯ���������а����������� */
	void   update_polymesh_and_query_tri();

	/**  ���¾�������polymesh,����ѯ���������а����Ķ��� */
	void   update_polymesh_and_query_point();

	/**  ������������*/
	MyMesh  build_polymesh();

    /** �޶������ʵĽڵ� */
	void  modification();

    /** �ҵ�����һ��������ľ������򣬲�ɾ�����ڵĽڵ���  */
	void  query_isolated_range();      

	bool  query_success;                 /**< query_isolated_range�����ı�־ */

    vector<double> uknots;               /**< ���е�u�ڵ� */
	vector<double> vknots;               /**< ���е�v�ڵ� */
	vector<double> uknots_new;           /**< ��������ÿ�������ӵ�u�ڵ� */
	vector<double> vknots_new;           /**< ��������ÿ�������ӵ�v�ڵ� */
	int unum;                            /**< uknots�ĸ���  */
	int vnum;                            /**< vknots�ĸ���  */

	 static int iter_num;              /**< ָʾ\see update_knots()��ִ�д��� */
	
	 double curaverage_;                /**< ���ʻ���ƽ��ֵ */
	

     vector<set<int> >  horizon_index; /**< ˮƽ���Σ��洢˳���Ǵ����浽���棬��С���󣬴洢�ڲ��ͱ߽�ڵ����*/
     vector<set<int> >  vertical_index;/**< ��ֱ���Σ��洢˳���Ǵ����ң���С���󣬴洢�ڲ��ͱ߽�ڵ���� */
  //   set<int>   test1;   //���������ĵ���
	 //set<int>   test2;   //���������ĵ���
	 //set<int>   test3;   //���������ĵ���

   

	void  set_max_error(double &err);         /**< �������������Ϊÿ����ϵ���ɫ */
    void  set_knots_iteration_times(int n);   /**<  ���ð������ʵ����ڵ��ߵĵ������� */

	double &get_mean_error();          /**< ���ؾ����������Ϣ mean_err */
	Max_Error &get_max_error();        /**< �����������������Ϣ max_err */
	double  get_max_knotcurerror();   /**< �������ľ������ʻ������ max_curerror */
    double  get_min_knotcurerror();   /**< ������С�ľ������ʻ������ min_curerror */

	Mesh *get_error_fitted_mesh();  /**< ����������� */
	void update_polymesh();         /**< ���°������ʷֲ��Ľڵ�����ɵ�polymesh���Ҳ�ѯ���ʻ��ֵ���� */
	MyMesh get_polymesh();          /**< ����polymesh  */
	QString  get_filename();        /**< ����������ļ���*/

private:

	Mesh *m_pOriginalMesh;     /**<  �����ԭʼ����  */
	int iter_times;            /**<  �������ʵ����ڵ��ߵĵ������� */
	int add_vertical_num;      /**< ÿ��Ӧ�����ӵ���ֱ�ڵ����������û�ָ�� */
    int add_horizon_num;       /**< ÿ��Ӧ�����ӵ�ˮƽ�ڵ����������û�ָ�� */
    B_parameter *parameter1;   /**< B���������p,q,m,n */
	bool  error_compute_ok;    /**< ����������Ƿ���� */
	CBSplineSurfaceView *bsurface; /**< ���������  */
	Mesh  *m_pFittedMesh;       /**< ��Ϻ������ */
	Mesh *m_pErrorMesh;         /**< ������������ɫ��������� */
	MyMesh polymesh;            /**< ��uknots,vknots��ɵľ����������� */

	QString fileName;            /**< ������ļ��� */

	vector<TexCoord> domain;      /**< ������������������ */
   

	Bbox_2 box;                   /**< ԭʼ������������İ�Χ�� */
	Bbox_3 originalbox;           /**< ԭʼ����İ�Χ�� */
	Bbox_3 fittingbox;            /**< �������İ�Χ�� */
	Bbox_3 errorbox;              /**< �������İ�Χ�� */

	int sample_num;              /**< ��������Ķ������ */

	double err_threshold;       /**< ���õ���������\see  KnotsViewer::error_fitting_view(),FittingMeshViewer::set_error_show() */
    Max_Error max_err;          /**< ����������,��������λ��,\see   CSurfaceData::compute_error() */
	double   max_err_real;      /**< ����ڰ�Χ����������ʵ��δ�޸ĵ���������� */
	double max_meancurvature;   /**< ����ƽ������ */
	double min_meancurvature;   /**< ��С��ƽ������ */
	double max_curerror;        /**< ����������� */
	double min_curerror;        /**< ��С��������� */
	double mean_err;            /**< ����ڰ�Χ�������ľ��������,\see CSurfaceData::compute_error() */
	double mean_e;              /**< ���������,\see CSurfaceData::compute_error() */
     bool   Curvature_loading;    /**< ������������Ƿ�ɹ� */  
	 bool   fitting_completed_;   /**< �������Ƿ���� */
	 bool    curvature_error;  /**< ������ʻ��ֵ�����Ƿ�������  */
	 void compute_box();       /**< ������������İ�Χ�� */

};

//--------------------------------------------------------
#endif