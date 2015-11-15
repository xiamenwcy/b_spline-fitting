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


/** KnotSubWindow central widget.
    It support OpenGL,include 2D drawing and 3D drawing.
*/
class KnotsViewer : public QGLViewer
{
	Q_OBJECT               /** \note 只有定义了这个才能使用信号-槽机制. */
public:
	KnotsViewer(void);
	~KnotsViewer(void);
	void set_surface_data(CSurfaceData *data);   /**< 设置CSurfaceData */

	void clear_data();                            /**< 清除数据 */

	void set_knots_view(bool kv);                  /**< 设置节点线窗口是否可视 */

	void set_curvature_show(bool kv);               /**< 设置曲率网格是否可视  */

	void error_fitting_view(bool kv);               /**< 设置拟合误差网格是否可视 */
	
	void update_view();                             /**< 更新KnotSubWindow窗口 */
	
	void set_domain_mesh_view(bool dv);              /**< 设置参数化的原始网格是否可视 */
	     
	void set_error_curvature_view(bool ev);           /**< 设置曲率积分误差是否可视  */
	
	vector<double> uknot;                             /**< u向节点向量*/
	vector<double> vknot;                             /**< v向节点向量*/
protected: 
	virtual void draw();                               /**< 三维绘图函数 */
	virtual void init();                                /**< 绘图初始化设置  */
	virtual void paintGL() { update(); };               /**< 更新绘图窗口*/
	virtual void paintEvent(QPaintEvent *event);        /**< 完整绘图函数，包含二维和三维绘图*/

	void draw_curvature_colorbar(QPainter *painter);      /**<绘制曲率颜色条 */

	void draw_curvature_error_colorbar(QPainter *painter); /**<绘制曲率积分误差颜色条 */

	void draw_fitting_error_colorbar(QPainter *painter);    /**<绘制拟合误差颜色条 */
private:
	CSurfaceData *surfacedata;                           /**< Core surface data */   

	B_parameter *parameter2;                              /**< B spline parameter:p,q,m,n ... */     

	void draw_knots();                          /**< 绘制节点线 */     
	
	void draw_domain_mesh();                    /**< 绘制参数化的原始网格 */     

	void draw_curvature_mesh();                 /**< 绘制曲率网格 */     

	void draw_curvature_error_domain();         /**< 绘制曲率误差矩形区域 */     

	void draw_fitting_error_domain();            /**< 绘制拟合误差网格 */     

	void set_scene(Bbox_3 &box);                 /**< Set scene center and scene radius*/
	
	bool bknotsview;     /**< 标记节点线窗口是否可视 */

	bool berror_show;      /**< 标记拟合误差是否可视 */

	bool bmeshdomainview;  /**< 标记参数化的原始网格是否可视 */

	bool bcurvature_show;  /**< 标记曲率网格是否可视 */

	bool berrordomainview; /**< 标记曲率积分误差区域是否可视 */

	double max_err;        /**< 拟合最大误差 */
};

//---------------------------------------------------------------
#endif

