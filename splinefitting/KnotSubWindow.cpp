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
	basisIndex = -1;
	knotsViewer = new SplineKnotsViewer();
	setCentralWidget(knotsViewer);
	setWindowTitle("Knots Viewer");
	surfacedata = NULL;
	createActions();
	createMenus();
	createContextMenu();
	parameter=new B_parameter(0,0,0,0);

	//createStatusBar();
}

KnotSubWindow::~KnotSubWindow(void)
{
	clear_data();
}
void KnotSubWindow::clear_data()
{
	knotsViewer->clear_data();
	domainAction->setChecked(false);
	knotsAction->setChecked(false);
	//centroidAction->setChecked(false);
	curvatureAction->setChecked(false);
	//basisAction->setChecked(false);
	//basissumAction->setChecked(false);
	domainmeshAction->setChecked(false);
	/*interiorAction->setChecked(false);
	triangluationTestAction->setChecked(false);
	basissupportAction->setChecked(false);
	delaunayAction->setChecked(false);
	ddtAction->setChecked(false);
	segmentAction->setChecked(false);*/
}

void KnotSubWindow::createActions()
{
	indexAction  = new QAction(tr("Basis index"), this);
	//connect(indexAction, SIGNAL(triggered()), this, SLOT(basis_index()));
    
	knotsAction = new QAction(tr("Knots"), this);
	knotsAction->setCheckable(true);
	connect(knotsAction, SIGNAL(toggled(bool)), this, SLOT(knots_view(bool)));

	curvatureAction = new QAction(tr("Curvature Mesh"), this);
	curvatureAction->setCheckable(true);
	connect(curvatureAction, SIGNAL(toggled(bool)), this, SLOT(set_curvature_show(bool)));

	domainAction = new QAction(tr("Domain"), this);
	domainAction->setCheckable(true);
	//connect(domainAction, SIGNAL(toggled(bool)), this, SLOT(domain_view(bool)));

	domainmeshAction  = new QAction(tr("Domain Mesh"), this);
	domainmeshAction->setCheckable(true);
	connect(domainmeshAction, SIGNAL(toggled(bool)), this, SLOT(set_mesh_view(bool)));

	errdomainAction = new QAction(tr("Curvature Error"), this);
	errdomainAction->setCheckable(true);
	connect(errdomainAction, SIGNAL(toggled(bool)), this, SLOT(error_domain_view(bool)));

	segmentAction = new QAction(tr("Fitting Error"), this);
	segmentAction->setCheckable(true);
	connect(segmentAction,	SIGNAL(toggled(bool)), this, SLOT(error_fitting_view(bool)));

	densitydomainAction = new QAction(tr("Density Domain"), this);
	densitydomainAction->setCheckable(true);
	//connect(densitydomainAction, SIGNAL(toggled(bool)), this, SLOT(density_domain_view(bool)));

	centroidAction= new QAction(tr("Centroid"), this);
	centroidAction->setCheckable(true);
	//	connect(centroidAction, SIGNAL(toggled(bool)), this, SLOT(centroid_view(bool)));

	basisAction = new QAction(tr("Basis"), this);
	basisAction->setCheckable(true);
	//connect(basisAction, SIGNAL(toggled(bool)), this, SLOT(basis_view(bool)));

	basissumAction = new QAction(tr("Basis Sum"), this);
	basissumAction->setCheckable(true);
	//	connect(basissumAction, SIGNAL(toggled(bool)), this, SLOT(basis_sum_view(bool)));

	openAction = new QAction(tr("&Open..."), this);
	openAction->setShortcut(QKeySequence::Open);
	openAction->setStatusTip(tr("Open an existing mesh file"));
	//connect(openAction, SIGNAL(triggered()), this, SLOT(open()));

	saveAsAction = new QAction(tr("Save &As..."), this);
	saveAsAction->setStatusTip(tr("Save the mesh under a new "
		"name"));
	//connect(saveAsAction, SIGNAL(triggered()), this, SLOT(saveAs()));

	interiorAction = new QAction(tr("Interior Points"), this);
	interiorAction->setCheckable(true);
	//connect(interiorAction, SIGNAL(toggled(bool)), this, SLOT(interior_knots_view(bool)));

	triangluationTestAction = new QAction(tr("Triangular Test"), this);
	//connect(triangluationTestAction, SIGNAL(triggered()), knotsViewer, SLOT(triangular_test()));

	basissupportAction = new QAction(tr("Basis Support"), this);
	basissupportAction->setCheckable(true);
	//connect(basissupportAction,	SIGNAL(toggled(bool)), this, SLOT(show_basis_support(bool)));

	triangulationAction = new QAction(tr("Triangulation View"), this);
	triangulationAction->setCheckable(true);
	//connect(triangulationAction,	SIGNAL(toggled(bool)), this, SLOT(show_triangulation_view(bool)));

	delaunayAction = new QAction(tr("Delaunay View"), this);
	delaunayAction->setCheckable(true);
	//connect(delaunayAction,	SIGNAL(toggled(bool)), this, SLOT(show_delaunay_view(bool)));

	ddtAction = new QAction(tr("DDT View"), this);
	ddtAction->setCheckable(true);
	//	connect(ddtAction,	SIGNAL(toggled(bool)), this, SLOT(show_ddt_view(bool)));

	principaldirectionAction =  new QAction(tr("Principal Direction"), this);
	principaldirectionAction->setCheckable(true);
	//connect(principaldirectionAction,	SIGNAL(toggled(bool)), this, SLOT(set_principal_direction_view(bool)));


	thinAction = new QAction(tr("Thin Triangles-Area"), this);
	thinAction->setCheckable(true);
	//	connect(thinAction,	SIGNAL(toggled(bool)), this, SLOT(set_thintriangles_view(bool)));

	angleAction  = new QAction(tr("Thin Triangles-Angle"), this);
	angleAction->setCheckable(true);
	//	connect(angleAction,	SIGNAL(toggled(bool)), this, SLOT(set_angletriangles_view(bool)));
}
void KnotSubWindow::createMenus()
{
	fileMenu = menuBar()->addMenu(tr("&File"));
	fileMenu->addAction(openAction);
	fileMenu->addAction(saveAsAction);

	viewMenu = menuBar()->addMenu(tr("&View"));
	viewMenu->addAction(domainmeshAction);
	viewMenu->addAction(knotsAction);
    viewMenu->addAction(domainAction);
	viewMenu->addAction(curvatureAction);
	viewMenu->addAction(errdomainAction);
	viewMenu->addAction(segmentAction);
	viewMenu->addAction(densitydomainAction);
	viewMenu->addAction(thinAction);
	viewMenu->addAction(angleAction);
	viewMenu->addAction(delaunayAction);
	viewMenu->addAction(ddtAction);
	viewMenu->addAction(principaldirectionAction);
	viewMenu->addAction(triangulationAction);
	viewMenu->addAction(centroidAction);
	viewMenu->addAction(basisAction);
	viewMenu->addAction(basissumAction);
	viewMenu->addAction(interiorAction);
	viewMenu->addAction(basissupportAction);

	verificationMenu = menuBar()->addMenu(tr("V&erification"));
	verificationMenu->addAction(triangluationTestAction);
}
void KnotSubWindow::createContextMenu()
{
	knotsViewer->addAction(domainmeshAction);
	knotsViewer->addAction(knotsAction);
	knotsViewer->addAction(curvatureAction);
	knotsViewer->addAction(errdomainAction);
	knotsViewer->addAction(segmentAction);
	knotsViewer->addAction(domainAction);
	knotsViewer->addAction(densitydomainAction);
	knotsViewer->addAction(thinAction);
	knotsViewer->addAction(angleAction);
	knotsViewer->addAction(delaunayAction);
	knotsViewer->addAction(ddtAction);
	knotsViewer->addAction(principaldirectionAction);
	knotsViewer->addAction(triangulationAction);
	knotsViewer->addAction(centroidAction);
	knotsViewer->addAction(basisAction);
	knotsViewer->addAction(basissumAction);
	knotsViewer->addAction(indexAction);
	knotsViewer->addAction(interiorAction);
	knotsViewer->addAction(basissupportAction);

	knotsViewer->setContextMenuPolicy(Qt::ActionsContextMenu);
}

