/*************************************************
* Copyright (c) 2014-����,���Ŵ�ѧ�����ͼ��ѧʵ����Graphics@XMU
* All rights reserved.
*
* �ļ����ƣ�FitSubWindow.h 
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