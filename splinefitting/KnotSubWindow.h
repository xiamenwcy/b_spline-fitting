/*************************************************
* Copyright (c) 2014-����,���Ŵ�ѧ�����ͼ��ѧʵ����Graphics@XMU
* All rights reserved.
*
* �ļ����ƣ�KnotSubWindow.h 
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


#ifndef _KNOTSUBWINDOW_H_
#define _KNOTSUBWINDOW_H_

//Qt
#include <QMainWindow>
#include <QtCore/QString>
#include <QtCore/QStringList>

#include "splineknotsviewer.h"

class QAction;
class QLabel;

/** Knot  subwindow class.
   It's a subwindow of main window.It's used to show  parameterized surface.
 */
class KnotSubWindow: public QMainWindow
{
	Q_OBJECT
public:
	KnotSubWindow(void);
	~KnotSubWindow(void);
	/**   ��������      */
	void clear_data();
	/**   ������ͼ,��ʾ����������  */
	void update_view();
	/**  ���ú�������  */
    void set_surface_data(CSurfaceData *data);
signals:
	void window_close();
protected:
	void closeEvent(QCloseEvent *event);
private:
	/**  core surface data */
	CSurfaceData *surfacedata;
	/** ��ά��ͼ�࣬��KnotSubWindow���������򲿼� */
	SplineKnotsViewer *knotsViewer;
	/** �ڵ�����ʾ�˵�ѡ��  */
	QAction *knotsAction;
	/** ������ʾ�˵�ѡ��   */
	QAction *curvatureAction;
	/** ������ʾ�˵�ѡ��  */
	QAction *domainmeshAction;
	/** ���ʻ������˵�ѡ�� */
	QAction *curvature_error_Action;
	/** ������˵�ѡ��    */
	QAction *fitting_error_Action;
	
    /** ��ͼ��ʾ�˵� */
	QMenu *viewMenu;
    /** �����˵�ѡ��*/
	void createActions();
	/** �����˵�*/
	void createMenus();
	/** ���������Ĳ˵���Ҳ�����Ҽ��˵� */
	void createContextMenu();

public slots:
	 /** ��ʾ�ڵ��� */
	void knots_view(bool kv);
	/**  ��ʾ����   */
	void set_curvature_show(bool cv);
	/**  ��ʾ����   */
	void set_mesh_view(bool dv);
	/** ��ʾ���ʻ������ */
	void error_curvature_view(bool ev);
	/** ��ʾ������    */
	void error_fitting_view(bool ev);
	
};

//-------------------------------------------------------
#endif