void KnotSubWindow::set_surface_data(CSurfaceData *data)
{
	surfacedata = data;
	knotsViewer->set_surface_data(data);

}
void KnotSubWindow::update_view()
{

	knotsViewer->update_view();
}

//void KnotSubWindow::closeEvent(QCloseEvent *event)
//{
//	event->accept();
//	emit window_close();
//}
//
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
//void KnotSubWindow::centroid_view(bool cv)
//{
//	/*knotsViewer->set_centroid_view(cv);*/
//}
//
//void KnotSubWindow::basis_view(bool bv)
//{
//	/*knotsViewer->set_basis_view(bv);*/
//}
//void KnotSubWindow::basis_sum_view(bool sv)
//{
//	/*knotsViewer->set_basis_sum_view(sv);*/
//}
//
//void KnotSubWindow::domain_view(bool dv)
//{
//	//knotsViewer->set_domain_view(dv);
//}
//
//void KnotSubWindow::basis_index()
//{
//	bool ok = false;
//	//if(bbasisview && surfacedata!=NULL)
//	if(surfacedata!=NULL)
//	{
//		int index =
//			QInputDialog::getInteger(this,
//			tr("Index of Basis Functions"),
//			tr("Enter index of Basis Functions"),
//			0,
//			0,
//			surfacedata->get_basis_size()-1, //basisCollection.size()-1,
//			1,
//			&ok);
//
//		if(ok)
//		{
//			basisIndex = index;
//			knotsViewer->set_basis_index(index);
//		}
//	}
//	updateStatusBar();
//}
//
//void KnotSubWindow::open()
//{
//	QString fileName = QFileDialog::getOpenFileName(this, tr("Select File..."), "./Data", tr("Knot Files(*.cin *.txt)")); 
//	if(fileName.length() == 0)
//	{ 
//		return; 
//	} 
//	QFile file(fileName.toAscii());
//	if ( file.open( QIODevice::ReadOnly ) ) 
//	{
//		surfacedata->clear_data();
//		QTextStream stream( &file );
//		QString line;
//		vector<Point_2> vp;
//		while ( !stream.atEnd() ) 
//		{
//			line = stream.readLine(); // 不包括“\n”的一行文本
//			QStringList ptlist = line.split(" ");
//			Point_2 pt(ptlist[0].toDouble(), ptlist[1].toDouble());
//			vp.push_back(pt);
//		}
//		surfacedata->add_knots(vp);
//		knotsAction->setChecked(true);
//		file.close();
//	}
//}
//
//bool KnotSubWindow::saveAs()
//{
//	//QString fileName = QFileDialog::getSaveFileName(this,  tr("Knots Output"), "./Data", tr("Knot Files (*.txt)"));
//	//if (!fileName.isNull())  
//	//{  
//	//	QFile file( fileName);
//	//	if ( file.open(QIODevice::WriteOnly)) 
//	//	{
//	//		QTextStream stream( &file );
//	//		//stream.setRealNumberPrecision(12);
//	//		Point_2 *knots = surfacedata->get_knots();
//	//		if(knots==NULL)
//	//			return false;
//	//		int num =surfacedata->get_nknots();
//	//		for (int i=0 ; i<num; i++ )
//	//		{ 
//	//			stream << knots[i].x() << " " << knots[i].y() << endl;
//	//		}
//	//		file.close();
//	//	 }
//	//}
//	knotsViewer->save_data();
//	return true;
//}
//
//void KnotSubWindow::interior_knots_view(bool iv)
//{
//	/*knotsViewer->set_basis_knots_view(iv);*/
//}
//
//void KnotSubWindow::show_basis_support(bool bv)
//{
//	/*knotsViewer->set_basis_support_view(bv);*/
//}
//
//void KnotSubWindow::show_triangulation_view(bool tv)
//{
//	/*knotsViewer->set_triangulation_view(tv);*/
//}
//
//void KnotSubWindow::show_delaunay_view(bool dv)
//{
//	//knotsViewer->set_delaunay_view(dv);
//}
//
//void KnotSubWindow::show_ddt_view(bool dv)
//{
//	/*knotsViewer->set_ddt_view(dv);*/
//}
//
//void KnotSubWindow::show_error_triangles(bool fv)
//{
//	/*knotsViewer->set_feature_blocks(fv);*/
//}
//
//void KnotSubWindow::set_thintriangles_view(bool tv)
//{
//	/*knotsViewer->set_thintriangles_view(tv);*/
//}
//
//void KnotSubWindow::set_angletriangles_view(bool av)
//{
//	/*knotsViewer->set_thintriangles_angle_view(av);*/
//}
//
void KnotSubWindow::set_mesh_view(bool dv)
{
	knotsViewer->set_domain_mesh_view(dv);
}

