/*************************************************
* Copyright (c) 2014-至今,厦门大学计算机图形学实验室Graphics@XMU
* All rights reserved.
*
* 文件名称：SurfaceData.cpp
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
/** To use the IO facility of OpenMesh make sure that the include MeshIO.hh is included first.*/
#include <OpenMesh/Core/IO/MeshIO.hh>
#include <OpenMesh/Tools/Utils/getopt.h>

//Qt
#include <QtGui/QtGui>
#include <QtGui/QInputDialog>
#include <QtGui/QMessageBox>
#include <QtGui/QFileDialog>
#include <QtCore/QDir>
#include <QtCore/QTextStream>

//CGAL
#include <CGAL/convex_hull_2.h>
#include <CGAL/convexity_check_2.h>

//Eigen
#include <Eigen/Dense>

//STL
#include <iostream>
#include <string>
#include <functional>//plus
#include <algorithm>
#include <utility>//pair
#include <fstream>


#include <windows.h>
#include <ctime>

#include "SurfaceData.h"




#define _READ_ 1 //没什么作用，弃用
typedef  vector<int> vecint;
typedef  vector<vector< OpenMesh::Vec3d> > vectex;
typedef vector<double>::iterator   viterator;
typedef  pair<viterator,viterator>  VIPair;
using namespace std;
using namespace Eigen;



vector<double>  error_sum;//每一次迭代的误差和
int CSurfaceData::iter_num=1;


CSurfaceData::CSurfaceData(void)
{
	m_pOriginalMesh = NULL; /**< 必须设为NULL，\see MeshViewer::draw(), MeshViewer::draw_original_mesh()*/
    parameter1=new(std::nothrow) B_parameter(0,0,0,0);
	m_pFittedMesh = NULL;
	m_pErrorMesh = NULL;
	bsurface=NULL;
	//error = NULL;
	sample_num = 0;

	err_threshold = 1;
	Curvature_loading=false;
	fitting_completed_=false;
	curvature_error=false;
	error_compute_ok=false;
	max_err.error = 1000;//初始化,已在void SplineKnotsViewer::error_fitting_view(bool kv)中尚未计算误差时使用
	max_err.index = -1;
	rangequery_vertical=false;
	rangequery_horizon=false;
    kdTree=NULL;    //必须加上NULL，否则析构CsurfaceData，就会出现错误
}


CSurfaceData::~CSurfaceData(void)
{
	 clear_data();
	 delete kdTree;
	 kdTree=NULL;
	 annClose();	

		
}
void CSurfaceData::setparameter(B_parameter *parameter_2)
{

	parameter1=parameter_2;
}
void CSurfaceData::set_knots_adding_num(int num)
{
	add_num=num;
}
void CSurfaceData::set_horizon_knots_addnum(int num)
{
	add_horizon_num=num;
}
int CSurfaceData::sum(std::vector<KnotData> &vec)
{
	int sum_=0;
	for (std::vector<KnotData>::iterator p=vec.begin();p!=vec.end();++p)
	{
		sum_+=(*p).num;
	}
	return sum_;
}
void CSurfaceData::range_query()
{
	polymesh.clear();
	// generate vertices
	vector<vector<MyMesh::VertexHandle> > vhandle;
	vhandle.resize(vnum,vector<MyMesh::VertexHandle>(unum));
	for(int j=0;j<vnum;j++)
		for (int i=0;i<unum;i++)
		{
			vhandle[j][i]=polymesh.add_vertex(MyMesh::Point(uknots[i],vknots[j],0.0));
		}

		// generate (quadrilateral) faces
		std::vector<MyMesh::VertexHandle>  face_vhandles;
		for(int j=0;j<vnum-1;j++)
			for (int i=0;i<unum-1;i++)
			{
				face_vhandles.clear();
				face_vhandles.push_back(vhandle[j][i]);
				face_vhandles.push_back(vhandle[j][i+1]);
				face_vhandles.push_back(vhandle[j+1][i+1]);
				face_vhandles.push_back(vhandle[j+1][i]);
				polymesh.add_face(face_vhandles);

			}
			//至此矩形面建立完毕

			Mesh::FaceIter  f_it,f_end(m_pOriginalMesh->faces_end());
			Mesh::FaceVertexIter fv_it2;

			OpenMesh::FPropHandleT<vecint> v_index;  //增加每个面的内部点和边界点序号属性
			polymesh.add_property(v_index,"v_index");

			OpenMesh::VPropHandleT<Location_Type> v_location; //添加点属性
			m_pOriginalMesh->get_property_handle(v_location,"v_location");

			OpenMesh::FPropHandleT<Location_Type> f_location; //添加面属性
			m_pOriginalMesh->get_property_handle(f_location,"f_location");

			//先对每个面对积分
			MyMesh::FaceIter f2_it,f2_end(polymesh.faces_end());  
			MyMesh::FaceVertexIter  fv2_it;
			for (f2_it=polymesh.faces_begin();f2_it!=f2_end;++f2_it)
			{

				fv2_it=polymesh.fv_iter(*f2_it);
				double uknots_min=polymesh.point(*fv2_it)[0];
				double vknots_min=polymesh.point(*fv2_it)[1];
				fv2_it++;
				fv2_it++;
				double uknots_max=polymesh.point(*fv2_it)[0];
				double vknots_max=polymesh.point(*fv2_it)[1];

				ANNpoint			queryPt;				// query point
				ANNidxArray			nnIdx;					// near neighbor indices
				ANNdistArray		dists;					// near neighbor distances
				double			     eps= 0.00001;			// error bound
				queryPt = annAllocPt(2);					// allocate query point
				nnIdx = new(std::nothrow) ANNidx[1];						// allocate near neigh indices
				dists = new(std::nothrow) ANNdist[1];						// allocate near neighbor dists

				queryPt[0]=(uknots_min+uknots_max)/2;
				queryPt[1]=(vknots_min+vknots_max)/2;

				kdTree->annkSearch(						// search
					queryPt,						// query point
					1,								// number of near neighbors
					nnIdx,							// nearest neighbors (returned)
					dists,							// distance (returned)
					eps);							// error bound
				int idx=nnIdx[0];
				delete [] nnIdx;							// clean things up
				delete [] dists;
				Mesh::VertexHandle  p=m_pOriginalMesh->vertex_handle(idx);								
				//扩散过程，求矩形区域包含的内部三角形，跨越三角形，外部三角形
				Mesh::VertexIter   v_it,v_end(m_pOriginalMesh->vertices_end());
				for (v_it=m_pOriginalMesh->vertices_begin();v_it!=v_end;++v_it)
				{
					m_pOriginalMesh->property(v_location,*v_it)=unknown;

				}

				for (f_it=m_pOriginalMesh->faces_begin(); f_it!=f_end; ++f_it)
				{
					m_pOriginalMesh->property(f_location,*f_it)=unknown;
				}

				TexCoord  p2=m_pOriginalMesh->texcoord2D(p);

				if(uknots_min<=p2[0]&&p2[0]<=uknots_max&&p2[1]>=vknots_min&&p2[1]<=vknots_max)	
				{
					m_pOriginalMesh->property(v_location,p)=inside;
					int turn=0;
					vector<int>  v1;//遍历一轮的顶点
					v1.push_back(p.idx());
					polymesh.property(v_index,*f2_it).push_back(p.idx());
					vector<int>  v4;//存储所有的外部顶点序号
					do 
					{
						vector<int> v3;//存储新一轮的顶点序号

						for (vector<int>::iterator  v2=v1.begin();v2!=v1.end();++v2)
						{
							for(Mesh::VertexFaceIter vf_it=m_pOriginalMesh->vf_iter(m_pOriginalMesh->vertex_handle(*v2));vf_it.is_valid();++vf_it)
							{
								if (m_pOriginalMesh->property(f_location,*vf_it)==unknown)
								{
									int inside_sum=0,outside_sum=0,boundary_sum=0;
									for (Mesh::FaceVertexIter  fv_it=m_pOriginalMesh->fv_iter(*vf_it);fv_it.is_valid();++fv_it)
									{

										if (m_pOriginalMesh->property(v_location,*fv_it)==unknown)
										{
											//判定位置
											TexCoord  p3=m_pOriginalMesh->texcoord2D(*fv_it);
											if ((abs(p3[0]-uknots_min)<=EPSILON2 &&(p3[1]>=vknots_min&&p3[1]<=vknots_max))
												||(abs(p3[0]-uknots_max)<=EPSILON2&&(p3[1]>=vknots_min&&p3[1]<=vknots_max))
												||(abs(p3[1]-vknots_min)<=EPSILON2&&(p3[0]>=uknots_min&&p3[0]<=uknots_max))
												||(abs(p3[1]-vknots_max)<=EPSILON2&&(p3[0]>=uknots_min&&p3[0]<=uknots_max)))
											{
												m_pOriginalMesh->property(v_location,*fv_it)=boundary;
												boundary_sum++;
												v3.push_back((*fv_it).idx());
												polymesh.property(v_index,*f2_it).push_back((*fv_it).idx());
											}
											else if (p3[0]>=uknots_min&&p3[0]<=uknots_max&&p3[1]>=vknots_min&&p3[1]<=vknots_max)
											{
												m_pOriginalMesh->property(v_location,*fv_it)=inside;
												inside_sum++;
												v3.push_back((*fv_it).idx());
												polymesh.property(v_index,*f2_it).push_back((*fv_it).idx());
											}  
											else
											{
												m_pOriginalMesh->property(v_location,*fv_it)=outside;
												outside_sum++;
												v4.push_back((*fv_it).idx());
											}

										}
										else if (m_pOriginalMesh->property(v_location,*fv_it)==inside)
										{
											inside_sum++;
										}
										else if (m_pOriginalMesh->property(v_location,*fv_it)==outside)
										{
											outside_sum++;
										}
										else
										{
											boundary_sum++;						
										}
									}
									//判断三角形的位置
									assert(outside_sum+boundary_sum+inside_sum==3);

									if (inside_sum==3)
									{
										m_pOriginalMesh->property(f_location,*vf_it)=inside;
									} 
									else if (inside_sum==2)
									{
										if (boundary_sum==1)
										{
											m_pOriginalMesh->property(f_location,*vf_it)=inside;

										}
										else
										{
											m_pOriginalMesh->property(f_location,*vf_it)=boundary;									

										}

									}
									else if (inside_sum==1)
									{
										if (boundary_sum==2)
										{
											m_pOriginalMesh->property(f_location,*vf_it)=inside;

										}
										else //1-0-2,1-1-1
										{
											m_pOriginalMesh->property(f_location,*vf_it)=boundary;

										}
									}
									else
									{
										assert(inside_sum==0);
										if (boundary_sum==3)
										{
											m_pOriginalMesh->property(f_location,*vf_it)=inside;

										}
										else //0-2-1,0-1-2,0-0-3
										{
											m_pOriginalMesh->property(f_location,*vf_it)=outside;
										}

									}
								}
							}

						}
						if (turn==0)
						{
							v1.assign(v3.begin(),v3.end());
						}
						if (turn>0)
						{
							turn++;
							v1.assign(v4.begin(),v4.end());
							v1.insert(v1.end(),v3.begin(),v3.end());
							v4.clear();
						}
						if (v1.empty())
						{
							if (turn>0)
								break;
							//assert(turn==0);
							v1.assign(v4.begin(),v4.end());
							v4.clear();
							turn++;
						}

					} while (turn<20);

			 }
				else
			 {
				 cout<<"需要的序号:"<<(*f2_it).idx()<<endl;
				 m_pOriginalMesh->property(v_location,p)=outside;
				 int turn=0;
				 vector<int>  v1;//遍历一轮的顶点
				 v1.push_back(p.idx());
				 do 
				 {
					 vector<int> v3;//存储新一轮的顶点序号
					 for (vector<int>::iterator  v2=v1.begin();v2!=v1.end();++v2)
					 {
						 for(Mesh::VertexFaceIter vf_it=m_pOriginalMesh->vf_iter(m_pOriginalMesh->vertex_handle(*v2));vf_it.is_valid();++vf_it)
						 {
							 if (m_pOriginalMesh->property(f_location,*vf_it)==unknown)
							 {
								 int inside_sum=0,outside_sum=0,boundary_sum=0;
								 for (Mesh::FaceVertexIter  fv_it=m_pOriginalMesh->fv_iter(*vf_it);fv_it.is_valid();++fv_it)
								 {

									 if (m_pOriginalMesh->property(v_location,*fv_it)==unknown)
									 {
										 //判定位置
										 TexCoord  p3=m_pOriginalMesh->texcoord2D(*fv_it);
										 if ((abs(p3[0]-uknots_min)<=EPSILON2 &&(p3[1]>=vknots_min&&p3[1]<=vknots_max))
											 ||(abs(p3[0]-uknots_max)<=EPSILON2&&(p3[1]>=vknots_min&&p3[1]<=vknots_max))
											 ||(abs(p3[1]-vknots_min)<=EPSILON2&&(p3[0]>=uknots_min&&p3[0]<=uknots_max))
											 ||(abs(p3[1]-vknots_max)<=EPSILON2&&(p3[0]>=uknots_min&&p3[0]<=uknots_max)))
										 {
											 m_pOriginalMesh->property(v_location,*fv_it)=boundary;
											 boundary_sum++;
											 v3.push_back((*fv_it).idx());
											 polymesh.property(v_index,*f2_it).push_back((*fv_it).idx());
										 }
										 else if (p3[0]>=uknots_min&&p3[0]<=uknots_max&&p3[1]>=vknots_min&&p3[1]<=vknots_max)
										 {
											 m_pOriginalMesh->property(v_location,*fv_it)=inside;
											 inside_sum++;
											 v3.push_back((*fv_it).idx());
											 polymesh.property(v_index,*f2_it).push_back((*fv_it).idx());
										 }  
										 else
										 {
											 m_pOriginalMesh->property(v_location,*fv_it)=outside;
											 outside_sum++;
											 v3.push_back((*fv_it).idx());
										 }

									 }
									 else if (m_pOriginalMesh->property(v_location,*fv_it)==inside)
									 {
										 inside_sum++;
									 }
									 else if (m_pOriginalMesh->property(v_location,*fv_it)==outside)
									 {
										 outside_sum++;
									 }
									 else
									 {
										 boundary_sum++;						
									 }
								 }
								 //判断三角形的位置

								 assert(outside_sum+boundary_sum+inside_sum==3);

								 if (inside_sum==3)
								 {
									 m_pOriginalMesh->property(f_location,*vf_it)=inside;
								 } 
								 else if (inside_sum==2)
								 {
									 if (boundary_sum==1)
									 {
										 m_pOriginalMesh->property(f_location,*vf_it)=inside;

									 }
									 else
									 {
										 m_pOriginalMesh->property(f_location,*vf_it)=boundary;									

									 }

								 }
								 else if (inside_sum==1)
								 {
									 if (boundary_sum==2)
									 {
										 m_pOriginalMesh->property(f_location,*vf_it)=inside;

									 }
									 else //1-0-2,1-1-1
									 {
										 m_pOriginalMesh->property(f_location,*vf_it)=boundary;

									 }
								 }
								 else
								 {
									 assert(inside_sum==0);
									 if (boundary_sum==3)
									 {
										 m_pOriginalMesh->property(f_location,*vf_it)=inside;

									 }
									 else //0-2-1,0-1-2,0-0-3
									 {
										 m_pOriginalMesh->property(f_location,*vf_it)=outside;
									 }

								 }
							 }
						 }

					 }
					 v1.assign(v3.begin(),v3.end());
					 if (v1.empty())
					 {
						 break;
					 }
					 turn++;

				 } while (turn<20);

				 vecint  vindex2=polymesh.property(v_index,*f2_it);
				 if (vindex2.empty())
				 {
					 polymesh.property(v_index,*f2_it).push_back(-1);
				 }
				}
			}
			//填充horizon_index,vertical_index			vertical_index.clear();
			horizon_index.clear();
			vertical_index.clear();
			vertical_index.resize(unum-1);
			horizon_index.resize(vnum-1);
			cout<<"每列面序号输出："<<endl;
			for (int i=0;i<unum-1;i++)
			{
				cout<<"第"<<i+1<<"列序号为:"<<endl;
				for (int j=0;j<vnum-1;j++)
				{
					int index=i+(unum-1)*j;
					cout<<index<<" ";
					MyMesh::FaceHandle f_h=polymesh.face_handle(index);
					vecint  vindex=polymesh.property(v_index,f_h);
					if (vindex[0]==-1)
					{
						continue;

					}
					copy(vindex.begin(),vindex.end(),back_inserter(vertical_index[i]));
				}
				cout<<endl;
				sort(vertical_index[i].begin(),vertical_index[i].end());
				vertical_index[i].erase( std::unique( vertical_index[i].begin(), vertical_index[i].end()), vertical_index[i].end() );
			}

			cout<<"每行面序号输出："<<endl;
			for (int i=0;i<vnum-1;i++)
			{
				cout<<"第"<<i+1<<"行序号为:"<<endl;
				for (int j=0;j<unum-1;j++)
				{
					int index=(unum-1)*i+j;
					cout<<index<<" ";
					MyMesh::FaceHandle f_h=polymesh.face_handle(index);
					vecint  vindex=polymesh.property(v_index,f_h);
					if (vindex[0]==-1)
					{
						continue;
					}
					copy(vindex.begin(),vindex.end(),back_inserter(horizon_index[i]));
				}
				cout<<endl;
				sort(horizon_index[i].begin(),horizon_index[i].end());
				horizon_index[i].erase( std::unique( horizon_index[i].begin(),horizon_index[i].end()), horizon_index[i].end() );

			}
			//查找新的节点线左右/上下两侧的条形的含内部点和边界点的情况
			vector<double>  u_delete;
			for (vector<knot_info>::iterator ibegin=uknots_new.begin();ibegin!=uknots_new.end();)
			{

				VIPair P=equal_range(uknots.begin(),uknots.end(),(*ibegin).knot);
				bool stop=false;
				if (P.first!=P.second)//找到特定值
				{
					assert(distance(P.first,P.second)==1);
					int idx1=distance(uknots.begin(),P.first)-1;
					int i=idx1;
					while (i<=idx1+1)
					{
						for (int j=0;j<vnum-1;j++)
					 {
						 int index=i+(unum-1)*j;

						 MyMesh::FaceHandle f_h=polymesh.face_handle(index);
						 vecint  vindex=polymesh.property(v_index,f_h);
						 if (vindex[0]==-1)//表明该面没有包含点
						 {
							 stop=true;
							 break;

						 }

						}
						if (stop)
							break;
						i++;
					}
					if (stop)
					{
						u_delete.push_back(*P.first);
						ibegin=uknots_new.erase(ibegin);
					}
					else
						ibegin++;
				}
				else
					ibegin++;
			}
			vector<double>  v_delete;
			for (vector<knot_info>::iterator ibegin=vknots_new.begin();ibegin!=vknots_new.end();)
			{

				VIPair P=equal_range(vknots.begin(),vknots.end(),(*ibegin).knot);
				bool stop=false;
				if (P.first!=P.second)//找到特定值
				{
					assert(distance(P.first,P.second)==1);
					int idx1=distance(vknots.begin(),P.first)-1;
					int i=idx1;
					while (i<=idx1+1)
					{
						for (int j=0;j<unum-1;j++)
						{
							int index=(unum-1)*i+j;

							MyMesh::FaceHandle f_h=polymesh.face_handle(index);
							vecint  vindex=polymesh.property(v_index,f_h);
							if (vindex[0]==-1)//表明该面没有包含点
						 {
							 stop=true;
							 break;

						 }

						}
						if (stop)
							break;
						i++;
					}
					if (stop)
					{
						v_delete.push_back(*P.first);
						ibegin=vknots_new.erase(ibegin);
					}
					else
						ibegin++;
				}
				else
					ibegin++;
			}
			for (vector<double>::iterator vi=u_delete.begin();vi!=u_delete.end();++vi)
			{
				uknots.erase(remove(uknots.begin(),uknots.end(),*vi),uknots.end());
			}
			for (vector<double>::iterator vi=v_delete.begin();vi!=v_delete.end();++vi)
			{
				vknots.erase(remove(vknots.begin(),vknots.end(),*vi),vknots.end());
			}

	

}
void CSurfaceData::range_query2()
{
	polymesh.clear();
	// generate vertices
	vector<vector<MyMesh::VertexHandle> > vhandle;
	vhandle.resize(vnum,vector<MyMesh::VertexHandle>(unum));
	for(int j=0;j<vnum;j++)
		for (int i=0;i<unum;i++)
		{
			vhandle[j][i]=polymesh.add_vertex(MyMesh::Point(uknots[i],vknots[j],0.0));
		}

		// generate (quadrilateral) faces
		std::vector<MyMesh::VertexHandle>  face_vhandles;
		for(int j=0;j<vnum-1;j++)
			for (int i=0;i<unum-1;i++)
			{
				face_vhandles.clear();
				face_vhandles.push_back(vhandle[j][i]);
				face_vhandles.push_back(vhandle[j][i+1]);
				face_vhandles.push_back(vhandle[j+1][i+1]);
				face_vhandles.push_back(vhandle[j+1][i]);
				polymesh.add_face(face_vhandles);

			}
			//至此矩形面建立完毕

			Mesh::FaceIter  f_it,f_end(m_pOriginalMesh->faces_end());
			Mesh::FaceVertexIter fv_it2;

			OpenMesh::FPropHandleT<vecint> v_index;  //增加每个面的内部点和边界点序号属性
			polymesh.add_property(v_index,"v_index");

			OpenMesh::VPropHandleT<Location_Type> v_location; //添加点属性
			m_pOriginalMesh->get_property_handle(v_location,"v_location");

			OpenMesh::FPropHandleT<Location_Type> f_location; //添加面属性
			m_pOriginalMesh->get_property_handle(f_location,"f_location");

			//先对每个面对积分
			MyMesh::FaceIter f2_it,f2_end(polymesh.faces_end());  
			MyMesh::FaceVertexIter  fv2_it;
			for (f2_it=polymesh.faces_begin();f2_it!=f2_end;++f2_it)
			{

				fv2_it=polymesh.fv_iter(*f2_it);
				double uknots_min=polymesh.point(*fv2_it)[0];
				double vknots_min=polymesh.point(*fv2_it)[1];
				fv2_it++;
				fv2_it++;
				double uknots_max=polymesh.point(*fv2_it)[0];
				double vknots_max=polymesh.point(*fv2_it)[1];

				ANNpoint			queryPt;				// query point
				ANNidxArray			nnIdx;					// near neighbor indices
				ANNdistArray		dists;					// near neighbor distances
				double			     eps= 0.00001;			// error bound
				queryPt = annAllocPt(2);					// allocate query point
				nnIdx = new(std::nothrow) ANNidx[1];						// allocate near neigh indices
				dists = new(std::nothrow) ANNdist[1];						// allocate near neighbor dists

				queryPt[0]=(uknots_min+uknots_max)/2;
				queryPt[1]=(vknots_min+vknots_max)/2;

				kdTree->annkSearch(						// search
					queryPt,						// query point
					1,								// number of near neighbors
					nnIdx,							// nearest neighbors (returned)
					dists,							// distance (returned)
					eps);							// error bound
				int idx=nnIdx[0];
				delete [] nnIdx;							// clean things up
				delete [] dists;
				Mesh::VertexHandle  p=m_pOriginalMesh->vertex_handle(idx);								
				//扩散过程，求矩形区域包含的内部三角形，跨越三角形，外部三角形
				Mesh::VertexIter   v_it,v_end(m_pOriginalMesh->vertices_end());
				for (v_it=m_pOriginalMesh->vertices_begin();v_it!=v_end;++v_it)
				{
					m_pOriginalMesh->property(v_location,*v_it)=unknown;

				}

				for (f_it=m_pOriginalMesh->faces_begin(); f_it!=f_end; ++f_it)
				{
					m_pOriginalMesh->property(f_location,*f_it)=unknown;
				}

				TexCoord  p2=m_pOriginalMesh->texcoord2D(p);

				if(uknots_min<=p2[0]&&p2[0]<=uknots_max&&p2[1]>=vknots_min&&p2[1]<=vknots_max)	
				{
					m_pOriginalMesh->property(v_location,p)=inside;
					int turn=0;
					vector<int>  v1;//遍历一轮的顶点
					v1.push_back(p.idx());
					polymesh.property(v_index,*f2_it).push_back(p.idx());
					vector<int>  v4;//存储所有的外部顶点序号
					do 
					{
						vector<int> v3;//存储新一轮的顶点序号

						for (vector<int>::iterator  v2=v1.begin();v2!=v1.end();++v2)
						{
							for(Mesh::VertexFaceIter vf_it=m_pOriginalMesh->vf_iter(m_pOriginalMesh->vertex_handle(*v2));vf_it.is_valid();++vf_it)
							{
								if (m_pOriginalMesh->property(f_location,*vf_it)==unknown)
								{
									int inside_sum=0,outside_sum=0,boundary_sum=0;
									for (Mesh::FaceVertexIter  fv_it=m_pOriginalMesh->fv_iter(*vf_it);fv_it.is_valid();++fv_it)
									{

										if (m_pOriginalMesh->property(v_location,*fv_it)==unknown)
										{
											//判定位置
											TexCoord  p3=m_pOriginalMesh->texcoord2D(*fv_it);
											if ((abs(p3[0]-uknots_min)<=EPSILON2 &&(p3[1]>=vknots_min&&p3[1]<=vknots_max))
												||(abs(p3[0]-uknots_max)<=EPSILON2&&(p3[1]>=vknots_min&&p3[1]<=vknots_max))
												||(abs(p3[1]-vknots_min)<=EPSILON2&&(p3[0]>=uknots_min&&p3[0]<=uknots_max))
												||(abs(p3[1]-vknots_max)<=EPSILON2&&(p3[0]>=uknots_min&&p3[0]<=uknots_max)))
											{
												m_pOriginalMesh->property(v_location,*fv_it)=boundary;
												boundary_sum++;
												v3.push_back((*fv_it).idx());
												polymesh.property(v_index,*f2_it).push_back((*fv_it).idx());
											}
											else if (p3[0]>=uknots_min&&p3[0]<=uknots_max&&p3[1]>=vknots_min&&p3[1]<=vknots_max)
											{
												m_pOriginalMesh->property(v_location,*fv_it)=inside;
												inside_sum++;
												v3.push_back((*fv_it).idx());
												polymesh.property(v_index,*f2_it).push_back((*fv_it).idx());
											}  
											else
											{
												m_pOriginalMesh->property(v_location,*fv_it)=outside;
												outside_sum++;
												v4.push_back((*fv_it).idx());
											}

										}
										else if (m_pOriginalMesh->property(v_location,*fv_it)==inside)
										{
											inside_sum++;
										}
										else if (m_pOriginalMesh->property(v_location,*fv_it)==outside)
										{
											outside_sum++;
										}
										else
										{
											boundary_sum++;						
										}
									}
									//判断三角形的位置
									assert(outside_sum+boundary_sum+inside_sum==3);

									if (inside_sum==3)
									{
										m_pOriginalMesh->property(f_location,*vf_it)=inside;
									} 
									else if (inside_sum==2)
									{
										if (boundary_sum==1)
										{
											m_pOriginalMesh->property(f_location,*vf_it)=inside;

										}
										else
										{
											m_pOriginalMesh->property(f_location,*vf_it)=boundary;									

										}

									}
									else if (inside_sum==1)
									{
										if (boundary_sum==2)
										{
											m_pOriginalMesh->property(f_location,*vf_it)=inside;

										}
										else //1-0-2,1-1-1
										{
											m_pOriginalMesh->property(f_location,*vf_it)=boundary;

										}
									}
									else
									{
										assert(inside_sum==0);
										if (boundary_sum==3)
										{
											m_pOriginalMesh->property(f_location,*vf_it)=inside;

										}
										else //0-2-1,0-1-2,0-0-3
										{
											m_pOriginalMesh->property(f_location,*vf_it)=outside;
										}

									}
								}
							}

						}
						if (turn==0)
						{
							v1.assign(v3.begin(),v3.end());
						}
						if (turn>0)
						{
							turn++;
							v1.assign(v4.begin(),v4.end());
							v1.insert(v1.end(),v3.begin(),v3.end());
							v4.clear();
						}
						if (v1.empty())
						{
							if (turn>0)
								break;
							//assert(turn==0);
							v1.assign(v4.begin(),v4.end());
							v4.clear();
							turn++;
						}

					} while (turn<20);

			 }
				else
			 {
				 cout<<"需要的序号:"<<(*f2_it).idx()<<endl;
				 m_pOriginalMesh->property(v_location,p)=outside;
				 int turn=0;
				 vector<int>  v1;//遍历一轮的顶点
				 v1.push_back(p.idx());
				 do 
				 {
					 vector<int> v3;//存储新一轮的顶点序号
					 for (vector<int>::iterator  v2=v1.begin();v2!=v1.end();++v2)
					 {
						 for(Mesh::VertexFaceIter vf_it=m_pOriginalMesh->vf_iter(m_pOriginalMesh->vertex_handle(*v2));vf_it.is_valid();++vf_it)
						 {
							 if (m_pOriginalMesh->property(f_location,*vf_it)==unknown)
							 {
								 int inside_sum=0,outside_sum=0,boundary_sum=0;
								 for (Mesh::FaceVertexIter  fv_it=m_pOriginalMesh->fv_iter(*vf_it);fv_it.is_valid();++fv_it)
								 {

									 if (m_pOriginalMesh->property(v_location,*fv_it)==unknown)
									 {
										 //判定位置
										 TexCoord  p3=m_pOriginalMesh->texcoord2D(*fv_it);
										 if ((abs(p3[0]-uknots_min)<=EPSILON2 &&(p3[1]>=vknots_min&&p3[1]<=vknots_max))
											 ||(abs(p3[0]-uknots_max)<=EPSILON2&&(p3[1]>=vknots_min&&p3[1]<=vknots_max))
											 ||(abs(p3[1]-vknots_min)<=EPSILON2&&(p3[0]>=uknots_min&&p3[0]<=uknots_max))
											 ||(abs(p3[1]-vknots_max)<=EPSILON2&&(p3[0]>=uknots_min&&p3[0]<=uknots_max)))
										 {
											 m_pOriginalMesh->property(v_location,*fv_it)=boundary;
											 boundary_sum++;
											 v3.push_back((*fv_it).idx());
											 polymesh.property(v_index,*f2_it).push_back((*fv_it).idx());
										 }
										 else if (p3[0]>=uknots_min&&p3[0]<=uknots_max&&p3[1]>=vknots_min&&p3[1]<=vknots_max)
										 {
											 m_pOriginalMesh->property(v_location,*fv_it)=inside;
											 inside_sum++;
											 v3.push_back((*fv_it).idx());
											 polymesh.property(v_index,*f2_it).push_back((*fv_it).idx());
										 }  
										 else
										 {
											 m_pOriginalMesh->property(v_location,*fv_it)=outside;
											 outside_sum++;
											 v3.push_back((*fv_it).idx());
										 }

									 }
									 else if (m_pOriginalMesh->property(v_location,*fv_it)==inside)
									 {
										 inside_sum++;
									 }
									 else if (m_pOriginalMesh->property(v_location,*fv_it)==outside)
									 {
										 outside_sum++;
									 }
									 else
									 {
										 boundary_sum++;						
									 }
								 }
								 //判断三角形的位置

								 assert(outside_sum+boundary_sum+inside_sum==3);

								 if (inside_sum==3)
								 {
									 m_pOriginalMesh->property(f_location,*vf_it)=inside;
								 } 
								 else if (inside_sum==2)
								 {
									 if (boundary_sum==1)
									 {
										 m_pOriginalMesh->property(f_location,*vf_it)=inside;

									 }
									 else
									 {
										 m_pOriginalMesh->property(f_location,*vf_it)=boundary;									

									 }

								 }
								 else if (inside_sum==1)
								 {
									 if (boundary_sum==2)
									 {
										 m_pOriginalMesh->property(f_location,*vf_it)=inside;

									 }
									 else //1-0-2,1-1-1
									 {
										 m_pOriginalMesh->property(f_location,*vf_it)=boundary;

									 }
								 }
								 else
								 {
									 assert(inside_sum==0);
									 if (boundary_sum==3)
									 {
										 m_pOriginalMesh->property(f_location,*vf_it)=inside;

									 }
									 else //0-2-1,0-1-2,0-0-3
									 {
										 m_pOriginalMesh->property(f_location,*vf_it)=outside;
									 }

								 }
							 }
						 }

					 }
					 v1.assign(v3.begin(),v3.end());
					 if (v1.empty())
					 {
						 break;
					 }
					 turn++;

				 } while (turn<20);

				 vecint  vindex2=polymesh.property(v_index,*f2_it);
				 if (vindex2.empty())
				 {
					 polymesh.property(v_index,*f2_it).push_back(-1);
				 }
				}
			}
			//填充horizon_index,vertical_index			vertical_index.clear();
			horizon_index.clear();
			vertical_index.clear();
			vertical_index.resize(unum-1);
			horizon_index.resize(vnum-1);
			cout<<"每列面序号输出："<<endl;
			for (int i=0;i<unum-1;i++)
			{
				cout<<"第"<<i+1<<"列序号为:"<<endl;
				for (int j=0;j<vnum-1;j++)
				{
					int index=i+(unum-1)*j;
					cout<<index<<" ";
					MyMesh::FaceHandle f_h=polymesh.face_handle(index);
					vecint  vindex=polymesh.property(v_index,f_h);
					if (vindex[0]==-1)
					{
						continue;

					}
					copy(vindex.begin(),vindex.end(),back_inserter(vertical_index[i]));
				}
				cout<<endl;
				sort(vertical_index[i].begin(),vertical_index[i].end());
				vertical_index[i].erase( std::unique( vertical_index[i].begin(), vertical_index[i].end()), vertical_index[i].end() );
			}

			cout<<"每行面序号输出："<<endl;
			for (int i=0;i<vnum-1;i++)
			{
				cout<<"第"<<i+1<<"行序号为:"<<endl;
				for (int j=0;j<unum-1;j++)
				{
					int index=(unum-1)*i+j;
					cout<<index<<" ";
					MyMesh::FaceHandle f_h=polymesh.face_handle(index);
					vecint  vindex=polymesh.property(v_index,f_h);
					if (vindex[0]==-1)
					{
						continue;
					}
					copy(vindex.begin(),vindex.end(),back_inserter(horizon_index[i]));
				}
				cout<<endl;
				sort(horizon_index[i].begin(),horizon_index[i].end());
				horizon_index[i].erase( std::unique( horizon_index[i].begin(),horizon_index[i].end()), horizon_index[i].end() );

			}
			//查找新的节点线左右/上下两侧的条形的含内部点和边界点的情况
			vector<double>  utemp;
			vector<double>  vtemp;//待删除的元素
			for (int i=0;i<unum-2;)
			{

				int idx1=i;
				bool stop=false;
				while (i<=idx1+1)
				{
					for (int j=0;j<vnum-1;j++)
					{
						int index=i+(unum-1)*j;

						MyMesh::FaceHandle f_h=polymesh.face_handle(index);
						vecint  vindex=polymesh.property(v_index,f_h);
						if (vindex[0]==-1)//表明该面没有包含点
						{
							stop=true;
							break;

						}

					}
					if (stop)
						break;
					i++;
				}
				if (stop)
				{
					utemp.push_back(uknots[idx1+1]);
				}
				i=idx1+1;

			}
			for (int i=0;i<vnum-2;)
			{

				int idx1=i;
				bool stop=false;
				while (i<=idx1+1)
				{
					for (int j=0;j<unum-1;j++)
					{
						int index=(unum-1)*i+j;

						MyMesh::FaceHandle f_h=polymesh.face_handle(index);
						vecint  vindex=polymesh.property(v_index,f_h);
						if (vindex[0]==-1)
						{
							stop=true;
							break;
						}
					}
					if (stop)
						break;
					i++;

				}

				if (stop)
				{
					vtemp.push_back(vknots[idx1+1]);
				}
				i=idx1+1;
			}
			for (vector<double>::iterator ui=utemp.begin();ui!=utemp.end();++ui)
			{
				uknots.erase(remove(uknots.begin(),uknots.end(),*ui),uknots.end());
			}
			for (vector<double>::iterator vi=vtemp.begin();vi!=vtemp.end();++vi)
			{
				vknots.erase(remove(vknots.begin(),vknots.end(),*vi),vknots.end());
			}

}

