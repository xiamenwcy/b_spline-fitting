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
	bool read_mesh(const QString &fileName);  /**< 载入网格 */
	int get_mesh_vertex_num();               /**< 返回载入的原始网格顶点数 */
	int get_mesh_face_num();                 /**< 返回载入的原始网格曲面数 */
	QString get_mesh_name();                  /**< 返回载入的原始网格文件名 */
	Mesh *get_original_mesh();               /**< 返回原始网格 */

    void   clear_data();                   /**<  清理数据  */
	void setparameter(B_parameter *parameter_2); /**< 设置B样条曲面的参数p,q,m,n */

	//增加节点线
	void set_knots_adding_num(int num);    /**< 每次应该增加的竖直节点线数，由用户指定 */
	void set_horizon_knots_addnum(int num);/**< 每次应该增加的水平节点线数，由用户指定*/
	bool add_vertical_knots();             /**< 增加竖直节点线，即u线 */
    bool add_horizon_knots();              /**< 增加水平节点线，即v线 */
	void  vertical_range_query();           /**< 查询竖直条形的顶点 */
	void  horizon_range_query();            /**< 查询水平条形的顶点 */
	bool  rangequery_vertical;             /**< 标定vertical_range_query()关于v_index属性增加的执行情况,\see vertical_range_query(),horizon_range_query() */
	bool  rangequery_horizon;              /**< 标定horizon_range_query()关于v_index属性增加的执行情况,\see vertical_range_query(),horizon_range_query() */
	
   
	/** uknots和vknots是水平和竖直的节点线，构成了若干个小矩形，p为其中一点，找到这个点所在的矩形区域序号 */
	int location(vector<double>& uknots,vector<double>& vknots,TexCoord& p);

	void   set_max_curvature(double curvature); /**< 为了防止最大曲率点过于孤立，设置最大的曲率 */
	double get_max_meancurvature();             /**< 返回最大的平均曲率  */
    double get_min_meancurvature();             /**< 返回最小的平均曲率  */
	B_parameter *getparameter();                /**< 返回B样条曲面的参数p,q,m,n */
	Mesh *get_fitted_mesh();                    /**< 返回拟合网格 */

	bool mesh_fitting();                       /**< 拟合原始网格，生成拟合网格 */
    CBSplineSurfaceView* getbspline();          /**< 返回拟合曲面类，用于具体的计算拟合过程  */
	vector<TexCoord>  get_domain();            /**< 返回原始网格的纹理坐标向量 */

	Bbox_2 &get_box();                        /**< 返回原始网格纹理坐标的包围盒,\see box*/
	Bbox_3 get_original_box();                 /**< 返回原始网格的包围盒 \see originalbox */
	Bbox_3 get_fitting_box();                  /**<  返回拟合网格的包围盒 \see fittingbox  */
    Bbox_3 get_error_box();                    /**< 返回误差网格的包围盒 \see errorbox */
	void   set_original_box(Bbox_3 box);       /**< 设置原始网格的包围盒 \see originalbox  */
    void   set_fitting_box(Bbox_3 box);        /**< 设置拟合网格的包围盒 \see fittingbox */

	bool mesh_parameterization();            /**< 曲面参数化 */
	void compute_error();                    /**< 计算拟合误差 */
	bool curvature_loading();                /**< 曲率是否载入成功? */
	bool fitting_completed();                /**< 拟合是否完成? */
	bool curvature_error_completed();        /**< 曲率积分误差是否计算完毕 \see CSurfaceData::update_polymesh()*/
	void set_fitting_completed(bool state);     /**< 设置拟合完成状态 */
	void set_curvature_loadingstate(bool state); /**< 设置曲率载入状态 */
	bool compute_curvature(QStringList files);   /**< 计算曲率*/
  

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
	
	/** 按照曲率调整节点线 */
	bool  adjust_knots_by_curvature();
	/** 按照拟合误差调整节点线 */
	bool  adjust_knots_by_fitting_error();

	/**  更新矩形网格polymesh,并查询矩形区域中包含的三角形 */
	void   update_polymesh_and_query_tri();

	/**  更新矩形网格polymesh,并查询矩形区域中包含的顶点 */
	void   update_polymesh_and_query_point();

	/**  建立矩形网格*/
	MyMesh  build_polymesh();

    /** 修订不合适的节点 */
	void  modification();

    /** 找到不含一个参数点的矩形区域，并删除相邻的节点线  */
	void  query_isolated_range();      

	bool  query_success;                 /**< query_isolated_range结束的标志 */

    vector<double> uknots;               /**< 现有的u节点 */
	vector<double> vknots;               /**< 现有的v节点 */
	vector<double> uknots_new;           /**< 用来储存每次新增加的u节点 */
	vector<double> vknots_new;           /**< 用来储存每次新增加的v节点 */
	int unum;                            /**< uknots的个数  */
	int vnum;                            /**< vknots的个数  */

	 static int iter_num;              /**< 指示\see update_knots()的执行次数 */
	
	 double curaverage_;                /**< 曲率积分平均值 */
	

     vector<set<int> >  horizon_index; /**< 水平条形，存储顺序是从下面到上面，从小到大，存储内部和边界节点序号*/
     vector<set<int> >  vertical_index;/**< 竖直条形，存储顺序是从左到右，从小到大，存储内部和边界节点序号 */
  //   set<int>   test1;   //检测面包含的点数
	 //set<int>   test2;   //检测面包含的点数
	 //set<int>   test3;   //检测面包含的点数

   

	void  set_max_error(double &err);         /**< 设置最大误差，并且为每个拟合点着色 */
    void  set_knots_iteration_times(int n);   /**<  设置按照曲率调整节点线的迭代次数 */

	double &get_mean_error();          /**< 返回均方根误差信息 mean_err */
	Max_Error &get_max_error();        /**< 返回最大的拟合误差点信息 max_err */
	double  get_max_knotcurerror();   /**< 返回最大的矩形曲率积分误差 max_curerror */
    double  get_min_knotcurerror();   /**< 返回最小的矩形曲率积分误差 min_curerror */

	Mesh *get_error_fitted_mesh();  /**< 返回误差网格 */
	void update_polymesh();         /**< 更新按照曲率分布的节点线组成的polymesh并且查询曲率积分的误差 */
	MyMesh get_polymesh();          /**< 返回polymesh  */
	QString  get_filename();        /**< 返回载入的文件名*/