void KnotSubWindow::error_domain_view(bool ev)
{
	if (ev)
	{
		if (!surfacedata->curvature_error_completed())
		{
			QMessageBox::critical(this,tr("Knot  comfiguration  Error:"),tr("Knot has not been comfigurated yet,please comfigure knots."),QMessageBox::Ok);
			return;
		}

	}
	knotsViewer->set_error_domain_view(ev);
}

//void KnotSubWindow::density_domain_view(bool dv)
//{
//	/*knotsViewer->set_density_domain_view(dv);*/
//}
//
//void KnotSubWindow::set_principal_direction_view(bool pv)
//{
//	/*knotsViewer->set_principal_direction_view(pv);*/
//}
//
void KnotSubWindow::createStatusBar()
{
	statusLabel = new QLabel;

	statusBar()->addWidget(statusLabel);
	updateStatusBar();
}

void KnotSubWindow::updateStatusBar()
{
//	if(basisIndex!=-1)
//	{
//		/*SplineBasis basis;
//		if(!surfacedata->getBasis(basis, basisIndex))
//			return;
//
//		QString str = "Basis ID: ";
//		str.append(QString::number(basisIndex));
//		str.append(" Boundaries: ");
//		for (int i=0; i<basis.boundaries.size(); i++)
//		{
//			str.append("(");
//			for (int j=0; j<2; j++)
//			{
//				str.append(QString::number(basis.boundaries[i][j]));
//				str.append(", ");
//			}
//			str.append(QString::number(basis.boundaries[i][2]));
//			str.append("), ");
//		}
//		str.append("Interior: ");
//		str.append("(");
//		for (int j=0; j<2; j++)
//		{
//			str.append(QString::number(basis.interior[j]));
//			str.append(", ");
//		}
//		str.append(QString::number(basis.interior[2]));
//		str.append(")");
//		statusLabel->setText(str);*/
//	}
}