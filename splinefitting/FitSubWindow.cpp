/*************************************************
* Copyright (c) 2014-至今,厦门大学计算机图形学实验室Graphics@XMU
* All rights reserved.
*
* 文件名称：FitSubWindow.cpp
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

#include "FitSubWindow.h"



FitSubWindow::FitSubWindow(void)
{
	m_pMeshViewer = new FittingMeshViewer;
	m_pMeshViewer->setAttribute(Qt::WA_OpaquePaintEvent);
	m_pMeshViewer->setAttribute(Qt::WA_NoSystemBackground);
	setCentralWidget(m_pMeshViewer);
	setWindowTitle("FittingMesh Viewer");

	surfacedata = NULL;
	createActions();
	createMenus();
	createContextMenus();
	createStatusBar();
	
	connect(m_pMeshViewer, SIGNAL(position_changed()), this, SLOT(updateStatusBar()));

}

FitSubWindow::~FitSubWindow(void)
{
	clear_data();

}
void FitSubWindow::clear_data()
{
	
	fittedmeshAction->setChecked(false);
	errorAction->setChecked(false);
	adjustableAction->setChecked(false);
	controlmeshAction->setChecked(false);
	m_pMeshViewer->clear_data();
}
FittingMeshViewer *FitSubWindow::get_mesh_viewer()
{
	return m_pMeshViewer;
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


	controlmeshAction = new QAction(tr("Control Mesh"), this);
	controlmeshAction->setCheckable(true);
	//connect(controlmeshAction, SIGNAL(toggled(bool)), this, SLOT(set_controlmesh_show(bool)));

	controledgesAction = new QAction(tr("Control Edges"), this);
	controledgesAction->setCheckable(true);
	connect(controledgesAction, SIGNAL(toggled(bool)), this, SLOT(set_controledges_show(bool)));

	adjustableAction = new QAction(tr("Adjustable"), this);
	adjustableAction->setCheckable(true);
	connect(adjustableAction, SIGNAL(toggled(bool)), this, SLOT(set_adjustpoints_enabled(bool)));
	
}

void FitSubWindow::createMenus()
{
	viewMenu = menuBar()->addMenu(tr("&View"));
	viewMenu->addAction(fittedmeshAction);
	viewMenu->addAction(controlmeshAction);
	viewMenu->addAction(controledgesAction);
	viewMenu->addAction(errorAction);
	viewMenu->addAction(max_error_pointAction);
	editMenu = menuBar()->addMenu(tr("&Option"));
	editMenu->addAction(adjustableAction);
}

void FitSubWindow::createContextMenus()
{
	m_pMeshViewer->addAction(fittedmeshAction);
	m_pMeshViewer->addAction(controledgesAction);
	m_pMeshViewer->addAction(controlmeshAction);
	m_pMeshViewer->addAction(errorAction);
	m_pMeshViewer->addAction(max_error_pointAction);
	m_pMeshViewer->setContextMenuPolicy(Qt::ActionsContextMenu);
}

void FitSubWindow::createStatusBar()
{/*
	filenameLabel = new QLabel;
	pointsLabel = new QLabel;
	faceLabel = new QLabel;
	controlnumLabel = new QLabel;
	controlindexLabel = new QLabel;

	statusBar()->addWidget(filenameLabel, 1);
	statusBar()->addWidget(pointsLabel, 1);
	statusBar()->addWidget(faceLabel, 1);
	statusBar()->addWidget(controlnumLabel, 1);
	statusBar()->addWidget(controlindexLabel, 1);
	updateStatusBar();*/
}

//QString &FitSubWindow::get_cur_file()
//{
//	return curFile;
//}

//void FitSubWindow::setCurrentFile(const QString &fileName)
//{
//	curFile = fileName;
//	setWindowModified(false);
//
//	QString shownName = tr("Untitled");
//	if (!curFile.isEmpty()) 
//	{
//		shownName = strippedName(curFile);
//		recentFiles.removeAll(curFile);
//		recentFiles.prepend(curFile);
//		emit current_file_changed();
//	}
//
//	setWindowTitle(tr("%1[*] - %2").arg(shownName)
//		.arg(tr("Simplex Spline")));
//	updateStatusBar();
//}

//QStringList &FitSubWindow::get_recent_files()
//{
//	return recentFiles;
//}
//
//QString FitSubWindow::strippedName(const QString &fullFileName)
//{
//	return QFileInfo(fullFileName).fileName();
//}

void FitSubWindow::updateStatusBar()
{
	/*if(surfacedata!=NULL)
	{
		QString points = "Point: ";
		int pointNum = surfacedata->get_mesh_vertex_num();
		if(pointNum == -1)
		{
			points = "";
		}
		else
		{
			points.append(QString::number(pointNum, 10));
		}
		QString faces = "Face: ";
		int faceNum = surfacedata->get_mesh_face_num();
		if(faceNum == -1)
		{
			faces = "";
		}
		else
		{
			faces.append(QString::number(faceNum, 10));
		}
		QString fileName = "Mesh: ";
		QString meshName = surfacedata->get_mesh_name();
		if(meshName.isEmpty())
		{
			fileName = "";
		}
		else
		{
			fileName = fileName.append(meshName);
		}
		filenameLabel->setText(fileName);
		pointsLabel->setText(points);
		faceLabel->setText(faces);
		QString controlstr = "Num of C.V.: ";
		int controlNum = surfacedata->get_basis_size();
		if(controlNum ==0)
		{
			controlstr = "";
		}
		else
		{
			controlstr.append(QString::number(controlNum, 10));
		}
		controlnumLabel->setText(controlstr);
		QString indexstr = "C.V. ";
		int index = m_pMeshViewer->get_index();
		if(index ==-1)
		{
			indexstr = "Pos:(";
		}
		else
		{
			indexstr.append(QString::number(index, 10));
			indexstr.append(": (");
		}
		Point_3 pos = m_pMeshViewer->get_postion();
		indexstr.append(QString::number(pos.x(), 'g', 3));
		indexstr.append(", ");
		indexstr.append(QString::number(pos.y(), 'g', 3));
		indexstr.append(", ");
		indexstr.append(QString::number(pos.z(), 'g', 3));
		indexstr.append(")");
		controlindexLabel->setText(indexstr);
	}*/
}