/***********************************************************************
void CSurfaceData::range_query2()
{
	polymesh.clear();
	parameter1->compute_knots();
	uknots=parameter1->getuknot();
	vknots=parameter1->getvknot();
	unum=uknots.size();
	vnum=vknots.size();
	//建立kd-tree
	int					nPts;					// actual number of data points
	ANNpointArray		dataPts;				// data points
	int				maxPts= m_pOriginalMesh->n_vertices();			// maximum number of data points 				
	dataPts = annAllocPts(maxPts, 2);			// allocate data points

	nPts = 0;	
	Mesh::VertexIter          v_it, v_end(m_pOriginalMesh->vertices_end());
	for (v_it=m_pOriginalMesh->vertices_begin(); v_it!=v_end; ++v_it,nPts++)
	{
		dataPts[nPts][0]=m_pOriginalMesh->texcoord2D(*v_it)[0];
		dataPts[nPts][1]=m_pOriginalMesh->texcoord2D(*v_it)[1];
		assert(nPts==(*v_it).idx());
	}
	kdTree = new ANNkd_tree(					// build search structure
		dataPts,					// the data points
		nPts,						// number of points
		2);						// dimension of space
	// generate vertices
	vector<vector<MyMesh::VertexHandle> > vhandle;
	vhandle.resize(vnum,vector<MyMesh::VertexHandle>(unum));
	for(int j=0;j<vnum;j++)
		for (int i=0;i<unum;i++)
		{
			vhandle[j][i]=polymesh.add_vertex(MyMesh::Point(uknots[i],vknots[j],0.0));
		}

		// generate (quadrilateral) faces
		std::vector<MyMesh::VertexHandle>  face_vhandles;
		for(int j=0;j<vnum-1;j++)
			for (int i=0;i<unum-1;i++)
			{
				face_vhandles.clear();
				face_vhandles.push_back(vhandle[j][i]);
				face_vhandles.push_back(vhandle[j][i+1]);
				face_vhandles.push_back(vhandle[j+1][i+1]);
				face_vhandles.push_back(vhandle[j+1][i]);
				polymesh.add_face(face_vhandles);

			}
			//至此矩形面建立完毕

			Mesh::FaceIter  f_it,f_end(m_pOriginalMesh->faces_end());
			Mesh::FaceVertexIter fv_it2;

			OpenMesh::FPropHandleT<vecint> v_index;  //增加每个面的内部点和边界点序号属性
			polymesh.add_property(v_index,"v_index");

			OpenMesh::VPropHandleT<Location_Type> v_location; //添加点属性
			m_pOriginalMesh->add_property(v_location,"v_location");

			OpenMesh::FPropHandleT<Location_Type> f_location; //添加面属性
			m_pOriginalMesh->add_property(f_location,"f_location");

			//先对每个面对积分
			MyMesh::FaceIter f2_it,f2_end(polymesh.faces_end());  
			MyMesh::FaceVertexIter  fv2_it;
			for (f2_it=polymesh.faces_begin();f2_it!=f2_end;++f2_it)
			{

				fv2_it=polymesh.fv_iter(*f2_it);
				double uknots_min=polymesh.point(*fv2_it)[0];
				double vknots_min=polymesh.point(*fv2_it)[1];
				fv2_it++;
				fv2_it++;
				double uknots_max=polymesh.point(*fv2_it)[0];
				double vknots_max=polymesh.point(*fv2_it)[1];

				ANNpoint			queryPt;				// query point
				ANNidxArray			nnIdx;					// near neighbor indices
				ANNdistArray		dists;					// near neighbor distances
				double			     eps= 0.00001;			// error bound
				queryPt = annAllocPt(2);					// allocate query point
				nnIdx = new ANNidx[1];						// allocate near neigh indices
				dists = new ANNdist[1];						// allocate near neighbor dists

				queryPt[0]=(uknots_min+uknots_max)/2;
				queryPt[1]=(vknots_min+vknots_max)/2;

				kdTree->annkSearch(						// search
					queryPt,						// query point
					1,								// number of near neighbors
					nnIdx,							// nearest neighbors (returned)
					dists,							// distance (returned)
					eps);							// error bound
				int idx=nnIdx[0];
				delete [] nnIdx;							// clean things up
				delete [] dists;
				Mesh::VertexHandle  p=m_pOriginalMesh->vertex_handle(idx);								
				//扩散过程，求矩形区域包含的内部三角形，跨越三角形，外部三角形
				Mesh::VertexIter   v_it,v_end(m_pOriginalMesh->vertices_end());
				for (v_it=m_pOriginalMesh->vertices_begin();v_it!=v_end;++v_it)
				{
					m_pOriginalMesh->property(v_location,*v_it)=unknown;

				}

				for (f_it=m_pOriginalMesh->faces_begin(); f_it!=f_end; ++f_it)
				{
					m_pOriginalMesh->property(f_location,*f_it)=unknown;
				}

				TexCoord  p2=m_pOriginalMesh->texcoord2D(p);

				if(uknots_min<=p2[0]&&p2[0]<=uknots_max&&p2[1]>=vknots_min&&p2[1]<=vknots_max)	
				{
					m_pOriginalMesh->property(v_location,p)=inside;
                     int turn=0;
					vector<int>  v1;//遍历一轮的顶点
					v1.push_back(p.idx());
					polymesh.property(v_index,*f2_it).push_back(p.idx());
					vector<int>  v4;//存储所有的外部顶点序号
					do 
					{
						vector<int> v3;//存储新一轮的顶点序号
					
						for (vector<int>::iterator  v2=v1.begin();v2!=v1.end();++v2)
						{
							for(Mesh::VertexFaceIter vf_it=m_pOriginalMesh->vf_iter(m_pOriginalMesh->vertex_handle(*v2));vf_it.is_valid();++vf_it)
							{
								if (m_pOriginalMesh->property(f_location,*vf_it)==unknown)
								{
									int inside_sum=0,outside_sum=0,boundary_sum=0;
									for (Mesh::FaceVertexIter  fv_it=m_pOriginalMesh->fv_iter(*vf_it);fv_it.is_valid();++fv_it)
									{

										if (m_pOriginalMesh->property(v_location,*fv_it)==unknown)
										{
											//判定位置
											TexCoord  p3=m_pOriginalMesh->texcoord2D(*fv_it);
											if ((abs(p3[0]-uknots_min)<=EPSILON2 &&(p3[1]>=vknots_min&&p3[1]<=vknots_max))
												||(abs(p3[0]-uknots_max)<=EPSILON2&&(p3[1]>=vknots_min&&p3[1]<=vknots_max))
												||(abs(p3[1]-vknots_min)<=EPSILON2&&(p3[0]>=uknots_min&&p3[0]<=uknots_max))
												||(abs(p3[1]-vknots_max)<=EPSILON2&&(p3[0]>=uknots_min&&p3[0]<=uknots_max)))
											{
												m_pOriginalMesh->property(v_location,*fv_it)=boundary;
												boundary_sum++;
												v3.push_back((*fv_it).idx());
												polymesh.property(v_index,*f2_it).push_back((*fv_it).idx());
											}
											else if (p3[0]>=uknots_min&&p3[0]<=uknots_max&&p3[1]>=vknots_min&&p3[1]<=vknots_max)
											{
												m_pOriginalMesh->property(v_location,*fv_it)=inside;
												inside_sum++;
												v3.push_back((*fv_it).idx());
												polymesh.property(v_index,*f2_it).push_back((*fv_it).idx());
											}  
											else
											{
												m_pOriginalMesh->property(v_location,*fv_it)=outside;
												outside_sum++;
												v4.push_back((*fv_it).idx());
											}

										}
										else if (m_pOriginalMesh->property(v_location,*fv_it)==inside)
										{
											inside_sum++;
										}
										else if (m_pOriginalMesh->property(v_location,*fv_it)==outside)
										{
											outside_sum++;
										}
										else
										{
											boundary_sum++;						
										}
									}
									//判断三角形的位置
									assert(outside_sum+boundary_sum+inside_sum==3);

									if (inside_sum==3)
									{
										m_pOriginalMesh->property(f_location,*vf_it)=inside;
									} 
									else if (inside_sum==2)
									{
										if (boundary_sum==1)
										{
											m_pOriginalMesh->property(f_location,*vf_it)=inside;

										}
										else
										{
											m_pOriginalMesh->property(f_location,*vf_it)=boundary;									

										}

									}
									else if (inside_sum==1)
									{
										if (boundary_sum==2)
										{
											m_pOriginalMesh->property(f_location,*vf_it)=inside;

										}
										else //1-0-2,1-1-1
										{
											m_pOriginalMesh->property(f_location,*vf_it)=boundary;

										}
									}
									else
									{
										assert(inside_sum==0);
										if (boundary_sum==3)
										{
											m_pOriginalMesh->property(f_location,*vf_it)=inside;

										}
										else //0-2-1,0-1-2,0-0-3
										{
											m_pOriginalMesh->property(f_location,*vf_it)=outside;
										}

									}
								}
							}

						}
						if (turn==0)
						{
						v1.assign(v3.begin(),v3.end());
						}
						if (turn>0)
						{
							turn++;
							v1.assign(v4.begin(),v4.end());
							v1.insert(v1.end(),v3.begin(),v3.end());
							v4.clear();
						}
						if (v1.empty())
						{
							if (turn>0)
							   break;
							//assert(turn==0);
							v1.assign(v4.begin(),v4.end());
							v4.clear();
							turn++;
						}
						 
					} while (turn<20);

			 }
				else
			 {
				 cout<<"需要的序号:"<<(*f2_it).idx()<<endl;
				 m_pOriginalMesh->property(v_location,p)=outside;
				 int turn=0;
				 vector<int>  v1;//遍历一轮的顶点
				 v1.push_back(p.idx());
				 do 
				 {
					 vector<int> v3;//存储新一轮的顶点序号
					 for (vector<int>::iterator  v2=v1.begin();v2!=v1.end();++v2)
					 {
						 for(Mesh::VertexFaceIter vf_it=m_pOriginalMesh->vf_iter(m_pOriginalMesh->vertex_handle(*v2));vf_it.is_valid();++vf_it)
						 {
							 if (m_pOriginalMesh->property(f_location,*vf_it)==unknown)
							 {
								 int inside_sum=0,outside_sum=0,boundary_sum=0;
								 for (Mesh::FaceVertexIter  fv_it=m_pOriginalMesh->fv_iter(*vf_it);fv_it.is_valid();++fv_it)
								 {

									 if (m_pOriginalMesh->property(v_location,*fv_it)==unknown)
									 {
										 //判定位置
										 TexCoord  p3=m_pOriginalMesh->texcoord2D(*fv_it);
										 if ((abs(p3[0]-uknots_min)<=EPSILON2 &&(p3[1]>=vknots_min&&p3[1]<=vknots_max))
											 ||(abs(p3[0]-uknots_max)<=EPSILON2&&(p3[1]>=vknots_min&&p3[1]<=vknots_max))
											 ||(abs(p3[1]-vknots_min)<=EPSILON2&&(p3[0]>=uknots_min&&p3[0]<=uknots_max))
											 ||(abs(p3[1]-vknots_max)<=EPSILON2&&(p3[0]>=uknots_min&&p3[0]<=uknots_max)))
										 {
											 m_pOriginalMesh->property(v_location,*fv_it)=boundary;
											 boundary_sum++;
											 v3.push_back((*fv_it).idx());
											 polymesh.property(v_index,*f2_it).push_back((*fv_it).idx());
										 }
										 else if (p3[0]>=uknots_min&&p3[0]<=uknots_max&&p3[1]>=vknots_min&&p3[1]<=vknots_max)
										 {
											 m_pOriginalMesh->property(v_location,*fv_it)=inside;
											 inside_sum++;
											 v3.push_back((*fv_it).idx());
											 polymesh.property(v_index,*f2_it).push_back((*fv_it).idx());
										 }  
										 else
										 {
											 m_pOriginalMesh->property(v_location,*fv_it)=outside;
											 outside_sum++;
											 v3.push_back((*fv_it).idx());
										 }

									 }
									 else if (m_pOriginalMesh->property(v_location,*fv_it)==inside)
									 {
										 inside_sum++;
									 }
									 else if (m_pOriginalMesh->property(v_location,*fv_it)==outside)
									 {
										 outside_sum++;
									 }
									 else
									 {
										 boundary_sum++;						
									 }
								 }
								 //判断三角形的位置

								 assert(outside_sum+boundary_sum+inside_sum==3);

								 if (inside_sum==3)
								 {
									 m_pOriginalMesh->property(f_location,*vf_it)=inside;
								 } 
								 else if (inside_sum==2)
								 {
									 if (boundary_sum==1)
									 {
										 m_pOriginalMesh->property(f_location,*vf_it)=inside;

									 }
									 else
									 {
										 m_pOriginalMesh->property(f_location,*vf_it)=boundary;									

									 }

								 }
								 else if (inside_sum==1)
								 {
									 if (boundary_sum==2)
									 {
										 m_pOriginalMesh->property(f_location,*vf_it)=inside;

									 }
									 else //1-0-2,1-1-1
									 {
										 m_pOriginalMesh->property(f_location,*vf_it)=boundary;

									 }
								 }
								 else
								 {
									 assert(inside_sum==0);
									 if (boundary_sum==3)
									 {
										 m_pOriginalMesh->property(f_location,*vf_it)=inside;

									 }
									 else //0-2-1,0-1-2,0-0-3
									 {
										 m_pOriginalMesh->property(f_location,*vf_it)=outside;
									 }

								 }
							 }
						 }

					 }
					 v1.assign(v3.begin(),v3.end());
					 if (v1.empty())
					 {
						 break;
					 }
					 turn++;

				 } while (turn<20);

				 vecint  vindex2=polymesh.property(v_index,*f2_it);
				 if (vindex2.empty())
				 {
					 polymesh.property(v_index,*f2_it).push_back(-1);
				 }
				}
			}
			//填充horizon_index,vertical_index			vertical_index.clear();
			horizon_index.clear();
			vertical_index.clear();
			vertical_index.resize(unum-1);
			horizon_index.resize(vnum-1);
			cout<<"每列面序号输出："<<endl;
			for (int i=0;i<unum-1;i++)
			{
				cout<<"第"<<i+1<<"列序号为:"<<endl;
				for (int j=0;j<vnum-1;j++)
				{
					int index=i+(unum-1)*j;
					cout<<index<<" ";
					MyMesh::FaceHandle f_h=polymesh.face_handle(index);
					vecint  vindex=polymesh.property(v_index,f_h);
					if (i==1&&j==0)
					{
						test.assign(vindex.begin(),vindex.end());
					}
					if (vindex[0]==-1)
					{
						continue;

					}
					copy(vindex.begin(),vindex.end(),back_inserter(vertical_index[i]));
				}
				cout<<endl;
				sort(vertical_index[i].begin(),vertical_index[i].end());
				vertical_index[i].erase( std::unique( vertical_index[i].begin(), vertical_index[i].end()), vertical_index[i].end() );
			}

			//if (label==2)
			//{
			//	m_pOriginalMesh->request_vertex_colors(); 
			//	if (!m_pOriginalMesh->has_vertex_colors())
			//		return;
			//	for (Mesh::VertexIter tv=m_pOriginalMesh->vertices_begin();tv!=m_pOriginalMesh->vertices_end();++tv)
			//	{
			//		m_pOriginalMesh->set_color(*tv,Mesh::Color(120,120,120));
			//	}	
			//	MyMesh::FaceHandle f_h=polymesh.face_handle(5);
			//	vecint  vindex=polymesh.property(v_index,f_h);
			//	//vecint  vindex=vertical_index[1];
			//	for(vecint::iterator tv2=vindex.begin();tv2!=vindex.end();++tv2)
			//	{
			//		Mesh::VertexHandle pv=m_pOriginalMesh->vertex_handle(*tv2);
			//		m_pOriginalMesh->set_color(pv,Mesh::Color(255,0,0));
			//	
			//	}
			//}
			cout<<"每行面序号输出："<<endl;
			for (int i=0;i<vnum-1;i++)
			{
				cout<<"第"<<i+1<<"行序号为:"<<endl;
				for (int j=0;j<unum-1;j++)
				{
					int index=(unum-1)*i+j;
					cout<<index<<" ";
					MyMesh::FaceHandle f_h=polymesh.face_handle(index);
					vecint  vindex=polymesh.property(v_index,f_h);
					if (vindex[0]==-1)
					{
						continue;
					}
					copy(vindex.begin(),vindex.end(),back_inserter(horizon_index[i]));
				}
				cout<<endl;
				sort(horizon_index[i].begin(),horizon_index[i].end());
				horizon_index[i].erase( std::unique( horizon_index[i].begin(),horizon_index[i].end()), horizon_index[i].end() );

			}



}
*****************************************************************************/
bool CSurfaceData::add_knots()
{

	if (!error_compute_ok||kdTree==NULL)
	   return false;	
	 label=0;

	 adjust_vertical_knots();
	//while(max_err_real>0.029)
	//{
	//	/*if (label==2)
	//	{
	//		range_query();
	//		 break;
	//	}*/
	//	 adjustparameter();
 //       if (!mesh_fitting())
 //       {
	//		cout<<"error"<<label<<endl;
	//		cout<<"本次迭代最大误差："<<max_err_real<<endl;
	//		return false;
 //       }
 //       compute_error();
	//	cout<<"本次迭代最大误差："<<max_err_real<<endl;
	//	label++;
	//}
 // 

	//cout<<"最终的结果是"<<"p:"<<parameter1->getp()<<"q:"<<parameter1->getq()<<"m:"<<parameter1->getm()<<"n:"<<parameter1->getn()<<endl;
    return true;
}
bool CSurfaceData::add_horizon_knots()
{

if (!error_compute_ok||kdTree==NULL)
	   return false;	

	 adjust_horizon_knots();

	//cout<<"最终的结果是"<<"p:"<<parameter1->getp()<<"q:"<<parameter1->getq()<<"m:"<<parameter1->getm()<<"n:"<<parameter1->getn()<<endl;
    return true;


}
void CSurfaceData::adjust_horizon_knots()
{
	if(!error_compute_ok)
		return;
	//计时开始
	_LARGE_INTEGER time_start;    /*开始时间*/
	_LARGE_INTEGER time_over;        /*结束时间*/
	double dqFreq;                /*计时器频率*/
	LARGE_INTEGER f;            /*计时器频率*/
	QueryPerformanceFrequency(&f);
	dqFreq=(double)f.QuadPart;
	QueryPerformanceCounter(&time_start);
	if(add_horizon_num==0)
	{
		vknots_new.clear();//在使用之前先清零
		rangequery_horizon=false;
		rangequery_vertical=false;
		return;
	}
	if(!rangequery_vertical)
	{
		range_query();
		add_num==0?rangequery_horizon=false:rangequery_horizon=true;
		double  maxerr=max_err.error;
		double  cur1=0.3*mean_err+0.7*maxerr;
		cout<<"整体平均误差:"<<mean_err<<" "<<"最大误差:"<<maxerr<<"加权误差"<<cur1<<endl;
	}
	else
	{
		rangequery_vertical=false;

	}
	vector<ErrorData> errorvec;
	vector<KnotData>  knotvec;
	OpenMesh::VPropHandleT<double>  verror; 
	m_pFittedMesh->get_property_handle(verror,"verror");
	vector<int>  horizon_max_index;

	for (int i=0;i<vnum-1;i++)
	{
		double  meanerr=0;
		int num=0;
		Max_Error maxerr={0,0};
		if (horizon_index[i].size()==0)
		{
			cout<<"第"<<i+1<<"条水平条形由于没有点包含，所以在此条形中不增加节点线"<<endl;
			errorvec.push_back(ErrorData(i+unum-1,1e10));
			horizon_max_index.push_back(-1);
			continue;
		}
		for (vector<int>::iterator p=horizon_index[i].begin();p!=horizon_index[i].end();++p)
		{
			Mesh::VertexHandle v=m_pFittedMesh->vertex_handle(*p);
			meanerr+=m_pFittedMesh->property(verror,v);
			num++;
			if(m_pFittedMesh->property(verror,v)>maxerr.error)
			{
				maxerr.error=m_pFittedMesh->property(verror,v);
				maxerr.index=v.idx();
			}
		}
		meanerr/=num;
		double cur1=0.3*meanerr+0.7*maxerr.error;
		cout<<"第"<<i+1<<"水平条形的平均误差:"<<meanerr<<" "<<"最大误差:"<<maxerr.error<<"加权误差"<<cur1<<endl;
		errorvec.push_back(ErrorData(i,cur1));
		horizon_max_index.push_back(maxerr.index);
	}


	//排序，求解各区域相对于最小误差的百分比

	sort(errorvec.begin(),errorvec.end(),greater<ErrorData>());
	double minerror=(*(max_element(errorvec.begin(),errorvec.end(),greater<ErrorData>()))).error;

	for(int i=0;i<errorvec.size()-1;i++)
	{
		double per=(errorvec[i].error-minerror)/minerror*100;
		cout<<"第"<<errorvec[i].index<<"区域的相对误差为："<<per<<"%"<<endl;
	}
	int sum_=0;
	int i_j=0;
	for(int i=0;i<errorvec.size()-1&sum_<add_horizon_num;i++,i_j++)
	{
#if 0
		
		if (abs(vknots[errorvec[i].index-unum+2]-vknots[errorvec[i].index-unum+1])<=EPSILON2)//如果节点线之间过于狭窄，不增加节点线
		{
			knotvec.push_back(KnotData(errorvec[i].index,0));
			continue;
		}

		
#endif
		double per=(errorvec[i].error-minerror)/minerror*100;
		if (errorvec[i].error>=1e8)//异常值
		{
			knotvec.push_back(KnotData(errorvec[i].index,0));
			continue;
		}
		//方案一，只增加一条

		int n=(add_horizon_num-sum_>=1?1:0);
		knotvec.push_back(KnotData(errorvec[i].index,n));
		sum_+=n;

		//方案二
#if 0
		if (per<100)
		{
			int n=(add_horizon_num-sum_>=1?1:0);
			knotvec.push_back(KnotData(errorvec[i].index,n));
			sum_+=n;
		}
		else 
		{
			int n=(add_horizon_num-sum_>=2?2:(add_horizon_num-sum_==1?1:0));
			knotvec.push_back(KnotData(errorvec[i].index,n));
			sum_+=n;
		}

#endif	

	}
	if (i_j<errorvec.size()-1&&sum_==add_horizon_num)
	{
		for(int i=i_j;i<errorvec.size()-1;i++)
			knotvec.push_back(KnotData(errorvec[i].index,0));
	}
	//设置各条形应该增加的节点线个数
	knotvec.push_back(KnotData(errorvec[errorvec.size()-1].index,0));//表明最小的误差区域暂时不放节点线
	int num_sum=sum_;
	cout<<"本次推荐增加"<<num_sum<<"条节点线"<<endl;
	while(num_sum<add_horizon_num)
	{
		for (int i=0;i<knotvec.size()&&num_sum<add_horizon_num;i++)
		{
			if (errorvec[i].error>=90000)//异常值
			{
				continue;
			}
#if 0
			
		if (abs(vknots[errorvec[i].index-unum+2]-vknots[errorvec[i].index-unum+1])<=EPSILON2)//如果节点线之间过于狭窄，不增加节点线
		{
			continue;
		}

			
#endif
			knotvec[i].num++;
			num_sum++;
		}
	}
	cout<<"本次实际增加"<<add_horizon_num<<"条节点线"<<endl;
	sort(knotvec.begin(),knotvec.end(),less<KnotData>());
	//增加节点线
	vknots_new.clear();//在使用之前先清零
	for (int j=0;j<vnum-1;j++)
	{
		assert(j==knotvec[j].index);
		if (knotvec[j].num==1)
		{
			double y1=0;
			double  err_sum=0;
			for (vector<int>::iterator p=horizon_index[j].begin();p!=horizon_index[j].end();++p)
			{
				Mesh::VertexHandle v=m_pFittedMesh->vertex_handle(*p);
				y1+=m_pFittedMesh->property(verror,v)*m_pFittedMesh->texcoord2D(v)[1];
				err_sum+=m_pFittedMesh->property(verror,v);
			}
			y1/=err_sum;
			if (abs(y1-vknots[j])<=EPSILON2||abs(y1-vknots[1+j])<=EPSILON2)//如果点在节点线上，就直接平均分配
			{
				y1=(vknots[j]+vknots[j+1])/2;

			}
			vknots_new.push_back(knot_info(0,y1));
			
		}
		else if (knotvec[j].num>1)
		{//如果增加的节点线个数大于2，则先布置第一条节点线为加权节点线
			double y1=0;
			double  err_sum=0;
			for (vector<int>::iterator p=horizon_index[j].begin();p!=horizon_index[j].end();++p)
			{
				Mesh::VertexHandle v=m_pFittedMesh->vertex_handle(*p);
				y1+=m_pFittedMesh->property(verror,v)*m_pFittedMesh->texcoord2D(v)[1];
				err_sum+=m_pFittedMesh->property(verror,v);
			}
			y1/=err_sum;
			if (abs(y1-vknots[j])<=EPSILON2||abs(y1-vknots[j+1])<=EPSILON2)//如果点在节点线上，就直接平均分配
			{
				double dy=(vknots[1+j]-vknots[j])/(knotvec[j].num+1);
				int vmin_index=j;
				for (int i=0;i<knotvec[j].num;i++)
				{
					double y2=vknots[vmin_index]+dy*(i+1);
					
					vknots_new.push_back(knot_info(0,y2));
					
				} 
				continue;
			}
			
			vknots_new.push_back(knot_info(0,y1));
		
			//再处理其它的节点线
			TexCoord QP=m_pFittedMesh->texcoord2D(m_pFittedMesh->vertex_handle(horizon_max_index[j]));
			if (QP[1]<y1)//其余的节点线放在离最大误差节点线近的地方，平均分配
			{

				double dy=(y1-vknots[j])/(knotvec[j].num);
				int vmin_index2=j;
				for (int i=0;i<knotvec[j].num-1;i++)
				{
					double y2=(vknots[vmin_index2])+dy*(i+1);
		
					vknots_new.push_back(knot_info(0,y2));
				
				} 
			}
			else
			{
				double dy=(vknots[1+j]-y1)/(knotvec[j].num);
				for (int i=0;i<knotvec[j].num-1;i++)
				{
					double y2=y1+dy*(i+1);
				
					vknots_new.push_back(knot_info(0,y2));
					
				} 

			}

		}

	}
//计时结束
	QueryPerformanceCounter(&time_over); 
	
	cout<<"增加水平节点线耗时:"<<((time_over.QuadPart-time_start.QuadPart)/dqFreq)<<"s"<<endl;//单位为秒，精度为1000 000/（cpu主频）微秒

}
void CSurfaceData::adjust_vertical_knots()
{
	if(!error_compute_ok)
		return;
	//计时开始
	_LARGE_INTEGER time_start;    /*开始时间*/
	_LARGE_INTEGER time_over;        /*结束时间*/
	double dqFreq;                /*计时器频率*/
	LARGE_INTEGER f;            /*计时器频率*/
	QueryPerformanceFrequency(&f);
	dqFreq=(double)f.QuadPart;
	QueryPerformanceCounter(&time_start);
	if(add_num==0)
	{
		uknots_new.clear();//在使用之前先清零
		rangequery_horizon=false;
		rangequery_vertical=false;
		return;
	}
	
	if(!rangequery_horizon)
	{
		range_query();
		add_horizon_num==0?rangequery_vertical=false:rangequery_vertical=true;
		double  maxerr=max_err.error;
		double  cur1=0.3*mean_err+0.7*maxerr;
		cout<<"整体平均误差:"<<mean_err<<" "<<"最大误差:"<<maxerr<<"加权误差"<<cur1<<endl;
	}
	else
	{
		rangequery_horizon=false;

	}
	vector<ErrorData> errorvec;
	vector<KnotData>  knotvec;
	OpenMesh::VPropHandleT<double>  verror; 
	m_pFittedMesh->get_property_handle(verror,"verror");
	vector<int> vertical_max_index;
	for (int i=0;i<unum-1;i++)
	{
		double  meanerr=0;
		int num=0;
		Max_Error maxerr={0,0};
		if (vertical_index[i].size()==0)
		{
			cout<<"第"<<i+1<<"竖直条形由于没有点包含，所以在此条形中不增加节点线"<<endl;
			errorvec.push_back(ErrorData(i,2e10));
			vertical_max_index.push_back(-2);
			continue;
		}
		for (vector<int>::iterator p=vertical_index[i].begin();p!=vertical_index[i].end();++p)
		{
			Mesh::VertexHandle v=m_pFittedMesh->vertex_handle(*p);
			meanerr+=m_pFittedMesh->property(verror,v);
			num++;
			if(m_pFittedMesh->property(verror,v)>maxerr.error)
			{
				maxerr.error=m_pFittedMesh->property(verror,v);
				maxerr.index=v.idx();
			}
		}
		meanerr/=num;
		double cur1=0.3*meanerr+0.7*maxerr.error;
		cout<<"第"<<i+1<<"条竖直条形的平均误差:"<<meanerr<<" "<<"最大误差:"<<maxerr.error<<"加权误差"<<cur1<<endl;
		errorvec.push_back(ErrorData(i,cur1));
		vertical_max_index.push_back(maxerr.index);
	}



	//排序，求解各区域相对于最小误差的百分比

	sort(errorvec.begin(),errorvec.end(),greater<ErrorData>());
	double minerror=(*(max_element(errorvec.begin(),errorvec.end(),greater<ErrorData>()))).error;

	for(int i=0;i<errorvec.size()-1;i++)
	{
		double per=(errorvec[i].error-minerror)/minerror*100;
		cout<<"第"<<errorvec[i].index<<"区域的相对误差为："<<per<<"%"<<endl;
	}
	int sum_=0;
	int i_j=0;
	for(int i=0;i<errorvec.size()-1&sum_<add_num;i++,i_j++)
	{
#if 0
		
			if (abs(uknots[errorvec[i].index+1]-uknots[errorvec[i].index])<=EPSILON2)//如果节点线之间过于狭窄，不增加节点线
			{
				knotvec.push_back(KnotData(errorvec[i].index,0));
				continue;
			}
		
	
#endif
		double per=(errorvec[i].error-minerror)/minerror*100;
		if (errorvec[i].error>=1e8)//异常值
		{
			knotvec.push_back(KnotData(errorvec[i].index,0));
			continue;
		}
		//方案一，只增加一条

		int n=(add_num-sum_>=1?1:0);
		knotvec.push_back(KnotData(errorvec[i].index,n));
		sum_+=n;

		//方案二
#if 0
		if (per<100)
		{
			int n=(add_num-sum_>=1?1:0);
			knotvec.push_back(KnotData(errorvec[i].index,n));
			sum_+=n;
		}
		else 
		{
			int n=(add_num-sum_>=2?2:(add_num-sum_==1?1:0));
			knotvec.push_back(KnotData(errorvec[i].index,n));
			sum_+=n;
		}
#endif


	}
	if (i_j<errorvec.size()-1&&sum_==add_num)
	{
		for(int i=i_j;i<errorvec.size()-1;i++)
			knotvec.push_back(KnotData(errorvec[i].index,0));
	}
	//设置各条形应该增加的节点线个数
	knotvec.push_back(KnotData(errorvec[errorvec.size()-1].index,0));//表明最小的误差区域暂时不放节点线
	int num_sum=sum_;
	cout<<"本次推荐增加"<<num_sum<<"条节点线"<<endl;
	while(num_sum<add_num)
	{
		for (int i=0;i<knotvec.size()&&num_sum<add_num;i++)
		{
			if (errorvec[i].error>=90000)//异常值
			{
				continue;
			}
#if 0
		if (abs(uknots[errorvec[i].index+1]-uknots[errorvec[i].index])<=EPSILON2)//如果节点线之间过于狭窄，不增加节点线
		{
			continue;
		}
			
#endif
			knotvec[i].num++;
			num_sum++;
		}
	}
	cout<<"本次实际增加"<<add_num<<"条节点线"<<endl;
	sort(knotvec.begin(),knotvec.end(),less<KnotData>());
	//增加节点线
	uknots_new.clear();//在使用之前先清零
	for (int i=0;i<unum-1;i++)
	{
		assert(i==knotvec[i].index);
		if (knotvec[i].num==1)
		{
			double x1=0;
			double  err_sum=0;
			for (vector<int>::iterator p=vertical_index[i].begin();p!=vertical_index[i].end();++p)
			{
				Mesh::VertexHandle v=m_pFittedMesh->vertex_handle(*p);
				x1+=m_pFittedMesh->property(verror,v)*m_pFittedMesh->texcoord2D(v)[0];
				err_sum+=m_pFittedMesh->property(verror,v);
			}
			x1/=err_sum;
			if (abs(x1-uknots[i])<=EPSILON2||abs(x1-uknots[1+i])<=EPSILON2)//如果点在节点线上，就直接平均分配
			{
				x1=(uknots[i]+uknots[i+1])/2;

			}
			uknots_new.push_back(knot_info(0,x1));
		}
		else if (knotvec[i].num>1)
		{//如果增加的节点线个数大于2，则先布置第一条节点线为加权节点线
			double x1=0;
			double  err_sum=0;
			for (vector<int>::iterator p=vertical_index[i].begin();p!=vertical_index[i].end();++p)
			{
				Mesh::VertexHandle v=m_pFittedMesh->vertex_handle(*p);
				x1+=m_pFittedMesh->property(verror,v)*m_pFittedMesh->texcoord2D(v)[0];
				err_sum+=m_pFittedMesh->property(verror,v);
			}
			x1/=err_sum;
			if (abs(x1-uknots[i])<=EPSILON2||abs(x1-uknots[i+1])<=EPSILON2)//如果点在节点线上，就直接平均分配
			{
				double dx=(uknots[1+i]-uknots[i])/(knotvec[i].num+1);
				int umin_index=i;
				for (int j=0;j<knotvec[i].num;j++)
				{
					double x2=uknots[umin_index]+dx*(j+1);
					uknots_new.push_back(knot_info(0,x2));
				
				} 
				continue;
			}
		uknots_new.push_back(knot_info(0,x1));
		
			//再处理其它的节点线
			TexCoord QP=m_pFittedMesh->texcoord2D(m_pFittedMesh->vertex_handle(vertical_max_index[i]));
			if (QP[0]<x1)//其余的节点线放在离最大误差节点线近的地方，平均分配
			{

				double dx=(x1-uknots[i])/(knotvec[i].num);
				int umin_index2=i;
				for (int j=0;j<knotvec[i].num-1;j++)
				{
					double x2=uknots[umin_index2]+dx*(j+1);
					uknots_new.push_back(knot_info(0,x2));
				
				} 
			}
			else
			{
				double dx=(uknots[1+i]-x1)/(knotvec[i].num);
				for (int j=0;j<knotvec[i].num-1;j++)
				{
					double x2=x1+dx*(j+1);
					uknots_new.push_back(knot_info(0,x2));
				} 

			}

		}


	}
//计时结束
	QueryPerformanceCounter(&time_over); 
	cout<<"增加竖直节点线耗时:"<<((time_over.QuadPart-time_start.QuadPart)/dqFreq)<<"s"<<endl;//单位为秒，精度为1000 000/（cpu主频）微秒

}

