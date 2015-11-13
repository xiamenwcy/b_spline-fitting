/*************************************************
* Copyright (c) 2014-至今,厦门大学计算机图形学实验室Graphics@XMU
* All rights reserved.
*
* 文件名称：FitSubWindow.h 
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

#ifndef _FITSUBWINDOW_H_
#define _FITSUBWINDOW_H_

//Qt
#include <QMainWindow>
#include <QtCore/QString>
#include <QtCore/QStringList>

#include "FittingMeshViewer.h"
#include "surfacedata.h"

class QLabel;
class QAction;
class QMenu;


class FitSubWindow: public QMainWindow
{
	Q_OBJECT
public:
	FitSubWindow(void);
	~FitSubWindow(void);
	/**  获取FitSubWindow的中心部件  */
	FittingMeshViewer *get_mesh_viewer();
	void set_surface_data(CSurfaceData *data);
	/** 拟合曲面   */
	bool mesh_fitting();
	/** 更新视图，显示拟合曲面  */
	void update_fitting_view();
	/** 清理数据，恢复初始化   */ 
	void clear_data();
	/** 将网格（这里指Fitted_Mesh,Error_Mesh）写入文件 */		
	bool write_mesh(QString &fileName, Mesh_Type type);
signals:
	void window_close();
protected:
	void closeEvent(QCloseEvent *event);	
private:
	void createActions();
	void createMenus();
	void createContextMenus();
	
	CSurfaceData *surfacedata;
	/** 三维显示类，是FitSubWindow的中心区域部件 */
    FittingMeshViewer *fittingviewer;
	/** 拟合网格菜单项 */
	QAction *fittedmeshAction;
	/** 控制网格菜单项 */
	QAction *controledgesAction;
	/** 误差曲面菜单项  */
	QAction *errorAction;
	/** 拟合最大误差点菜单项 */
	QAction *max_error_pointAction;
	QMenu *viewMenu;

private slots:
	 
		void set_fitted_mesh_view(bool fv);
	    void set_error_show(bool ev);
		void set_max_error_point_show(bool mv);
		void set_controledges_show(bool cv);
	
	
};

//--------------------------------------------------
#endif