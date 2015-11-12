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
	//error = NULL;
	//dens_ind = NULL;
	//err_ind = NULL;
	bfitted_mesh_view = true;
	 berror_show = false;
	 bmax_error_point_show = false;
	//bcurvature_show = false;
	//bdensityshow = false;
	//bcontrolmesh_show = false;
	 bcontroledge_show = false;
	badjustedmesh_show = false;
	//badjusted_control_mesh = false;
	  badjust = false;
	  max_err = 1;
	//index = -1;*/
	setAutoBufferSwap( false );
	setAutoFillBackground( false );
	setMouseTracking(true);
	/*bsparsemeshview = false;
	bsparsecurvesview = false;*/
	bBox = Bbox_3(-1, 1, -1, 1, -1, 1);
}

FittingMeshViewer::~FittingMeshViewer(void)
{
		clear_data();
}
void FittingMeshViewer::clear_data()
{
	/*error = NULL;
	dens_ind = NULL;
	err_ind = NULL;*/
	bfitted_mesh_view = false;
	 berror_show = false;
	//bcurvature_show = false;
	//bdensityshow = false;
	//bcontrolmesh_show = false;
	bcontroledge_show = false;
	badjustedmesh_show = false;
	/*badjusted_control_mesh = false;
	badjust = false;
	bsparsemeshview = false;
	bsparsecurvesview = false;*/
	/*originalMeshVertices.clear();
	originalMeshNormals.clear();
	fittedMeshVertices.clear();
	fittedMeshNormals.clear();*/
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

	/*if(bcurvature_show)
		draw_curvature_mesh();
	if(bsparsemeshview)
		draw_sparsemesh();
	if(bsparsecurvesview)
		draw_sparsecurves();
	if(bdensityshow)
		draw_density_mesh();
	if(bcontrolmesh_show)
		draw_control_mesh();

	if(badjusted_control_mesh)
		draw_adjusted_control_mesh();

	if(badjustedmesh_show)
		draw_adjusted_mesh();*/

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
#if 0
	Polyhedron *mesh = surfacedata->get_fitted_mesh();
	if (!mesh)
		return;

	if (bfitted_mesh_view && fittedMeshVertices.empty())
	{
		Polyhedron::Facet_iterator fit = mesh->facets_begin();
		for (; fit!=mesh->facets_end(); fit++)
		{
			Polyhedron::Halfedge_around_facet_circulator hc = fit->facet_begin(), hs = hc;
			do 
			{
				fittedMeshVertices.push_back(QVector3D(hc->vertex()->point().x(), hc->vertex()->point().y(), hc->vertex()->point().z()));
				fittedMeshNormals.push_back(QVector3D(hc->vertex()->normal().x(), hc->vertex()->normal().y(), hc->vertex()->normal().z()));
			} while (++hc!=hs);

		}
	}
#endif
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

void FittingMeshViewer::set_adjustedmesh_enabled(bool av)
{
	badjustedmesh_show = av;
	updateGL();
}

void FittingMeshViewer::paintEvent(QPaintEvent *event)
{
	Q_UNUSED(event)
	QPainter painter;
	painter.begin(this);
	painter.setRenderHint(QPainter::Antialiasing);
	
	// Save current OpenGL state
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
	

	painter.begin(this);
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


//void FittingMeshViewer::colormap(int* color, double x, double min, double max)
// {
//	double r[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0.00588235294117645,0.02156862745098032,0.03725490196078418,0.05294117647058827,0.06862745098039214,0.084313725490196,0.1000000000000001,0.115686274509804,0.1313725490196078,0.1470588235294117,0.1627450980392156,0.1784313725490196,0.1941176470588235,0.2098039215686274,0.2254901960784315,0.2411764705882353,0.2568627450980392,0.2725490196078431,0.2882352941176469,0.303921568627451,0.3196078431372549,0.3352941176470587,0.3509803921568628,0.3666666666666667,0.3823529411764706,0.3980392156862744,0.4137254901960783,0.4294117647058824,0.4450980392156862,0.4607843137254901,0.4764705882352942,0.4921568627450981,0.5078431372549019,0.5235294117647058,0.5392156862745097,0.5549019607843135,0.5705882352941174,0.5862745098039217,0.6019607843137256,0.6176470588235294,0.6333333333333333,0.6490196078431372,0.664705882352941,0.6803921568627449,0.6960784313725492,0.7117647058823531,0.7274509803921569,0.7431372549019608,0.7588235294117647,0.7745098039215685,0.7901960784313724,0.8058823529411763,0.8215686274509801,0.8372549019607844,0.8529411764705883,0.8686274509803922,0.884313725490196,0.8999999999999999,0.9156862745098038,0.9313725490196076,0.947058823529412,0.9627450980392158,0.9784313725490197,0.9941176470588236,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0.9862745098039216,0.9705882352941178,0.9549019607843139,0.93921568627451,0.9235294117647062,0.9078431372549018,0.892156862745098,0.8764705882352941,0.8607843137254902,0.8450980392156864,0.8294117647058825,0.8137254901960786,0.7980392156862743,0.7823529411764705,0.7666666666666666,0.7509803921568627,0.7352941176470589,0.719607843137255,0.7039215686274511,0.6882352941176473,0.6725490196078434,0.6568627450980391,0.6411764705882352,0.6254901960784314,0.6098039215686275,0.5941176470588236,0.5784313725490198,0.5627450980392159,0.5470588235294116,0.5313725490196077,0.5156862745098039,0.5};
//	double g[] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0.001960784313725483,0.01764705882352935,0.03333333333333333,0.0490196078431373,0.06470588235294117,0.08039215686274503,0.09607843137254901,0.111764705882353,0.1274509803921569,0.1431372549019607,0.1588235294117647,0.1745098039215687,0.1901960784313725,0.2058823529411764,0.2215686274509804,0.2372549019607844,0.2529411764705882,0.2686274509803921,0.2843137254901961,0.3,0.3156862745098039,0.3313725490196078,0.3470588235294118,0.3627450980392157,0.3784313725490196,0.3941176470588235,0.4098039215686274,0.4254901960784314,0.4411764705882353,0.4568627450980391,0.4725490196078431,0.4882352941176471,0.503921568627451,0.5196078431372548,0.5352941176470587,0.5509803921568628,0.5666666666666667,0.5823529411764705,0.5980392156862746,0.6137254901960785,0.6294117647058823,0.6450980392156862,0.6607843137254901,0.6764705882352942,0.692156862745098,0.7078431372549019,0.723529411764706,0.7392156862745098,0.7549019607843137,0.7705882352941176,0.7862745098039214,0.8019607843137255,0.8176470588235294,0.8333333333333333,0.8490196078431373,0.8647058823529412,0.8803921568627451,0.8960784313725489,0.9117647058823528,0.9274509803921569,0.9431372549019608,0.9588235294117646,0.9745098039215687,0.9901960784313726,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0.9901960784313726,0.9745098039215687,0.9588235294117649,0.943137254901961,0.9274509803921571,0.9117647058823528,0.8960784313725489,0.8803921568627451,0.8647058823529412,0.8490196078431373,0.8333333333333335,0.8176470588235296,0.8019607843137253,0.7862745098039214,0.7705882352941176,0.7549019607843137,0.7392156862745098,0.723529411764706,0.7078431372549021,0.6921568627450982,0.6764705882352944,0.6607843137254901,0.6450980392156862,0.6294117647058823,0.6137254901960785,0.5980392156862746,0.5823529411764707,0.5666666666666669,0.5509803921568626,0.5352941176470587,0.5196078431372548,0.503921568627451,0.4882352941176471,0.4725490196078432,0.4568627450980394,0.4411764705882355,0.4254901960784316,0.4098039215686273,0.3941176470588235,0.3784313725490196,0.3627450980392157,0.3470588235294119,0.331372549019608,0.3156862745098041,0.2999999999999998,0.284313725490196,0.2686274509803921,0.2529411764705882,0.2372549019607844,0.2215686274509805,0.2058823529411766,0.1901960784313728,0.1745098039215689,0.1588235294117646,0.1431372549019607,0.1274509803921569,0.111764705882353,0.09607843137254912,0.08039215686274526,0.06470588235294139,0.04901960784313708,0.03333333333333321,0.01764705882352935,0.001960784313725483,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
//	double b[] = {0.5,0.5156862745098039,0.5313725490196078,0.5470588235294118,0.5627450980392157,0.5784313725490196,0.5941176470588235,0.6098039215686275,0.6254901960784314,0.6411764705882352,0.6568627450980392,0.6725490196078432,0.6882352941176471,0.7039215686274509,0.7196078431372549,0.7352941176470589,0.7509803921568627,0.7666666666666666,0.7823529411764706,0.7980392156862746,0.8137254901960784,0.8294117647058823,0.8450980392156863,0.8607843137254902,0.8764705882352941,0.892156862745098,0.907843137254902,0.9235294117647059,0.9392156862745098,0.9549019607843137,0.9705882352941176,0.9862745098039216,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0.9941176470588236,0.9784313725490197,0.9627450980392158,0.9470588235294117,0.9313725490196079,0.915686274509804,0.8999999999999999,0.884313725490196,0.8686274509803922,0.8529411764705883,0.8372549019607844,0.8215686274509804,0.8058823529411765,0.7901960784313726,0.7745098039215685,0.7588235294117647,0.7431372549019608,0.7274509803921569,0.7117647058823531,0.696078431372549,0.6803921568627451,0.6647058823529413,0.6490196078431372,0.6333333333333333,0.6176470588235294,0.6019607843137256,0.5862745098039217,0.5705882352941176,0.5549019607843138,0.5392156862745099,0.5235294117647058,0.5078431372549019,0.4921568627450981,0.4764705882352942,0.4607843137254903,0.4450980392156865,0.4294117647058826,0.4137254901960783,0.3980392156862744,0.3823529411764706,0.3666666666666667,0.3509803921568628,0.335294117647059,0.3196078431372551,0.3039215686274508,0.2882352941176469,0.2725490196078431,0.2568627450980392,0.2411764705882353,0.2254901960784315,0.2098039215686276,0.1941176470588237,0.1784313725490199,0.1627450980392156,0.1470588235294117,0.1313725490196078,0.115686274509804,0.1000000000000001,0.08431372549019622,0.06862745098039236,0.05294117647058805,0.03725490196078418,0.02156862745098032,0.00588235294117645,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
//	int i = 0;
//	
//	i = (int)((x-min)/(max-min)*255);
//
//	if(i<0)
//		i = 0;
//	if(i>255)
//		i = 255;
//	color[0] = (int)(r[i]*255);
//	color[1] = (int)(g[i]*255);
//	color[2] = (int)(b[i]*255);
//	return;
//}
//
//int FittingMeshViewer::check_selected(Point_3 &pt, double dist)
//{
//	if(!surfacedata)
//		return -1;
//	return surfacedata->check_selected(pt, dist);
//}
//
//void FittingMeshViewer::mousePressEvent(QMouseEvent *event)
//{
//	if(badjust&&badjusted_control_mesh)
//	{
//		GLdouble mvmatrix[16], projmatrix[16];
//		GLfloat  winX, winY; 
//		GLdouble posX, posY, posZ;
//		GLint    viewport[4]; 
//
//		glPushMatrix();
//		/*Bbox_3 box = surfacedata->get_original_mesh()->bbox().bbox();
//		set_scene(box);*/
//		glPolygonOffset(3.0f, 1.0f);
//
//		glGetIntegerv(GL_VIEWPORT, viewport);
//		glGetDoublev(GL_MODELVIEW_MATRIX, mvmatrix);
//		glGetDoublev(GL_PROJECTION_MATRIX, projmatrix);
//
//		glPopMatrix();
//
//		double dist = 3.5*radius/viewport[3];
//		
//		QPoint lastPos = event->pos();
//		GLint x = lastPos.x();
//		GLint y = lastPos.y();
//
//		winX = (float)x;
//		winY = viewport[3] - (GLint)y;
//
//		glReadPixels((int)winX, (int)winY, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ);
//		
//		gluUnProject(winX, winY, winZ, mvmatrix, projmatrix, viewport, &posX, &posY, &posZ);
//		
//		
//		pos = Point_3(posX, posY, posZ);
//		index =  check_selected(pos, dist);
//		emit position_changed();
//		if(index!=-1)
//		{
//			double **controlvertices_colors = surfacedata->get_controlvertices_color();
//			controlvertices_colors[index][0] = 1.0;
//			controlvertices_colors[index][1] = 1.0;
//			controlvertices_colors[index][2] = 0.0;
//			updateGL();
//		}
//	}
//	else
//	{
//		GLdouble mvmatrix[16], projmatrix[16];
//		GLfloat  winX, winY;
//		GLdouble posX, posY, posZ;
//		GLint    viewport[4]; 
//
//		glPushMatrix();
//		
//		glPolygonOffset(3.0f, 1.0f);
//		glGetIntegerv(GL_VIEWPORT, viewport);
//		glGetDoublev(GL_MODELVIEW_MATRIX, mvmatrix);
//		glGetDoublev(GL_PROJECTION_MATRIX, projmatrix);
//		glPopMatrix();
//
//		QPoint lastPos = event->pos();
//		GLint x = lastPos.x();
//		GLint y = lastPos.y();
//
//		winX = (float)x;
//		winY = viewport[3] - (GLint)y;
//		
//		gluUnProject(winX, winY, winZ, mvmatrix, projmatrix, viewport, &posX, &posY, &posZ);
//		pos = Point_3(posX, posY, posZ);
//		emit position_changed();
//		QGLViewer::mousePressEvent(event);
//	}
//}
//
//void FittingMeshViewer::mouseMoveEvent(QMouseEvent *event)
//{
//	if(badjust&&badjusted_control_mesh)
//	{
//		if(index==-1)
//			return;
//		GLdouble mvmatrix[16], projmatrix[16];
//		GLfloat  winX, winY;
//		GLdouble posX, posY, posZ;
//		GLint    viewport[4]; 
//
//		glPushMatrix();
//		/*Bbox_3 box = surfacedata->get_original_mesh()->bbox().bbox();
//		set_scene(box);*/
//		glPolygonOffset(3.0f, 1.0f);
//		glGetIntegerv(GL_VIEWPORT, viewport);
//		glGetDoublev(GL_MODELVIEW_MATRIX, mvmatrix);
//		glGetDoublev(GL_PROJECTION_MATRIX, projmatrix);
//		glPopMatrix();
//
//		QPoint lastPos = event->pos();
//		GLint x = lastPos.x();
//		GLint y = lastPos.y();
//
//		winX = (float)x;
//		winY = viewport[3] - (GLint)y;
//		
//		gluUnProject(winX, winY, winZ, mvmatrix, projmatrix, viewport, &posX, &posY, &posZ);
//		pos = Point_3(posX, posY, posZ);
//		surfacedata->update_mesh(index, pos);
//		emit position_changed();
//		updateGL();
//	}
//	else
//	{
//		GLdouble mvmatrix[16], projmatrix[16];
//		GLfloat  winX, winY;
//		GLdouble posX, posY, posZ;
//		GLint    viewport[4]; 
//
//		glPushMatrix();
//		
//		glPolygonOffset(3.0f, 1.0f);
//		glGetIntegerv(GL_VIEWPORT, viewport);
//		glGetDoublev(GL_MODELVIEW_MATRIX, mvmatrix);
//		glGetDoublev(GL_PROJECTION_MATRIX, projmatrix);
//		glPopMatrix();
//
//		QPoint lastPos = event->pos();
//		GLint x = lastPos.x();
//		GLint y = lastPos.y();
//
//		winX = (float)x;
//		winY = viewport[3] - (GLint)y;
//		
//		gluUnProject(winX, winY, winZ, mvmatrix, projmatrix, viewport, &posX, &posY, &posZ);
//		pos = Point_3(posX, posY, posZ);
//		emit position_changed();
//		QGLViewer::mouseMoveEvent(event);
//	}
//}
//
//void FittingMeshViewer::mouseReleaseEvent(QMouseEvent *event)
//{
//	if(index!=-1)
//		{
//			double **controlvertices_colors = surfacedata->get_controlvertices_color();
//			controlvertices_colors[index][0] = 0.0;
//			controlvertices_colors[index][1] = 0.0;
//			controlvertices_colors[index][2] = 1.0;
//			updateGL();
//		}
//	QGLViewer::mouseReleaseEvent(event);
//}
//
void FittingMeshViewer::set_adjustpoints_enabled(bool av)
{
	badjust = av;
	updateGL();
}
//
//void FittingMeshViewer::set_adjusted_control_enabled(bool av)
//{
//	badjusted_control_mesh = av;
//	updateGL();
//}
//
//int &FittingMeshViewer::get_index()
//{
//	return index;
//}
//
//Point_3 &FittingMeshViewer::get_postion()
//{
//	return pos;
//}
//
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




	/*if(type==Curvature_Mesh)
	{
		mesh = surfacedata->get_original_mesh();
		if(mesh)
		{
			mesh->write_obj(fileName, true);
		}
	}
	if(type==Density_Mesh)
	{
		mesh = surfacedata->get_density_mesh();
		if(mesh)
		{
			mesh->write_obj(fileName, true);
		}
	}*/
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
 
	//if(type==Control_Mesh)
	//{//Control Mesh
	//	//mesh = surfacedata->get_control_mesh();
	//	///*Polyhedron::Edge_iterator e_it = mesh->edges_begin();
	//	//*/
	//	//
	//	//if(mesh)
	//	//{
	//	//	mesh->write_ve_obj(fileName);
	//	//}
	//	Point_3 *control_vertices = surfacedata->get_control_vertices();
	//	if(!control_vertices)
	//		return false;
	//	QFile file( fileName);
	//	if ( file.open(QIODevice::WriteOnly)) 
	//	{
	//		QTextStream out( &file );
	//		basis_size = surfacedata->get_basis_size();
	//		out << "g object" << endl;
	//		for(int i=0; i<basis_size; i++)
	//		{
	//			out << "v " <<control_vertices[i].x() << " " << control_vertices[i].y() << " " << control_vertices[i].z() << endl;
	//		}
	//		vector<Edge> edges = surfacedata->get_control_edges_auxiliary();
	//		for(int i=0; i<edges.size(); i++)
	//		{
	//			int first = edges[i].first+1;
	//			int second = edges[i].second+1;
	//			out << "l " << first << " " << second << endl;
	//		}
	//	}
	//}

	//if(type==Control_Edges)
	//{
	//	Point_3 *control_vertices = surfacedata->get_control_vertices();
	//	if(!control_vertices)
	//		return false;
	//	QFile file( fileName);
	//	if ( file.open(QIODevice::WriteOnly)) 
	//	{
	//		QTextStream out( &file );
	//		basis_size = surfacedata->get_basis_size();
	//		out << "g object" << endl;
	//		for(int i=0; i<basis_size; i++)
	//		{
	//			out << "v " <<control_vertices[i].x() << " " << control_vertices[i].y() << " " << control_vertices[i].z() << endl;
	//		}

	//		vector<Edge> edges = surfacedata->get_control_edges();
	//		vector<Edge> edges_auxiliary = surfacedata->get_control_edges_auxiliary();
	//		int sz = edges_auxiliary.size();
	//		for(int i=0; i<edges.size(); i++)
	//		{
	//			int first = edges[i].first;
	//			int second = edges[i].second;
	//			for(int j=0; j<sz; j++)
	//			{
	//				if(first == edges_auxiliary[j].first && second == edges_auxiliary[j].second)
	//				{
	//					out << "l " << first+1 << " " << second+1 << endl;
	//					if(second == edges_auxiliary[(j+1)%sz].first && first == edges_auxiliary[(j+1)%sz].second)
	//					{
	//						out << "l " << second+1 << " " << first+1 << endl;
	//						break;
	//					}
	//					if(second == edges_auxiliary[(j-1)%sz].first && first == edges_auxiliary[(j-1)%sz].second)
	//					{
	//						out << "l " << second+1 << " " << first+1 << endl;
	//						break;
	//					}
	//					break;
	//				}	
	//			}
	//		}
	//	}
	//}

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

//void FittingMeshViewer::set_sparsemesh_view(bool sv)
//{
//	bsparsemeshview = sv;
//}
//
//void FittingMeshViewer::set_sparsecurves_view(bool sv)
//{
//	bsparsecurvesview = sv;
//}
//
//void FittingMeshViewer::set_featureline_view(bool fv)
//{
//	bfeaturelineview = fv;
//}
//
//void FittingMeshViewer::draw_featurelines()
//{
//	if(surfacedata==NULL)
//		return;
//
//	Polyhedron *mesh = surfacedata->get_original_mesh();
//	if(mesh != NULL)
//	{
//		struct FeaturesInfo ridges = mesh->getRidges();
//		glColor3d(1.0, 0.0, 0.0);
//		glLineWidth(2.0f);
//		for (int i=0; i<ridges.fFNum; i++)
//		{
//			if(ridges.eFids[i].fId != -1)
//			{
//				glBegin(GL_LINES);
//					glVertex3d(ridges.fps[ridges.eFids[i].vId1].x, ridges.fps[ridges.eFids[i].vId1].y, ridges.fps[ridges.eFids[i].vId1].z);
//					glVertex3d(ridges.fps[ridges.eFids[i].vId2].x, ridges.fps[ridges.eFids[i].vId2].y, ridges.fps[ridges.eFids[i].vId2].z);
//				glEnd();
//			}
//		}
//		
//	}
//	
//}