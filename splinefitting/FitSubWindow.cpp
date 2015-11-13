/*************************************************
* Copyright (c) 2014-����,���Ŵ�ѧ�����ͼ��ѧʵ����Graphics@XMU
* All rights reserved.
*
* �ļ����ƣ�FitSubWindow.cpp
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

//Qt
#include <QtGui/QtGui>

#include "FitSubWindow.h"



FitSubWindow::FitSubWindow(void)
{
	fittingviewer = new FittingMeshViewer();
	fittingviewer->setAttribute(Qt::WA_OpaquePaintEvent);
	fittingviewer->setAttribute(Qt::WA_NoSystemBackground);
	setCentralWidget(fittingviewer);
	setWindowTitle("FittingMesh Viewer");

	surfacedata = NULL;
	createActions();
	createMenus();
	createContextMenus();
	

}

FitSubWindow::~FitSubWindow(void)
{
	clear_data();

}
void FitSubWindow::clear_data()
{
	
	fittedmeshAction->setChecked(false);
	errorAction->setChecked(false);
	fittingviewer->clear_data();

}
FittingMeshViewer *FitSubWindow::get_mesh_viewer()
{
	return fittingviewer;
}

void FitSubWindow::createActions()
{
	
	fittedmeshAction = new QAction(tr("Fitted Mesh"), this);
	fittedmeshAction->setCheckable(true);
	connect(fittedmeshAction, SIGNAL(toggled(bool)), this, SLOT(set_fitted_mesh_view(bool)));
	errorAction = new QAction(tr("Error Mesh"), this);
	errorAction->setCheckable(true);
	connect(errorAction, SIGNAL(toggled(bool)), this, SLOT(set_error_show(bool)));

	max_error_pointAction = new QAction(tr("Max Error Point"), this);
	max_error_pointAction->setCheckable(true);
	connect(max_error_pointAction, SIGNAL(toggled(bool)), this, SLOT(set_max_error_point_show(bool)));

	controledgesAction = new QAction(tr("Control Edges"), this);
	controledgesAction->setCheckable(true);
	connect(controledgesAction, SIGNAL(toggled(bool)), this, SLOT(set_controledges_show(bool)));


	
}

void FitSubWindow::createMenus()
{
	viewMenu = menuBar()->addMenu(tr("&View"));
	viewMenu->addAction(fittedmeshAction);
	viewMenu->addAction(controledgesAction);
	viewMenu->addAction(errorAction);
	viewMenu->addAction(max_error_pointAction);


}

void FitSubWindow::createContextMenus()
{
	fittingviewer->addAction(fittedmeshAction);
	fittingviewer->addAction(controledgesAction);
	fittingviewer->addAction(errorAction);
	fittingviewer->addAction(max_error_pointAction);
	fittingviewer->setContextMenuPolicy(Qt::ActionsContextMenu);
}


void FitSubWindow::closeEvent(QCloseEvent *event)
{
	event->accept();
	emit window_close();
}

void FitSubWindow::set_surface_data(CSurfaceData *data)
{
	surfacedata = data;
	fittingviewer->set_surface_data(data);
}

void FitSubWindow::update_fitting_view()
{
	fittingviewer->update_fitting_view();
}


bool FitSubWindow::mesh_fitting()
{
	return surfacedata->mesh_fitting();
}


void FitSubWindow::set_fitted_mesh_view(bool fv)
{
	fittingviewer->set_fitted_mesh_view(fv);
}

void FitSubWindow::set_error_show(bool ev)
{
	fittingviewer->set_error_show(ev);
}


bool FitSubWindow::write_mesh(QString &fileName, Mesh_Type type)
{
	return fittingviewer->write_mesh(fileName, type);
}

void FitSubWindow::set_max_error_point_show(bool mv)
{
	fittingviewer->set_max_error_point_show(mv);
}

void FitSubWindow::set_controledges_show(bool cv)
{
	fittingviewer->set_controledge_show(cv);
}

