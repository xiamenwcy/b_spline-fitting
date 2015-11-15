/*************************************************
* Copyright (c) 2014-至今,厦门大学计算机图形学实验室Graphics@XMU
* All rights reserved.
*
* 文件名称：mainwindow.cpp
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
#include <QtGui/QInputDialog>

#include "mainwindow.h"

mainwindow::mainwindow(void)
{
	
	mdi=new QMdiArea(this);
	setCentralWidget(mdi);

	meshwindow = new(std::nothrow) MeshSubWindow();
	meshwindow->setAttribute(Qt::WA_DeleteOnClose);
	mesh = mdi->addSubWindow(meshwindow);
	meshwindow->setWindowTitle("Mesh Window");
	

	knotswindow = new(std::nothrow)  KnotSubWindow();
	knotswindow->setAttribute(Qt::WA_DeleteOnClose);
	knots = mdi->addSubWindow(knotswindow);
    knotswindow->setWindowTitle("Knot Window");


	fitwindow=new(std::nothrow) FitSubWindow();
	fitwindow->setAttribute(Qt::WA_DeleteOnClose);
	fitting=mdi->addSubWindow(fitwindow);
	fitwindow->setWindowTitle("Fitting Window");
	
    is_ever_open=false;

	surfacedata = new(std::nothrow)  CSurfaceData();
	meshwindow->set_surface_data(surfacedata); //浅复制，共享一个地址
	knotswindow->set_surface_data(surfacedata);
	fitwindow->set_surface_data(surfacedata);

	createActions();
	createMenus();
	(void)statusBar();//创建状态栏

    setWindowFilePath(QString());//This property holds the file path associated with a widget.

	connect(meshwindow, SIGNAL(window_close()), this, SLOT(meshwindow_close()));
	connect(knotswindow, SIGNAL(window_close()), this, SLOT(knotswindow_close()));
    connect(fitwindow, SIGNAL(window_close()), this, SLOT(fitwindow_close()));
	setWindowTitle("B_Spline Fitting");
}

mainwindow::~mainwindow(void)
{
	delete surfacedata;
	surfacedata=NULL;
}
void mainwindow::createActions()
{

	openAction = new QAction(QIcon(":/splinefitting/images/open.png"),tr("&Open..."), this);
    openAction->setShortcut(QKeySequence::Open);
    openAction->setStatusTip(tr("Open an existing mesh file"));
    connect(openAction, SIGNAL(triggered()), this, SLOT(open()));

	for (int i = 0; i < MaxRecentFiles; ++i) {
		recentFileActions[i] = new QAction(this);
		recentFileActions[i]->setVisible(false);
		 connect(recentFileActions[i], SIGNAL(triggered()),
		       this, SLOT(openRecentFile()));
	}

    saveCurvatureAction = new QAction(tr("Save &Curvature Mesh..."), this);
    saveCurvatureAction->setStatusTip(tr("Save the mesh under a new "
                                  "name"));
    connect(saveCurvatureAction, SIGNAL(triggered()), this, SLOT(saveAs()));

	saveFitAction = new QAction(tr("Save &Fitted Mesh..."), this);
    saveFitAction->setStatusTip(tr("Save the mesh under a new "
                                  "name"));
    connect(saveFitAction, SIGNAL(triggered()), this, SLOT(saveAs()));

	saveErrorAction = new QAction(tr("Save &Error Mesh..."), this);
    saveErrorAction->setStatusTip(tr("Save the mesh under a new "
                                  "name"));
    connect(saveErrorAction, SIGNAL(triggered()), this, SLOT(saveAs()));


    exitAction = new QAction(QIcon(":/splinefitting/images/exit.png"),tr("E&xit"), this);
    exitAction->setShortcut(tr("Ctrl+Q"));
    exitAction->setStatusTip(tr("Exit the application"));
    connect(exitAction, SIGNAL(triggered()), this, SLOT(close()));

	aboutAction = new QAction(tr("&About"), this);
    aboutAction->setStatusTip(tr("Show the application's About box"));
    connect(aboutAction, SIGNAL(triggered()), this, SLOT(about()));

    aboutQtAction = new QAction(tr("About &Qt"), this);
    aboutQtAction->setStatusTip(tr("Show the Qt library's About box"));
    connect(aboutQtAction, SIGNAL(triggered()), qApp, SLOT(aboutQt()));

	parametrizationAction = new QAction(tr("Mesh Parametrization"), this);
	parametrizationAction->setCheckable(true);
	connect(parametrizationAction, SIGNAL(triggered()), this, SLOT(mesh_parametrization()));
	meshfittingAction = new QAction(tr("Mesh Fitting"), this);
	connect(meshfittingAction, SIGNAL(triggered()), this, SLOT(mesh_fitting()));

	CurvatureUpperboundAction=new QAction(tr("Adjust Curvature Upper Bound"),this);
	CurvatureUpperboundAction->setCheckable(true);
	connect(CurvatureUpperboundAction, SIGNAL(triggered()), this, SLOT(adjust_upper()));

	add_v_Action = new QAction(tr("Add vertical knots"), this);
	add_v_Action->setCheckable(true);
	connect(add_v_Action, SIGNAL(triggered()), this, SLOT(add_vertical_knots()));

	add_h_Action = new QAction(tr("Add horizon knots"), this);
	add_h_Action->setCheckable(true);
	connect(add_h_Action, SIGNAL(triggered()), this, SLOT(add_horizon_knots()));

	CurvatureLoadingAction=new QAction(tr("Loading Curvature"),this);
	CurvatureLoadingAction->setCheckable(true);
	connect(CurvatureLoadingAction, SIGNAL(triggered()), this, SLOT(load_curvature()));


	ParaInputAction=new QAction(tr("Parameters Input"),this);
	ParaInputAction->setCheckable(true);
	connect(ParaInputAction, SIGNAL(triggered()), this, SLOT(input_parameter()));

	KnotAdjust_curvature_Action=new QAction(tr("Adjust  knots  by curvature"),this);
	KnotAdjust_curvature_Action->setCheckable(true);
	connect(KnotAdjust_curvature_Action, SIGNAL(triggered()), this, SLOT(adjust_knots_by_curvature()));
    
    KnotAdjust_fitting_Action=new QAction(tr("Adjust knots by fitting_error"),this);
    KnotAdjust_fitting_Action->setCheckable(true);
	connect(KnotAdjust_fitting_Action,SIGNAL(triggered()),this,SLOT(adjust_knots_by_fitting_error()));
    

	meshAction = new QAction(tr("Mesh Window"), this);
	meshAction->setCheckable(true);
	meshAction->setStatusTip(tr("Show or hide the mesh window's"));
	connect(meshAction, SIGNAL(toggled(bool)), this, SLOT(set_window_visible(bool)));

	kontsAction = new QAction(tr("Knots Window"), this);
	kontsAction->setCheckable(true);
	kontsAction->setStatusTip(tr("Show or hide the knots window's"));
	connect(kontsAction, SIGNAL(toggled(bool)), this, SLOT(set_window_visible(bool)));

	fittingAction = new QAction(tr("fitting Window"), this);
	fittingAction->setCheckable(true);
	fittingAction->setStatusTip(tr("Show or hide the fitting window's"));
	connect(fittingAction, SIGNAL(toggled(bool)), this, SLOT(set_window_visible(bool)));

	windowActionGroup = new QActionGroup(this);
	windowActionGroup->addAction(meshAction);
	windowActionGroup->addAction(kontsAction);
	windowActionGroup->addAction(fittingAction);
	meshAction->setChecked(true);

	
}

void mainwindow::createMenus()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->addAction(openAction);

	saveAsMenu = fileMenu->addMenu(QIcon(":/splinefitting/images/saveAs.png"),tr("Save As"));
	saveAsMenu->addAction(saveCurvatureAction);
	saveAsMenu->addAction(saveFitAction);
	saveAsMenu->addAction(saveErrorAction);
	separatorAction = fileMenu->addSeparator();
	for (int i = 0; i < MaxRecentFiles; ++i) 
	{
		fileMenu->addAction(recentFileActions[i]);
	}
	updateRecentFileActions();
	 fileMenu->addSeparator();
	fileMenu->addAction(exitAction);
   

    fittingMenu = menuBar()->addMenu(tr("&Fitting"));
	meshMenu = fittingMenu->addMenu(tr("&Mesh"));
    meshMenu->addAction(parametrizationAction);
    meshMenu->addAction(meshfittingAction);
	fittingMenu->setDisabled(true);//设置菜单灰色，不可用

	optionsMenu = menuBar()->addMenu(tr("&Options"));
	optionsMenu->addAction(CurvatureLoadingAction);
	optionsMenu->addAction(ParaInputAction);
	optionsMenu->addAction(CurvatureUpperboundAction);
	optionsMenu->addAction(KnotAdjust_curvature_Action);

	optionsMenu->addSeparator();

	optionsMenu->addAction(add_v_Action);
	optionsMenu->addAction(add_h_Action);
	optionsMenu->addAction(KnotAdjust_fitting_Action);


	windowMenu = menuBar()->addMenu(tr("&Windows"));
	windowMenu->addAction(kontsAction);
	windowMenu->addAction(meshAction);
	windowMenu->addAction(fittingAction);

    menuBar()->addSeparator();

    helpMenu = menuBar()->addMenu(tr("&Help"));
    helpMenu->addAction(aboutAction);
    helpMenu->addAction(aboutQtAction);
}

/**   \brief Open a new  model.

      If ever a model has opened,we delete this model and
      then open a new model.
*/
void mainwindow::open()
{
	QString fileName = QFileDialog::getOpenFileName(this, 
		tr("Select File..."), "./splinefitting/model", tr("Model Files(*.off *.obj *.stl)"));
	if(!fileName.isEmpty())
	{
		loadmodel(fileName);
	}
}
void mainwindow::loadmodel(const QString &fileName)
{
	if(is_ever_open)
	{
		clear_data();
	}
	if (!surfacedata->read_mesh(fileName))
	{
		statusBar()->showMessage(tr("Loading mesh unsuccessfully"), 2000);
		return;
	}
	statusBar()->showMessage(tr("Loading mesh successfully"), 2000);
	setCurrentFile(fileName);
	meshwindow->update_view();//显示model
	fittingMenu->setEnabled(true);

	meshfittingAction->setDisabled(true);
	meshwindow->showMaximized();
	is_ever_open=true;

}
void mainwindow::clear_data()
{
	meshwindow->clear_data();
	knotswindow->clear_data();
	fitwindow->clear_data();
	delete surfacedata ;
	surfacedata=NULL;
	surfacedata = new(std::nothrow) CSurfaceData();
	surfacedata->iter_num=1;
	meshwindow->set_surface_data(surfacedata);
	knotswindow->set_surface_data(surfacedata);
	fitwindow->set_surface_data(surfacedata);
}
void mainwindow::openRecentFile()
{
	QAction *action = qobject_cast<QAction *>(sender());
	if (action)
	{
		loadmodel(action->data().toString());
	}
}
void mainwindow::setCurrentFile(const QString &fileName)
{
	curFile = fileName;
	setWindowFilePath(curFile);

	//亲测，必须使用QSettings ( const QString & organization, const QString & application = QString(), QObject * parent = 0 )    
	QSettings settings("xmu","b_spline");
	QStringList files = settings.value("recentModelList").toStringList();
	files.removeAll(fileName);
	files.prepend(fileName);
	while (files.size() > MaxRecentFiles)
		files.removeLast();

	settings.setValue("recentModelList", files);

	foreach (QWidget *widget, QApplication::topLevelWidgets()) {
		mainwindow *mainWin = qobject_cast<mainwindow *>(widget);
		if (mainWin)
			mainWin->updateRecentFileActions();
	}
	
}
void mainwindow::updateRecentFileActions()
{
	//亲测，必须使用QSettings ( const QString & organization, const QString & application = QString(), QObject * parent = 0 )    
	QSettings settings("xmu","b_spline");
	QStringList files = settings.value("recentModelList").toStringList();

	int numRecentFiles = qMin(files.size(), (int)MaxRecentFiles);

	for (int i = 0; i < numRecentFiles; ++i) {
		QString text = tr("&%1 %2").arg(i + 1).arg(strippedName(files[i]));
		recentFileActions[i]->setText(text);
		recentFileActions[i]->setData(files[i]);
		recentFileActions[i]->setVisible(true);
	}
	for (int j = numRecentFiles; j < MaxRecentFiles; ++j)
		recentFileActions[j]->setVisible(false);
    separatorAction->setVisible(numRecentFiles > 0);
}

