/*************************************************
* Copyright (c) 2014-至今,厦门大学计算机图形学实验室Graphics@XMU
* All rights reserved.
*
* 文件名称：splineknotsviewer.cpp
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

//CGAL
#include <CGAL/convex_hull_2.h>
#include <CGAL/Polygon_2.h>

//Qt
#include <QtGui/QInputDialog>
#include <QtGui/QMessageBox>
#include <QtGui/QFileDialog>

#include <fstream>

#include "KnotsViewer.h"


KnotsViewer::KnotsViewer(void)
{	
	surfacedata = NULL;
	parameter2=NULL;

	bknotsview = false;
	bcurvature_show=false;
	berror_show=false;
    bmeshdomainview= false;
	berrordomainview = false;

	setAutoBufferSwap( false );
	setAutoFillBackground( false );
}

void KnotsViewer::clear_data()
{
	
	bknotsview=false;
	bmeshdomainview=false;	
	bcurvature_show=false;
	
}  

KnotsViewer::~KnotsViewer(void)
{
	clear_data();
}  

void KnotsViewer::set_surface_data(CSurfaceData *data)
{
	surfacedata = data;
	
}

void KnotsViewer::update_view() 
{
	Bbox_2 box_2 = surfacedata->get_box();
	Bbox_3 box_3(box_2.xmin(), box_2.ymin(), 0, box_2.xmax(), box_2.ymax(), 1);
	set_scene(box_3); 
    bmeshdomainview= true;
	updateGL();
}

void KnotsViewer::set_scene(Bbox_3 &box)
{
	
	Mesh::Point     bbMin(box.xmin(), box.ymin(), box.zmin());
	Mesh::Point     bbMax(box.xmax(), box.ymax(), box.zmax());
	Mesh::Point     center= (bbMin + bbMax)*0.5;
	double radius=0.5*(bbMin - bbMax).norm();
	setSceneCenter(Vec(center[0], center[1], center[2]));
	setSceneRadius(radius);
	camera()->showEntireScene();
}

void KnotsViewer::draw()
{
	glMatrixMode( GL_MODELVIEW );
	glPushMatrix();
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_POLYGON_OFFSET_FILL);
	glPolygonOffset(3.0f, 1.0f);
	glDisable(GL_LIGHTING);
	glShadeModel(GL_SMOOTH);
	if(bknotsview)
		draw_knots();
	if(berrordomainview) 
		draw_curvature_error_domain();
	if(bmeshdomainview)
		draw_domain_mesh();
	if(bcurvature_show)
		draw_curvature_mesh();
	if (berror_show)
	    draw_fitting_error_domain();
	
	glDisable(GL_POLYGON_OFFSET_FILL);
	glPopMatrix();
}

void KnotsViewer::init()
{
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	// Restore previous viewer state.
	restoreStateFromFile();
	setBackgroundColor(QColor(255,255,255,0));
    setMouseBinding(Qt::NoModifier, Qt::LeftButton, NO_CLICK_ACTION);
	setMouseBinding(Qt::ShiftModifier, Qt::LeftButton, CAMERA, SCREEN_ROTATE);//重新设置LeftButton，旋转屏幕或者物体
}

void KnotsViewer::draw_knots()
{ 	
	
	if(surfacedata==NULL||parameter2==NULL)
		return;
	else if(parameter2->getq()==0)//弹出窗口输入参数时没有输入完毕。
	    return;
	glColor3d(0.0, 0.0, 0.5);
	glLineWidth(0.5);
	int p=parameter2->getp();
	int q=parameter2->getq();
	int m=parameter2->getm();
	int n=parameter2->getn();
	int unum=parameter2->getnum_u();
	int vnum=parameter2->getnum_v();
	if (uknot.size()!=m+2-p||vknot.size()!=n+2-q)
	 return;
	/*for (int j=q;j<n+1;j++)
	  for (int i=p;i<m+1;i++)
		{
			glBegin(GL_LINE_LOOP);
			glVertex2d(uknot[i-p],vknot[j-q]);
			glVertex2d(uknot[i-p+1],vknot[j-q]);
			glVertex2d(uknot[i-p+1],vknot[j-q+1]);
			glVertex2d(uknot[i-p],vknot[j-q+1]);
			glEnd();
		
		}*/
   //绘制节点线
//先画u线
	for (int i=0;i<unum;i++)
	{
		glBegin(GL_LINES);
		glVertex2d(uknot[i],0);
		glVertex2d(uknot[i],1);
		glEnd();
	}
	//再画v线
	for (int i=0;i<vnum;i++)
	{
		glBegin(GL_LINES);
		glVertex2d(0,vknot[i]);
		glVertex2d(1,vknot[i]);
		glEnd();
	}
