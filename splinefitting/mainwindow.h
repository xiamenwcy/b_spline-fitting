/*************************************************
* Copyright (c) 2014-至今,厦门大学计算机图形学实验室Graphics@XMU
* All rights reserved.
*
* 文件名称：mainwindow.h 
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
#ifndef _MAINWINDOW_H_
#define _MAINWINDOW_H_

//Qt
#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QGridLayout>
#include <QtGui/QHeaderView>
#include <QtGui/QMainWindow>
#include <QtGui/QMdiArea>
#include <QtGui/QMenu>
#include <QtGui/QMenuBar>
#include <QtGui/QStatusBar>
#include <QtGui/QToolBar>
#include <QtGui/QWidget>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtGui/QLabel>
#include <QtCore/QList>
#include <QtCore/QSettings>
#include <QtGui/QFileDialog>
#include <QtCore/QTextCodec>

//Openmesh
#include <OpenMesh/Core/Geometry/VectorT.hh>
#include <iostream>

#include "MeshSubWindow.h"
#include "KnotSubWindow.h"
#include "FitSubWindow.h"


class QMdiArea;
class QMenu;
class QAction;
class QMdiSubWindow;
class QActionGroup;


/** A main window class.
   It's a Qt GUI framework class. It contains several menus  and a midArea.
 */
class mainwindow:public  QMainWindow
{
	Q_OBJECT
public:
	  mainwindow(void);
	 ~mainwindow(void);

private slots: 

    // File menu 
	/**   open a new  model                 */
		void open();
	/**    save a model                     */
		void saveAs();
    /**   open a old  model               */
		void openRecentFile();

    // Fitting menu
	/**    mesh parametrization            */
		void mesh_parametrization();

	/**      mesh fitting                  */
		void mesh_fitting();


    //Options menu
	/**      load curvature               */
		void load_curvature();
	/**      input parameter:p,q,m,n       */
		void input_parameter();
	/**      set max curvature             */
		void adjust_upper();
	/**      adjust knots by curvature       */
		void adjust_knots_by_curvature();
	/**      add vertical knots            */
		void add_vertical_knots();
	/**       add horizon knots            */
		void add_horizon_knots();
	/**       adjust knots by fitting error    */
		void adjust_knots_by_fitting_error();


	// Windows menu
	/**      set window visible             */
	    void set_window_visible(bool bVisible);

	//  Help menu    
	/**     about  program             */
		void about();

    //close window slots
	/**   close meshwindow                */
		void meshwindow_close();
	/**   close knotwindow                */
		void knotswindow_close();
	/**   close fitwindow                 */
		void fitwindow_close();		

private:
	/**      create Actions                 */
	void createActions();
	/**      create menus                   */
	void createMenus();
	/**     load a new modle      */
	void loadmodel(const QString &fileName);
	/**     clear old data and build new data      */
	bool clear_data();

    /**    extracte file name from the full path     */
	QString strippedName(const QString &fullFileName);
    /**     set current file                         */
	void setCurrentFile(const QString &fileName);
    /**     update recent file                     */
	void updateRecentFileActions();
    
	/**   Max Recent Files number             */
	enum { 
		MaxRecentFiles = 5  /**<  5 */  
	};
	/**  RecentFile Actions                 */
	QAction *recentFileActions[MaxRecentFiles];
	/**     Current file                    */
    QString curFile;


	/**     File   Menu                      */
	QMenu *fileMenu;
	/**      SaveAs Menu                     */
	QMenu *saveAsMenu;
	/**     Fitting  Menu                   */
	QMenu *fittingMenu;
	/**    Mesh Menu                         */
	QMenu *meshMenu;
	/**     Options Menu                    */
	QMenu *optionsMenu;
	/**    Windows Menu                     */
	QMenu *windowMenu;
	/**    Help Menu                        */
	QMenu *helpMenu;
	
    // File menu 
	/**      Separator   Action            */
	QAction *separatorAction;
	/**      Open Action                 */
	QAction *openAction;
	/**      SaveCurvature Action           */
	QAction *saveCurvatureAction;
	/**       SaveFit  Action                */
	QAction *saveFitAction;
	/**     SaveError Action               */
	QAction *saveErrorAction;
	/**      Exit  the whole program Action     */
	QAction *exitAction;

   // Fitting menu
	/**    Parametrization Action           */
	QAction *parametrizationAction;
	/**    Meshfitting Action               */
	QAction *meshfittingAction;

	//Options menu
	/**      Load Curvature Action           */
	QAction *CurvatureLoadingAction;
	/**      Input Parameter:p,q,m,n        */
	QAction *ParaInputAction;
	/**     Set max curvature  Action         */
	QAction *CurvatureUpperboundAction;
	/**     Adjust knots by curvature  Action        */
	QAction *KnotAdjust_curvature_Action;
	/**      Add horizon knots  Action        */
	QAction *add_h_Action;
	/**     Add vertical knots  Action       */
	QAction *add_v_Action;
	/**     Adjust knots by fitting error  Action        */
	QAction *KnotAdjust_fitting_Action;
	/**      WindowAction Group           */
	QActionGroup *windowActionGroup;
	/**      Knots Window Action          */
	QAction *kontsAction;
	/**      Mesh Window Action            */
	QAction *meshAction;
	/**      Fitting Window Action         */
	QAction *fittingAction;
	
	//  Help menu  
	/**  about  program Action         */
	QAction *aboutAction;
	/**   about Qt Action                  */
	QAction *aboutQtAction;
	
	/**  A model is  opened  ever or not?     */
	bool is_ever_open;
	/**  MDI Area            */
	QMdiArea *mdi;

	/**    Mesh SubWindow               */
	MeshSubWindow *meshwindow;
	/**    Knot SubWindow                */
	KnotSubWindow *knotswindow;
	/**    Fit  SubWindow                */
	FitSubWindow  *fitwindow;

	/**   Core surface data                 */
	CSurfaceData *surfacedata;

	/**    Init  mesh SubWindow      */
	QMdiSubWindow* mesh;
	/**    Init  knot SubWindow    */
	QMdiSubWindow* knots;
	/**    Init  fit  SubWindow    */
	QMdiSubWindow* fitting;
};


//------------------------------------------------------------------------

#endif