void FitSubWindow::closeEvent(QCloseEvent *event)
{
	event->accept();
	emit window_close();
}

void FitSubWindow::set_surface_data(CSurfaceData *data)
{
	surfacedata = data;
	updateStatusBar();
	m_pMeshViewer->set_surface_data(data);
}

void FitSubWindow::update_fitting_view()
{
	updateStatusBar();
	m_pMeshViewer->update_fitting_view();
}
//void FitSubWindow::surface_sampling(int index)
//{
//	int nRow;
//	int nCol;
//	RowColumnDialog rowcolumn;
//	if(rowcolumn.exec() == QDialog::Accepted)
//	{
//		nRow = rowcolumn.rowspin->value();
//		nCol = rowcolumn.columnspin->value();
//		nRow += 1;
//		nCol += 1;
//		surfacedata->set_row_col(nRow, nCol);
//		surfacedata->surface_sampling(index);
//	}
//}

bool FitSubWindow::mesh_fitting()
{
	return surfacedata->mesh_fitting();
}

//void FitSubWindow::surface_fitting()
//{
//	surfacedata->surface_fitting();
//}
//
bool FitSubWindow::mesh_parametrization()
{
	return surfacedata->mesh_parameterization();
}

//void FitSubWindow::add_knots()
//{
//	int nseed = get_nseed();
//	if(nseed==0)
//	{
//		return;
//	}
//
//	int iter_num = surfacedata->get_iterator_num();
//	if(iter_num==0)
//	{
//		surfacedata->set_nseed(nseed);
//
//		int degree = get_degree();
//		if(degree == -1)
//		{
//			return;
//		}
//		surfacedata->set_degree(degree);
//		//surfacedata->knots_generation();
//	}
//	else
//	{
//		surfacedata->add_knots(nseed);
//	}
//
//}
//
//int FitSubWindow::get_nseed()
//{
//	bool ok = false;
//	int num = surfacedata->get_mesh_vertex_num();
//	if(num<40000)
//		num = 40;
//	else
//		num = (surfacedata->get_mesh_vertex_num()/10000+1)*10;
//	int nseed = 
//		QInputDialog::getInteger(this, 
//		tr("Number of seeds"),
//		tr("What is the number of seeds you want to add?"),
//		num,
//		1,
//		surfacedata->get_mesh_vertex_num(),
//		10,
//		&ok);
//	if(ok)
//		return nseed;
//	else
//		return 0;
//}
//
//int FitSubWindow::get_degree()
//{
//	bool ok = false;
//	int degree = 
//		QInputDialog::getInteger(this, 
//		tr("Degree"),
//		tr("Degree of triangulation configurations that wants to elevate:"),
//		4,
//		0,
//		10,
//		1,
//		&ok);
//	if(ok)
//		return degree;
//	else
//		return -1;
//}


void FitSubWindow::set_fitted_mesh_view(bool fv)
{
	m_pMeshViewer->set_fitted_mesh_view(fv);
}

void FitSubWindow::set_error_show(bool ev)
{
	m_pMeshViewer->set_error_show(ev);
}

//void FitSubWindow::set_curvature_show(bool cv)
//{
//	m_pMeshViewer->set_curvature_show(cv);
//}
//
//void FitSubWindow::set_sparsemesh_show(bool sv)
//{
//	m_pMeshViewer->set_sparsemesh_view(sv);
//}
//
//void FitSubWindow::set_sparsecurves_show(bool sv)
//{
//	m_pMeshViewer->set_sparsecurves_view(sv);
//}
//
//void FitSubWindow::set_density_show(bool dv)
//{
//	m_pMeshViewer->set_density_show(dv);
//}
//
//void FitSubWindow::set_controlmesh_show(bool cm)
//{
//	m_pMeshViewer->set_controlmesh_show(cm);
//}
//
void FitSubWindow::set_adjusted_enabled(bool av)
{
	m_pMeshViewer->set_adjustedmesh_enabled(av);
}

void FitSubWindow::set_adjustpoints_enabled(bool av)
{
	m_pMeshViewer->set_adjustpoints_enabled(av);
}

//void FitSubWindow::set_adjusted_control_enabled(bool av)
//{
//	m_pMeshViewer->set_adjusted_control_enabled(av);
//}
//
bool FitSubWindow::write_mesh(QString &fileName, Mesh_Type type)
{
	return m_pMeshViewer->write_mesh(fileName, type);
}

void FitSubWindow::set_max_error_point_show(bool mv)
{
	m_pMeshViewer->set_max_error_point_show(mv);
}

void FitSubWindow::set_controledges_show(bool cv)
{
	m_pMeshViewer->set_controledge_show(cv);
}

//void FitSubWindow::set_featurecurves_enabled(bool fv)
//{
//	m_pMeshViewer->set_featureline_view(fv);
//}