//再绘制一遍新增加的线，使用红色
	glColor3d(1.0, 0.0, 0.0);
	//先画u线
	int n_u=surfacedata->uknots_new.size();
	int n_v=surfacedata->vknots_new.size();
	for (int i=0;i<n_u;i++)
	{
		glBegin(GL_LINES);
		glVertex2d(surfacedata->uknots_new[i],0);
		glVertex2d(surfacedata->uknots_new[i],1);
		glEnd();
	}
	//再画v线
	for (int i=0;i<n_v;i++)
	{
		glBegin(GL_LINES);
		glVertex2d(0,surfacedata->vknots_new[i]);
		glVertex2d(1,surfacedata->vknots_new[i]);
		glEnd();
	}

}
void KnotsViewer::draw_fitting_error_domain()
{
	if(surfacedata==NULL)
		return;
	if ((surfacedata->get_domain()).empty())
		return;
	Mesh *mesh = surfacedata->get_error_fitted_mesh();
	if (mesh==NULL)
		return;
	Mesh::ConstFaceIter        f_it(mesh->faces_begin()),f_end(mesh->faces_end());
	Mesh::ConstFaceVertexIter  fv_it;
	//glLineWidth(0.5);
	for (; f_it!=f_end; ++f_it)
	{
		glBegin(GL_TRIANGLES);
		fv_it = mesh->cfv_iter(*f_it); 
		glColor3ubv(mesh->color(*fv_it).data());
		glVertex2dv(&mesh->texcoord2D(fv_it)[0]);
		++fv_it;
		glColor3ubv(mesh->color(*fv_it).data());
		glVertex2dv(&mesh->texcoord2D(fv_it)[0]);
		++fv_it;
		glColor3ubv(mesh->color(*fv_it).data());
		glVertex2dv(&mesh->texcoord2D(fv_it)[0]);
		glEnd();
	}


   
}


void KnotsViewer::draw_domain_mesh()
{

	if(surfacedata==NULL)
		return;
	if ((surfacedata->get_domain()).empty())
		return;
	Mesh *mesh = surfacedata->get_original_mesh();
	if (mesh==NULL)
		return;
	Mesh::ConstFaceIter        f_it(mesh->faces_begin()),f_end(mesh->faces_end());
	Mesh::ConstFaceVertexIter  fv_it;
	glColor3d(0.4, 0.4, 0.5);
	glLineWidth(0.5);

	for (; f_it!=f_end; ++f_it)
	{
		glBegin(GL_LINE_LOOP);
		fv_it = mesh->cfv_iter(*f_it); 
		glVertex2dv(&mesh->texcoord2D(fv_it)[0]);
		++fv_it;
		glVertex2dv(&mesh->texcoord2D(fv_it)[0]);
		++fv_it;
		glVertex2dv(&mesh->texcoord2D(fv_it)[0]);
		glEnd();
	}
	
	
	//检测每个条形所包含的点
	/*glColor3d(1.0, 0.0, 0.0);
	glPointSize(2);
	vector<int>  h=surfacedata->test;
   glBegin(GL_POINTS);
   for (vector<int>::iterator p=h.begin();p!=h.end();++p)
   {
	   Mesh::VertexHandle f=mesh->vertex_handle(*p);
	   glVertex2dv(&mesh->texcoord2D(f)[0]);
   }
   glEnd();*/

   //检测每个条形所包含的三角形
	/*glColor3d(1.0, 0.0, 0.0);
	set<int>  h=surfacedata->test1;
	Mesh::FaceVertexIter  fv_it2;
   for (set<int>::iterator p=h.begin();p!=h.end();++p)
   {
	   Mesh::FaceHandle f_it=mesh->face_handle(*p);
	   glBegin(GL_TRIANGLES);
	   fv_it2 = mesh->fv_iter(f_it); 
	   glVertex2dv(&mesh->texcoord2D(fv_it2)[0]);
	   ++fv_it2;
	   glVertex2dv(&mesh->texcoord2D(fv_it2)[0]);
	   ++fv_it2;
	   glVertex2dv(&mesh->texcoord2D(fv_it2)[0]);
	   glEnd();
   }
 */
	
}
void KnotsViewer::draw_curvature_mesh()
{
	if(surfacedata==NULL)
		return;
	if ((surfacedata->get_domain()).empty())
		return;
	Mesh *mesh = surfacedata->get_original_mesh();
	if (mesh==NULL)
		return;
	Mesh::ConstFaceIter        f_it(mesh->faces_begin()),f_end(mesh->faces_end());
	Mesh::ConstFaceVertexIter  fv_it;
	//glLineWidth(0.5);
	for (; f_it!=f_end; ++f_it)
	{
		
		glBegin(GL_TRIANGLES);
		fv_it = mesh->cfv_iter(*f_it); 
		glColor3ubv(mesh->color(*fv_it).data());
		glVertex2dv(&mesh->texcoord2D(fv_it)[0]);
		++fv_it;
		glColor3ubv(mesh->color(*fv_it).data());
		glVertex2dv(&mesh->texcoord2D(fv_it)[0]);
		++fv_it;
		glColor3ubv(mesh->color(*fv_it).data());
		glVertex2dv(&mesh->texcoord2D(fv_it)[0]);
		glEnd();
	}
	

	
}
void KnotsViewer::draw_curvature_colorbar(QPainter *painter) 
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
void KnotsViewer::draw_curvature_error_domain()
{

	if(surfacedata==NULL)
		return;
	if ((surfacedata->get_domain()).empty())
		return;
	MyMesh polymesh = surfacedata->get_polymesh();
	if (polymesh.edges_empty()==true)
		return;
	MyMesh::ConstFaceIter        f_it(polymesh.faces_begin()),f_end(polymesh.faces_end());
	MyMesh::ConstFaceVertexIter  fv_it;
	//glLineWidth(0.5);
	for (; f_it!=f_end; ++f_it)
	{

		glColor3ubv(polymesh.color(*f_it).data());
		glBegin(GL_POLYGON);
		fv_it = polymesh.cfv_iter(*f_it); 
		glVertex3dv(&polymesh.point(fv_it)[0]);
		++fv_it;
		glVertex3dv(&polymesh.point(fv_it)[0]);
		++fv_it;
		glVertex3dv(&polymesh.point(fv_it)[0]);
		++fv_it;
		glVertex3dv(&polymesh.point(fv_it)[0]);
		glEnd();
	}


}

