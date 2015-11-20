/*************************************************
* Copyright (c) 2014-至今,厦门大学计算机图形学实验室Graphics@XMU
* All rights reserved.
*
* 文件名称：SurfaceData.h 
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
#include <assert.h> //仅供debug模式使用
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

//ANN
#include <ANN/ANN.h>					// ANN declarations

using namespace std;

const double  EPSILON = 10e-16;
const double  EPSILON2 = 10e-5;

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
	bool read_mesh(const QString &fileName);
	int get_mesh_vertex_num();
	int get_mesh_face_num();
	QString get_mesh_name();
	Mesh *get_original_mesh();

    void   clear_data();                   /**<  清理数据  */
	void setparameter(B_parameter *parameter_2);

	//增加节点线
	void set_knots_adding_num(int num);    /**< 每次应该增加的竖直节点线数，由用户指定 */
	void set_horizon_knots_addnum(int num);/**< 每次应该增加的水平节点线数，由用户指定*/
	bool add_knots();                      /**< 增加竖直节点线，即u线,\see adjust_vertical_knots() */
    bool add_horizon_knots();              /**< 增加水平节点线，即v线 \see adjust_horizon_knots()  */
	void  adjust_vertical_knots();         /**< 具体实施竖直节点的增加 */
	void  adjust_horizon_knots();          /**< 具体实施水平节点的增加 */
	void  range_query();                   /**< 查询每个条形的顶点 */
	void  range_query2();                  /**< 修订节点线         */
	bool  rangequery_vertical;             /**< 标定adjust_vertical_knots()关于range_query()的执行情况*/
	bool  rangequery_horizon;              /**< 标定adjust_horizon_knots()关于range_query()的执行情况*/

	/** uknots和vknots是水平和竖直的节点线，构成了若干个小矩形，p为其中一点，找到这个点所在的矩形区域序号 */
	int location(vector<double>& uknots,vector<double>& vknots,TexCoord& p);

	int   sum(std::vector<KnotData> &vec);
	void   set_max_curvature(double curvature);
	double get_max_meancurvature();
    double get_min_meancurvature();
	B_parameter *getparameter();
	Mesh *get_fitted_mesh();

	bool mesh_fitting();
    CBSplineSurfaceView* getbspline();
	vector<TexCoord>  get_domain();

	Bbox_2 &get_box();
	Bbox_3 get_original_box();
	Bbox_3 get_fitting_box();
    Bbox_3 get_error_box();
	void   set_original_box(Bbox_3 box);
    void   set_fitting_box(Bbox_3 box);

	bool mesh_parameterization();
	void compute_error();
	bool curvature_loading();
	bool fitting_completed();
	bool curvature_error_completed();
	void set_fitting_completed(bool state);
	void set_curvature_loadingstate(bool state);
	bool compute_curvature(QStringList files);
    int  label;

   /**
       * Triangle and  line segments intersect whether or not ?
	   * @param[out] length  intersecting line length.
	   * @param[in]  tri   A planar triangle.
	   * @param[in]  seg   A planar line segment.
       * @return The intersection results: True or False.
       */
	 bool intersection_st( Triangle_2 tri,Segment_2 seg,double *length); 

	void  update_curvature_color();

	/** \note 曲率拟合所需 */
	 void  update_knots(int k);

    /** \note 误差拟合所需 */
     void  update_knots2(int k);
	
	/** 按照曲率调整节点线 */
	bool  adjust_knots_by_curvature();
	/** 按照拟合误差调整节点线 */
	bool  adjust_knots_by_fitting_error();
    /** 扰动与修订不合适的节点 */
	void  modification();

    vector<double> uknots;
	vector<double> vknots;
	vector<knot_info> uknots_new;
	vector<knot_info> vknots_new;//用来储存每次新增加的节点线
	int unum;
	int vnum;

	 static int iter_num;
	 ANNkd_tree*			kdTree;		/**< ANN search    */
	 double curaverage_;                /**< 曲率积分平均值 */
	 double erroraverage_;              /**< 误差积分平均值 */

     vector<vector<int> >  horizon_index; /**< 水平条形，存储顺序是从下面到上面，从小到大，存储内部和边界节点序号*/
     vector<vector<int> >  vertical_index;/**< 竖直条形，存储顺序是从左到右，从小到大，存储内部和边界节点序号 */
  //   set<int>   test1;   //检测面包含的点数
	 //set<int>   test2;   //检测面包含的点数
	 //set<int>   test3;   //检测面包含的点数

   

	void  set_max_error(double &err);     /**< 设置最大误差，并且为每个拟合点着色 */
    void  set_knots_iteration_times(int n);

	double &get_mean_error();
	Max_Error &get_max_error();
	double  get_max_knotcurerror();
    double  get_min_knotcurerror();

	Mesh *get_error_fitted_mesh();
	void update_polymesh();
	MyMesh get_polymesh();
	QString  get_filename(); //返回载入的文件名

private:

	Mesh *m_pOriginalMesh;     /**<  读入的原始网格  */
	int iter_times;            /**<  按照曲率调整节点线的迭代次数 */
	int add_num;               /**< 每次应该增加的竖直节点线数，由用户指定 */
    int add_horizon_num;       /**< 每次应该增加的水平节点线数，由用户指定 */
    B_parameter *parameter1;
	bool  error_compute_ok;
	CBSplineSurfaceView *bsurface;
	Mesh  *m_pFittedMesh;
	Mesh *m_pErrorMesh;
	MyMesh polymesh;

	QString fileName;            /**< 读入的文件名 */

	vector<TexCoord> domain;      /**< 所有网格点的纹理坐标 */
   

	Bbox_2 box;
	Bbox_3 originalbox;
	Bbox_3 fittingbox;
	Bbox_3 errorbox;

	int sample_num;

	double err_threshold;
    Max_Error max_err;      /**< 可修改的最大误差 */
	double   max_err_real;  /**< 真实的未修改的最大误差 */
	double max_meancurvature;
	double min_meancurvature;
	double max_curerror;
	double min_curerror;
	double mean_err;        /**< 均方根误差 */
	double mean_e;
     bool   Curvature_loading;
	 bool   fitting_completed_;
	 bool    curvature_error;
	 void compute_box();

};

//--------------------------------------------------------
#endif