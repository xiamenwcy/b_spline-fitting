/*************************************************
* Copyright (c) 2014-至今,厦门大学计算机图形学实验室Graphics@XMU
* All rights reserved.
*
* 文件名称：MeshSubWindow.h 
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

#ifndef  _MESHSUBWINDOW_H_
#define  _MESHSUBWINDOW_H_

//Qt
#include <QtGui/QMainWindow>
#include <QtCore/QString>
#include <QtCore/QStringList>

#include "meshviewer.h"
#include "surfacedata.h"

class QLabel;
class QAction;
class QMenu;

/** Mesh  subwindow class.
   It's a subwindow of main window.It's used to show  mesh surface.
 */

class MeshSubWindow: public QMainWindow
{
	Q_OBJECT
public:
	MeshSubWindow(void);
	~MeshSubWindow(void);
    /** 返回MeshSubWindow的中心区域部件 */
	MeshViewer *get_mesh_viewer();
	
	void set_surface_data(CSurfaceData *data);
   
	bool mesh_parametrization();
    /**  更新窗口显示 */
	void update_view();
    /** 清理窗口所有数据，恢复初始化 */
	void clear_data();
    /** 将显示的网格（这里是曲率网格）写入文件中 */
	bool write_mesh(QString &fileName, Mesh_Type type);
  
signals:
    /**  发送窗口关闭信号 */
	void window_close();

protected:
	void closeEvent(QCloseEvent *event);

private:
	void createStatusBar();
	void createActions();
	void createMenus();
	/**  创立上下文菜单，也就是右键菜单 */
	void createContextMenus();
   

	//正常显示在状态栏中
	/**  载入模型名   */
	QLabel *filenameLabel;
	/**  载入模型顶点数目 */
	QLabel *pointsLabel;
    /**  载入模型三角面片数目 */
	QLabel *faceLabel;

	/**    三维绘图类，是MeshSubWindow的中心区域部件  */
	MeshViewer *m_pMeshViewer;
    /**  core surface data  */
	CSurfaceData *surfacedata;
    
    /**  original mesh*/
	QAction *originalmeshAction;
    /**  original mesh with  curvature  */
	QAction *curvatureAction;
   /** 显示不同网格的菜单 */
	QMenu *viewMenu;


private slots:

		void set_origin_mesh_view(bool ov);

		void set_curvature_show(bool cv);

		void updateStatusBar();
	
};

//-----------------------------------------------------------------
#endif