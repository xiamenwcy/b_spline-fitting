/*************************************************
* Copyright (c) 2014-至今,厦门大学计算机图形学实验室Graphics@XMU
* All rights reserved.
*
* 文件名称：FittingMeshViewer.cpp
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

//Openmesh


//Qt
#include <QtGui/QMessageBox>
#include <QtGui/QInputDialog>

#include <fstream>
#include "gl.h"
#include "FittingMeshViewer.h"

using namespace std;


FittingMeshViewer::FittingMeshViewer(void)
{
	restoreStateFromFile();
	surfacedata = NULL;
	
	 bfitted_mesh_view = true;
     berror_show = false;
	 bmax_error_point_show = false;
	 bcontroledge_show = false;

	 max_err = 1;
	
	setAutoBufferSwap( false );
	setAutoFillBackground( false );
	setMouseTracking(true);
	
	bBox = Bbox_3(-1, 1, -1, 1, -1, 1);
}

FittingMeshViewer::~FittingMeshViewer(void)
{
		clear_data();
}
void FittingMeshViewer::clear_data()
{

	bfitted_mesh_view = false;

    berror_show = false;
	
	bcontroledge_show = false;
	
	bBox = Bbox_3(-1, 1, -1, 1, -1, 1);

}

void FittingMeshViewer::set_scene(Bbox_3 &box)
{
	
	Mesh::Point     bbMin(box.xmin(), box.ymin(), box.zmin());
	Mesh::Point     bbMax(box.xmax(), box.ymax(), box.zmax());
	Mesh::Point     center= (bbMin + bbMax)*0.5;
	radius=0.5*(bbMin - bbMax).norm();
	setSceneCenter(Vec(center[0], center[1], center[2]));
	setSceneRadius(radius);
	camera()->showEntireScene();
}

void FittingMeshViewer::draw()
{
	//在当前窗口中进行OpenGL的绘制
	makeCurrent();

	const Vec cameraPos = camera()->position();
	const GLfloat pos[4] = {cameraPos[0], cameraPos[1], cameraPos[2], 1.0};


	glLightfv(GL_LIGHT1, GL_POSITION, pos);

	// Orientate light along view direction
	glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, camera()->viewDirection());

	//! [4]
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	//! [4]

	//! [6]
	glClearColor(1.0, 1.0, 1.0, 1.0);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_DEPTH_TEST);

	if(bfitted_mesh_view)
	{
		glEnable(GL_LIGHTING);
		glColor3d(0.7, 0.7, 0.5);
		
	}

	if(bfitted_mesh_view)
		draw_fitted_mesh();

	if( bfitted_mesh_view)
	{
		glDisable(GL_LIGHTING);
	}

	if(bcontroledge_show)
	{
		glColor3ub(0,255,255);
		draw_control_mesh_orginal();
	}
	if(berror_show)
		draw_error_mesh();

	glShadeModel(GL_FLAT);
	glDisable(GL_DEPTH_TEST);
	
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	//swapBuffers();//若是只是显示三维，而没有显示painter，要加上这句话。但是若是二三维混合，则只需在paintevent中加这句话即可。
}

void FittingMeshViewer::setDefaultMaterial()
{
	GLfloat mat_a[] = {0.1f, 0.1f, 0.1f, 1.0f};
	GLfloat mat_d[] = {0.7f, 0.7f, 0.5f, 1.0f};
	GLfloat mat_s[] = {1.0f, 1.0f, 1.0f, 1.0f};
	GLfloat shine[] = {120.0f};
	glEnable(GL_COLOR_MATERIAL);						//打开材料着色功能
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);	//指定材料着色的面
	glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT,   mat_a);
	glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE,   mat_d);
	glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR,  mat_s);//指定材料对镜面光的反射
	glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, shine);//指定反射系数
}

void FittingMeshViewer::init()
{
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	// Light0 is the default ambient light
	glEnable(GL_LIGHT0);
	float pos[4] = {1.0, 0.5, 1.0, 0.0};
	// Directionnal light
	glLightfv(GL_LIGHT0, GL_POSITION, pos);

	glDisable(GL_LIGHT1);

	// Light default parameters
	const GLfloat light_ambient[4]  = {1.0, 1.0, 1.0, 1.0};
	const GLfloat light_specular[4] = {1.0, 1.0, 1.0, 1.0};
	const GLfloat light_diffuse[4]  = {1.0, 1.0, 1.0, 1.0};

	glLightf( GL_LIGHT1, GL_SPOT_EXPONENT, 3.0);
	glLightf( GL_LIGHT1, GL_SPOT_CUTOFF,   10.0);
	glLightf( GL_LIGHT1, GL_CONSTANT_ATTENUATION,  0.1f);
	glLightf( GL_LIGHT1, GL_LINEAR_ATTENUATION,    0.3f);
	glLightf( GL_LIGHT1, GL_QUADRATIC_ATTENUATION, 0.3f);
	glLightfv(GL_LIGHT1, GL_AMBIENT,  light_ambient);
	glLightfv(GL_LIGHT1, GL_SPECULAR, light_specular);
	glLightfv(GL_LIGHT1, GL_DIFFUSE,  light_diffuse);




	// Restore previous viewer state.恢复以前的状态
	restoreStateFromFile();

	//setDefaultMaterial();
	setDefaultMaterial();

	setBackgroundColor(QColor(205,205,205,0));
}



void FittingMeshViewer::draw_control_mesh_orginal()
{
	if (surfacedata==NULL)
	 return;
	CBSplineSurfaceView* bsurface=surfacedata->getbspline();
	ControlPoint=bsurface->getControlPoint();
	if (ControlPoint.empty())
		return;
	int m=ControlPoint.size();
	int n=ControlPoint[0].size();
	int i,j;
	for (i=0;i<m;i++)
	{
		glBegin(GL_LINE_STRIP);	
		for (j=0;j<n;j++)
		{
			glVertex3d(ControlPoint[i][j].x,ControlPoint[i][j].y,ControlPoint[i][j].z);

		}
		glEnd();
	}
	for (j=0;j<n;j++)
	{
		glBegin(GL_LINE_STRIP);	
		for (i=0;i<m;i++)
		{
			glVertex3d(ControlPoint[i][j].x,ControlPoint[i][j].y,ControlPoint[i][j].z);

		}
		glEnd();
	}




}

void FittingMeshViewer::draw_fitted_mesh()
{
	if(surfacedata==NULL)
		return;
	Mesh *mesh = surfacedata->get_fitted_mesh();
	if(mesh != NULL)
	{
		Mesh::ConstFaceIter        f_it(mesh->faces_begin()),f_end(mesh->faces_end());
		Mesh::ConstFaceVertexIter  fv_it;

		glShadeModel(GL_SMOOTH);

		glBegin(GL_TRIANGLES);
		for (; f_it!=f_end; ++f_it)
		{
			fv_it = mesh->cfv_iter(*f_it); 
			glNormal3dv(&mesh->normal(*fv_it)[0]);
			GL::glVertex(mesh->point(*fv_it));
			++fv_it;
			glNormal3dv(&mesh->normal(*fv_it)[0]);
			GL::glVertex(mesh->point(*fv_it));
			++fv_it;
			glNormal3dv(&mesh->normal(*fv_it)[0]);
			GL::glVertex(mesh->point(*fv_it));
		}
		glEnd();
	}
}


void FittingMeshViewer::draw_error_mesh()
{
	if(surfacedata==NULL)
		return;

	Mesh *mesh = surfacedata->get_error_fitted_mesh();
	if(!mesh)
		return;

	if(mesh!= NULL)
	{
		Mesh::ConstFaceIter        f_it(mesh->faces_begin()),f_end(mesh->faces_end());
		Mesh::ConstFaceVertexIter  fv_it;

		glBegin(GL_TRIANGLES);
		for (; f_it!=f_end; ++f_it)
		{
			fv_it = mesh->cfv_iter(*f_it); 
			glColor3ubv(mesh->color(*fv_it).data());
			glNormal3dv(&mesh->normal(*fv_it)[0]);
			GL::glVertex(mesh->point(*fv_it));
			++fv_it;
			glColor3ubv(mesh->color(*fv_it).data());
			glNormal3dv(&mesh->normal(*fv_it)[0]);
			GL::glVertex(mesh->point(*fv_it));
			++fv_it;
			glColor3ubv(mesh->color(*fv_it).data());
			glNormal3dv(&mesh->normal(*fv_it)[0]);
			GL::glVertex(mesh->point(*fv_it));
		}
		glEnd();
		
	}

	if(bmax_error_point_show)
		draw_max_error_point();
}

void FittingMeshViewer::draw_max_error_point()
{
	Max_Error error = surfacedata->get_max_error();
	Mesh *error_mesh = surfacedata->get_error_fitted_mesh(); 
	if(!error_mesh||error.index==-1)
		return;
	
	Mesh::VertexHandle  max_point=error_mesh->vertex_handle(error.index);

	glColor3d(1.0, 0.0, 0.0);
	glPointSize(8.0);
	glBegin(GL_POINTS);
		GL::glVertex(error_mesh->point(max_point));
	glEnd();
}

void FittingMeshViewer::set_surface_data(CSurfaceData *data)
{
	surfacedata = data;

}
void FittingMeshViewer::set_control_point(Array2 point)
{
	ControlPoint=point;
}

void FittingMeshViewer::update_fitting_view()
{
	Mesh *mesh = surfacedata->get_fitted_mesh();
	if(mesh == NULL)
	{
		return;
	}
	// compute box
	Mesh::ConstVertexIter  v_it(mesh->vertices_begin()), 
		v_end(mesh->vertices_end());
	Mesh::Point            bbMin, bbMax;

	bbMin = bbMax = mesh->point(v_it);
	for (; v_it!=v_end; ++v_it)
	{
		bbMin.minimize(mesh->point(v_it));
		bbMax.maximize(mesh->point(v_it));
	}
	Bbox_3 box(bbMin[0],bbMin[1],bbMin[2],bbMax[0],bbMax[1],bbMax[2]);
	bBox = box;
	set_scene(bBox);
	surfacedata->set_fitting_box(bBox);
	updateGL();
}



void FittingMeshViewer::set_fitted_mesh_view(bool fv)
{
	bfitted_mesh_view = fv;
	set_scene(surfacedata->get_fitting_box());
	updateGL();
}


void FittingMeshViewer::set_error_show(bool ev)
{
	if(ev)
	{
		Mesh *m_pFittedMesh = surfacedata->get_fitted_mesh();
		if(!m_pFittedMesh)
			return;
		double mean_error;
		/*Max_Error max_error=surfacedata->get_max_error();
		if(max_error.index==-1)
		{*/
			surfacedata->compute_error();
		/*}*/
			mean_error = surfacedata->get_mean_error();
			QString str = "The mean error between original surface and the fitted surface is ";
			str.append(QString::number(mean_error));
			Max_Error max_error = surfacedata->get_max_error();
			str.append(", the maximal relative error between original surface and the fitted surface is ");
			str.append(QString::number(max_error.error));
			QMessageBox::information(this, "Error of Fitted Surface", str, QMessageBox::Yes, QMessageBox::Yes);
		//}
		bool ok = false;
		max_err = 
			QInputDialog::getDouble(this, 
									tr("Max Error"),
									tr("Please input number:"),
						0.01,
						0,
						1,
						8,
						&ok);//设置最大误差，一旦我们计算的最大误差点比较孤立，比如只有一个，则我们需要降低最大误差的标准，
		//使之整体误差比较大的点突出来。
		if(!ok)
			max_err = 1;
		surfacedata->set_max_error(max_err);
		set_scene(surfacedata->get_error_box());
	}
	berror_show = ev;
    bfitted_mesh_view=false;  
	updateGL();
}