void KnotsViewer::draw_curvature_error_colorbar(QPainter *painter) 
{
const int barwidth = 20;
	const int barheight = 200;
	painter->save();
    painter->translate(0.8*width(), 0.7*height());
	painter->save();
	int h1, s1, v1;
    int h2, s2, v2;
	int color[3];
	color[0] = 127;
	color[1] = 0;
	color[2] = 0;
	QColor d_light(color[0], color[1], color[2]);

	color[0] = 0;
	color[1] = 0;
	color[2] = 127;
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
	double max_curve=surfacedata->get_max_knotcurerror();
	double min_curve=surfacedata->get_min_knotcurerror();
	double cur_step = (max_curve-min_curve)/ sectionSize;
	for ( int i = 0; i <= sectionSize; i++ )
	{
		QString str = QString::number(i*cur_step+min_curve);
		painter->drawText(5, barheight/2-barheight/8*i, str);
	}
	painter->restore();

}
void KnotsViewer::draw_fitting_error_colorbar(QPainter *painter)
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

	color[0] = 0;
	color[1] = 127;
	color[2] = 0;
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
void KnotsViewer::paintEvent(QPaintEvent *event)
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

	if(bcurvature_show)
		draw_curvature_colorbar(&painter);
	if (berrordomainview)
	    draw_curvature_error_colorbar(&painter);
	if (berror_show)
	     draw_fitting_error_colorbar(&painter);
	painter.end();
	swapBuffers( );
}

void KnotsViewer::set_knots_view(bool kv) 
{ 	
	bknotsview = kv;
	if(kv)
	{
		if(surfacedata==NULL)
			return;
		parameter2 = surfacedata->getparameter();

		/*if (parameter2==NULL)
			return;*/
		B_parameter p(0,0,0,0);
		if(*parameter2==p)
			return;
		if (!parameter2->configure_knots())
			return;
			uknot=parameter2->getuknot();
			vknot=parameter2->getvknot();
		Bbox_3 box_3(0,0,0, 1, 1, 1);
		set_scene(box_3);
	}
	updateGL();
}
void KnotsViewer::set_curvature_show(bool kv) 
{ 	
	if (kv)
	{
		Mesh *m_pOriginalMesh = surfacedata->get_original_mesh();      
		if(!m_pOriginalMesh)
			return;
		surfacedata->update_curvature_color();
       bmeshdomainview=false;
       berror_show = false;
	}
	bcurvature_show=kv;
	updateGL();
}
void KnotsViewer::error_fitting_view(bool kv)
{
	
	if(kv)
	{
		Mesh *m_pFittedMesh = surfacedata->get_fitted_mesh();
		if(!m_pFittedMesh)
			return;
		double mean_error;
	
		surfacedata->compute_error();

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
	}
	berror_show = kv;
	bmeshdomainview=false;
	bcurvature_show=false;
	updateGL();

}

void KnotsViewer::set_domain_mesh_view(bool dv)
{
	bmeshdomainview=dv;
	updateGL();
}
	
void KnotsViewer::set_error_curvature_view(bool ev)
{
	berrordomainview = ev;
	 berror_show=false;
	 bcurvature_show=false;
	updateGL();
}

