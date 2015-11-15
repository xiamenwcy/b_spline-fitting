/*************************************************
* Copyright (c) 2014-至今,厦门大学计算机图形学实验室Graphics@XMU
* All rights reserved.
*
* 文件名称：MeshViewer.cpp
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

#include "MeshViewer.h"
#include "gl.h"

//Qt
#include <QtGui/QMessageBox>
#include <QtGui/QInputDialog>

#include <fstream>



using namespace std;

MeshViewer::MeshViewer()
{
	restoreStateFromFile();
	surfacedata = NULL;
	
	borigina_mesh_view = true;

	bcurvature_show = false;

	setAutoBufferSwap( false );
	setAutoFillBackground( false );
	setMouseTracking(true);
	
	bBox = Bbox_3(-1, 1, -1, 1, -1, 1);
}

MeshViewer::~MeshViewer(void)
{
	clear_data();
}
void MeshViewer::clear_data()
{
	
	borigina_mesh_view = true;

	bcurvature_show = false;

	
	bBox = Bbox_3(-1, 1, -1, 1, -1, 1);
	
}

void MeshViewer::set_scene(Bbox_3 &box)
{
	
	Mesh::Point     bbMin(box.xmin(), box.ymin(), box.zmin());
	Mesh::Point     bbMax(box.xmax(), box.ymax(), box.zmax());
	Mesh::Point     center= (bbMin + bbMax)*0.5;
	radius=0.5*(bbMin - bbMax).norm();
	setSceneCenter(Vec(center[0], center[1], center[2]));
	setSceneRadius(radius);
	camera()->showEntireScene();
}

void MeshViewer::draw()
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
	glClearColor(1.0,1.0, 1.0, 1.0);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_DEPTH_TEST);

	if(borigina_mesh_view )
	{
		glEnable(GL_LIGHTING);
		glColor3d(0.7, 0.7, 0.5);
	}

	if(borigina_mesh_view)
		draw_original_mesh();

	if(borigina_mesh_view )
	{
		glDisable(GL_LIGHTING);
	}
	if(bcurvature_show)
		draw_curvature_mesh();
	

	glShadeModel(GL_FLAT);
	glDisable(GL_DEPTH_TEST);
	
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
	
}

void MeshViewer::setDefaultMaterial()
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

void MeshViewer::init()
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

void MeshViewer::draw_original_mesh()
{
	if(surfacedata==NULL)
		return;

	Mesh *mesh = surfacedata->get_original_mesh();
	if(mesh != NULL)
	{
		Mesh::ConstFaceIter        f_it(mesh->faces_begin()),f_end(mesh->faces_end());
		Mesh::ConstFaceVertexIter  fv_it;

		glShadeModel(GL_SMOOTH);

		glBegin(GL_TRIANGLES);
		for (; f_it!=f_end; ++f_it)
		{
			//GL::glNormal(mesh.normal(f_it));
			fv_it = mesh->cfv_iter(f_it.handle()); 
			glNormal3dv(&mesh->normal(fv_it)[0]);
			GL::glVertex(mesh->point(fv_it));
			++fv_it;
			glNormal3dv(&mesh->normal(fv_it)[0]);
			GL::glVertex(mesh->point(fv_it));
			++fv_it;
			glNormal3dv(&mesh->normal(fv_it)[0]);
			GL::glVertex(mesh->point(fv_it));
		}
		glEnd();
	}
}


void MeshViewer::draw_curvature_mesh()
{
	if(surfacedata==NULL)
		return;
	Mesh *mesh = surfacedata->get_original_mesh();
	if(!mesh)
		return;

	if(mesh!= NULL)
	{
		Mesh::ConstFaceIter        f_it(mesh->faces_begin()),f_end(mesh->faces_end());
		Mesh::ConstFaceVertexIter  fv_it;


		glBegin(GL_TRIANGLES);
		for (; f_it!=f_end; ++f_it)
		{
			fv_it = mesh->cfv_iter(f_it.handle()); 
			glColor3ubv(mesh->color(*fv_it).data());
			glNormal3dv(&mesh->normal(fv_it)[0]);
			GL::glVertex(mesh->point(fv_it));
			++fv_it;
			glColor3ubv(mesh->color(*fv_it).data());
			glNormal3dv(&mesh->normal(fv_it)[0]);
			GL::glVertex(mesh->point(fv_it));
			++fv_it;
			glColor3ubv(mesh->color(*fv_it).data());
			glNormal3dv(&mesh->normal(fv_it)[0]);
			GL::glVertex(mesh->point(fv_it));
		}
		glEnd();

	}

}


void MeshViewer::set_surface_data(CSurfaceData *data)
{
	surfacedata = data;

}

void MeshViewer::update_view()
{
	Mesh *m_pOriginalMesh = surfacedata->get_original_mesh();
	if(m_pOriginalMesh == NULL)
	{
		return;
	}
	// compute box
	Mesh::ConstVertexIter  v_it(m_pOriginalMesh->vertices_begin()), 
		v_end(m_pOriginalMesh->vertices_end());
	Mesh::Point            bbMin, bbMax;

	bbMin = bbMax = m_pOriginalMesh->point(v_it);
	for (; v_it!=v_end; ++v_it)
	{
		bbMin.minimize(m_pOriginalMesh->point(v_it));
		bbMax.maximize(m_pOriginalMesh->point(v_it));
	}
	Bbox_3 box(bbMin[0],bbMin[1],bbMin[2],bbMax[0],bbMax[1],bbMax[2]);
	bBox = box;
	set_scene(bBox);
	surfacedata->set_original_box(bBox);
	updateGL();
}

void MeshViewer::set_origin_mesh_view(bool ov)
{
	borigina_mesh_view = ov;
	updateGL();
}


void MeshViewer::set_curvature_show(bool cv)
{
	if(cv)
	{
		Mesh *m_pOriginalMesh = surfacedata->get_original_mesh();      

		if(!m_pOriginalMesh)
			return;
		surfacedata->update_curvature_color();
		set_scene(surfacedata->get_original_box());
        borigina_mesh_view = false;
	}
	bcurvature_show = cv;
	updateGL();
}




void MeshViewer::paintEvent(QPaintEvent *event)
{
	Q_UNUSED(event)
	QPainter painter;
	painter.begin(this);
	painter.setRenderHint(QPainter::Antialiasing);

	//// Save current OpenGL state,
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

	if(bcurvature_show)
		draw_curvature_colorbar(&painter);
	painter.end();
	swapBuffers( );
}



void MeshViewer::draw_curvature_colorbar(QPainter *painter) 
{
	const int barwidth = 20;
	const int barheight = 200;
	painter->save();
    painter->translate(0.8*width(), 0.7*height());
	painter->save();
	int h1, s1, v1;
    int h2, s2, v2;
	int color[3];
	color[0] = 250;
	color[1] = 0;
	color[2] = 0;
	QColor d_light(color[0], color[1], color[2]);

	color[0] = 0;
	color[1] = 0;
	color[2] = 255;
    QColor d_dark(color[0], color[1], color[2]);
    d_light.getHsv( &h1, &s1, &v1 );
    d_dark.getHsv( &h2, &s2, &v2 );
	QRect rect(0, -100, barwidth, barheight);
    painter->setClipRect( rect );
    painter->setClipping( true );

    painter->fillRect( rect, d_dark );

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
		/*int color[3];
		c.getRgb(&color[0], &color[1], &color[2]);*/
        painter->fillRect( colorsection, c );
    }
	painter->restore();
	sectionSize = 8;
	painter->translate(barwidth, 0);
	QFont font("Times", 10);
	painter->setPen(Qt::black);
	painter->setFont(font);
	double max_curve=surfacedata->get_max_meancurvature();
	double min_curve=surfacedata->get_min_meancurvature();
	double cur_step = (max_curve-min_curve)/ sectionSize;
	for ( int i = 0; i <= sectionSize; i++ )
	{
		QString str = QString::number(i*cur_step+min_curve);
		painter->drawText(5, barheight/2-barheight/8*i, str);
	}
	painter->restore();
}


bool MeshViewer::write_mesh(QString &fileName, Mesh_Type type)
{
	Mesh* mesh = NULL;

	//Curvature_Mesh,要用off格式写入,才能载入颜色
	OpenMesh::IO::Options wopt2;
	wopt2+=OpenMesh::IO::Options::VertexNormal;
	wopt2+=OpenMesh::IO::Options::VertexColor;

	if(type==Curvature_Mesh)
	{
		mesh = surfacedata->get_original_mesh();
		if(mesh)
		{
			if (!OpenMesh::IO::write_mesh( *mesh, fileName.toStdString(),wopt2))
				return false;

		}
	}
	
	return true;
}