void FittingMeshViewer::paintEvent(QPaintEvent *event)
{
	Q_UNUSED(event)
	QPainter painter;
	painter.begin(this);
	painter.setRenderHint(QPainter::Antialiasing);
	
	// Save current OpenGL state
	glPushAttrib(GL_ALL_ATTRIB_BITS);
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();

	// Classical 3D drawing, usually performed by paintGL().
	preDraw();
	draw();
	postDraw();

	// Restore OpenGL state
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glPopAttrib();
	

	if(berror_show)
		draw_error_colorbar(&painter);
	/*if(bdensityshow)
		draw_density_colorbar(&painter);
	if(bcurvature_show)
		draw_curvature_colorbar(&painter);*/
	painter.end();
	swapBuffers( );
}



void FittingMeshViewer::draw_error_colorbar(QPainter *painter) 
{
	const int barwidth = 20;
	const int barheight = 200;
	painter->save();
    painter->translate(0.8*width(), 0.7*height());
	painter->save();
	int h1, s1, v1;
    int h2, s2, v2;
	int color[3];
	color[0] =250;
	color[1] = 0;
	color[2] = 250;
	QColor d_light(color[0], color[1], color[2]);

	color[0] =0;
	color[1] = 127;
	color[2] =0;
    QColor d_dark(color[0], color[1], color[2]);
    d_light.getHsv( &h1, &s1, &v1 );
    d_dark.getHsv( &h2, &s2, &v2 );
	QRect rect(0, -100, barwidth, barheight);
    painter->setClipRect( rect );//设置裁剪，默认用裁剪后的替代原先的。
    painter->setClipping( true );

    painter->fillRect( rect, d_dark );//用d_dark来填充rect

    int sectionSize = 4;

    int numIntervals;
    numIntervals = rect.height() / sectionSize;

    for ( int i = 0; i < numIntervals; i++ )
    {
        QRect colorsection;
        colorsection.setRect( rect.x(), rect.y() + i * sectionSize,
            rect.width(), sectionSize );

        const double ratio = i / static_cast<double>( numIntervals );
        QColor c;
        c.setHsv( h1 + qRound( ratio * ( h2 - h1 ) ),
            s1 + qRound( ratio * ( s2 - s1 ) ),
            v1 + qRound( ratio * ( v2 - v1 ) ) );

        painter->fillRect( colorsection, c );
    }
	painter->restore();
	sectionSize = 8;
	painter->translate(barwidth, 0);
	QFont font("Times", 10);
	painter->setPen(Qt::black);
	painter->setFont(font);
	double err_step = max_err / sectionSize;
	for ( int i = 0; i <= sectionSize; i++ )
    {
		QString str = QString::number(i*err_step);
        painter->drawText(5, barheight/2-barheight/8*i, str);
    }
    painter->restore();
}




