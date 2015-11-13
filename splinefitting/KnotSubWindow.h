/*************************************************
* Copyright (c) 2014-至今,厦门大学计算机图形学实验室Graphics@XMU
* All rights reserved.
*
* 文件名称：KnotSubWindow.h 
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


#ifndef _KNOTSUBWINDOW_H_
#define _KNOTSUBWINDOW_H_

//Qt
#include <QMainWindow>
#include <QtCore/QString>
#include <QtCore/QStringList>

#include "splineknotsviewer.h"

class QAction;
class QLabel;

/** Knot  subwindow class.
   It's a subwindow of main window.It's used to show  parameterized surface.
 */
class KnotSubWindow: public QMainWindow
{
	Q_OBJECT
public:
	KnotSubWindow(void);
	~KnotSubWindow(void);
	/**   清理数据      */
	void clear_data();
	/**   更新视图,显示参数化曲面  */
	void update_view();
	/**  设置核心数据  */
    void set_surface_data(CSurfaceData *data);
signals:
	void window_close();
protected:
	void closeEvent(QCloseEvent *event);
private:
	/**  core surface data */
	CSurfaceData *surfacedata;
	/** 三维绘图类，是KnotSubWindow的中心区域部件 */
	SplineKnotsViewer *knotsViewer;
	/** 节点线显示菜单选项  */
	QAction *knotsAction;
	/** 曲率显示菜单选项   */
	QAction *curvatureAction;
	/** 网格显示菜单选项  */
	QAction *domainmeshAction;
	/** 曲率积分误差菜单选项 */
	QAction *curvature_error_Action;
	/** 拟合误差菜单选项    */
	QAction *fitting_error_Action;
	
    /** 视图显示菜单 */
	QMenu *viewMenu;
    /** 创立菜单选项*/
	void createActions();
	/** 创立菜单*/
	void createMenus();
	/** 创立上下文菜单，也就是右键菜单 */
	void createContextMenu();

public slots:
	 /** 显示节点线 */
	void knots_view(bool kv);
	/**  显示曲率   */
	void set_curvature_show(bool cv);
	/**  显示网格   */
	void set_mesh_view(bool dv);
	/** 显示曲率积分误差 */
	void error_curvature_view(bool ev);
	/** 显示拟合误差    */
	void error_fitting_view(bool ev);
	
};

//-------------------------------------------------------
#endif