QString mainwindow::strippedName(const QString &fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}



/**  Save a model.
     we save  three models below:
	-  Curvature_Mesh
	-  Fitted_Mesh
	-  Error_Mesh

*/
void mainwindow::saveAs()
{
	bool ret;
    QString fileName = QFileDialog::getSaveFileName(this,
                               tr("Save Mesh"), ".",
                               tr("Mesh files (*.off *.obj)"));
    if (fileName.isEmpty())
	{
		ret=false;
		return ;
	}
	
	QObject *send = sender();
	if(send==saveCurvatureAction)
		ret =  meshwindow->write_mesh(fileName, Curvature_Mesh);
	if(send==saveFitAction)
		ret =  fitwindow->write_mesh(fileName, Fitted_Mesh);
	if(send==saveErrorAction)
		ret =  fitwindow->write_mesh(fileName, Error_Mesh);
    if (ret==true)
		statusBar()->showMessage(tr("Writing mesh successfully"), 2000);
	else
		statusBar()->showMessage(tr("Writing mesh Unsuccessfully"), 2000);

	
}
void mainwindow::about()
{
	QMessageBox::about(this, tr("About B-spline fitting"),
		tr("The <b>B-spline</b> example demonstrates how to "
		"use B-splie to fitting we-known mesh, "
		"It's very easy,but so perfect! "));
}