bool FittingMeshViewer::write_mesh(QString &fileName, Mesh_Type type)
{
	Mesh* mesh = NULL;

	//Fitted_Mesh
	OpenMesh::IO::Options wopt;
	wopt+=OpenMesh::IO::Options::VertexNormal;


	//Error_Mesh,要用off格式写入
	OpenMesh::IO::Options wopt2;
	wopt2+=OpenMesh::IO::Options::VertexNormal;
	wopt2+=OpenMesh::IO::Options::VertexColor;


	if(type==Fitted_Mesh)
	{
		mesh = surfacedata->get_fitted_mesh();
		if(mesh)
		{
			if (!OpenMesh::IO::write_mesh( *mesh, fileName.toStdString(),wopt))
			return false;
			
		}
	}
	if(type==Error_Mesh)
	{
		mesh = surfacedata->get_error_fitted_mesh();
		if(mesh)
		{
		if (!OpenMesh::IO::write_mesh( *mesh, fileName.toStdString(),wopt2))
		return false;

		}
		
	}

	return true;
}

void FittingMeshViewer::set_max_error_point_show(bool mv)
{
	bmax_error_point_show = mv;
}

void FittingMeshViewer::set_controledge_show(bool cv)
{
	bcontroledge_show = cv;
	if(cv)
	{
		if(surfacedata==NULL)
			return;
		CBSplineSurfaceView* bsurface=surfacedata->getbspline();
        ControlPoint=bsurface->getControlPoint();
		if (ControlPoint.empty())
			return;
		int m=ControlPoint.size();
		int n=ControlPoint[0].size();
       double xmin=0,xmax=0,ymin=0,ymax=0,zmin=0,zmax=0;
        for (int i=0;i<m;i++)
            for (int j=0;j<n;j++)
            {
              if ( ControlPoint[i][j].x<xmin)
              {
				  xmin=ControlPoint[i][j].x;
              }
			  if (ControlPoint[i][j].x>xmax)
			  {
				  xmax=ControlPoint[i][j].x;
			  }
			  if ( ControlPoint[i][j].y<ymin)
			  {
				  ymin=ControlPoint[i][j].y;
			  }
			  if (ControlPoint[i][j].y>ymax)
			  {
				  ymax=ControlPoint[i][j].y;
			  }
			  if ( ControlPoint[i][j].z<zmin)
			  {
				  zmin=ControlPoint[i][j].z;
			  }
			  if (ControlPoint[i][j].z>zmax)
			  {
				  zmax=ControlPoint[i][j].z;
			  }
            }
		Bbox_3 box_3(xmin,xmax,ymin,ymax,zmin,zmax);
		set_scene(box_3);
	}
	updateGL();


}
