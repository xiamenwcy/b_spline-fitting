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
signals:

	void window_close();
protected:
	void closeEvent(QCloseEvent *event);
public:
	FittingMeshViewer *get_mesh_viewer();
	void set_surface_data(CSurfaceData *data);
	bool mesh_fitting();
	bool mesh_parametrization();
	void update_fitting_view();
	void clear_data();
	bool write_mesh(QString &fileName, Mesh_Type type);
private:
	void createStatusBar();
	void createActions();
	void createMenus();
	void createContextMenus();
	
	CSurfaceData *surfacedata;
    FittingMeshViewer *m_pMeshViewer;
	QAction *fittedmeshAction;
	QAction *controlmeshAction;
	QAction *controledgesAction;
	QAction *errorAction;
	QAction *max_error_pointAction;
	QAction *adjustableAction;


	QMenu *viewMenu;
	QMenu *editMenu;

private slots:
		void set_fitted_mesh_view(bool fv);
		void set_adjustpoints_enabled(bool av);
		void set_adjusted_enabled(bool av);
	    void set_error_show(bool ev);
		void set_max_error_point_show(bool mv);
		//void set_controlmesh_show(bool cm);
		void set_controledges_show(bool cv);
		void updateStatusBar();  
	
};

//--------------------------------------------------
#endif