void mainwindow::meshwindow_close()
{
	delete meshwindow;
	meshwindow = NULL;
}
void mainwindow::knotswindow_close()
{
	delete knotswindow;
	knotswindow = NULL;
}
void mainwindow::fitwindow_close()
{
	delete fitwindow;
	fitwindow = NULL;
}
void mainwindow::mesh_fitting()
{
	if(fitwindow == NULL)
	{
		return;
	}
	if(surfacedata->get_original_mesh() != NULL)
	{
		if(fitwindow->mesh_fitting())
		{
			surfacedata->set_fitting_completed(true);
			fitwindow->update_fitting_view();
		}
		else
		{
          QMessageBox::critical(this,tr("Fitting Completing:"),tr("Fitting Failure,please check error."),QMessageBox::Ok);

		}
	}
}


void mainwindow::mesh_parametrization()
{
	
		if(meshwindow == NULL)
		{
			return;
		}
		if(surfacedata->get_original_mesh() != NULL)
		{
			if(meshwindow->mesh_parametrization())
				knotswindow->update_view();

		}
	
}
/** 设置曲率上界.
     一旦我们计算的最大曲率点比较孤立，比如只有一个，则我们需要降低曲率上界的标准，默认最大曲率=曲率上界
    使之整体曲率比较大的点突出来。
*/
void mainwindow::adjust_upper()
{
	double max_meancur = surfacedata->get_max_meancurvature();
	QString str = "The maximal mean curvature of original surface is ";
	str.append(QString::number(max_meancur));
	QMessageBox::information(this, "maximal curvature of Original Surface", str, QMessageBox::Yes, QMessageBox::Yes);
	bool ok = false;

	double uppercur = 
		QInputDialog::getDouble(this, 
		tr("Curvature Upper bound"),
		tr("Please input number:"),
		0.00,
		-500,
		500,
		8,
		&ok);
	if(!ok)
		uppercur = max_meancur;
	surfacedata->set_max_curvature(uppercur);

}
void  mainwindow::load_curvature()
{
	QStringList files = QFileDialog::getOpenFileNames(this, 
                tr("Load two curvature files:"), "./splinefitting/model", tr("Curvature Files(*.txt)"));
	if (files.size()==2&& surfacedata->compute_curvature(files))
	{
		statusBar()->showMessage(tr("Loading curvature successfully"), 2000);
		surfacedata->set_curvature_loadingstate(true);
	}
	else
	{
		statusBar()->showMessage(tr("Loading curvature unsuccessfully"), 2000);
		surfacedata->set_curvature_loadingstate(false);
	}


}