B_parameter* CSurfaceData::getparameter()
{
	return parameter1;
}

void CSurfaceData::clear_data()
{
	if(m_pOriginalMesh!=NULL)
	{
		delete m_pOriginalMesh;
		m_pOriginalMesh = NULL;
	}
	if(m_pFittedMesh!=NULL)
	{
		delete m_pFittedMesh;
		m_pFittedMesh = NULL;
	}
	if(m_pErrorMesh!=NULL)
	{
		delete m_pErrorMesh;
		m_pErrorMesh = NULL;
	}
    if (parameter1!=NULL)
    {
        delete parameter1;
		parameter1=NULL;
    }
	if(bsurface!=NULL)
	{
		delete bsurface;
		bsurface=NULL;
	}
	fileName.clear();
	err_threshold = 1;
	iter_num = 0;

}

bool CSurfaceData::read_mesh(const QString &name)
{
	fileName = name;
	if(m_pOriginalMesh == NULL)
	{
		m_pOriginalMesh = new(std::nothrow) Mesh;
	}
    m_pOriginalMesh->request_vertex_texcoords2D();
	m_pOriginalMesh->request_face_normals();
	m_pOriginalMesh->request_vertex_normals();
	OpenMesh::IO::Options opt;
	opt+=OpenMesh::IO::Options::VertexTexCoord;
	opt+=OpenMesh::IO::Options::VertexNormal;
	// Returns a substring that contains the n rightmost characters of the string.
	QString extension = fileName.right(4);
	if(extension == ".off")
	{
		if(!OpenMesh::IO::read_mesh(*m_pOriginalMesh, fileName.toStdString(),opt))
		{
			QMessageBox::critical(NULL, "Failed to open", "Unable to open file", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
			return false;
		}
		
	}
	else if(extension == ".obj")
	{
       
		if(!OpenMesh::IO::read_mesh(*m_pOriginalMesh, fileName.toStdString(),opt))
		{
			QMessageBox::critical(NULL, "Failed to open", "Unable to open file", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
			return false;
		}
		
		
	}
	else if(extension == ".stl")
	{
		if(!OpenMesh::IO::read_mesh(*m_pOriginalMesh, fileName.toStdString(),opt))
		{
			QMessageBox::critical(NULL, "Failed to open", "Unable to open file", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
			return false;
		}
		
	}
	else
	{
		QMessageBox::critical(NULL, "Unknown extension", "Unknown extension", QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
		return false;
	}
	
	if(opt.check( OpenMesh::IO::Options::VertexTexCoord))
	{
		int sz = m_pOriginalMesh->n_vertices();
		domain.reserve(sz);
		domain.clear();
		Mesh::VertexIter       v_it, v_end(m_pOriginalMesh->vertices_end());

		for (v_it=m_pOriginalMesh->vertices_begin(); v_it!=v_end; ++v_it)
		{
			domain.push_back(m_pOriginalMesh->texcoord2D(*v_it));
			
		}
	}

	if ( !opt.check( OpenMesh::IO::Options::VertexNormal ) &&
		m_pOriginalMesh->has_face_normals() && m_pOriginalMesh->has_vertex_normals() )
	{
		// let the mesh update the normals
		m_pOriginalMesh->update_normals();
	}
	sample_num = m_pOriginalMesh->n_vertices();
	return true;
}


bool CSurfaceData::mesh_parameterization()
{
	if(domain.empty())
	{
		return false;//如果空的话，需要根据原始网格计算domain，这里先不计算domain,所以我们的网格一定要带有参数化坐标
	}
	compute_box();

	return true;
}


vector<TexCoord> CSurfaceData::get_domain()
{
	return domain;
}


int CSurfaceData::get_mesh_vertex_num()
{
	if(m_pOriginalMesh!=NULL)
	{
		return m_pOriginalMesh->n_vertices();
	}
	return -1;
}

int CSurfaceData::get_mesh_face_num()
{
	if(m_pOriginalMesh!=NULL)
	{
		return m_pOriginalMesh->n_faces();
	}
	return -1;
}

QString CSurfaceData::get_mesh_name()
{
	return fileName;
}

Mesh *CSurfaceData::get_original_mesh()
{
	return m_pOriginalMesh;
}

Mesh *CSurfaceData::get_fitted_mesh()
{
	return m_pFittedMesh;
}



bool CSurfaceData::mesh_fitting()
{
	if (m_pOriginalMesh==NULL||parameter1==NULL||!parameter1->configure_knots())
	return false;
    bsurface=new(std::nothrow) CBSplineSurfaceView;
	bsurface->setBparameter(parameter1);
	 bool p=bsurface->fitting_bspline(m_pOriginalMesh);
	 m_pFittedMesh=NULL;
	 if (p==true)
	 {
		 m_pFittedMesh=bsurface->getFittingMesh();
	 }
    
	return p;
}
CBSplineSurfaceView* CSurfaceData::getbspline()
{
	return bsurface;
}

bool CSurfaceData::compute_curvature(QStringList files)
{
	OpenMesh::VPropHandleT<double> v_mean_curvature;
	m_pOriginalMesh->add_property(v_mean_curvature,"v_mean_curvature");
	m_pOriginalMesh->property(v_mean_curvature).set_persistent(true);

	OpenMesh::VPropHandleT<double> v_k1_curvature;
	m_pOriginalMesh->add_property(v_k1_curvature,"v_k1_curvature");
	m_pOriginalMesh->property(v_k1_curvature).set_persistent(true);

	OpenMesh::VPropHandleT<double> v_k2_curvature;
	m_pOriginalMesh->add_property(v_k2_curvature,"v_k2_curvature");
	m_pOriginalMesh->property(v_k2_curvature).set_persistent(true);

	OpenMesh::FPropHandleT<double> f_mean_curvature;
	m_pOriginalMesh->add_property(f_mean_curvature,"f_mean_curvature");
	m_pOriginalMesh->property(f_mean_curvature).set_persistent(true);

	if (files.size()!=2)
		return false;	

	QFile file1(files.takeFirst()); //takeFirst去掉第一项并返回被去掉的项
	if (!file1.open(QFile::ReadOnly | QFile::Text))
	return false;
	QFile file2(files.takeFirst()); 
	if (!file2.open(QFile::ReadOnly | QFile::Text))
	return false;

	
	QTextStream  in1(&file1); 
	QTextStream  in2(&file2);
	QString line1 = in1.readLine();
    QString line2 = in2.readLine();    
	Mesh::VertexIter v_it = m_pOriginalMesh->vertices_begin();
	while (!line1.isEmpty()&&!line2.isEmpty()) 
	{
		double k1= line1.toDouble();
		double k2= line2.toDouble();
		double k=(abs(k1)+abs(k2))/2;
		m_pOriginalMesh->property(v_k1_curvature,*v_it)=k1;
        m_pOriginalMesh->property(v_k2_curvature,*v_it)=k2; 
        m_pOriginalMesh->property(v_mean_curvature,*v_it)=k;
		line1 = in1.readLine();
		line2 = in2.readLine();
		v_it++;
		
	}
	file1.close();
	file2.close();

	max_meancurvature = -1e100;
	min_meancurvature = 1e100;

	Mesh::VertexIter  vit = m_pOriginalMesh->vertices_begin();
	for(; vit!=m_pOriginalMesh->vertices_end(); ++vit)
	{
		double cur=m_pOriginalMesh->property(v_mean_curvature,*vit);
		if(max_meancurvature<cur)
			max_meancurvature = cur;
		if(min_meancurvature>cur)
			min_meancurvature =cur;
	}
	Mesh::FaceIter fit = m_pOriginalMesh->faces_begin();
	Mesh::FaceVertexIter    fv_it;
	for(; fit!=m_pOriginalMesh->faces_end(); ++fit)
	{ 
		fv_it=m_pOriginalMesh->fv_iter( *fit );
		m_pOriginalMesh->property(f_mean_curvature,*fit)=m_pOriginalMesh->property(v_mean_curvature,*fv_it);
		++fv_it;
		m_pOriginalMesh->property(f_mean_curvature,*fit)+=m_pOriginalMesh->property(v_mean_curvature,*fv_it);
		++fv_it;
		m_pOriginalMesh->property(f_mean_curvature,*fit)+=m_pOriginalMesh->property(v_mean_curvature,*fv_it);
		m_pOriginalMesh->property(f_mean_curvature,*fit)/=3;

	}
    return true;
}
bool CSurfaceData::curvature_loading()
{
	return Curvature_loading;
}
void CSurfaceData::set_curvature_loadingstate(bool state)
{
     Curvature_loading=state;
}
void CSurfaceData::set_fitting_completed(bool state)
{
	fitting_completed_=state;
}
bool CSurfaceData::fitting_completed()
{
	return fitting_completed_;

}
bool CSurfaceData::curvature_error_completed()
{
	return curvature_error;
}

void CSurfaceData::compute_box()
{
	Mesh::TexCoord2D   bbMin, bbMax;

	bbMin = bbMax = domain[0];
	for (int i=1;i!=sample_num; ++i)
	{
		bbMin.minimize(domain[i]);
		bbMax.maximize(domain[i]);
	}
	Bbox_2 box_(bbMin[0],bbMin[1],bbMax[0],bbMax[1]);
    box=box_;
}

void CSurfaceData::compute_error()
{
	max_err.error = 0;
	max_err.index = -1;
	mean_err = 0;
    
	double max_length = 0;
	max_length = max(originalbox.xmax()-originalbox.xmin(), originalbox.ymax()-originalbox.ymin());
	max_length = max(max_length, originalbox.zmax()-originalbox.zmin());

	Mesh::VertexIter  v_it1 = m_pOriginalMesh->vertices_begin(), v_end1 = m_pOriginalMesh->vertices_end();
	Mesh::VertexIter  v_it2 = m_pFittedMesh->vertices_begin(), v_end2 = m_pFittedMesh->vertices_end();

	OpenMesh::VPropHandleT<double> verror;
	m_pFittedMesh->add_property(verror,"verror");
	
	for (; v_it1!=v_end1 && v_it2!=v_end2; ++v_it1, ++v_it2)
	{
		Mesh::Point p1 = Mesh::Point(m_pOriginalMesh->point(*v_it1)[0],m_pOriginalMesh->point(*v_it1)[1],m_pOriginalMesh->point(*v_it1)[2]);
		Mesh::Point p2 = Mesh::Point(m_pFittedMesh->point(*v_it2)[0],m_pFittedMesh->point(*v_it2)[1],m_pFittedMesh->point(*v_it2)[2]);
		Mesh::Point vec= p1 - p2;
		double error= vec.sqrnorm();
		mean_err += error;
		m_pFittedMesh->property(verror,*v_it2)=vec.norm()/max_length;//归一化处理，消除单位造成的误差
	}

	for( v_it2 = m_pFittedMesh->vertices_begin();v_it2!=v_end2;++v_it2 )
	{
		if(max_err.error<m_pFittedMesh->property(verror,*v_it2))
		{
			max_err.error = m_pFittedMesh->property(verror,*v_it2);
			max_err.index = (*v_it2).idx();
		}
	}

	mean_err /= sample_num;
	mean_err = sqrt(mean_err);
	mean_e = mean_err;
	mean_err /= max_length;
	error_compute_ok=true;
	max_err_real=max_err.error;
	//设置每个拟合曲面的平均误差
	OpenMesh::FPropHandleT<double> f_mean_error;
	m_pFittedMesh->add_property(f_mean_error,"f_mean_error");
	m_pFittedMesh->property(f_mean_error).set_persistent(true);

	Mesh::FaceIter fit = m_pFittedMesh->faces_begin();
	Mesh::FaceVertexIter    fv_it;
	for(; fit!=m_pFittedMesh->faces_end(); ++fit)
	{ 
		fv_it=m_pFittedMesh->fv_iter( *fit );
		m_pFittedMesh->property(f_mean_error,*fit)=m_pFittedMesh->property(verror,*fv_it);
		++fv_it;
		m_pFittedMesh->property(f_mean_error,*fit)+=m_pFittedMesh->property(verror,*fv_it);
		++fv_it;
		m_pFittedMesh->property(f_mean_error,*fit)+=m_pFittedMesh->property(verror,*fv_it);
		m_pFittedMesh->property(f_mean_error,*fit)/=3;

	}
}

double &CSurfaceData::get_mean_error()
{
	return mean_err;
}

Max_Error &CSurfaceData::get_max_error()
{
	return max_err;
}
double CSurfaceData::get_max_knotcurerror()
{
	return max_curerror;
}
double CSurfaceData::get_min_knotcurerror()
{
	return min_curerror;
}
void    CSurfaceData::set_max_curvature(double curvature)
{
	max_meancurvature=curvature;
}


Bbox_2 &CSurfaceData::get_box()
{
	return box;
}
void CSurfaceData::set_original_box(Bbox_3 box)
{
	originalbox=box;
}
void CSurfaceData::set_fitting_box(Bbox_3 box)
{
	fittingbox=box;
}
Bbox_3 CSurfaceData::get_original_box()
{
	return originalbox;
}
Bbox_3 CSurfaceData::get_fitting_box()
{
	return fittingbox;
}
Bbox_3 CSurfaceData::get_error_box()
{
	return errorbox;
}

void CSurfaceData::update_curvature_color()
{
   
   OpenMesh::VPropHandleT<double> mean_curvature;
    m_pOriginalMesh->get_property_handle(mean_curvature,"v_mean_curvature");  

	double vcolor[3];
	int h1, s1, v1;
	int h2, s2, v2;
	{
		int color[3];
		color[0] = 250;
		color[1] = 0;
		color[2] = 0;
		QColor d_light(color[0], color[1], color[2]);

		color[0] = 0;
		color[1] = 0;
		color[2] =255;
		QColor d_dark(color[0], color[1], color[2]);
		d_light.getHsv( &h1, &s1, &v1 );
		d_dark.getHsv( &h2, &s2, &v2 );
	}
		

	m_pOriginalMesh->request_vertex_colors(); 
	if (!m_pOriginalMesh->has_vertex_colors())
		return;
	Mesh::VertexIter vit = m_pOriginalMesh->vertices_begin();
	for(; vit!=m_pOriginalMesh->vertices_end(); ++vit)
	{
		double ratio = (m_pOriginalMesh->property(mean_curvature,*vit)-min_meancurvature)/(max_meancurvature-min_meancurvature);
		ratio = ratio>1?1:ratio;
		QColor c;
		/*c.setHsv( h1 + qRound( ratio * ( h2 - h1 ) ),
        s1 + qRound( ratio * ( s2 - s1 ) ),
        v1 + qRound( ratio * ( v2 - v1 ) ) );*/

		c.setHsv( h2 + qRound( ratio * ( h1 - h2 ) ),
            s2 + qRound( ratio * ( s1 - s2 ) ),
            v2 + qRound( ratio * ( v1 - v2 ) ) );

		int color[3];
		c.getRgb(&color[0], &color[1], &color[2]);
	m_pOriginalMesh->set_color(*vit,Mesh::Color(color[0],color[1],color[2]));
	}
	
}
void CSurfaceData::modification()
{
	//删除过密的节点线
	double  epsi=pow(sqrt(double(m_pOriginalMesh->n_vertices())),-1);
	viterator uiter=uknots.begin();
	advance(uiter,1);
	//u[1]确定
	while(*uiter<epsi&&uiter!=uknots.end())
	{
		uiter=uknots.erase(uiter);
	}
	viterator  upre=uiter;
	viterator  ulast=upre+1;
	while(upre!=uknots.end()-1&&ulast!=uknots.end()-1)//upre!=uknots.end()-1加上这句是为了防止u区间只剩下0与1节点线的情况。
	{
		while(*ulast-*upre<epsi &&ulast!=uknots.end()-1)
		{
			double utemp=(*upre+*ulast)/2;
			upre=uknots.erase(upre);
			upre=uknots.erase(upre);
			upre=uknots.insert(upre,utemp);
			ulast=upre+1;
		}
		if (ulast!=uknots.end()-1)
		{
			upre++;
			ulast=upre+1;
		}

	}

	if(upre!=uknots.end()-1)
	{ 
		assert(ulast==uknots.end()-1);
		//处理倒数第二个元素
		if(*ulast-*upre<epsi)
		{
			uknots.erase(upre);
		}
	}

	//处理vknots

	viterator viter=vknots.begin();
	advance(viter,1);
	//u[1]确定
	while(*viter<epsi &&viter!=vknots.end())
	{
		viter=vknots.erase(viter);
	}
	viterator  vpre=viter;
	viterator  vlast=vpre+1;
	while(vpre!=vknots.end()-1&&vlast!=vknots.end()-1)//vpre!=vknots.end()-1加上这句是为了防止u区间只剩下0与1节点线的情况。
	{
		while(*vlast-*vpre<epsi &&vlast!=vknots.end()-1)
		{
			double vtemp=(*vpre+*vlast)/2;
			vpre=vknots.erase(vpre);
			vpre=vknots.erase(vpre);
			vpre=vknots.insert(vpre,vtemp);
			vlast=vpre+1;
		}
		if (vlast!=vknots.end()-1)
		{
			vpre++;
			vlast=vpre+1;
		}

	}

	if(vpre!=vknots.end()-1)
	{ 
		assert(vlast==vknots.end()-1);
		//处理倒数第二个元素
		if(*vlast-*vpre<epsi)
		{
			vknots.erase(vpre);
		}
	}


	sort(uknots.begin(),uknots.end());
	sort(vknots.begin(),vknots.end());
}
bool   CSurfaceData::adjust_knots_by_curvature()
{
	add_horizon_num=1;
	add_num=1;

	if (m_pOriginalMesh==NULL||parameter1==NULL)
		return false;

	uknots=parameter1->getuknot();
	vknots=parameter1->getvknot();

	if (uknots.empty()||vknots.empty())
	{
		if (!parameter1->compute_knots())
		{
			return false;
		}
		 uknots=parameter1->getuknot();
		 vknots=parameter1->getvknot();
	 }

	/************************************************************************/
	/* bool OpenMesh::BaseKernel::get_property_handle(VPropHandleT< T > & _ph,const std::string & _name )	const 
	
	/* Retrieves(找回) the handle to a named property by it's name.

	/* Parameters:
	_ph	A property handle. On success the handle is valid else invalid.
	_name	Name of wanted property.
	Returns
	true if such a named property is available, else false.
	
	*/
	/************************************************************************/
	 OpenMesh::FPropHandleT<double> f2_mean_curvature;
	bool p_result=m_pOriginalMesh->get_property_handle(f2_mean_curvature,"f_mean_curvature"); //p_result指明了f2_mean_curvature是否有效


	 //计时开始
	 _LARGE_INTEGER time_start;    /*开始时间*/
	 _LARGE_INTEGER time_over;        /*结束时间*/
	 double dqFreq;                /*计时器频率*/
	 LARGE_INTEGER f;            /*计时器频率*/
	 QueryPerformanceFrequency(&f);
	 dqFreq=(double)f.QuadPart;
	 QueryPerformanceCounter(&time_start);

	
	//求解平均曲率积分总和
     double sum=0.0;
	  Mesh::FaceIter  f_it,f_end(m_pOriginalMesh->faces_end());
	  Mesh::FaceVertexIter fv_it;
	  for (f_it=m_pOriginalMesh->faces_begin();f_it!=f_end;++f_it)
	  {
		  double ts2=m_pOriginalMesh->property(f2_mean_curvature,*f_it);
		  TexCoord A,B,C;
		  fv_it=m_pOriginalMesh->fv_iter(*f_it);
		  A=m_pOriginalMesh->texcoord2D(*fv_it);
		  ++fv_it;
		  B=m_pOriginalMesh->texcoord2D(*fv_it);
		  ++fv_it;
		  C=m_pOriginalMesh->texcoord2D(*fv_it);
		  sum+=ts2*abs(CGAL::area(Point_2(A[0],A[1]),Point_2(B[0],B[1]),Point_2(C[0],C[1])));
	  }
	  double num3=(unum-1)*(vnum-1);
	  double  fa=sum/num3;
	  curaverage_=fa;
	  cout<<"平均曲率积分为:"<<curaverage_<<endl;

//-------------------------------------------------------------------- kd-tree

	  int					nPts;					// actual number of data points
	  ANNpointArray		dataPts;				// data points
	  int				maxPts= m_pOriginalMesh->n_vertices();			// maximum number of data points 				
	  dataPts = annAllocPts(maxPts, 2);			// allocate data points

	  nPts = 0;	
	  Mesh::VertexIter          v_it, v_end(m_pOriginalMesh->vertices_end());
	  for (v_it=m_pOriginalMesh->vertices_begin(); v_it!=v_end; ++v_it,nPts++)
	  {
		  dataPts[nPts][0]=m_pOriginalMesh->texcoord2D(*v_it)[0];
		  dataPts[nPts][1]=m_pOriginalMesh->texcoord2D(*v_it)[1];
		  assert(nPts==(*v_it).idx());
	  }
	  kdTree = new(std::nothrow) ANNkd_tree(					// build search structure
		  dataPts,					// the data points
		  nPts,						// number of points
		  2);						// dimension of space
//-------------------------------------------------------------------- kd-tree

	  //增加原始曲面的两个属性
	  OpenMesh::VPropHandleT<Location_Type> v_location; //添加点属性
	  m_pOriginalMesh->add_property(v_location,"v_location");

	  OpenMesh::FPropHandleT<Location_Type> f_location; //添加面属性
	  m_pOriginalMesh->add_property(f_location,"f_location");

	  for (v_it=m_pOriginalMesh->vertices_begin();v_it!=v_end;++v_it)
	  {
		  m_pOriginalMesh->property(v_location,*v_it)=unknown;

	  }
	  for (f_it=m_pOriginalMesh->faces_begin(); f_it!=f_end; ++f_it)
	  {
		  m_pOriginalMesh->property(f_location,*f_it)=unknown;
	  }

//-------------------------------------------------------------------- 迭代

	  int  i=0;
	  while(i<iter_times)
	  {
		  update_knots(i);
		  i++;
	  }
	  sort(uknots.begin(),uknots.end());
	  sort(vknots.begin(),vknots.end());
	  unum=uknots.size();
	  vnum=vknots.size();
	  modification();
	  unum=uknots.size();
	  vnum=vknots.size();
	  range_query2();
	  unum=uknots.size();
	  vnum=vknots.size();
	  sort(uknots.begin(),uknots.end());
	  sort(vknots.begin(),vknots.end());
	  parameter1->setnum(uknots.size(),vknots.size());
	  parameter1->setm(uknots.size()+parameter1->getp()-2);
	  parameter1->setn(vknots.size()+parameter1->getq()-2);
	  parameter1->setuknots(uknots);
	  parameter1->setvknots(vknots);
	  update_polymesh();
	  parameter1->iteration_ok=true;

	  ofstream  result("result.txt");
	  for (vector<double>::iterator p=error_sum.begin();p!=error_sum.end();++p)
	  {
		  result<<*p<<endl;
	  }
	  error_sum.clear();
   cout<<"初始的结果是"<<"p:"<<parameter1->getp()<<"q:"<<parameter1->getq()<<"m:"<<parameter1->getm()<<"n:"<<parameter1->getn()<<endl;
   result<<"初始的结果是"<<"p:"<<parameter1->getp()<<"q:"<<parameter1->getq()<<"m:"<<parameter1->getm()<<"n:"<<parameter1->getn()<<endl;
   //计时结束
   QueryPerformanceCounter(&time_over); 
   cout<<"依据曲率修订节点线耗时:"<<((time_over.QuadPart-time_start.QuadPart)/dqFreq)<<"s"<<endl;//单位为秒，精度为1000 000/（cpu主频）微秒
	return true;
}
bool CSurfaceData::adjust_knots_by_fitting_error()
{
	add_horizon_num=1;
	add_num=1;
	if (m_pFittedMesh==NULL||parameter1==NULL)
		return false;
	OpenMesh::FPropHandleT<double>  f_mean_error; 
	m_pFittedMesh->get_property_handle(f_mean_error,"f_mean_error");  
	//计时开始
	_LARGE_INTEGER time_start;    /*开始时间*/
	_LARGE_INTEGER time_over;        /*结束时间*/
	double dqFreq;                /*计时器频率*/
	LARGE_INTEGER f;            /*计时器频率*/
	QueryPerformanceFrequency(&f);
	dqFreq=(double)f.QuadPart;
	QueryPerformanceCounter(&time_start);
	
//配置节点
	vector<double>  temporary_uknot(uknots);//保存旧的节点
	vector<double>  temporary_vknot(vknots);
	for (int i=0;i<uknots_new.size();i++)
	{
	uknots.push_back(uknots_new[i].knot);
	}
	for (int i=0;i<vknots_new.size();i++)
	{
	vknots.push_back(vknots_new[i].knot);
	}
  sort(uknots.begin(),uknots.end());
 
 //查找新增节点在总的节点中的序号
	 typedef std::vector<double>::iterator vwiter;
	 typedef std::pair<vwiter,vwiter> VWIterPair;
	 for (int i=0;i<uknots_new.size();i++)
	 {
		 VWIterPair p=std::equal_range(uknots.begin(),uknots.end(),uknots_new[i].knot);
		 assert(p.first!=p.second);
		 int index1=distance(uknots.begin(),p.first);
		 uknots_new[i].index=index1;
	 }
	sort(vknots.begin(),vknots.end());
	//查找新增节点在总的节点中的序号
	for (int i=0;i<vknots_new.size();i++)
	{
		VWIterPair p=std::equal_range(vknots.begin(),vknots.end(),vknots_new[i].knot);
		assert(p.first!=p.second);
		int index1=distance(vknots.begin(),p.first);
		vknots_new[i].index=index1;
	}
	vnum=vknots.size();
	parameter1->setnum(uknots.size(),vknots.size());
	parameter1->setn(vknots.size()+parameter1->getq()-2);
	parameter1->setvknots(vknots);
	unum=uknots.size();
	parameter1->setm(uknots.size()+parameter1->getp()-2);
	parameter1->setuknots(uknots);
	ofstream  result("result2.txt");
	if (iter_times!=-1)//一旦迭代次数取-1，表明不需要通过积分法求解新节点
	{
	//求解平均误差积分总和
	double sum=0.0;
	Mesh::FaceIter  f_it,f_end(m_pFittedMesh->faces_end());
	Mesh::FaceVertexIter fv_it;
	for (f_it=m_pFittedMesh->faces_begin();f_it!=f_end;++f_it)
	{
		double ts2=m_pFittedMesh->property(f_mean_error,*f_it);
		TexCoord A,B,C;
		fv_it=m_pFittedMesh->fv_iter(*f_it);
		A=m_pFittedMesh->texcoord2D(*fv_it);
		++fv_it;
		B=m_pFittedMesh->texcoord2D(*fv_it);
		++fv_it;
		C=m_pFittedMesh->texcoord2D(*fv_it);
		sum+=ts2*abs(CGAL::area(Point_2(A[0],A[1]),Point_2(B[0],B[1]),Point_2(C[0],C[1])));
	}
	double num3=(unum-1)*(vnum-1);
	double  fa=sum/num3;
	erroraverage_=fa;
	cout<<"平均误差积分为:"<<erroraverage_<<endl;
	//增加原始曲面的两个属性
	OpenMesh::VPropHandleT<Location_Type> v_location; //添加点属性
	m_pFittedMesh->add_property(v_location,"v_location_fit");

	OpenMesh::FPropHandleT<Location_Type> f_location; //添加面属性
	m_pFittedMesh->add_property(f_location,"f_location_fit");

	 Mesh::VertexIter     v_it,v_end(m_pFittedMesh->vertices_end());
	for (v_it=m_pFittedMesh->vertices_begin();v_it!=v_end;++v_it)
	{
		m_pFittedMesh->property(v_location,*v_it)=unknown;

	}
	for (f_it=m_pFittedMesh->faces_begin(); f_it!=f_end; ++f_it)
	{
		m_pFittedMesh->property(f_location,*f_it)=unknown;
	}
	//迭代
	int  i=0;
	while(i<iter_times)
	{
		update_knots2(i);
		i++;
	}
	sort(uknots.begin(),uknots.end());
	sort(vknots.begin(),vknots.end());
//更新新增加的节点位置,保持旧的节点位置

	for (int i=0;i<uknots_new.size();i++)
	{
		int index=uknots_new[i].index;
		uknots_new[i].knot=uknots[index]; 
		
	}
	for (int i=0;i<vknots_new.size();i++)
	{
		int index=vknots_new[i].index;
		vknots_new[i].knot=vknots[index];
	}

	//如果新加的节点线与原来的节点线重合，则去掉
	vector<knot_info> ni;//标记应该删除的节点
	for (int i=0;i<uknots_new.size();i++)
	{
		for (int j=0;j<temporary_uknot.size();j++)
		{
			if (abs(uknots_new[i].knot-temporary_uknot[j])<=0.01)
			{
				ni.push_back(uknots_new[i]);
				continue;
			}
		}
	}
	for (int i=0;i<ni.size();i++)
	{
		uknots_new.erase(remove(uknots_new.begin(),uknots_new.end(),ni[i]),uknots_new.end());
	}
	ni.clear();
	for (int i=0;i<vknots_new.size();i++)
	{
		for (int j=0;j<temporary_vknot.size();j++)
		{
			if (abs(vknots_new[i].knot-temporary_vknot[j])<=0.01)
			{
				ni.push_back(vknots_new[i]);
				continue;
			}
		}
	}
	for (int i=0;i<ni.size();i++)
	{
		vknots_new.erase(remove(vknots_new.begin(),vknots_new.end(),ni[i]),vknots_new.end());
	}
	for (int i=0;i<uknots_new.size();i++)
	{
		temporary_uknot.push_back(uknots_new[i].knot);
	}
	for (int i=0;i<vknots_new.size();i++)
	{
		temporary_vknot.push_back(vknots_new[i].knot);
	}
	std::sort(temporary_uknot.begin(),temporary_uknot.end());
	std::sort(temporary_vknot.begin(),temporary_vknot.end());
    uknots.assign(temporary_uknot.begin(),temporary_uknot.end());
	vknots.assign(temporary_vknot.begin(),temporary_vknot.end());

	
	for (vector<double>::iterator p=error_sum.begin();p!=error_sum.end();++p)
	{
		result<<*p<<endl;
	}
	unum=uknots.size();
	vnum=vknots.size();
	parameter1->setnum(uknots.size(),vknots.size());
	parameter1->setm(uknots.size()+parameter1->getp()-2);
	parameter1->setn(vknots.size()+parameter1->getq()-2);
	parameter1->setuknots(uknots);
	parameter1->setvknots(vknots);
	}
	range_query();//删选不合适的节点线
	sort(uknots.begin(),uknots.end());
	sort(vknots.begin(),vknots.end());
	unum=uknots.size();
	vnum=vknots.size();
	parameter1->setnum(uknots.size(),vknots.size());
	parameter1->setm(uknots.size()+parameter1->getp()-2);
	parameter1->setn(vknots.size()+parameter1->getq()-2);
	parameter1->setuknots(uknots);
	parameter1->setvknots(vknots);
	parameter1->iteration_ok=true;
	error_sum.clear();
	cout<<"最终的结果是"<<"p:"<<parameter1->getp()<<"q:"<<parameter1->getq()<<"m:"<<parameter1->getm()<<"n:"<<parameter1->getn()<<endl;
	result<<"最终的结果是"<<"p:"<<parameter1->getp()<<"q:"<<parameter1->getq()<<"m:"<<parameter1->getm()<<"n:"<<parameter1->getn()<<endl;
//计时结束
	QueryPerformanceCounter(&time_over); 
	cout<<"依据误差修订节点线耗时:"<<((time_over.QuadPart-time_start.QuadPart)/dqFreq)<<"s"<<endl;//单位为秒，精度为1000 000/（cpu主频）微秒
	return true;

}

void CSurfaceData::update_polymesh()
{
	polymesh.clear();
	// generate vertices
	vector<vector<MyMesh::VertexHandle> > vhandle;
	vhandle.resize(vnum,vector<MyMesh::VertexHandle>(unum));
	for(int j=0;j<vnum;j++)
		for (int i=0;i<unum;i++)
		{
			vhandle[j][i]=polymesh.add_vertex(MyMesh::Point(uknots[i],vknots[j],0.0));
		}

		// generate (quadrilateral) faces
		std::vector<MyMesh::VertexHandle>  face_vhandles;
		for(int j=0;j<vnum-1;j++)
			for (int i=0;i<unum-1;i++)
			{
				face_vhandles.clear();
				face_vhandles.push_back(vhandle[j][i]);
				face_vhandles.push_back(vhandle[j][i+1]);
				face_vhandles.push_back(vhandle[j+1][i+1]);
				face_vhandles.push_back(vhandle[j+1][i]);
				polymesh.add_face(face_vhandles);

			}
			//至此矩形面建立完毕
			polymesh.request_face_colors(); 
			if (!polymesh.has_face_colors())
				return;
			Mesh::FaceIter  f_it,f_end(m_pOriginalMesh->faces_end());
			Mesh::FaceVertexIter fv_it2;

			OpenMesh::FPropHandleT<double> f_area;  //增加每个面的曲率面积属性
			polymesh.add_property(f_area,"f_area");

			OpenMesh::FPropHandleT<double> f_error;  //增加每个面的曲率误差属性
			polymesh.add_property(f_error,"f_error");

			OpenMesh::VPropHandleT<Location_Type> v_location; //添加点属性
			m_pOriginalMesh->get_property_handle(v_location,"v_location");

			OpenMesh::FPropHandleT<Location_Type> f_location; //添加面属性
			m_pOriginalMesh->get_property_handle(f_location,"f_location");


			for (MyMesh::FaceIter f2_it=polymesh.faces_begin();f2_it!=polymesh.faces_end();++f2_it)
			{
				polymesh.property(f_area,*f2_it)=0.0;
			}

			OpenMesh::FPropHandleT<double> f_mean_curvature2;
			m_pOriginalMesh->get_property_handle(f_mean_curvature2,"f_mean_curvature");

			//先对每个面对积分
			MyMesh::FaceIter f2_it,f2_end(polymesh.faces_end());  
			MyMesh::FaceVertexIter  fv2_it;
			for (f2_it=polymesh.faces_begin();f2_it!=f2_end;++f2_it)
			{
				polymesh.property(f_area,*f2_it)=0.0;
				fv2_it=polymesh.fv_iter(*f2_it);
				double uknots_min=polymesh.point(*fv2_it)[0];
				double vknots_min=polymesh.point(*fv2_it)[1];
				fv2_it++;
				fv2_it++;
				double uknots_max=polymesh.point(*fv2_it)[0];
				double vknots_max=polymesh.point(*fv2_it)[1];

				ANNpoint			queryPt;				// query point
				ANNidxArray			nnIdx;					// near neighbor indices
				ANNdistArray		dists;					// near neighbor distances
				double			     eps= 0.00001;			// error bound
				queryPt = annAllocPt(2);					// allocate query point
				nnIdx = new(std::nothrow) ANNidx[1];						// allocate near neigh indices
				dists = new(std::nothrow) ANNdist[1];						// allocate near neighbor dists

				queryPt[0]=(uknots_min+uknots_max)/2;
				queryPt[1]=(vknots_min+vknots_max)/2;

				kdTree->annkSearch(						// search
					queryPt,						// query point
					1,								// number of near neighbors
					nnIdx,							// nearest neighbors (returned)
					dists,							// distance (returned)
					eps);							// error bound
				int idx=nnIdx[0];
				delete [] nnIdx;							// clean things up
				delete [] dists;
				Mesh::VertexHandle  p=m_pOriginalMesh->vertex_handle(idx);								
				//扩散过程，求矩形区域包含的内部三角形，跨越三角形，外部三角形
				Mesh::VertexIter   v_it,v_end(m_pOriginalMesh->vertices_end());
				for (v_it=m_pOriginalMesh->vertices_begin();v_it!=v_end;++v_it)
				{
					m_pOriginalMesh->property(v_location,*v_it)=unknown;

				}

				for (f_it=m_pOriginalMesh->faces_begin(); f_it!=f_end; ++f_it)
				{
					m_pOriginalMesh->property(f_location,*f_it)=unknown;
				}

				TexCoord  p2=m_pOriginalMesh->texcoord2D(p);

				if(uknots_min<=p2[0]&&p2[0]<=uknots_max&&p2[1]>=vknots_min&&p2[1]<=vknots_max)
				{
					m_pOriginalMesh->property(v_location,p)=inside;
					vector<int>  v1;//遍历一轮的顶点
					v1.push_back(p.idx());
					int turn=0;
                    vector<int>  v4;//存储所有的外部顶点序号
					do 
					{
						vector<int> v3;//存储新一轮的顶点序号
						for (vector<int>::iterator  v2=v1.begin();v2!=v1.end();++v2)
						{
							for(Mesh::VertexFaceIter vf_it=m_pOriginalMesh->vf_iter(m_pOriginalMesh->vertex_handle(*v2));vf_it.is_valid();++vf_it)
							{
								if (m_pOriginalMesh->property(f_location,*vf_it)==unknown)
								{
									int inside_sum=0,outside_sum=0,boundary_sum=0;
									for (Mesh::FaceVertexIter  fv_it=m_pOriginalMesh->fv_iter(*vf_it);fv_it.is_valid();++fv_it)
									{

										if (m_pOriginalMesh->property(v_location,*fv_it)==unknown)
										{
											//判定位置
											TexCoord  p3=m_pOriginalMesh->texcoord2D(*fv_it);
											if ((abs(p3[0]-uknots_min)<=EPSILON2 &&(p3[1]>=vknots_min&&p3[1]<=vknots_max))
												||(abs(p3[0]-uknots_max)<=EPSILON2&&(p3[1]>=vknots_min&&p3[1]<=vknots_max))
												||(abs(p3[1]-vknots_min)<=EPSILON2&&(p3[0]>=uknots_min&&p3[0]<=uknots_max))
												||(abs(p3[1]-vknots_max)<=EPSILON2&&(p3[0]>=uknots_min&&p3[0]<=uknots_max)))
											{
												m_pOriginalMesh->property(v_location,*fv_it)=boundary;
												boundary_sum++;
												v3.push_back((*fv_it).idx());

											}
											else if (p3[0]>=uknots_min&&p3[0]<=uknots_max&&p3[1]>=vknots_min&&p3[1]<=vknots_max)
											{
												m_pOriginalMesh->property(v_location,*fv_it)=inside;
												inside_sum++;
												v3.push_back((*fv_it).idx());

											}  
											else
											{
												m_pOriginalMesh->property(v_location,*fv_it)=outside;
												outside_sum++;
												v4.push_back((*fv_it).idx());
											}

										}
										else if (m_pOriginalMesh->property(v_location,*fv_it)==inside)
										{
											inside_sum++;
										}
										else if (m_pOriginalMesh->property(v_location,*fv_it)==outside)
										{
											outside_sum++;
										}
										else
										{
											boundary_sum++;						
										}
									}
									//判断三角形的位置
									assert(outside_sum+boundary_sum+inside_sum==3);
									double t22=m_pOriginalMesh->property(f_mean_curvature2,*vf_it);
									//求解三角形面积
									TexCoord A,B,C;
									fv_it2=m_pOriginalMesh->fv_iter(*vf_it);
									A=m_pOriginalMesh->texcoord2D(*fv_it2);
									++fv_it2;
									B=m_pOriginalMesh->texcoord2D(*fv_it2);
									++fv_it2;
									C=m_pOriginalMesh->texcoord2D(*fv_it2);
									double  Area=abs(CGAL::area(Point_2(A[0],A[1]),Point_2(B[0],B[1]),Point_2(C[0],C[1])));

									if (inside_sum==3)
									{
										m_pOriginalMesh->property(f_location,*vf_it)=inside;
										polymesh.property(f_area,*f2_it)+=Area*t22;		 

									} 
									else if (inside_sum==2)
									{
										if (boundary_sum==1)
										{
											m_pOriginalMesh->property(f_location,*vf_it)=inside;
											polymesh.property(f_area,*f2_it)+=Area*t22;	
										}
										else
										{
											m_pOriginalMesh->property(f_location,*vf_it)=boundary;
											Triangle_2  tri(Point_2 (A[0],A[1]),Point_2 (B[0],B[1]),Point_2 (C[0],C[1]));
											Iso_rectangle_2  rec(uknots_min,vknots_min,uknots_max,vknots_max);
											if(intersection_rt(tri,rec,&Area))
											{
												polymesh.property(f_area,*f2_it)+=Area*t22;		
											}


										}

									}
									else if (inside_sum==1)
									{
										if (boundary_sum==2)
										{
											m_pOriginalMesh->property(f_location,*vf_it)=inside;
											polymesh.property(f_area,*f2_it)+=Area*t22;	
										}
										else //1-0-2,1-1-1
										{
											m_pOriginalMesh->property(f_location,*vf_it)=boundary;
											Triangle_2  tri(Point_2 (A[0],A[1]),Point_2 (B[0],B[1]),Point_2 (C[0],C[1]));
											Iso_rectangle_2  rec(uknots_min,vknots_min,uknots_max,vknots_max);
											if(intersection_rt(tri,rec,&Area))
											{
												polymesh.property(f_area,*f2_it)+=Area*t22;		
											}

										}
									}
									else
									{
										assert(inside_sum==0);
										if (boundary_sum==3)
										{
											m_pOriginalMesh->property(f_location,*vf_it)=inside;
											polymesh.property(f_area,*f2_it)+=Area*t22;	
										}
										else //0-2-1,0-1-2，0-0-3
										{

											double  tri_umin=(A[0]<=B[0]?A[0]:(B[0]<=C[0]?B[0]:C[0]));
											double  tri_umax=(A[0]<=B[0]?B[0]:(A[0]<=C[0]?C[0]:A[0]));
											double  tri_vmin=(A[1]<=B[1]?A[1]:(B[1]<=C[1]?B[1]:C[1]));
											double  tri_vmax=(A[1]<=B[1]?B[1]:(A[1]<=C[1]?C[1]:A[1]));
											if(tri_umax<=uknots_min||tri_umin>=uknots_max||tri_vmin>=vknots_max||tri_vmax<=vknots_min)
											{
												m_pOriginalMesh->property(f_location,*vf_it)=outside;
											}
											else
											{
												Triangle_2  tri(Point_2 (A[0],A[1]),Point_2 (B[0],B[1]),Point_2 (C[0],C[1]));
												Iso_rectangle_2  rec(uknots_min,vknots_min,uknots_max,vknots_max);
												if(intersection_rt(tri,rec,&Area))
												{
													polymesh.property(f_area,*f2_it)+=Area*t22;		
												}
												m_pOriginalMesh->property(f_location,*vf_it)=boundary;
											}
										}

									}
								}
							}

						}
						if (turn==0)
						{
							v1.assign(v3.begin(),v3.end());
						}
						if (turn>0)
						{
							turn++;
							v1.assign(v4.begin(),v4.end());
							v1.insert(v1.end(),v3.begin(),v3.end());
							v4.clear();
						}
						if (v1.empty())
						{
							if (turn>0)
								break;
							//assert(turn==0);
							v1.assign(v4.begin(),v4.end());
							v4.clear();
							turn++;
						}

					} while (turn<20);

				}
				else
				{
					m_pOriginalMesh->property(v_location,p)=outside;

					vector<int>  v1;//遍历一轮的顶点
					v1.push_back(p.idx());

					int turn=0;//扩延的次数
					do 
					{
						vector<int> v3;//存储新一轮的顶点序号
						for (vector<int>::iterator  v2=v1.begin();v2!=v1.end();++v2)
						{
							for(Mesh::VertexFaceIter vf_it=m_pOriginalMesh->vf_iter(m_pOriginalMesh->vertex_handle(*v2));vf_it.is_valid();++vf_it)
							{
								if (m_pOriginalMesh->property(f_location,*vf_it)==unknown)
								{
									int inside_sum=0,outside_sum=0,boundary_sum=0;
									for (Mesh::FaceVertexIter  fv_it=m_pOriginalMesh->fv_iter(*vf_it);fv_it.is_valid();++fv_it)
									{

										if (m_pOriginalMesh->property(v_location,*fv_it)==unknown)
										{
											//判定位置
											TexCoord  p3=m_pOriginalMesh->texcoord2D(*fv_it);
											if ((abs(p3[0]-uknots_min)<=EPSILON2 &&(p3[1]>=vknots_min&&p3[1]<=vknots_max))
												||(abs(p3[0]-uknots_max)<=EPSILON2&&(p3[1]>=vknots_min&&p3[1]<=vknots_max))
												||(abs(p3[1]-vknots_min)<=EPSILON2&&(p3[0]>=uknots_min&&p3[0]<=uknots_max))
												||(abs(p3[1]-vknots_max)<=EPSILON2&&(p3[0]>=uknots_min&&p3[0]<=uknots_max)))
											{
												m_pOriginalMesh->property(v_location,*fv_it)=boundary;
												boundary_sum++;
												v3.push_back((*fv_it).idx());
											}
											else if (p3[0]>=uknots_min&&p3[0]<=uknots_max&&p3[1]>=vknots_min&&p3[1]<=vknots_max)
											{
												m_pOriginalMesh->property(v_location,*fv_it)=inside;
												inside_sum++;
												v3.push_back((*fv_it).idx());
											}  
											else
											{
												m_pOriginalMesh->property(v_location,*fv_it)=outside;
												outside_sum++;
												v3.push_back((*fv_it).idx());
											}

										}
										else if (m_pOriginalMesh->property(v_location,*fv_it)==inside)
										{
											inside_sum++;
										}
										else if (m_pOriginalMesh->property(v_location,*fv_it)==outside)
										{
											outside_sum++;
										}
										else
										{
											boundary_sum++;						
										}
									}
									//判断三角形的位置
									assert(outside_sum+boundary_sum+inside_sum==3);
									double t22=m_pOriginalMesh->property(f_mean_curvature2,*vf_it);
									//求解三角形面积
									TexCoord A,B,C;
									fv_it2=m_pOriginalMesh->fv_iter(*vf_it);
									A=m_pOriginalMesh->texcoord2D(*fv_it2);
									++fv_it2;
									B=m_pOriginalMesh->texcoord2D(*fv_it2);
									++fv_it2;
									C=m_pOriginalMesh->texcoord2D(*fv_it2);
									double  Area=abs(CGAL::area(Point_2(A[0],A[1]),Point_2(B[0],B[1]),Point_2(C[0],C[1])));

									if (inside_sum==3)
									{
										m_pOriginalMesh->property(f_location,*vf_it)=inside;
										polymesh.property(f_area,*f2_it)+=Area*t22;		 

									} 
									else if (inside_sum==2)
									{
										if (boundary_sum==1)
										{
											m_pOriginalMesh->property(f_location,*vf_it)=inside;
											polymesh.property(f_area,*f2_it)+=Area*t22;	
										}
										else
										{
											m_pOriginalMesh->property(f_location,*vf_it)=boundary;
											Triangle_2  tri(Point_2 (A[0],A[1]),Point_2 (B[0],B[1]),Point_2 (C[0],C[1]));
											Iso_rectangle_2  rec(uknots_min,vknots_min,uknots_max,vknots_max);
											if(intersection_rt(tri,rec,&Area))
											{
												polymesh.property(f_area,*f2_it)+=Area*t22;		
											}

										}

									}
									else if (inside_sum==1)
									{
										if (boundary_sum==2)
										{
											m_pOriginalMesh->property(f_location,*vf_it)=inside;
											polymesh.property(f_area,*f2_it)+=Area*t22;	
										}
										else //1-0-2,1-1-1
										{
											m_pOriginalMesh->property(f_location,*vf_it)=boundary;
											Triangle_2  tri(Point_2 (A[0],A[1]),Point_2 (B[0],B[1]),Point_2 (C[0],C[1]));
											Iso_rectangle_2  rec(uknots_min,vknots_min,uknots_max,vknots_max);
											if(intersection_rt(tri,rec,&Area))
											{
												polymesh.property(f_area,*f2_it)+=Area*t22;		
											}

										}
									}
									else
									{
										assert(inside_sum==0);
										if (boundary_sum==3)
										{
											m_pOriginalMesh->property(f_location,*vf_it)=inside;
											polymesh.property(f_area,*f2_it)+=Area*t22;	
										}
										else //0-2-1,0-1-2,0-0-3
										{
											double  tri_umin=(A[0]<=B[0]?A[0]:(B[0]<=C[0]?B[0]:C[0]));
											double  tri_umax=(A[0]<=B[0]?B[0]:(A[0]<=C[0]?C[0]:A[0]));
											double  tri_vmin=(A[1]<=B[1]?A[1]:(B[1]<=C[1]?B[1]:C[1]));
											double  tri_vmax=(A[1]<=B[1]?B[1]:(A[1]<=C[1]?C[1]:A[1]));
											if(tri_umax<=uknots_min||tri_umin>=uknots_max||tri_vmin>=vknots_max||tri_vmax<=vknots_min)
											{
												m_pOriginalMesh->property(f_location,*vf_it)=outside;
											}
											else
											{
												Triangle_2  tri(Point_2 (A[0],A[1]),Point_2 (B[0],B[1]),Point_2 (C[0],C[1]));
												Iso_rectangle_2  rec(uknots_min,vknots_min,uknots_max,vknots_max);
												if(intersection_rt(tri,rec,&Area))
												{
													polymesh.property(f_area,*f2_it)+=Area*t22;		
												}

												m_pOriginalMesh->property(f_location,*vf_it)=boundary;
											}
										}

									}
								}
							}

						}
						v1.assign(v3.begin(),v3.end());
						if (v1.empty())
						{
							break;
						}
						turn++;

					} while (turn<20);

					assert(polymesh.property(f_area,*f2_it)!=0.0);


				}
			}

			//求解各个小矩形的误差
			MyMesh::FaceIter face_iter,face_end(polymesh.faces_end());
			vector<double> error_cur;
			double g=0;
			for (face_iter=polymesh.faces_begin();face_iter!=face_end;++face_iter)
			{
				g=abs(polymesh.property(f_area,*face_iter)-curaverage_)/curaverage_;
				error_cur.push_back(g);
				polymesh.property(f_error,*face_iter)=g;

			}
			double maxerror=*(max_element(error_cur.begin(),error_cur.end()));
			double minerror=*(min_element(error_cur.begin(),error_cur.end()));
			max_curerror=maxerror;
			min_curerror=minerror;
			/*double vcolor[3];*/
			int h1, s1, v1;
			int h2, s2, v2;
			{
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
			}
			for (face_iter=polymesh.faces_begin();face_iter!=face_end;++face_iter)
			{
				double error=polymesh.property(f_error,*face_iter);
				QColor c;
				double ratio = error/maxerror;
				c.setHsv( h2 + qRound( ratio * ( h1 - h2 ) ),
					s2 + qRound( ratio * ( s1 - s2 ) ),
					v2 + qRound( ratio * ( v1 - v2 ) ) );//获取hsv

				int color[3];
				c.getRgb(&color[0], &color[1], &color[2]);//转化成rgb
				//vcolor[0] = color[0]/255.0;//转化成double
				//vcolor[1] = color[1]/255.0;
				//vcolor[2] = color[2]/255.0;
				polymesh.set_color(*face_iter,MyMesh::Color(color[0],color[1],color[2]));
			}
			curvature_error=true;



}
MyMesh CSurfaceData::get_polymesh()
{
	return polymesh;

}
bool CSurfaceData::intersection_st( Triangle_2 tri,Segment_2 seg,double *length)
{
	CGAL::Object result;
	Point_2 ipoint;
	Segment_2 iseg;

	result = intersection(seg, tri);
	if (assign(ipoint, result)) {
		*length=0;
		return true;
	}
	else if (assign(iseg, result))
	{
		*length=sqrt(iseg.squared_length());
		return true;
	} 
	else
	{
		return false;
	}
}
bool  CSurfaceData::intersection_rt(Triangle_2 tri,Iso_rectangle_2 rec,double *Area)
{
	//求三角形与矩形的交集
	CGAL::Object result;
	Point_2 ipoint;
	Segment_2 iseg;
	Triangle_2 itri;
	std::vector<Point_2>  vecpoint;

	result = intersection(tri,rec);
	if (CGAL::assign(itri, result)) 
	{
		*Area=itri.area();
		return true;
	} 
	else if (CGAL::assign(vecpoint, result))
	{
		Polygon_2 P(vecpoint.begin(),vecpoint.end());
		*Area=P.area();
		return  true;
	}
	else if (CGAL::assign(ipoint, result)||assign(iseg,result))
	{
		*Area=0;
		return  true;
	}
	else
	{
		return false;
	}
}


void  CSurfaceData::update_knots(int k)
{
//--------------------------------------------------------------------begin build polymesh

	sort(uknots.begin(),uknots.end());
	sort(vknots.begin(),vknots.end());

	//update_polymesh
	polymesh.clear();

	// generate vertices
	vector<vector<MyMesh::VertexHandle> > vhandle;
	vhandle.resize(vnum,vector<MyMesh::VertexHandle>(unum));
	for(int j=0;j<vnum;j++)
		for (int i=0;i<unum;i++)
		{
			vhandle[j][i]=polymesh.add_vertex(MyMesh::Point(uknots[i],vknots[j],0.0));
		}

	// generate (quadrilateral) faces
	std::vector<MyMesh::VertexHandle>  face_vhandles;
	for(int j=0;j<vnum-1;j++)
		for (int i=0;i<unum-1;i++)
		{
			face_vhandles.clear();
			face_vhandles.push_back(vhandle[j][i]);
			face_vhandles.push_back(vhandle[j][i+1]);
			face_vhandles.push_back(vhandle[j+1][i+1]);
			face_vhandles.push_back(vhandle[j+1][i]);
			polymesh.add_face(face_vhandles);

		}
	 //至此矩形面建立完毕
//--------------------------------------------------------------------  end 
			Mesh::FaceIter  f_it,f_end(m_pOriginalMesh->faces_end());
			Mesh::FaceVertexIter fv_it2;

			OpenMesh::FPropHandleT<double> f_area;  //增加每个面的曲率面积属性
			polymesh.add_property(f_area,"f_area");

			OpenMesh::EPropHandleT<double> e_area;  //增加每条边的曲率积分属性
			polymesh.add_property(e_area,"e_area");

			OpenMesh::VPropHandleT<Location_Type> v_location; //添加点属性
			m_pOriginalMesh->get_property_handle(v_location,"v_location");

			OpenMesh::FPropHandleT<Location_Type> f_location; //添加面属性
			m_pOriginalMesh->get_property_handle(f_location,"f_location");


			for (MyMesh::EdgeIter  ed=polymesh.edges_begin();ed!=polymesh.edges_end();++ed)
			{
				polymesh.property(e_area,*ed)=0.0;
			}

			for (MyMesh::FaceIter f2_it=polymesh.faces_begin();f2_it!=polymesh.faces_end();++f2_it)
			{
				polymesh.property(f_area,*f2_it)=0.0;
			}

			OpenMesh::FPropHandleT<double> f_mean_curvature2;
			m_pOriginalMesh->get_property_handle(f_mean_curvature2,"f_mean_curvature");


//-------------------------------------------------------------  先对每个面对积分
			MyMesh::FaceIter f2_it,f2_end(polymesh.faces_end());  
			MyMesh::FaceVertexIter  fv2_it;
			for (f2_it=polymesh.faces_begin();f2_it!=f2_end;++f2_it)
			{
				polymesh.property(f_area,*f2_it)=0.0;
				fv2_it=polymesh.fv_iter(*f2_it);
				double uknots_min=polymesh.point(*fv2_it)[0];
				double vknots_min=polymesh.point(*fv2_it)[1];
				fv2_it++;
				fv2_it++;
				double uknots_max=polymesh.point(*fv2_it)[0];
				double vknots_max=polymesh.point(*fv2_it)[1];

				ANNpoint			queryPt;				// query point
				ANNidxArray			nnIdx;					// near neighbor indices
				ANNdistArray		dists;					// near neighbor distances
				double			     eps= 0.00001;			// error bound
				queryPt = annAllocPt(2);					// allocate query point
				nnIdx = new(std::nothrow) ANNidx[1];						// allocate near neigh indices
				dists = new(std::nothrow) ANNdist[1];						// allocate near neighbor dists

				queryPt[0]=(uknots_min+uknots_max)/2;
				queryPt[1]=(vknots_min+vknots_max)/2;

				kdTree->annkSearch(						// search
					queryPt,						// query point
					1,								// number of near neighbors
					nnIdx,							// nearest neighbors (returned)
					dists,							// distance (returned)
					eps);							// error bound
				int idx=nnIdx[0];
				delete [] nnIdx;							// clean things up
				delete [] dists;
				Mesh::VertexHandle  p=m_pOriginalMesh->vertex_handle(idx);								
				//扩散过程，求矩形区域包含的内部三角形，跨越三角形，外部三角形
				Mesh::VertexIter   v_it,v_end(m_pOriginalMesh->vertices_end());
				for (v_it=m_pOriginalMesh->vertices_begin();v_it!=v_end;++v_it)
				{
					m_pOriginalMesh->property(v_location,*v_it)=unknown;

				}

				for (f_it=m_pOriginalMesh->faces_begin(); f_it!=f_end; ++f_it)
				{
					m_pOriginalMesh->property(f_location,*f_it)=unknown;
				}

				TexCoord  p2=m_pOriginalMesh->texcoord2D(p);
				if(uknots_min<=p2[0]&&p2[0]<=uknots_max&&p2[1]>=vknots_min&&p2[1]<=vknots_max)
				{
					m_pOriginalMesh->property(v_location,p)=inside;
					vector<int>  v1;//遍历一轮的顶点
					v1.push_back(p.idx());
					vector<int>  f_boundary;//储存跨界三角形的序号
					vector<int>  v4;//存储所有的外部顶点序号
					int turn=0;
					do 
					{
						vector<int> v3;//存储新一轮的顶点序号
						for (vector<int>::iterator  v2=v1.begin();v2!=v1.end();++v2)
						{
							for(Mesh::VertexFaceIter vf_it=m_pOriginalMesh->vf_iter(m_pOriginalMesh->vertex_handle(*v2));vf_it.is_valid();++vf_it)
							{
								if (m_pOriginalMesh->property(f_location,*vf_it)==unknown)
								{
									int inside_sum=0,outside_sum=0,boundary_sum=0;
									for (Mesh::FaceVertexIter  fv_it=m_pOriginalMesh->fv_iter(*vf_it);fv_it.is_valid();++fv_it)
									{

										if (m_pOriginalMesh->property(v_location,*fv_it)==unknown)
										{
											//判定位置
											TexCoord  p3=m_pOriginalMesh->texcoord2D(*fv_it);
											if ((abs(p3[0]-uknots_min)<=EPSILON2 &&(p3[1]>=vknots_min&&p3[1]<=vknots_max))
												||(abs(p3[0]-uknots_max)<=EPSILON2&&(p3[1]>=vknots_min&&p3[1]<=vknots_max))
												||(abs(p3[1]-vknots_min)<=EPSILON2&&(p3[0]>=uknots_min&&p3[0]<=uknots_max))
												||(abs(p3[1]-vknots_max)<=EPSILON2&&(p3[0]>=uknots_min&&p3[0]<=uknots_max)))
											{
												m_pOriginalMesh->property(v_location,*fv_it)=boundary;
												boundary_sum++;
												v3.push_back((*fv_it).idx());
											}
											else if (p3[0]>=uknots_min&&p3[0]<=uknots_max&&p3[1]>=vknots_min&&p3[1]<=vknots_max)
											{
												m_pOriginalMesh->property(v_location,*fv_it)=inside;
												inside_sum++;
												v3.push_back((*fv_it).idx());
											}  
											else
											{
												m_pOriginalMesh->property(v_location,*fv_it)=outside;
												outside_sum++;
												v4.push_back((*fv_it).idx());
											}

										}
										else if (m_pOriginalMesh->property(v_location,*fv_it)==inside)
										{
											inside_sum++;
										}
										else if (m_pOriginalMesh->property(v_location,*fv_it)==outside)
										{
											outside_sum++;
										}
										else
										{
											boundary_sum++;						
										}
									}
									//判断三角形的位置
									assert(outside_sum+boundary_sum+inside_sum==3);
									double t22=m_pOriginalMesh->property(f_mean_curvature2,*vf_it);
									//求解三角形面积
									TexCoord A,B,C;
									fv_it2=m_pOriginalMesh->fv_iter(*vf_it);
									A=m_pOriginalMesh->texcoord2D(*fv_it2);
									++fv_it2;
									B=m_pOriginalMesh->texcoord2D(*fv_it2);
									++fv_it2;
									C=m_pOriginalMesh->texcoord2D(*fv_it2);
									double  Area=abs(CGAL::area(Point_2(A[0],A[1]),Point_2(B[0],B[1]),Point_2(C[0],C[1])));

									if (inside_sum==3)
									{
										m_pOriginalMesh->property(f_location,*vf_it)=inside;
										polymesh.property(f_area,*f2_it)+=Area*t22;		 

									} 
									else if (inside_sum==2)
									{
										if (boundary_sum==1)
										{
											m_pOriginalMesh->property(f_location,*vf_it)=inside;
											polymesh.property(f_area,*f2_it)+=Area*t22;	
										}
										else
										{
											m_pOriginalMesh->property(f_location,*vf_it)=boundary;
											Triangle_2  tri(Point_2 (A[0],A[1]),Point_2 (B[0],B[1]),Point_2 (C[0],C[1]));
											Iso_rectangle_2  rec(uknots_min,vknots_min,uknots_max,vknots_max);
											if(intersection_rt(tri,rec,&Area))
											{
												polymesh.property(f_area,*f2_it)+=Area*t22;		
											}
											f_boundary.push_back((*vf_it).idx());

										}

									}
									else if (inside_sum==1)
									{
										if (boundary_sum==2)
										{
											m_pOriginalMesh->property(f_location,*vf_it)=inside;
											polymesh.property(f_area,*f2_it)+=Area*t22;	
										}
										else //1-0-2,1-1-1
										{
											m_pOriginalMesh->property(f_location,*vf_it)=boundary;
											Triangle_2  tri(Point_2 (A[0],A[1]),Point_2 (B[0],B[1]),Point_2 (C[0],C[1]));
											Iso_rectangle_2  rec(uknots_min,vknots_min,uknots_max,vknots_max);
											if(intersection_rt(tri,rec,&Area))
											{
												polymesh.property(f_area,*f2_it)+=Area*t22;		
											}
											f_boundary.push_back((*vf_it).idx());
										}
									}
									else
									{
										assert(inside_sum==0);
										if (boundary_sum==3)
										{
											m_pOriginalMesh->property(f_location,*vf_it)=inside;
											polymesh.property(f_area,*f2_it)+=Area*t22;	
										}
										else //0-2-1,0-1-2,0-0-3
										{

											double  tri_umin=(A[0]<=B[0]?A[0]:(B[0]<=C[0]?B[0]:C[0]));
											double  tri_umax=(A[0]<=B[0]?B[0]:(A[0]<=C[0]?C[0]:A[0]));
											double  tri_vmin=(A[1]<=B[1]?A[1]:(B[1]<=C[1]?B[1]:C[1]));
											double  tri_vmax=(A[1]<=B[1]?B[1]:(A[1]<=C[1]?C[1]:A[1]));
											if(tri_umax<=uknots_min||tri_umin>=uknots_max||tri_vmin>=vknots_max||tri_vmax<=vknots_min)
											{
												m_pOriginalMesh->property(f_location,*vf_it)=outside;
											}
											else
											{
												Triangle_2  tri(Point_2 (A[0],A[1]),Point_2 (B[0],B[1]),Point_2 (C[0],C[1]));
												Iso_rectangle_2  rec(uknots_min,vknots_min,uknots_max,vknots_max);
												if(intersection_rt(tri,rec,&Area))
												{
													polymesh.property(f_area,*f2_it)+=Area*t22;		
												}
												f_boundary.push_back((*vf_it).idx());
												m_pOriginalMesh->property(f_location,*vf_it)=boundary;
											}

										}

									}
								}
							}

						}
						if (turn==0)
						{
							v1.assign(v3.begin(),v3.end());
						}
						if (turn>0)
						{
							turn++;
							v1.assign(v4.begin(),v4.end());
							v1.insert(v1.end(),v3.begin(),v3.end());
							v4.clear();
						}
						if (v1.empty())
						{
							if (turn>0)
								break;
							//assert(turn==0);
							v1.assign(v4.begin(),v4.end());
							v4.clear();
							turn++;
						}

					} while (turn<20);

					//处理矩形区域的边积分
					//通过识别一个面是否是其中半边所属的面，来决定是否由这个面来求解这条边积分
					MyMesh::FaceEdgeIter  fe2_it;
					for (fe2_it=polymesh.fe_iter(*f2_it);fe2_it.is_valid();++fe2_it)
					{
						if (!polymesh.is_boundary(*fe2_it))
						{
							MyMesh::HalfedgeHandle  he1=polymesh.halfedge_handle(*fe2_it,0);
							MyMesh::FaceHandle   f1=polymesh.face_handle(he1);
							if (f1.idx()==(*f2_it).idx())
							{
								for (vector<int>::iterator  ff=f_boundary.begin();ff!=f_boundary.end();++ff)
								{
									TexCoord A,B,C;
									Mesh::FaceHandle ff_handle=m_pOriginalMesh->face_handle(*ff);
									double t23=m_pOriginalMesh->property(f_mean_curvature2,ff_handle);
									fv_it2=m_pOriginalMesh->fv_iter(ff_handle);
									A=m_pOriginalMesh->texcoord2D(*fv_it2);
									++fv_it2;
									B=m_pOriginalMesh->texcoord2D(*fv_it2);
									++fv_it2;
									C=m_pOriginalMesh->texcoord2D(*fv_it2);
									Triangle_2  tri(Point_2 (A[0],A[1]),Point_2 (B[0],B[1]),Point_2 (C[0],C[1]));
									MyMesh::VertexHandle     fv=polymesh.from_vertex_handle(he1),tv=polymesh.to_vertex_handle(he1);
									MyMesh::Point     p1=polymesh.point(fv),p2=polymesh.point(tv);
									double length=0;
									bool is=intersection_st(tri,Segment_2(Point_2(p1[0],p1[1]),Point_2(p2[0],p2[1])),&length);
									if (is)
									{
										polymesh.property(e_area,*fe2_it)+=t23*length;
									}
								}


							}
						}


					}
				}
				else
				{
					m_pOriginalMesh->property(v_location,p)=outside;

					vector<int>  v1;//遍历一轮的顶点
					v1.push_back(p.idx());
					vector<int>  f_boundary;//储存跨界三角形的序号
					int turn=0;//扩延的次数
					do 
					{
						vector<int> v3;//存储新一轮的顶点序号
						for (vector<int>::iterator  v2=v1.begin();v2!=v1.end();++v2)
						{
							for(Mesh::VertexFaceIter vf_it=m_pOriginalMesh->vf_iter(m_pOriginalMesh->vertex_handle(*v2));vf_it.is_valid();++vf_it)
							{
								if (m_pOriginalMesh->property(f_location,*vf_it)==unknown)
								{
									int inside_sum=0,outside_sum=0,boundary_sum=0;
									for (Mesh::FaceVertexIter  fv_it=m_pOriginalMesh->fv_iter(*vf_it);fv_it.is_valid();++fv_it)
									{

										if (m_pOriginalMesh->property(v_location,*fv_it)==unknown)
										{
											//判定位置
											TexCoord  p3=m_pOriginalMesh->texcoord2D(*fv_it);
											if ((abs(p3[0]-uknots_min)<=EPSILON2 &&(p3[1]>=vknots_min&&p3[1]<=vknots_max))
												||(abs(p3[0]-uknots_max)<=EPSILON2&&(p3[1]>=vknots_min&&p3[1]<=vknots_max))
												||(abs(p3[1]-vknots_min)<=EPSILON2&&(p3[0]>=uknots_min&&p3[0]<=uknots_max))
												||(abs(p3[1]-vknots_max)<=EPSILON2&&(p3[0]>=uknots_min&&p3[0]<=uknots_max)))
											{
												m_pOriginalMesh->property(v_location,*fv_it)=boundary;
												boundary_sum++;
												v3.push_back((*fv_it).idx());
											}
											else if (p3[0]>=uknots_min&&p3[0]<=uknots_max&&p3[1]>=vknots_min&&p3[1]<=vknots_max)
											{
												m_pOriginalMesh->property(v_location,*fv_it)=inside;
												inside_sum++;
												v3.push_back((*fv_it).idx());
											}  
											else
											{
												m_pOriginalMesh->property(v_location,*fv_it)=outside;
												outside_sum++;
												v3.push_back((*fv_it).idx());
											}

										}
										else if (m_pOriginalMesh->property(v_location,*fv_it)==inside)
										{
											inside_sum++;
										}
										else if (m_pOriginalMesh->property(v_location,*fv_it)==outside)
										{
											outside_sum++;
										}
										else
										{
											boundary_sum++;						
										}
									}
									//判断三角形的位置
									assert(outside_sum+boundary_sum+inside_sum==3);
									double t22=m_pOriginalMesh->property(f_mean_curvature2,*vf_it);
									//求解三角形面积
									TexCoord A,B,C;
									fv_it2=m_pOriginalMesh->fv_iter(*vf_it);
									A=m_pOriginalMesh->texcoord2D(*fv_it2);
									++fv_it2;
									B=m_pOriginalMesh->texcoord2D(*fv_it2);
									++fv_it2;
									C=m_pOriginalMesh->texcoord2D(*fv_it2);
									double  Area=abs(CGAL::area(Point_2(A[0],A[1]),Point_2(B[0],B[1]),Point_2(C[0],C[1])));

									if (inside_sum==3)
									{
										m_pOriginalMesh->property(f_location,*vf_it)=inside;
										polymesh.property(f_area,*f2_it)+=Area*t22;		 

									} 
									else if (inside_sum==2)
									{
										if (boundary_sum==1)
										{
											m_pOriginalMesh->property(f_location,*vf_it)=inside;
											polymesh.property(f_area,*f2_it)+=Area*t22;	
										}
										else
										{
											m_pOriginalMesh->property(f_location,*vf_it)=boundary;
											Triangle_2  tri(Point_2 (A[0],A[1]),Point_2 (B[0],B[1]),Point_2 (C[0],C[1]));
											Iso_rectangle_2  rec(uknots_min,vknots_min,uknots_max,vknots_max);
											if(intersection_rt(tri,rec,&Area))
											{
												polymesh.property(f_area,*f2_it)+=Area*t22;		
											}
											f_boundary.push_back((*vf_it).idx());

										}

									}
									else if (inside_sum==1)
									{
										if (boundary_sum==2)
										{
											m_pOriginalMesh->property(f_location,*vf_it)=inside;
											polymesh.property(f_area,*f2_it)+=Area*t22;	
										}
										else //1-0-2,1-1-1
										{
											m_pOriginalMesh->property(f_location,*vf_it)=boundary;
											Triangle_2  tri(Point_2 (A[0],A[1]),Point_2 (B[0],B[1]),Point_2 (C[0],C[1]));
											Iso_rectangle_2  rec(uknots_min,vknots_min,uknots_max,vknots_max);
											if(intersection_rt(tri,rec,&Area))
											{
												polymesh.property(f_area,*f2_it)+=Area*t22;		
											}
											f_boundary.push_back((*vf_it).idx());
										}
									}
									else
									{
										assert(inside_sum==0);
										if (boundary_sum==3)
										{
											m_pOriginalMesh->property(f_location,*vf_it)=inside;
											polymesh.property(f_area,*f2_it)+=Area*t22;	
										}
										else //0-2-1,0-1-2,0-0-3
										{
											double  tri_umin=(A[0]<=B[0]?A[0]:(B[0]<=C[0]?B[0]:C[0]));
											double  tri_umax=(A[0]<=B[0]?B[0]:(A[0]<=C[0]?C[0]:A[0]));
											double  tri_vmin=(A[1]<=B[1]?A[1]:(B[1]<=C[1]?B[1]:C[1]));
											double  tri_vmax=(A[1]<=B[1]?B[1]:(A[1]<=C[1]?C[1]:A[1]));
											if(tri_umax<=uknots_min||tri_umin>=uknots_max||tri_vmin>=vknots_max||tri_vmax<=vknots_min)
											{
												m_pOriginalMesh->property(f_location,*vf_it)=outside;
											}
											else
											{
												Triangle_2  tri(Point_2 (A[0],A[1]),Point_2 (B[0],B[1]),Point_2 (C[0],C[1]));
												Iso_rectangle_2  rec(uknots_min,vknots_min,uknots_max,vknots_max);
												if(intersection_rt(tri,rec,&Area))
												{
													polymesh.property(f_area,*f2_it)+=Area*t22;		
												}
												f_boundary.push_back((*vf_it).idx());
												m_pOriginalMesh->property(f_location,*vf_it)=boundary;
											}
										}

									}
								}
							}

						}
						v1.assign(v3.begin(),v3.end());
						if (v1.empty())
					 {
						 break;
					 }
						turn++;

					} while (turn<20);

					//处理矩形区域的边积分
					MyMesh::FaceEdgeIter  fe2_it;
					for (fe2_it=polymesh.fe_iter(*f2_it);fe2_it.is_valid();++fe2_it)
					{
						if (!polymesh.is_boundary(*fe2_it))
						{
							MyMesh::HalfedgeHandle  he1=polymesh.halfedge_handle(*fe2_it,0);
							MyMesh::FaceHandle   f1=polymesh.face_handle(he1);
							if (f1.idx()==(*f2_it).idx())
							{
								for (vector<int>::iterator  ff=f_boundary.begin();ff!=f_boundary.end();++ff)
								{
									TexCoord A,B,C;
									Mesh::FaceHandle ff_handle=m_pOriginalMesh->face_handle(*ff);
									double t23=m_pOriginalMesh->property(f_mean_curvature2,ff_handle);
									fv_it2=m_pOriginalMesh->fv_iter(ff_handle);
									A=m_pOriginalMesh->texcoord2D(*fv_it2);
									++fv_it2;
									B=m_pOriginalMesh->texcoord2D(*fv_it2);
									++fv_it2;
									C=m_pOriginalMesh->texcoord2D(*fv_it2);
									Triangle_2  tri(Point_2 (A[0],A[1]),Point_2 (B[0],B[1]),Point_2 (C[0],C[1]));
									MyMesh::VertexHandle     fv=polymesh.from_vertex_handle(he1),tv=polymesh.to_vertex_handle(he1);
									MyMesh::Point     p1=polymesh.point(fv),p2=polymesh.point(tv);
									double length=0;
									bool is=intersection_st(tri,Segment_2(Point_2(p1[0],p1[1]),Point_2(p2[0],p2[1])),&length);
									if (is)
									{
										polymesh.property(e_area,*fe2_it)+=t23*length;
									}
								}

								//assert(polymesh.property(e_area,*fe2_it)!=0.0);
							}
						}


					}
					//assert(polymesh.property(f_area,*f2_it)!=0.0);
				}
			}

			//求解目标值
			MyMesh::FaceIter face_iter,face_end(polymesh.faces_end());
			double g=0;
			for (face_iter=polymesh.faces_begin();face_iter!=face_end;++face_iter)
			{
				g+=pow(polymesh.property(f_area,*face_iter)-curaverage_,2);
			}
			error_sum.push_back(g);
			std::cout<<"第"<<" "<<iter_num++<<" "<<"次函数值:"<<g<<" ";

			//准备梯度
			vector<double> uknot_grad(unum-2),vknot_grad(vnum-2);//分别存储每条节点线的梯度
			{
				//遍历列
				int i,j,r;
				int m1=unum-1,n1=vnum-1;
				for( i=1;i<m1;i++)
				{
					if (i==1)
					{
						j=1;
						r=1;
						MyMesh::EdgeHandle   fe=polymesh.edge_handle(r);
						MyMesh::HalfedgeHandle  he1=polymesh.halfedge_handle(fe,0);
						MyMesh::HalfedgeHandle  he2=polymesh.halfedge_handle(fe,1);
						MyMesh::FaceHandle   f1=polymesh.face_handle(he1),f2=polymesh.face_handle(he2);
						uknot_grad[i-1]+=(polymesh.property(f_area,f1)-polymesh.property(f_area,f2))*polymesh.property(e_area,fe); 
						for (j=2;j<=n1;j++)
						{
							r=3*m1+1+(2*m1+1)*(j-2);
							MyMesh::EdgeHandle   fe=polymesh.edge_handle(r);
							MyMesh::HalfedgeHandle  he1=polymesh.halfedge_handle(fe,0);
							MyMesh::HalfedgeHandle  he2=polymesh.halfedge_handle(fe,1);
							MyMesh::FaceHandle   f1=polymesh.face_handle(he1),f2=polymesh.face_handle(he2);
							uknot_grad[i-1]+=(polymesh.property(f_area,f1)-polymesh.property(f_area,f2))*polymesh.property(e_area,fe); 
						}

					}
					else 
					{

						j=1;
						r=3*i-1;
						MyMesh::EdgeHandle   fe=polymesh.edge_handle(r);
						MyMesh::HalfedgeHandle  he1=polymesh.halfedge_handle(fe,0);
						MyMesh::HalfedgeHandle  he2=polymesh.halfedge_handle(fe,1);
						MyMesh::FaceHandle   f1=polymesh.face_handle(he1),f2=polymesh.face_handle(he2);
						uknot_grad[i-1]+=(polymesh.property(f_area,f1)-polymesh.property(f_area,f2))*polymesh.property(e_area,fe); 
						for (j=2;j<=n1;j++)
						{
							r=2*i+3*m1+(j-2)*(2*m1+1);
							MyMesh::EdgeHandle   fe=polymesh.edge_handle(r);
							MyMesh::HalfedgeHandle  he1=polymesh.halfedge_handle(fe,0);
							MyMesh::HalfedgeHandle  he2=polymesh.halfedge_handle(fe,1);
							MyMesh::FaceHandle   f1=polymesh.face_handle(he1),f2=polymesh.face_handle(he2);
							uknot_grad[i-1]+=(polymesh.property(f_area,f1)-polymesh.property(f_area,f2))*polymesh.property(e_area,fe); 
						}	
					}	
					uknot_grad[i-1]*=2;
				}

				//再遍历各行的边
				for(i=1;i<n1;i++)
				{
					if (i==1)
					{

						j=1;
						r=2;
						MyMesh::EdgeHandle   fe=polymesh.edge_handle(r);
						MyMesh::HalfedgeHandle  he1=polymesh.halfedge_handle(fe,0);
						MyMesh::HalfedgeHandle  he2=polymesh.halfedge_handle(fe,1);
						MyMesh::FaceHandle   f1=polymesh.face_handle(he1),f2=polymesh.face_handle(he2);
						vknot_grad[i-1]+=(polymesh.property(f_area,f1)-polymesh.property(f_area,f2))*polymesh.property(e_area,fe); 
						for (j=2;j<=m1;j++)
						{
							r=3*j;
							MyMesh::EdgeHandle   fe=polymesh.edge_handle(r);
							MyMesh::HalfedgeHandle  he1=polymesh.halfedge_handle(fe,0);
							MyMesh::HalfedgeHandle  he2=polymesh.halfedge_handle(fe,1);
							MyMesh::FaceHandle   f1=polymesh.face_handle(he1),f2=polymesh.face_handle(he2);
							vknot_grad[i-1]+=(polymesh.property(f_area,f1)-polymesh.property(f_area,f2))*polymesh.property(e_area,fe); 
						}		 

					}
					else
					{

						j=1;
						r=(2*m1+1)*i-m1;
						MyMesh::EdgeHandle   fe=polymesh.edge_handle(r);
						MyMesh::HalfedgeHandle  he1=polymesh.halfedge_handle(fe,0);
						MyMesh::HalfedgeHandle  he2=polymesh.halfedge_handle(fe,1);
						MyMesh::FaceHandle   f1=polymesh.face_handle(he1),f2=polymesh.face_handle(he2);
						vknot_grad[i-1]+=(polymesh.property(f_area,f1)-polymesh.property(f_area,f2))*polymesh.property(e_area,fe); 
						for (j=2;j<=m1;j++)
						{
							r=(2*m1+1)*i+2*j-1-m1;
							MyMesh::EdgeHandle   fe=polymesh.edge_handle(r);
							MyMesh::HalfedgeHandle  he1=polymesh.halfedge_handle(fe,0);
							MyMesh::HalfedgeHandle  he2=polymesh.halfedge_handle(fe,1);
							MyMesh::FaceHandle   f1=polymesh.face_handle(he1),f2=polymesh.face_handle(he2);
							vknot_grad[i-1]+=(polymesh.property(f_area,f1)-polymesh.property(f_area,f2))*polymesh.property(e_area,fe); 
						}		
					}
					vknot_grad[i-1]*=2;
				}
			}
			double gradnorm=0.0;
			for(int i=0;i<unum-2;i++)
			{
				gradnorm+=pow(uknot_grad[i],2);
			}
			for (int j=0;j<vnum-2;j++)
			{
				gradnorm+=pow(vknot_grad[j],2);
			}
			std::cout<<"梯度范数"<<sqrt(gradnorm)<<std::endl;
			//节点迭代
			double  udmin=10.0;
			for (int r=0;r<unum-1;r++)
			{
				if (udmin>(uknots[r+1]-uknots[r]))
				{
					udmin=uknots[r+1]-uknots[r];
				}
			}
			double vdmin=10.0;
			for (int r=0;r<vnum-1;r++)
			{
				if (vdmin>(vknots[r+1]-vknots[r]))
				{
					vdmin=vknots[r+1]-vknots[r];
				}
			}
			double uh=udmin/10*pow(0.5,k/static_cast<double>(iter_times-k));
			double vh=vdmin/10*pow(0.5,k/static_cast<double>(iter_times-k));
			for (int i=1;i<=unum-2;i++)
			{
				double  dis2=uh*uknot_grad[i-1]/abs(uknot_grad[i-1]);
				uknots[i]-=dis2;
			}
			for (int j=1;j<=vnum-2;j++)
			{
				double  dis2=vh*vknot_grad[j-1]/abs(vknot_grad[j-1]);
				vknots[j]-=dis2;

			}

}

void  CSurfaceData::update_knots2(int k)
{
	sort(uknots.begin(),uknots.end());
	sort(vknots.begin(),vknots.end());
	//update_polymesh
	polymesh.clear();
	// generate vertices
	vector<vector<MyMesh::VertexHandle> > vhandle;
	vhandle.resize(vnum,vector<MyMesh::VertexHandle>(unum));
	for(int j=0;j<vnum;j++)
		for (int i=0;i<unum;i++)
		{
			vhandle[j][i]=polymesh.add_vertex(MyMesh::Point(uknots[i],vknots[j],0.0));
		}

		// generate (quadrilateral) faces
		std::vector<MyMesh::VertexHandle>  face_vhandles;
		for(int j=0;j<vnum-1;j++)
			for (int i=0;i<unum-1;i++)
			{
				face_vhandles.clear();
				face_vhandles.push_back(vhandle[j][i]);
				face_vhandles.push_back(vhandle[j][i+1]);
				face_vhandles.push_back(vhandle[j+1][i+1]);
				face_vhandles.push_back(vhandle[j+1][i]);
				polymesh.add_face(face_vhandles);

			}
			//至此矩形面建立完毕
			Mesh::FaceIter  f_it,f_end(m_pFittedMesh->faces_end());
			Mesh::FaceVertexIter fv_it2;

			OpenMesh::FPropHandleT<double> f_area;  //增加每个面的曲率面积属性
			polymesh.add_property(f_area,"f_area");

			OpenMesh::EPropHandleT<double> e_area;  //增加每条边的曲率积分属性
			polymesh.add_property(e_area,"e_area");

			OpenMesh::VPropHandleT<Location_Type> v_location; //添加点属性
			m_pFittedMesh->get_property_handle(v_location,"v_location_fit");

			OpenMesh::FPropHandleT<Location_Type> f_location; //添加面属性
			m_pFittedMesh->get_property_handle(f_location,"f_location_fit");


			for (MyMesh::EdgeIter  ed=polymesh.edges_begin();ed!=polymesh.edges_end();++ed)
			{
				polymesh.property(e_area,*ed)=0.0;
			}

			for (MyMesh::FaceIter f2_it=polymesh.faces_begin();f2_it!=polymesh.faces_end();++f2_it)
			{
				polymesh.property(f_area,*f2_it)=0.0;
			}

			OpenMesh::FPropHandleT<double>  f_mean_error; 
			m_pFittedMesh->get_property_handle(f_mean_error,"f_mean_error");  

			//先对每个面对积分
			MyMesh::FaceIter f2_it,f2_end(polymesh.faces_end());  
			MyMesh::FaceVertexIter  fv2_it;
			for (f2_it=polymesh.faces_begin();f2_it!=f2_end;++f2_it)
			{
				polymesh.property(f_area,*f2_it)=0.0;
				fv2_it=polymesh.fv_iter(*f2_it);
				double uknots_min=polymesh.point(*fv2_it)[0];
				double vknots_min=polymesh.point(*fv2_it)[1];
				fv2_it++;
				fv2_it++;
				double uknots_max=polymesh.point(*fv2_it)[0];
				double vknots_max=polymesh.point(*fv2_it)[1];

				ANNpoint			queryPt;				// query point
				ANNidxArray			nnIdx;					// near neighbor indices
				ANNdistArray		dists;					// near neighbor distances
				double			     eps= 0.00001;			// error bound
				queryPt = annAllocPt(2);					// allocate query point
				nnIdx = new(std::nothrow) ANNidx[1];						// allocate near neigh indices
				dists = new(std::nothrow) ANNdist[1];						// allocate near neighbor dists

				queryPt[0]=(uknots_min+uknots_max)/2;
				queryPt[1]=(vknots_min+vknots_max)/2;

				kdTree->annkSearch(						// search
					queryPt,						// query point
					1,								// number of near neighbors
					nnIdx,							// nearest neighbors (returned)
					dists,							// distance (returned)
					eps);							// error bound
				int idx=nnIdx[0];
				delete [] nnIdx;							// clean things up
				delete [] dists;
				Mesh::VertexHandle  p=m_pFittedMesh->vertex_handle(idx);								
				//扩散过程，求矩形区域包含的内部三角形，跨越三角形，外部三角形
				Mesh::VertexIter   v_it,v_end(m_pFittedMesh->vertices_end());
				for (v_it=m_pFittedMesh->vertices_begin();v_it!=v_end;++v_it)
				{
					m_pFittedMesh->property(v_location,*v_it)=unknown;

				}

				for (f_it=m_pFittedMesh->faces_begin(); f_it!=f_end; ++f_it)
				{
					m_pFittedMesh->property(f_location,*f_it)=unknown;
				}

				TexCoord  p2=m_pFittedMesh->texcoord2D(p);
				if(uknots_min<=p2[0]&&p2[0]<=uknots_max&&p2[1]>=vknots_min&&p2[1]<=vknots_max)
				{
					m_pFittedMesh->property(v_location,p)=inside;
					vector<int>  v1;//遍历一轮的顶点
					v1.push_back(p.idx());
					vector<int>  f_boundary;//储存跨界三角形的序号
					int turn=0;
					vector<int>  v4;//存储所有的外部顶点序号
					do 
					{
						vector<int> v3;//存储新一轮的顶点序号
						for (vector<int>::iterator  v2=v1.begin();v2!=v1.end();++v2)
						{
							for(Mesh::VertexFaceIter vf_it=m_pFittedMesh->vf_iter(m_pFittedMesh->vertex_handle(*v2));vf_it.is_valid();++vf_it)
							{
								if (m_pFittedMesh->property(f_location,*vf_it)==unknown)
								{
									int inside_sum=0,outside_sum=0,boundary_sum=0;
									for (Mesh::FaceVertexIter  fv_it=m_pFittedMesh->fv_iter(*vf_it);fv_it.is_valid();++fv_it)
									{

										if (m_pFittedMesh->property(v_location,*fv_it)==unknown)
										{
											//判定位置
											TexCoord  p3=m_pFittedMesh->texcoord2D(*fv_it);
											if ((abs(p3[0]-uknots_min)<=EPSILON2 &&(p3[1]>=vknots_min&&p3[1]<=vknots_max))
												||(abs(p3[0]-uknots_max)<=EPSILON2&&(p3[1]>=vknots_min&&p3[1]<=vknots_max))
												||(abs(p3[1]-vknots_min)<=EPSILON2&&(p3[0]>=uknots_min&&p3[0]<=uknots_max))
												||(abs(p3[1]-vknots_max)<=EPSILON2&&(p3[0]>=uknots_min&&p3[0]<=uknots_max)))
											{
												m_pFittedMesh->property(v_location,*fv_it)=boundary;
												boundary_sum++;
												v3.push_back((*fv_it).idx());
											}
											else if (p3[0]>=uknots_min&&p3[0]<=uknots_max&&p3[1]>=vknots_min&&p3[1]<=vknots_max)
											{
												m_pFittedMesh->property(v_location,*fv_it)=inside;
												inside_sum++;
												v3.push_back((*fv_it).idx());
											}  
											else
											{
												m_pFittedMesh->property(v_location,*fv_it)=outside;
												outside_sum++;
												v4.push_back((*fv_it).idx());
											}

										}
										else if (m_pFittedMesh->property(v_location,*fv_it)==inside)
										{
											inside_sum++;
										}
										else if (m_pFittedMesh->property(v_location,*fv_it)==outside)
										{
											outside_sum++;
										}
										else
										{
											boundary_sum++;						
										}
									}
									//判断三角形的位置
									assert(outside_sum+boundary_sum+inside_sum==3);
									double t22=m_pFittedMesh->property(f_mean_error,*vf_it);
									//求解三角形面积
									TexCoord A,B,C;
									fv_it2=m_pFittedMesh->fv_iter(*vf_it);
									A=m_pFittedMesh->texcoord2D(*fv_it2);
									++fv_it2;
									B=m_pFittedMesh->texcoord2D(*fv_it2);
									++fv_it2;
									C=m_pFittedMesh->texcoord2D(*fv_it2);
									double  Area=abs(CGAL::area(Point_2(A[0],A[1]),Point_2(B[0],B[1]),Point_2(C[0],C[1])));

									if (inside_sum==3)
									{
										m_pFittedMesh->property(f_location,*vf_it)=inside;
										polymesh.property(f_area,*f2_it)+=Area*t22;		 

									} 
									else if (inside_sum==2)
									{
										if (boundary_sum==1)
										{
											m_pFittedMesh->property(f_location,*vf_it)=inside;
											polymesh.property(f_area,*f2_it)+=Area*t22;	
										}
										else
										{
											m_pFittedMesh->property(f_location,*vf_it)=boundary;
											Triangle_2  tri(Point_2 (A[0],A[1]),Point_2 (B[0],B[1]),Point_2 (C[0],C[1]));
											Iso_rectangle_2  rec(uknots_min,vknots_min,uknots_max,vknots_max);
											if(intersection_rt(tri,rec,&Area))
											{
												polymesh.property(f_area,*f2_it)+=Area*t22;		
											}
											f_boundary.push_back((*vf_it).idx());

										}

									}
									else if (inside_sum==1)
									{
										if (boundary_sum==2)
										{
											m_pFittedMesh->property(f_location,*vf_it)=inside;
											polymesh.property(f_area,*f2_it)+=Area*t22;	
										}
										else //1-0-2,1-1-1
										{
											m_pFittedMesh->property(f_location,*vf_it)=boundary;
											Triangle_2  tri(Point_2 (A[0],A[1]),Point_2 (B[0],B[1]),Point_2 (C[0],C[1]));
											Iso_rectangle_2  rec(uknots_min,vknots_min,uknots_max,vknots_max);
											if(intersection_rt(tri,rec,&Area))
											{
												polymesh.property(f_area,*f2_it)+=Area*t22;		
											}
											f_boundary.push_back((*vf_it).idx());
										}
									}
									else
									{
										assert(inside_sum==0);
										if (boundary_sum==3)
										{
											m_pFittedMesh->property(f_location,*vf_it)=inside;
											polymesh.property(f_area,*f2_it)+=Area*t22;	
										}
										else //0-2-1,0-1-2,0-0-3
										{

											double  tri_umin=(A[0]<=B[0]?A[0]:(B[0]<=C[0]?B[0]:C[0]));
											double  tri_umax=(A[0]<=B[0]?B[0]:(A[0]<=C[0]?C[0]:A[0]));
											double  tri_vmin=(A[1]<=B[1]?A[1]:(B[1]<=C[1]?B[1]:C[1]));
											double  tri_vmax=(A[1]<=B[1]?B[1]:(A[1]<=C[1]?C[1]:A[1]));
											if(tri_umax<=uknots_min||tri_umin>=uknots_max||tri_vmin>=vknots_max||tri_vmax<=vknots_min)
											{
												m_pFittedMesh->property(f_location,*vf_it)=outside;
											}
											else
											{
												Triangle_2  tri(Point_2 (A[0],A[1]),Point_2 (B[0],B[1]),Point_2 (C[0],C[1]));
												Iso_rectangle_2  rec(uknots_min,vknots_min,uknots_max,vknots_max);
												if(intersection_rt(tri,rec,&Area))
												{
													polymesh.property(f_area,*f2_it)+=Area*t22;		
												}
												f_boundary.push_back((*vf_it).idx());
												m_pFittedMesh->property(f_location,*vf_it)=boundary;
											}
										}

									}
								}
							}

						}
						if (turn==0)
						{
							v1.assign(v3.begin(),v3.end());
						}
						if (turn>0)
						{
							turn++;
							v1.assign(v4.begin(),v4.end());
							v1.insert(v1.end(),v3.begin(),v3.end());
							v4.clear();
						}
						if (v1.empty())
						{
							if (turn>0)
								break;
							//assert(turn==0);
							v1.assign(v4.begin(),v4.end());
							v4.clear();
							turn++;
						}

					} while (turn<20);

					//处理矩形区域的边积分
					MyMesh::FaceEdgeIter  fe2_it;
					for (fe2_it=polymesh.fe_iter(*f2_it);fe2_it.is_valid();++fe2_it)
					{
						if (!polymesh.is_boundary(*fe2_it))
						{
							MyMesh::HalfedgeHandle  he1=polymesh.halfedge_handle(*fe2_it,0);
							MyMesh::FaceHandle   f1=polymesh.face_handle(he1);
							if (f1.idx()==(*f2_it).idx())
							{
								for (vector<int>::iterator  ff=f_boundary.begin();ff!=f_boundary.end();++ff)
								{
									TexCoord A,B,C;
									Mesh::FaceHandle ff_handle=m_pFittedMesh->face_handle(*ff);
									double t23=m_pFittedMesh->property(f_mean_error,ff_handle);
									fv_it2=m_pFittedMesh->fv_iter(ff_handle);
									A=m_pFittedMesh->texcoord2D(*fv_it2);
									++fv_it2;
									B=m_pFittedMesh->texcoord2D(*fv_it2);
									++fv_it2;
									C=m_pFittedMesh->texcoord2D(*fv_it2);
									Triangle_2  tri(Point_2 (A[0],A[1]),Point_2 (B[0],B[1]),Point_2 (C[0],C[1]));
									MyMesh::VertexHandle     fv=polymesh.from_vertex_handle(he1),tv=polymesh.to_vertex_handle(he1);
									MyMesh::Point     p1=polymesh.point(fv),p2=polymesh.point(tv);
									double length=0;
									bool is=intersection_st(tri,Segment_2(Point_2(p1[0],p1[1]),Point_2(p2[0],p2[1])),&length);
									if (is)
									{
										polymesh.property(e_area,*fe2_it)+=t23*length;
									}
								}


							}
						}


					}

				}
				else
				{
					m_pFittedMesh->property(v_location,p)=outside;
					vector<int>  v1;//遍历一轮的顶点
					v1.push_back(p.idx());
					vector<int>  f_boundary;//储存跨界三角形的序号
					int turn=0;//扩延的次数
					do 
					{
						vector<int> v3;//存储新一轮的顶点序号
						for (vector<int>::iterator  v2=v1.begin();v2!=v1.end();++v2)
						{
							for(Mesh::VertexFaceIter vf_it=m_pFittedMesh->vf_iter(m_pFittedMesh->vertex_handle(*v2));vf_it.is_valid();++vf_it)
							{
								if (m_pFittedMesh->property(f_location,*vf_it)==unknown)
								{
									int inside_sum=0,outside_sum=0,boundary_sum=0;
									for (Mesh::FaceVertexIter  fv_it=m_pFittedMesh->fv_iter(*vf_it);fv_it.is_valid();++fv_it)
									{

										if (m_pFittedMesh->property(v_location,*fv_it)==unknown)
										{
											//判定位置
											TexCoord  p3=m_pFittedMesh->texcoord2D(*fv_it);
											if ((abs(p3[0]-uknots_min)<=EPSILON2 &&(p3[1]>=vknots_min&&p3[1]<=vknots_max))
												||(abs(p3[0]-uknots_max)<=EPSILON2&&(p3[1]>=vknots_min&&p3[1]<=vknots_max))
												||(abs(p3[1]-vknots_min)<=EPSILON2&&(p3[0]>=uknots_min&&p3[0]<=uknots_max))
												||(abs(p3[1]-vknots_max)<=EPSILON2&&(p3[0]>=uknots_min&&p3[0]<=uknots_max)))
											{
												m_pFittedMesh->property(v_location,*fv_it)=boundary;
												boundary_sum++;
												v3.push_back((*fv_it).idx());
											}
											else if (p3[0]>=uknots_min&&p3[0]<=uknots_max&&p3[1]>=vknots_min&&p3[1]<=vknots_max)
											{
												m_pFittedMesh->property(v_location,*fv_it)=inside;
												inside_sum++;
												v3.push_back((*fv_it).idx());
											}  
											else
											{
												m_pFittedMesh->property(v_location,*fv_it)=outside;
												outside_sum++;
												v3.push_back((*fv_it).idx());
											}

										}
										else if (m_pFittedMesh->property(v_location,*fv_it)==inside)
										{
											inside_sum++;
										}
										else if (m_pFittedMesh->property(v_location,*fv_it)==outside)
										{
											outside_sum++;
										}
										else
										{
											boundary_sum++;						
										}
									}
									//判断三角形的位置
									assert(outside_sum+boundary_sum+inside_sum==3);
									double t22=m_pFittedMesh->property(f_mean_error,*vf_it);
									//求解三角形面积
									TexCoord A,B,C;
									fv_it2=m_pFittedMesh->fv_iter(*vf_it);
									A=m_pFittedMesh->texcoord2D(*fv_it2);
									++fv_it2;
									B=m_pFittedMesh->texcoord2D(*fv_it2);
									++fv_it2;
									C=m_pFittedMesh->texcoord2D(*fv_it2);
									double  Area=abs(CGAL::area(Point_2(A[0],A[1]),Point_2(B[0],B[1]),Point_2(C[0],C[1])));

									if (inside_sum==3)
									{
										m_pFittedMesh->property(f_location,*vf_it)=inside;
										polymesh.property(f_area,*f2_it)+=Area*t22;		 

									} 
									else if (inside_sum==2)
									{
										if (boundary_sum==1)
										{
											m_pFittedMesh->property(f_location,*vf_it)=inside;
											polymesh.property(f_area,*f2_it)+=Area*t22;	
										}
										else
										{
											m_pFittedMesh->property(f_location,*vf_it)=boundary;
											Triangle_2  tri(Point_2 (A[0],A[1]),Point_2 (B[0],B[1]),Point_2 (C[0],C[1]));
											Iso_rectangle_2  rec(uknots_min,vknots_min,uknots_max,vknots_max);
											if(intersection_rt(tri,rec,&Area))
											{
												polymesh.property(f_area,*f2_it)+=Area*t22;		
											}
											f_boundary.push_back((*vf_it).idx());

										}

									}
									else if (inside_sum==1)
									{
										if (boundary_sum==2)
										{
											m_pFittedMesh->property(f_location,*vf_it)=inside;
											polymesh.property(f_area,*f2_it)+=Area*t22;	
										}
										else //1-0-2,1-1-1
										{
											m_pFittedMesh->property(f_location,*vf_it)=boundary;
											Triangle_2  tri(Point_2 (A[0],A[1]),Point_2 (B[0],B[1]),Point_2 (C[0],C[1]));
											Iso_rectangle_2  rec(uknots_min,vknots_min,uknots_max,vknots_max);
											if(intersection_rt(tri,rec,&Area))
											{
												polymesh.property(f_area,*f2_it)+=Area*t22;		
											}
											f_boundary.push_back((*vf_it).idx());
										}
									}
									else
									{
										assert(inside_sum==0);
										if (boundary_sum==3)
										{
											m_pFittedMesh->property(f_location,*vf_it)=inside;
											polymesh.property(f_area,*f2_it)+=Area*t22;	
										}
										else //0-2-1,0-1-2,0-0-3
										{
											double  tri_umin=(A[0]<=B[0]?A[0]:(B[0]<=C[0]?B[0]:C[0]));
											double  tri_umax=(A[0]<=B[0]?B[0]:(A[0]<=C[0]?C[0]:A[0]));
											double  tri_vmin=(A[1]<=B[1]?A[1]:(B[1]<=C[1]?B[1]:C[1]));
											double  tri_vmax=(A[1]<=B[1]?B[1]:(A[1]<=C[1]?C[1]:A[1]));
											if(tri_umax<=uknots_min||tri_umin>=uknots_max||tri_vmin>=vknots_max||tri_vmax<=vknots_min)
											{
												m_pFittedMesh->property(f_location,*vf_it)=outside;
											}
											else
											{
												Triangle_2  tri(Point_2 (A[0],A[1]),Point_2 (B[0],B[1]),Point_2 (C[0],C[1]));
												Iso_rectangle_2  rec(uknots_min,vknots_min,uknots_max,vknots_max);
												if(intersection_rt(tri,rec,&Area))
												{
													polymesh.property(f_area,*f2_it)+=Area*t22;		
												}
												f_boundary.push_back((*vf_it).idx());
												m_pFittedMesh->property(f_location,*vf_it)=boundary;
											}
										}

									}
								}
							}

						}
						v1.assign(v3.begin(),v3.end());
						if (v1.empty())
					 {
						 break;
					 }
						turn++;

					} while (turn<20);

					//处理矩形区域的边积分
					MyMesh::FaceEdgeIter  fe2_it;
					for (fe2_it=polymesh.fe_iter(*f2_it);fe2_it.is_valid();++fe2_it)
					{
						if (!polymesh.is_boundary(*fe2_it))
						{
							MyMesh::HalfedgeHandle  he1=polymesh.halfedge_handle(*fe2_it,0);
							MyMesh::FaceHandle   f1=polymesh.face_handle(he1);
							if (f1.idx()==(*f2_it).idx())
							{
								for (vector<int>::iterator  ff=f_boundary.begin();ff!=f_boundary.end();++ff)
								{
									TexCoord A,B,C;
									Mesh::FaceHandle ff_handle=m_pFittedMesh->face_handle(*ff);
									double t23=m_pFittedMesh->property(f_mean_error,ff_handle);
									fv_it2=m_pFittedMesh->fv_iter(ff_handle);
									A=m_pFittedMesh->texcoord2D(*fv_it2);
									++fv_it2;
									B=m_pFittedMesh->texcoord2D(*fv_it2);
									++fv_it2;
									C=m_pFittedMesh->texcoord2D(*fv_it2);
									Triangle_2  tri(Point_2 (A[0],A[1]),Point_2 (B[0],B[1]),Point_2 (C[0],C[1]));
									MyMesh::VertexHandle     fv=polymesh.from_vertex_handle(he1),tv=polymesh.to_vertex_handle(he1);
									MyMesh::Point     p1=polymesh.point(fv),p2=polymesh.point(tv);
									double length=0;
									bool is=intersection_st(tri,Segment_2(Point_2(p1[0],p1[1]),Point_2(p2[0],p2[1])),&length);
									if (is)
									{
										polymesh.property(e_area,*fe2_it)+=t23*length;
									}
								}

								//assert(polymesh.property(e_area,*fe2_it)!=0.0);
							}
						}


					}

					//assert(polymesh.property(f_area,*f2_it)!=0.0);
				}
			}

			//求解目标值
			MyMesh::FaceIter face_iter,face_end(polymesh.faces_end());
			double g=0;
			for (face_iter=polymesh.faces_begin();face_iter!=face_end;++face_iter)
			{
				g+=pow(polymesh.property(f_area,*face_iter)-erroraverage_,2);
			}
			error_sum.push_back(g);
			std::cout<<"第"<<" "<<iter_num++<<" "<<"次函数值:"<<g<<" ";

			//准备梯度
			vector<double> uknot_grad(unum-2),vknot_grad(vnum-2);//分别存储每条节点线的梯度
			{
				//遍历列
				int i,j,r;
				int m1=unum-1,n1=vnum-1;
				for( i=1;i<m1;i++)
				{
					if (i==1)
					{
						j=1;
						r=1;
						MyMesh::EdgeHandle   fe=polymesh.edge_handle(r);
						MyMesh::HalfedgeHandle  he1=polymesh.halfedge_handle(fe,0);
						MyMesh::HalfedgeHandle  he2=polymesh.halfedge_handle(fe,1);
						MyMesh::FaceHandle   f1=polymesh.face_handle(he1),f2=polymesh.face_handle(he2);
						uknot_grad[i-1]+=(polymesh.property(f_area,f1)-polymesh.property(f_area,f2))*polymesh.property(e_area,fe); 
						for (j=2;j<=n1;j++)
						{
							r=3*m1+1+(2*m1+1)*(j-2);
							MyMesh::EdgeHandle   fe=polymesh.edge_handle(r);
							MyMesh::HalfedgeHandle  he1=polymesh.halfedge_handle(fe,0);
							MyMesh::HalfedgeHandle  he2=polymesh.halfedge_handle(fe,1);
							MyMesh::FaceHandle   f1=polymesh.face_handle(he1),f2=polymesh.face_handle(he2);
							uknot_grad[i-1]+=(polymesh.property(f_area,f1)-polymesh.property(f_area,f2))*polymesh.property(e_area,fe); 
						}

					}
					else 
					{

						j=1;
						r=3*i-1;
						MyMesh::EdgeHandle   fe=polymesh.edge_handle(r);
						MyMesh::HalfedgeHandle  he1=polymesh.halfedge_handle(fe,0);
						MyMesh::HalfedgeHandle  he2=polymesh.halfedge_handle(fe,1);
						MyMesh::FaceHandle   f1=polymesh.face_handle(he1),f2=polymesh.face_handle(he2);
						uknot_grad[i-1]+=(polymesh.property(f_area,f1)-polymesh.property(f_area,f2))*polymesh.property(e_area,fe); 
						for (j=2;j<=n1;j++)
						{
							r=2*i+3*m1+(j-2)*(2*m1+1);
							MyMesh::EdgeHandle   fe=polymesh.edge_handle(r);
							MyMesh::HalfedgeHandle  he1=polymesh.halfedge_handle(fe,0);
							MyMesh::HalfedgeHandle  he2=polymesh.halfedge_handle(fe,1);
							MyMesh::FaceHandle   f1=polymesh.face_handle(he1),f2=polymesh.face_handle(he2);
							uknot_grad[i-1]+=(polymesh.property(f_area,f1)-polymesh.property(f_area,f2))*polymesh.property(e_area,fe); 
						}	
					}	
					uknot_grad[i-1]*=2;
				}

				//再遍历各行的边
				for(i=1;i<n1;i++)
				{
					if (i==1)
					{

						j=1;
						r=2;
						MyMesh::EdgeHandle   fe=polymesh.edge_handle(r);
						MyMesh::HalfedgeHandle  he1=polymesh.halfedge_handle(fe,0);
						MyMesh::HalfedgeHandle  he2=polymesh.halfedge_handle(fe,1);
						MyMesh::FaceHandle   f1=polymesh.face_handle(he1),f2=polymesh.face_handle(he2);
						vknot_grad[i-1]+=(polymesh.property(f_area,f1)-polymesh.property(f_area,f2))*polymesh.property(e_area,fe); 
						for (j=2;j<=m1;j++)
						{
							r=3*j;
							MyMesh::EdgeHandle   fe=polymesh.edge_handle(r);
							MyMesh::HalfedgeHandle  he1=polymesh.halfedge_handle(fe,0);
							MyMesh::HalfedgeHandle  he2=polymesh.halfedge_handle(fe,1);
							MyMesh::FaceHandle   f1=polymesh.face_handle(he1),f2=polymesh.face_handle(he2);
							vknot_grad[i-1]+=(polymesh.property(f_area,f1)-polymesh.property(f_area,f2))*polymesh.property(e_area,fe); 
						}		 

					}
					else
					{

						j=1;
						r=(2*m1+1)*i-m1;
						MyMesh::EdgeHandle   fe=polymesh.edge_handle(r);
						MyMesh::HalfedgeHandle  he1=polymesh.halfedge_handle(fe,0);
						MyMesh::HalfedgeHandle  he2=polymesh.halfedge_handle(fe,1);
						MyMesh::FaceHandle   f1=polymesh.face_handle(he1),f2=polymesh.face_handle(he2);
						vknot_grad[i-1]+=(polymesh.property(f_area,f1)-polymesh.property(f_area,f2))*polymesh.property(e_area,fe); 
						for (j=2;j<=m1;j++)
						{
							r=(2*m1+1)*i+2*j-1-m1;
							MyMesh::EdgeHandle   fe=polymesh.edge_handle(r);
							MyMesh::HalfedgeHandle  he1=polymesh.halfedge_handle(fe,0);
							MyMesh::HalfedgeHandle  he2=polymesh.halfedge_handle(fe,1);
							MyMesh::FaceHandle   f1=polymesh.face_handle(he1),f2=polymesh.face_handle(he2);
							vknot_grad[i-1]+=(polymesh.property(f_area,f1)-polymesh.property(f_area,f2))*polymesh.property(e_area,fe); 
						}		
					}
					vknot_grad[i-1]*=2;
				}
			}
			double gradnorm=0.0;
			for(int i=0;i<unum-2;i++)
			{
				gradnorm+=pow(uknot_grad[i],2);
			}
			for (int j=0;j<vnum-2;j++)
			{
				gradnorm+=pow(vknot_grad[j],2);
			}
			std::cout<<"梯度范数"<<sqrt(gradnorm)<<std::endl;
			//节点迭代
			double  udmin=10.0;
			for (int r=0;r<unum-1;r++)
			{
				if (udmin>(uknots[r+1]-uknots[r]))
				{
					udmin=uknots[r+1]-uknots[r];
				}
			}
			double vdmin=10.0;
			for (int r=0;r<vnum-1;r++)
			{
				if (vdmin>(vknots[r+1]-vknots[r]))
				{
					vdmin=vknots[r+1]-vknots[r];
				}
			}
			double uh=udmin/10*pow(0.5,k/static_cast<double>(iter_times-k));
			double vh=vdmin/10*pow(0.5,k/static_cast<double>(iter_times-k));
			for (int i=1;i<=unum-2;i++)
			{
				double  dis2=uh*uknot_grad[i-1]/abs(uknot_grad[i-1]);
				uknots[i]-=dis2;
			}
			for (int j=1;j<=vnum-2;j++)
			{
				double  dis2=vh*vknot_grad[j-1]/abs(vknot_grad[j-1]);
				vknots[j]-=dis2;

			}



}




double CSurfaceData::get_max_meancurvature()
{
	return max_meancurvature;
}
double CSurfaceData::get_min_meancurvature()
{
	return min_meancurvature;
}


void CSurfaceData::set_max_error(double &err)
{
	if(err_threshold!=err)
	{
		err_threshold = err;
	}
		if(!m_pFittedMesh)
		{
			return;
		}
		else
		{
			if(!m_pErrorMesh)
			{
				m_pErrorMesh = new(std::nothrow) Mesh;
				*m_pErrorMesh = *m_pFittedMesh;
			}
			else
			{
	            *m_pErrorMesh = *m_pFittedMesh;
			}
		}
		// compute box
		Mesh::ConstVertexIter  v_it(m_pErrorMesh->vertices_begin()), 
			v_end(m_pErrorMesh->vertices_end());
		Mesh::Point            bbMin, bbMax;

		bbMin = bbMax = m_pErrorMesh->point(*v_it);
		for (; v_it!=v_end; ++v_it)
		{
			bbMin.minimize(m_pErrorMesh->point(*v_it));
			bbMax.maximize(m_pErrorMesh->point(*v_it));
		}
		Bbox_3 box(bbMin[0],bbMin[1],bbMin[2],bbMax[0],bbMax[1],bbMax[2]);
		errorbox = box;

		Mesh::VertexIter vit = m_pErrorMesh->vertices_begin();
		OpenMesh::VPropHandleT<double>  verror; 
		m_pErrorMesh->get_property_handle(verror,"verror");

		m_pErrorMesh->request_vertex_colors(); 
		if (!m_pErrorMesh->has_vertex_colors())
		    return;

	
		/*double vcolor[3];*/
		int h1, s1, v1;
		int h2, s2, v2;
		{
			int color[3];
			color[0] =250;
			color[1] = 0;
			color[2] =250;
			QColor d_light(color[0], color[1], color[2]);
		
			color[0] = 0;
			color[1] = 127;
			color[2] = 0;
			QColor d_dark(color[0], color[1], color[2]);
			d_light.getHsv( &h1, &s1, &v1 );
			d_dark.getHsv( &h2, &s2, &v2 );
		}
		for(; vit!=m_pErrorMesh->vertices_end(); ++vit)
		{
			double error=m_pErrorMesh->property(verror,*vit);
			QColor c;
			double ratio = error/err_threshold;
			ratio = ratio>1?1:ratio;
			c.setHsv( h2 + qRound( ratio * ( h1 - h2 ) ),
            s2 + qRound( ratio * ( s1 - s2 ) ),
            v2 + qRound( ratio * ( v1 - v2 ) ) );//获取hsv

			int color[3];
			c.getRgb(&color[0], &color[1], &color[2]);//转化成rgb
			//vcolor[0] = color[0]/255.0;//转化成double
			//vcolor[1] = color[1]/255.0;
			//vcolor[2] = color[2]/255.0;
			m_pErrorMesh->set_color(*vit,Mesh::Color(color[0],color[1],color[2]));
		}
	
}
void CSurfaceData::set_knots_iteration_times(int n)
{
     iter_times=n;
}

Mesh *CSurfaceData::get_error_fitted_mesh() 
{  
	if(!m_pFittedMesh)
		return NULL;

	return m_pErrorMesh;
}


