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
	//error = NULL;
	//dens_ind = NULL;
	//err_ind = NULL;
	borigina_mesh_view = true;
//	bfitted_mesh_view = true;
	///*berror_show = false;
	//bmax_error_point_show = false;
	bcurvature_show = false;
	//bdensityshow = false;
	//bcontrolmesh_show = false;
	//bcontroledge_show = false;
	badjustedmesh_show = false;
	//badjusted_control_mesh = false;
	  badjust = false;
	//max_err = 1;
	//index = -1;*/
	setAutoBufferSwap( false );
	setAutoFillBackground( false );
	setMouseTracking(true);
	/*bsparsemeshview = false;
	bsparsecurvesview = false;*/
	bBox = Bbox_3(-1, 1, -1, 1, -1, 1);
}

MeshViewer::~MeshViewer(void)
{
	clear_data();
}
void MeshViewer::clear_data()
{
	/*error = NULL;
	dens_ind = NULL;
	err_ind = NULL;*/
	borigina_mesh_view = true;
	//bfitted_mesh_view = false;
	//berror_show = false;
	bcurvature_show = false;
	//bdensityshow = false;
	//bcontrolmesh_show = false;
	//bcontroledge_show = false;
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
	/*if(berror_show)
		draw_error_mesh();
	if(bsparsemeshview)
		draw_sparsemesh();
	if(bsparsecurvesview)
		draw_sparsecurves();
	if(bdensityshow)
		draw_density_mesh();

	if(bcontroledge_show)
		draw_control_mesh_orginal();

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

//void MeshViewer::draw_density_mesh()
//{
//	if(surfacedata==NULL)
//		return;
//	Polyhedron *densitymesh = surfacedata->get_density_mesh();
//	if(densitymesh != NULL && densitymesh->is_valid() && !densitymesh->empty())
//	{
//		/*glColor3d(0.5, 0.5, 0.5);*/
//		densitymesh->gl_draw(true, true);
//		/*glColor3d(0.1, 0.1, 0.1);
//		densitymesh->gl_draw_mesh();*/
//	}
//}
//
//void MeshViewer::draw_control_mesh_orginal()
//{
//	Point_3 *control_vertices = surfacedata->get_control_vertices();
//	if(!control_vertices)
//		return;
//	{
//		basis_size = surfacedata->get_basis_size();
//		glColor3d(1.0, 1.0, 0.0);
//		glPointSize(7.0);
//		glBegin(GL_POINTS);
//			for(int i=0; i<basis_size; i++)
//			{
//				glVertex3d(control_vertices[i].x(), control_vertices[i].y(), control_vertices[i].z());
//			}
//		glEnd();
//	}
//
//	glColor3d(0.0, 1.0, 0.0);
//	glLineWidth(2.0f);
//	vector<Edge> edges = surfacedata->get_control_edges();
//	for(int i=0; i<edges.size(); i++)
//	{
//		glBegin(GL_LINES);
//			glVertex3d(control_vertices[edges[i].first].x(), control_vertices[edges[i].first].y(), control_vertices[edges[i].first].z());
//			glVertex3d(control_vertices[edges[i].second].x(), control_vertices[edges[i].second].y(), control_vertices[edges[i].second].z());
//		glEnd();
//	}
//}
//
//void MeshViewer::draw_control_mesh()
//{
//#if 0
//	Polyhedron *control_mesh = surfacedata->get_control_mesh();
//	if(!control_mesh)
//		return;
//	Polyhedron::Vertex_iterator vit = control_mesh->vertices_begin();
//	glColor3d(1.0, 1.0, 0.0);
//	glPointSize(7.0);
//	glBegin(GL_POINTS);
//		for(; vit!=control_mesh->vertices_end(); vit++)
//		{
//			glVertex3d(vit->point().x(), vit->point().y(), vit->point().z());
//		}
//	glEnd();
//
//
//	glColor3d(1.0, 0.0, 0.0);
//	glLineWidth(2.0f);
//	Polyhedron::Facet_iterator fit = control_mesh->facets_begin();
//	for(; fit!=control_mesh->facets_end(); fit++)
//	{
//		glBegin(GL_LINE_LOOP);
//			 Polyhedron::Halfedge_around_facet_circulator pHalfedge = fit->facet_begin();
//			 do
//			{
//				Polyhedron::Point& point  = pHalfedge->vertex()->point();
//				glVertex3d(point.x(), point.y(), point.z());
//			}while(++pHalfedge != fit->facet_begin());
//		glEnd();
//	}
//#else
//	Point_3 *control_vertices = surfacedata->get_control_vertices();
//	if(!control_vertices)
//		return;
//
//	int sz = surfacedata->get_basis_size();
//	glColor3d(1.0, 1.0, 0.0);
//	glPointSize(7.0);
//	glBegin(GL_POINTS);
//		for(int i=0; i<sz; i++)
//		{
//			glVertex3d(control_vertices[i].x(), control_vertices[i].y(), control_vertices[i].z());
//		}
//	glEnd();
//
//	vector<Edge> edges = surfacedata->get_control_edges_auxiliary();
//	sz = edges.size();
//	if(!sz)
//		return;
//	glColor3d(1.0, 0, 0.0);
//	glLineWidth(2.0f);
//	
//	for(int i=0; i<sz; i++)
//	{
//			glBegin(GL_LINES);
//				unsigned index = edges[i].first;
//				glVertex3d(control_vertices[index].x(), control_vertices[index].y(), control_vertices[index].z());
//				index = edges[i].second;
//				glVertex3d(control_vertices[index].x(), control_vertices[index].y(), control_vertices[index].z());
//			glEnd();
//	}
//#endif
//}
//
//void MeshViewer::draw_adjusted_control_mesh()
//{
//	Point_3 *control_vertices = surfacedata->get_adjusted_control_vertices();
//
//	if(control_vertices!=NULL)
//	{
//		basis_size = surfacedata->get_basis_size();
//		double **controlvertices_colors = surfacedata->get_controlvertices_color();
//		glColor3d(1.0, 1.0, 0.0);
//		glPointSize(7.0);
//		glBegin(GL_POINTS);
//			for(int i=0; i<basis_size; i++)
//			{
//				glColor3d(controlvertices_colors[i][0], controlvertices_colors[i][1], controlvertices_colors[i][2]);
//				glVertex3d(control_vertices[i].x(), control_vertices[i].y(), control_vertices[i].z());
//			}
//		glEnd();
//	}
//	
//	vector<vector<unsigned>> vertices_triangles = surfacedata->get_vertices_triangles_space();
//	glColor3d(1.0, 0.0, 0.0);
//	glLineWidth(2.0f);
//	for(int i=0; i<vertices_triangles.size(); i++)
//	{
//		glBegin(GL_LINE_LOOP);
//			for(int j=0; j<3; j++)
//			{
//				unsigned index = vertices_triangles[i].at(j);
//				glVertex3d(control_vertices[index].x(), control_vertices[index].y(), control_vertices[index].z());
//			}
//		glEnd();
//	}
//}
//
//void MeshViewer::draw_fitted_mesh()
//{
//	if(surfacedata==NULL)
//		return;
//	Mesh *mesh = surfacedata->get_fitted_mesh();
//	if(mesh != NULL)
//	{
//		Mesh::ConstFaceIter        f_it(mesh->faces_begin()),f_end(mesh->faces_end());
//		Mesh::ConstFaceVertexIter  fv_it;
//
//		glShadeModel(GL_SMOOTH);
//
//		glBegin(GL_TRIANGLES);
//		for (; f_it!=f_end; ++f_it)
//		{
//			fv_it = mesh->cfv_iter(f_it.handle()); 
//			glNormal3fv(&mesh->normal(fv_it)[0]);
//			GL::glVertex(mesh->point(fv_it));
//			++fv_it;
//			glNormal3fv(&mesh->normal(fv_it)[0]);
//			GL::glVertex(mesh->point(fv_it));
//			++fv_it;
//			glNormal3fv(&mesh->normal(fv_it)[0]);
//			GL::glVertex(mesh->point(fv_it));
//		}
//		glEnd();
//	}
//}

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

//void MeshViewer::draw_sparsemesh()
//{
//	if(!surfacedata)
//		return;
//
//	vector<BoundaryInteriorConfigs> configs = surfacedata->get_configs0();
//	vector<Knot_Data> knots = surfacedata->get_knots();
//	glColor3d(1.0, 0.0, 0.0);
//	glPointSize(4.0f);
//	glBegin(GL_POINTS);
//	for(int i=0; i<knots.size(); i++)
//	{
//		glVertex3d(knots[i].vertex.x(), knots[i].vertex.y(), knots[i].vertex.z());
//	}
//	glEnd();
//
//	vector<BoundaryInteriorConfigs>::iterator it = configs.begin();
//	glColor3d(0.0, 0.0, 0.0);
//	glLineWidth(2.0f);
//	for(; it!=configs.end(); it++)
//	{
//		glBegin(GL_LINE_LOOP);
//		for(int i=0;i<3;i++)
//		{ 				
//			glVertex3d(knots[it->boundary[i]].vertex.x(), knots[it->boundary[i]].vertex.y(), knots[it->boundary[i]].vertex.z());	
//		}
//		glEnd();
//	}
//}
//
//void MeshViewer::draw_sparsecurves()
//{
//	if(!surfacedata)
//		return;
//
//	vector<vector<Point_3>> sparseCurves = surfacedata->getSparseCurves();
//
//	vector<vector<Point_3>>::iterator it = sparseCurves.begin();
//	glColor3d(1.0, 0.0, 0.0);
//	glLineWidth(2.0f);
//	for(; it!=sparseCurves.end(); it++)
//	{
//		glBegin(GL_LINE_STRIP);
//		for(int i=0;i<it->size();i++)
//		{ 				
//			glVertex3d(it->at(i).x(), it->at(i).y(), it->at(i).z());	
//		}
//		glEnd();
//	}
//
//	/*it = sparseCurves.begin();
//	glColor3d(0.0, 0.0, 1.0);
//	glPointSize(3.0f);
//	glBegin(GL_POINTS);
//	for(; it!=sparseCurves.end(); it++)
//	{
//		for(int i=0;i<it->size();i++)
//		{ 				
//			glVertex3d(it->at(i).x(), it->at(i).y(), it->at(i).z());	
//		}
//	}
//	glEnd();*/
//
//	vector<Knot_Data> knots = surfacedata->get_knots();
//	glColor3d(1.0, 1.0, 0.0);
//	glPointSize(6.0f);
//	glBegin(GL_POINTS);
//	for(int i=0; i<knots.size(); i++)
//	{
//		glVertex3d(knots[i].vertex.x(), knots[i].vertex.y(), knots[i].vertex.z());
//	}
//	glEnd();
//}
//
//void MeshViewer::draw_adjusted_mesh()
//{
//	Polyhedron *m_pAdjustedMesh =  surfacedata->get_adjusted_mesh();
//	if(m_pAdjustedMesh != NULL && m_pAdjustedMesh->is_valid() && !m_pAdjustedMesh->empty())
//	{
//		glColor3d(0.5, 0.5, 0.5);
//		m_pAdjustedMesh->gl_draw(true, true);
//		/*glColor3d(0.1, 0.1, 0.1);
//		m_pAdjustedMesh->gl_draw_mesh();*/
//	}
//}
//
//void MeshViewer::draw_error_mesh()
//{
//	if(surfacedata==NULL)
//		return;
//
//	Polyhedron *error_mesh = surfacedata->get_error_fitted_mesh();
//	if(!error_mesh)
//		return;
//
//	if(error_mesh != NULL && error_mesh->is_valid() && !error_mesh->empty())
//	{
//		error_mesh->gl_draw(true, true);
//	}
//
//	if(bmax_error_point_show)
//		draw_max_error_point();
//}
//
//void MeshViewer::draw_max_error_point()
//{
//	Max_Error error = surfacedata->get_max_error();
//	Polyhedron *error_mesh = surfacedata->get_error_fitted_mesh();
//	if(!error_mesh)
//		return;
//	Polyhedron::Vertex_iterator it = error_mesh->vertices_begin();
//	std::advance(it, error.index);
//
//	glColor3d(0.0, 0.0, 0.0);
//	glPointSize(8.0);
//	glBegin(GL_POINTS);
//		glVertex3d(it->point().x(), it->point().y(), it->point().z());
//	glEnd();
//}
//
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
//void MeshViewer::update_fitting_view()
//{
//	Mesh *mesh = surfacedata->get_fitted_mesh();
//	if(mesh == NULL)
//	{
//		return;
//	}
//	// compute box
//	Mesh::ConstVertexIter  v_it(mesh->vertices_begin()), 
//		v_end(mesh->vertices_end());
//	Mesh::Point            bbMin, bbMax;
//
//	bbMin = bbMax = mesh->point(v_it);
//	for (; v_it!=v_end; ++v_it)
//	{
//		bbMin.minimize(mesh->point(v_it));
//		bbMax.maximize(mesh->point(v_it));
//	}
//	Bbox_3 box(bbMin[0],bbMin[1],bbMin[2],bbMax[0],bbMax[1],bbMax[2]);
//	bBox = box;
//	set_scene(bBox);
//	updateGL();
//}

void MeshViewer::set_origin_mesh_view(bool ov)
{
	borigina_mesh_view = ov;
//#if 0
//	Polyhedron *mesh = surfacedata->get_original_mesh();
//	if (!mesh)
//		return;
//	
//	if (borigina_mesh_view && originalMeshVertices.empty())
//	{
//		Polyhedron::Facet_iterator fit = mesh->facets_begin();
//		for (; fit!=mesh->facets_end(); fit++)
//		{
//			Polyhedron::Halfedge_around_facet_circulator hc = fit->facet_begin(), hs = hc;
//			do 
//			{
//				originalMeshVertices.push_back(QVector3D(hc->vertex()->point().x(), hc->vertex()->point().y(), hc->vertex()->point().z()));
//				originalMeshNormals.push_back(QVector3D(hc->vertex()->normal().x(), hc->vertex()->normal().y(), hc->vertex()->normal().z()));
//			} while (++hc!=hs);
//			
//		}
//	}
//#endif
	updateGL();
}

//void MeshViewer::set_fitted_mesh_view(bool fv)
//{
//	bfitted_mesh_view = fv;
//#if 0
//	Polyhedron *mesh = surfacedata->get_fitted_mesh();
//	if (!mesh)
//		return;
//
//	if (bfitted_mesh_view && fittedMeshVertices.empty())
//	{
//		Polyhedron::Facet_iterator fit = mesh->facets_begin();
//		for (; fit!=mesh->facets_end(); fit++)
//		{
//			Polyhedron::Halfedge_around_facet_circulator hc = fit->facet_begin(), hs = hc;
//			do 
//			{
//				fittedMeshVertices.push_back(QVector3D(hc->vertex()->point().x(), hc->vertex()->point().y(), hc->vertex()->point().z()));
//				fittedMeshNormals.push_back(QVector3D(hc->vertex()->normal().x(), hc->vertex()->normal().y(), hc->vertex()->normal().z()));
//			} while (++hc!=hs);
//
//		}
//	}
//#endif
//	updateGL();
//}

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

//void MeshViewer::set_density_show(bool dv)
//{
//	if(dv)
//	{
//		Polyhedron *mesh = surfacedata->get_original_mesh();
//		if(!mesh)
//		{
//			bdensityshow = dv;
//			return;
//		}
//		double max_dens = mesh->getMaxDensity();
//		double min_dens = mesh->getMinDensity();
//		QString str = "The maximal density of the original surface is ";
//		str.append(QString::number(max_dens));
//		str.append(", The maximal density of the original surface is ");
//		str.append(QString::number(min_dens));
//		QMessageBox::information(this, "Error of Fitted Surface", str, QMessageBox::Yes, QMessageBox::Yes);
//		bool ok = false;
//		double ratio;
//		ratio = 
//			QInputDialog::getDouble(this, 
//			tr("Ratio"),
//			tr("Please input number:"),
//			1,
//			0,
//			1,
//			5,
//			&ok);
//		if(!ok)
//			ratio = 1;
//		surfacedata->get_domain_mesh();
//		surfacedata->set_max_density(ratio);
//	}
//	bdensityshow = dv;
//	updateGL();
//}
//
//void MeshViewer::set_error_show(bool ev)
//{
//	if(ev)
//	{
//		Polyhedron *m_pFittedMesh = surfacedata->get_fitted_mesh();
//		if(!m_pFittedMesh)
//			return;
//		double cum_error, mean_error, rel_error;
//		Max_Error max_error;
//		//if(error == NULL)
//		//{
//			surfacedata->compute_error();
//			mean_error = surfacedata->get_mean_error();
//			QString str = "The mean error between original surface and the fitted surface is ";
//			str.append(QString::number(mean_error));
//			max_error = surfacedata->get_max_error();
//			str.append(", the maximal relative error between original surface and the fitted surface is ");
//			str.append(QString::number(max_error.error));
//			QMessageBox::information(this, "Error of Fitted Surface", str, QMessageBox::Yes, QMessageBox::Yes);
//			error = surfacedata->get_error();
//		//}
//		bool ok = false;
//		max_err = 
//			QInputDialog::getDouble(this, 
//									tr("Max Error"),
//									tr("Please input number:"),
//						0.01,
//						0,
//						1,
//						10,
//						&ok);
//		if(!ok)
//			max_err = 1;
//		surfacedata->set_max_error(max_err);
//	}
//	berror_show = ev;
//	updateGL();
//}
//void MeshViewer::set_controlmesh_show(bool cm)
//{
//	basis_size = surfacedata->get_basis_size();
//	if(cm)
//	{
//		/*surfacedata->get_control_mesh();*/
//		int  basis_sz = surfacedata->get_basis_size();
//		QString str = "The number of the control vertices is ";
//		str.append(QString::number(basis_sz));
//		QMessageBox::information(this, "Number of the Control Vertices", str, QMessageBox::Yes, QMessageBox::Yes);
//	}
//	bcontrolmesh_show = cm;
//	updateGL();
//}
//
void MeshViewer::set_adjustedmesh_enabled(bool av)
{
	badjustedmesh_show = av;
	updateGL();
}

void MeshViewer::paintEvent(QPaintEvent *event)
{
	Q_UNUSED(event)
	QPainter painter;
	painter.begin(this);
	painter.setRenderHint(QPainter::Antialiasing);

	//// Save current OpenGL state,
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


//int MeshViewer::check_selected(Point_3 &pt, double dist)
//{
//	if(!surfacedata)
//		return -1;
//	return surfacedata->check_selected(pt, dist);
//}
//
//void MeshViewer::mousePressEvent(QMouseEvent *event)
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
//void MeshViewer::mouseMoveEvent(QMouseEvent *event)
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
//void MeshViewer::mouseReleaseEvent(QMouseEvent *event)
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
void MeshViewer::set_adjustpoints_enabled(bool av)
{
	badjust = av;
	updateGL();
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

//void MeshViewer::set_max_error_point_show(bool mv)
//{
//	bmax_error_point_show = mv;
//}
//
//void MeshViewer::set_controledge_show(bool cv)
//{
//	bcontroledge_show = cv;
//}
//
//void MeshViewer::set_sparsemesh_view(bool sv)
//{
//	bsparsemeshview = sv;
//}
//
//void MeshViewer::set_sparsecurves_view(bool sv)
//{
//	bsparsecurvesview = sv;
//}
//
//void MeshViewer::set_featureline_view(bool fv)
//{
//	bfeaturelineview = fv;
//}
//
//void MeshViewer::draw_featurelines()
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