/*************************************************
* Copyright (c) 2014-����,���Ŵ�ѧ�����ͼ��ѧʵ����Graphics@XMU
* All rights reserved.
*
* �ļ����ƣ�MeshSubWindow.h 
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
    /** ����MeshSubWindow���������򲿼� */
	MeshViewer *get_mesh_viewer();
	
	void set_surface_data(CSurfaceData *data);
   
	bool mesh_parametrization();
    /**  ���´�����ʾ */
	void update_view();
    /** �������������ݣ��ָ���ʼ�� */
	void clear_data();
    /** ����ʾ��������������������д���ļ��� */
	bool write_mesh(QString &fileName, Mesh_Type type);
  
signals:
    /**  ���ʹ��ڹر��ź� */
	void window_close();

protected:
	void closeEvent(QCloseEvent *event);

private:
	void createStatusBar();
	void createActions();
	void createMenus();
	/**  ���������Ĳ˵���Ҳ�����Ҽ��˵� */
	void createContextMenus();
   

	//������ʾ��״̬����
	/**  ����ģ����   */
	QLabel *filenameLabel;
	/**  ����ģ�Ͷ�����Ŀ */
	QLabel *pointsLabel;
    /**  ����ģ��������Ƭ��Ŀ */
	QLabel *faceLabel;

	/**    ��ά��ͼ�࣬��MeshSubWindow���������򲿼�  */
	MeshViewer *m_pMeshViewer;
    /**  core surface data  */
	CSurfaceData *surfacedata;
    
    /**  original mesh*/
	QAction *originalmeshAction;
    /**  original mesh with  curvature  */
	QAction *curvatureAction;
   /** ��ʾ��ͬ����Ĳ˵� */
	QMenu *viewMenu;


private slots:

		void set_origin_mesh_view(bool ov);

		void set_curvature_show(bool cv);

		void updateStatusBar();
	
};

//-----------------------------------------------------------------
#endif