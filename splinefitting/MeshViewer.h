/*************************************************
* Copyright (c) 2014-至今,厦门大学计算机图形学实验室Graphics@XMU
* All rights reserved.
*
* 文件名称：MeshViewer.h 
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

#ifndef _MESHVIEWER_H_
#define _MESHVIEWER_H_

/************************************************************************/
/* OpenMesh要求"Include MeshIO.hh before including a mesh type!"，所以
   先有MeshIO.hh，后有TriMesh_ArrayKernelT.hh或者PolyMesh_ArrayKernelT.hh
   参考：D:\Program Files (x86)\OpenMesh-3.2\include\OpenMesh\Core\IO\MeshIO.hh
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
	Q_OBJECT        /** \note 只有定义了这个才能使用信号-槽机制. */
public:
	MeshViewer();
	~MeshViewer(void);
	void set_surface_data(CSurfaceData *data);      /**< 设置CSurfaceData */

	void update_view();                             /**< 更新MeshSubWindow的窗口 */

	void clear_data();                              /**< 清除数据 */

	void set_origin_mesh_view(bool ov);             /**<设置原始网格是否可视 */

	void set_curvature_show(bool cv);                /**<设置曲率网格是否可视 */

	bool write_mesh(QString &fileName, Mesh_Type type);/**< 将网格写入文件 */
	
signals:
	
protected:
	virtual void draw();                            /**< 三维绘图函数 */
	virtual void init();                            /**< 绘图初始化设置  */
	virtual void paintGL() { update(); };           /**< 更新绘图窗口*/
    virtual void paintEvent(QPaintEvent *event);    /**< 完整绘图函数，包含二维和三维绘图*/
	void draw_curvature_colorbar(QPainter *painter); /**<绘制曲率颜色条 */


private:
	void setDefaultMaterial();                        /**< 设置材质，反射参数等 */   

	void draw_original_mesh();                        /**< 绘制原始网格 */
	
	void draw_curvature_mesh();                        /**< 绘制曲率网格 */
	
	void set_scene(Bbox_3 &box);                       /**< Set scene center and scene radius*/
	

	CSurfaceData *surfacedata;                         /**< Core surface data */     

	bool borigina_mesh_view;                           /**< 标记原始网格是否可视 */

	bool bcurvature_show;                              /**< 标记曲率网格是否可视 */
	
	double radius;                                     /**< scene radius */

	Bbox_3 bBox;                                       /**< scene bounding box */




};

//-----------------------------------------------------
#endif