void mainwindow::input_parameter()
{
	bool ok;
	
	B_parameter *parameter=new(std::nothrow) B_parameter(0,0,0,0);
	if (parameter==NULL)//分配失败
	{
		return;
	}
	int p,q,m,n;
input:
	 p=QInputDialog::getInt(this,tr("parameter inputting"),tr("Please input parameter p:"),3,1,100,1,&ok);
	if (ok){	
		parameter->setp(p);
		 q=QInputDialog::getInt(this,tr("parameter inputting"),tr("Please input parameter q:"),3,1,100,1,&ok);
		if (ok)
		{
			parameter->setq(q);
			 m=QInputDialog::getInt(this,tr("parameter inputting"),tr("Please input parameter m:"),3,1,100,1,&ok);
			if (ok)
			{
				parameter->setm(m);
				 n=QInputDialog::getInt(this,tr("parameter inputting"),tr("Please input parameter n:"),3,1,100,1,&ok);
				if (ok)
				{
					parameter->setn(n);
				}
			}
		}

	}
	// m>=p,n>=q 
    if (m<p||n<q)
    {
		QMessageBox::critical(this,tr("Input Error:"),tr("Note: m>=p,n>=q. Please input again."),QMessageBox::Ok);
		goto input;
    }
	int a=m+2-p;
	int b=n+2-q;
	parameter->setnum(a,b);
	if (surfacedata==NULL)
		return;
	surfacedata->unum=a;
	surfacedata->vnum=b;
	surfacedata->setparameter(parameter);
	meshfittingAction->setEnabled(true);

}

