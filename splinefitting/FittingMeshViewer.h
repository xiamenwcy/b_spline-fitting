/*************************************************
* Copyright (c) 2014-至今,厦门大学计算机图形学实验室Graphics@XMU
* All rights reserved.
*
* 文件名称：FittingMeshViewer.h 
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

#ifndef _FITTINGMESHVIEWER_H_
#define _FITTINGMESHVIEWER_H_

//OpenMesh
/************************************************************************/
/* OpenMesh要求"Include MeshIO.hh before including a mesh type!"，所以
   先有MeshIO.hh，后有TriMesh_ArrayKernelT.hh或者PolyMesh_ArrayKernelT.hh
   参考：D:\Program Files (x86)\OpenMesh-3.2\include\OpenMesh\Core\IO\MeshIO.hh
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
	Q_OBJECT           /** \note 只有定义了这个才能使用信号-槽机制. */
public:
	FittingMeshViewer(void);
	~FittingMeshViewer(void);
	void set_surface_data(CSurfaceData *data);
	void update_fitting_view();
	void clear_data();
	void set_fitted_mesh_view(bool fv);
	void set_control_point(Array2 point);   /**< 设置控制顶点*/

    void set_error_show(bool ev);            /**< 设置误差网格是否显示 */  
	void set_max_error_point_show(bool mv);  /**< 设置最大的误差点是否显示*/
	void set_controledge_show(bool cv);      /**< 设置控制网格是否显示 */

	bool write_mesh(QString &fileName, Mesh_Type type);
	
signals:
	
protected:
	virtual void draw();
	virtual void init();
	virtual void paintGL() { update(); };
	virtual void paintEvent(QPaintEvent *event);

    void draw_error_colorbar(QPainter *painter); /**< 绘制拟合误差颜色条 */
	

private:
	void setDefaultMaterial();
	void draw_fitted_mesh();             /**< 绘制拟合网格 */
	void draw_control_mesh_orginal();    /**< 绘制控制网格*/
	void draw_error_mesh();              /**< 绘制误差网格 */
	void draw_max_error_point();         /**< 绘制最大拟合误差点*/
	
	void set_scene(Bbox_3 &box);         /**< 设置场景半径和中心*/
	

	CSurfaceData *surfacedata;
	Array2 ControlPoint;               /**< 控制顶点矩阵 */
	
	 double max_err;                    /**< 最大拟合误差*/

	bool bfitted_mesh_view;             /**< 拟合网格是否可视*/

	  bool berror_show;                  /**< 拟合误差网格是否可视*/

	  bool bcontroledge_show;             /**< 拟合控制网格是否可视*/

	  bool bmax_error_point_show;         /**< 拟合最大误差点是否可视*/

	
	 double radius;    /**< 场景半径*/           
	 
	  Bbox_3 bBox;     /**< 场景包围盒*/
};

//--------------------------------------------
#endif