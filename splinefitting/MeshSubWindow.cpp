/*************************************************
* Copyright (c) 2014-����,���Ŵ�ѧ�����ͼ��ѧʵ����Graphics@XMU
* All rights reserved.
*
* �ļ����ƣ�MeshSubWindow.cpp
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

#include "MeshSubWindow.h"
#include <QtGui/QtGui>

MeshSubWindow::MeshSubWindow(void)
{
	m_pMeshViewer = new MeshViewer;
	m_pMeshViewer->setAttribute(Qt::WA_OpaquePaintEvent);
	m_pMeshViewer->setAttribute(Qt::WA_NoSystemBackground);
	setCentralWidget(m_pMeshViewer);
	setWindowTitle("Mesh Viewer");

	surfacedata = NULL;
	createActions();
	createMenus();
	createContextMenus();
	createStatusBar();

	
}

MeshSubWindow::~MeshSubWindow(void)
{
	clear_data();
}
void MeshSubWindow::clear_data()
{
	originalmeshAction->setChecked(true);
	curvatureAction->setChecked(false);
	m_pMeshViewer->clear_data();
}

MeshViewer *MeshSubWindow::get_mesh_viewer()
{
	return m_pMeshViewer;
}

void MeshSubWindow::createActions()
{
	originalmeshAction = new QAction(tr("Original Mesh"), this);
	originalmeshAction->setCheckable(true);
	originalmeshAction->setChecked(true);
	connect(originalmeshAction, SIGNAL(toggled(bool)), this, SLOT(set_origin_mesh_view(bool)));


	curvatureAction = new QAction(tr("Curvature Mesh"), this);
	curvatureAction->setCheckable(true);
	connect(curvatureAction, SIGNAL(toggled(bool)), this, SLOT(set_curvature_show(bool)));


}

void MeshSubWindow::createMenus()
{
	viewMenu = menuBar()->addMenu(tr("&View"));
	viewMenu->addAction(originalmeshAction);
	viewMenu->addAction(curvatureAction);

	

}

void MeshSubWindow::createContextMenus()
{
	m_pMeshViewer->addAction(originalmeshAction);
	m_pMeshViewer->addAction(curvatureAction);
	
	m_pMeshViewer->setContextMenuPolicy(Qt::ActionsContextMenu);
}

/** ����״̬��.
*   ״̬����Ϣ��Ϊ����:
    - ��ʱ��Ϣ,��һ�����ʾ��Ϣ,������showMessage()��ʾ.
	- ������Ϣ,����ʾҳ���ͺ���,������addWidget()���һ��QLabel��״̬���������������ߣ����ܻᱻ��ʱ��Ϣ�ڸ�
	- ������Ϣ,����ʾ�汾�Ż����ڣ�������addPermanentWidget()���һ��QLable��״̬��,����������ұߣ����ᱻ��ʱ��Ϣ�ڸ�.
*/
void MeshSubWindow::createStatusBar()
{
	filenameLabel = new QLabel(this);
	pointsLabel = new QLabel(this);
	faceLabel = new QLabel(this);
	 
	statusBar()->addWidget(filenameLabel, 1);
	statusBar()->addWidget(pointsLabel, 1);
	statusBar()->addWidget(faceLabel, 1);
	updateStatusBar();
}


void MeshSubWindow::updateStatusBar()
{
	if(surfacedata!=NULL)
	{
		QString points = "Point: ";
		int pointNum = surfacedata->get_mesh_vertex_num();
		if(pointNum == -1)
		{
			points = "";
		}
		else
		{
			points.append(QString::number(pointNum, 10));//��pointNum����ʮ�������
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
	}
}

void MeshSubWindow::closeEvent(QCloseEvent *event)
{
	event->accept();
	emit window_close();
}

void MeshSubWindow::set_surface_data(CSurfaceData *data)
{
	surfacedata = data;
	updateStatusBar();
	m_pMeshViewer->set_surface_data(data);
}
void MeshSubWindow::update_view()
{
	updateStatusBar();
	m_pMeshViewer->update_view();
}

bool MeshSubWindow::mesh_parametrization()
{
	return surfacedata->mesh_parameterization();
}



void MeshSubWindow::set_origin_mesh_view(bool ov)
{
	m_pMeshViewer->set_origin_mesh_view(ov);
}


void MeshSubWindow::set_curvature_show(bool cv)
{
	if (cv)
	{
		if (!surfacedata->curvature_loading())
		{
			QMessageBox::critical(this,tr("Curvature Loading Error:"),tr("Curvature has not been loadded yet,please loading curvature."),QMessageBox::Ok);
			return;
		}
		
	}
	m_pMeshViewer->set_curvature_show(cv);

}


bool MeshSubWindow::write_mesh(QString &fileName, Mesh_Type type)
{
	return m_pMeshViewer->write_mesh(fileName, type);
}