void mainwindow::add_vertical_knots()
{
	if (surfacedata==NULL)
		return;
	bool ok;
	int value1=QInputDialog::getInt(this,tr("Knots adding num every time"),tr("Please input Vertical_Knots_adding num:"),10,0,100,1,&ok);
	if(!ok)
		return;
	surfacedata->set_knots_adding_num(value1);
	if(!surfacedata->add_knots())
	{
		QMessageBox::critical(this,tr("Knots Updating:"),tr("Knots has NOT been updated yet,please check error."),QMessageBox::Ok);
		return;
	}
	QMessageBox::information(this,tr("Knots Updating:"),tr("Knots has  been updated ,please check knots configuration in the Knot Window."),QMessageBox::Ok);


}
void mainwindow::add_horizon_knots()
{
	if (surfacedata==NULL)
		return;
	bool ok;
	int value1=QInputDialog::getInt(this,tr("Knots adding num every time"),tr("Please input Horizon_Knots_adding num:"),10,0,100,1,&ok);
	if(!ok)
		return;
	surfacedata->set_horizon_knots_addnum(value1);
	if(!surfacedata->add_horizon_knots())
	{
		QMessageBox::critical(this,tr("Knots Updating:"),tr("Horizonal  knots has NOT been updated yet,please check error."),QMessageBox::Ok);
		return;
	}
	QMessageBox::information(this,tr("Knots Updating:"),tr("Horizonal  Knots has  been updated ,please check knots configuration in the Knot Window."),QMessageBox::Ok);



}
void  mainwindow::adjust_knots_by_curvature()
{
	bool ok;
	int value1=QInputDialog::getInt(this,tr("Iteration Times inputting"),tr("Please input iteration times :"),0,0,100,1,&ok);
	if(!ok)
		return;
	int ret=QMessageBox::question(this,tr("Curvature Loding:"),tr("Curvature lodded successfully?  If not,please Loading Curvature."),QMessageBox::Yes,QMessageBox::No);
	if(ret==QMessageBox::No)
		return;
   surfacedata->set_knots_iteration_times(value1);
	if(!surfacedata->adjust_knots_by_curvature())
   {
	   QMessageBox::critical(this,tr("Knots Updating:"),tr("Knots has NOT been updated yet,please check error."),QMessageBox::Ok);
	   return;
   }
 QMessageBox::information(this,tr("Knots Updating:"),tr("Knots has  been updated ,please check knots configuration in the Knot Window."),QMessageBox::Ok);
  

}
void mainwindow::adjust_knots_by_fitting_error()
{
	bool ok;
	int value1=QInputDialog::getInt(this,tr("Iteration Times inputting"),tr("Please input iteration times :"),-1,-1,100,1,&ok);
	if(!ok)
		return;
	surfacedata->set_knots_iteration_times(value1);
	if(!surfacedata->adjust_knots_by_fitting_error())
	{
		QMessageBox::critical(this,tr("Knots Updating:"),tr("Knots has NOT been updated yet,please check error."),QMessageBox::Ok);
		return;
	}
	QMessageBox::information(this,tr("Knots Updating:"),tr("Knots has  been updated ,please check knots configuration in the Knot Window."),QMessageBox::Ok);


}

void mainwindow::set_window_visible(bool bVisible)
{
	if(mesh!=NULL)
	{
		QObject *send = sender();
		if(send == kontsAction)
		{
			mdi->setActiveSubWindow(qobject_cast<QMdiSubWindow *>(knotswindow));
		    knotswindow->showMaximized();
		}
		else if(send == meshAction)
		{
			mdi->setActiveSubWindow(qobject_cast<QMdiSubWindow *>(meshwindow));
			 meshwindow->showMaximized();
		}
		else
		{
			mdi->setActiveSubWindow(qobject_cast<QMdiSubWindow *>(fitwindow));
			fitwindow->showMaximized();
		}
	}
}

