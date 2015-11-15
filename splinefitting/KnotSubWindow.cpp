/*************************************************
* Copyright (c) 2014-至今,厦门大学计算机图形学实验室Graphics@XMU
* All rights reserved.
*
* 文件名称：KnotSubWindow.cpp
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


//Qt
#include <QtGui/QtGui>

#include "KnotSubWindow.h"


KnotSubWindow::KnotSubWindow(void)
{

	knotsViewer = new KnotsViewer();
	setCentralWidget(knotsViewer);
	setWindowTitle("Knots Viewer");
	surfacedata = NULL;
	createActions();
	createMenus();
	createContextMenu();

	(void)statusBar();//创建状态栏
}

KnotSubWindow::~KnotSubWindow(void)
{
	clear_data();
}
void KnotSubWindow::clear_data()
{
	knotsViewer->clear_data();

	knotsAction->setChecked(false);
	
	curvatureAction->setChecked(false);

	domainmeshAction->setChecked(false);


}

void KnotSubWindow::createActions()
{
 
	knotsAction = new QAction(tr("Knots"), this);
	knotsAction->setCheckable(true);
	connect(knotsAction, SIGNAL(toggled(bool)), this, SLOT(knots_view(bool)));

	curvatureAction = new QAction(tr("Curvature Mesh"), this);
	curvatureAction->setCheckable(true);
	connect(curvatureAction, SIGNAL(toggled(bool)), this, SLOT(set_curvature_show(bool)));


	domainmeshAction  = new QAction(tr("Domain Mesh"), this);
	domainmeshAction->setCheckable(true);
    domainmeshAction->setDisabled(true);
	connect(domainmeshAction, SIGNAL(toggled(bool)), this, SLOT(set_mesh_view(bool)));

	curvature_error_Action = new QAction(tr("Curvature Error"), this);
	curvature_error_Action->setCheckable(true);
	connect(curvature_error_Action, SIGNAL(toggled(bool)), this, SLOT(error_curvature_view(bool)));

	fitting_error_Action = new QAction(tr("Fitting Error"), this);
	fitting_error_Action->setCheckable(true);
	connect(fitting_error_Action,	SIGNAL(toggled(bool)), this, SLOT(error_fitting_view(bool)));

	
}
void KnotSubWindow::createMenus()
{

	viewMenu = menuBar()->addMenu(tr("&View"));
	viewMenu->addAction(domainmeshAction);
	viewMenu->addAction(knotsAction);
	viewMenu->addAction(curvatureAction);
	viewMenu->addAction(curvature_error_Action);
	viewMenu->addAction(fitting_error_Action);

}
void KnotSubWindow::createContextMenu()
{
	knotsViewer->addAction(domainmeshAction);
	knotsViewer->addAction(knotsAction);
	knotsViewer->addAction(curvatureAction);
	knotsViewer->addAction(curvature_error_Action);
	knotsViewer->addAction(fitting_error_Action);
	knotsViewer->setContextMenuPolicy(Qt::ActionsContextMenu);
}

void KnotSubWindow::set_surface_data(CSurfaceData *data)
{
	surfacedata = data;
	knotsViewer->set_surface_data(data);

}
void KnotSubWindow::update_view()
{
    domainmeshAction->setEnabled(true);
	knotsViewer->update_view();
}

void KnotSubWindow::closeEvent(QCloseEvent *event)
{
	event->accept();
	emit window_close();
}

void KnotSubWindow::knots_view(bool kv)
{
	knotsViewer->set_knots_view(kv);
}
void KnotSubWindow::set_curvature_show(bool cv)
{
	if (cv)
	{
		if (!surfacedata->curvature_loading())
		{
			QMessageBox::critical(this,tr("Curvature Loading Error:"),tr("Curvature has not been loadded yet,please loading curvature."),QMessageBox::Ok);
			return;
		}

	}
	knotsViewer->set_curvature_show(cv);
}
void KnotSubWindow::error_fitting_view(bool ev)
{
	if (ev)
	{
		if (!surfacedata->fitting_completed())
		{
			QMessageBox::critical(this,tr("Mesh fitting Error:"),tr("Mesh has not been fitted yet,please fitting mesh."),QMessageBox::Ok);
			return;
		}

	}
	knotsViewer->error_fitting_view(ev);

}


void KnotSubWindow::set_mesh_view(bool dv)
{
	knotsViewer->set_domain_mesh_view(dv);
}

void KnotSubWindow::error_curvature_view(bool ev)
{
	if (ev)
	{
		if (!surfacedata->curvature_error_completed())
		{
			QMessageBox::critical(this,tr("Knot  comfiguration  Error:"),tr("Knot has not been comfigurated yet,please comfigure knots."),QMessageBox::Ok);
			return;
		}

	}
	knotsViewer->set_error_curvature_view(ev);
}