private:

	Mesh *m_pOriginalMesh;     /**<  读入的原始网格  */
	int iter_times;            /**<  按照曲率调整节点线的迭代次数 */
	int add_vertical_num;      /**< 每次应该增加的竖直节点线数，由用户指定 */
    int add_horizon_num;       /**< 每次应该增加的水平节点线数，由用户指定 */
    B_parameter *parameter1;   /**< B样条曲面的p,q,m,n */
	bool  error_compute_ok;    /**< 拟合误差计算是否完成 */
	CBSplineSurfaceView *bsurface; /**< 拟合曲面类  */
	Mesh  *m_pFittedMesh;       /**< 拟合后的网格 */
	Mesh *m_pErrorMesh;         /**< 带有拟合误差颜色的拟合网格 */
	MyMesh polymesh;            /**< 由uknots,vknots组成的矩形曲面网格 */

	QString fileName;            /**< 读入的文件名 */

	vector<TexCoord> domain;      /**< 所有网格点的纹理坐标 */
   

	Bbox_2 box;                   /**< 原始网格纹理坐标的包围盒 */
	Bbox_3 originalbox;           /**< 原始网格的包围盒 */
	Bbox_3 fittingbox;            /**< 拟合网格的包围盒 */
	Bbox_3 errorbox;              /**< 误差网格的包围盒 */

	int sample_num;              /**< 载入网格的顶点个数 */

	double err_threshold;       /**< 设置的最大拟合误差，\see  KnotsViewer::error_fitting_view(),FittingMeshViewer::set_error_show() */
    Max_Error max_err;          /**< 最大拟合误差点,包含误差和位置,\see   CSurfaceData::compute_error() */
	double   max_err_real;      /**< 相对于包围盒最长距离的真实的未修改的最大拟合误差 */
	double max_meancurvature;   /**< 最大的平均曲率 */
	double min_meancurvature;   /**< 最小的平均曲率 */
	double max_curerror;        /**< 最大的曲率误差 */
	double min_curerror;        /**< 最小的曲率误差 */
	double mean_err;            /**< 相对于包围盒最长距离的均方根误差,\see CSurfaceData::compute_error() */
	double mean_e;              /**< 均方根误差,\see CSurfaceData::compute_error() */
     bool   Curvature_loading;    /**< 标记曲率载入是否成功 */  
	 bool   fitting_completed_;   /**< 标记拟合是否完成 */
	 bool    curvature_error;  /**< 标记曲率积分的误差是否计算完毕  */
	 void compute_box();       /**< 计算纹理坐标的包围盒 */

};

//--------------------------------------------------------
#endif