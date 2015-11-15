/*************************************************
* Copyright (c) 2014-����,���Ŵ�ѧ�����ͼ��ѧʵ����Graphics@XMU
* All rights reserved.
*
* �ļ����ƣ�SurfaceData.cpp
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




#define _READ_ 1 //ûʲô���ã�����
typedef  vector<int> vecint;
typedef  vector<vector< OpenMesh::Vec3d> > vectex;
typedef vector<double>::iterator   viterator;
typedef  pair<viterator,viterator>  VIPair;
using namespace std;
using namespace Eigen;



vector<double>  error_sum;//ÿһ�ε���������
int CSurfaceData::iter_num=1;


CSurfaceData::CSurfaceData(void)
{
	m_pOriginalMesh = NULL; /**< ������ΪNULL��\see MeshViewer::draw(), MeshViewer::draw_original_mesh()*/
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
	max_err.error = 1000;//��ʼ��,����void SplineKnotsViewer::error_fitting_view(bool kv)����δ�������ʱʹ��
	max_err.index = -1;
	rangequery_vertical=false;
	rangequery_horizon=false;
    kdTree=NULL;    //�������NULL����������CsurfaceData���ͻ���ִ���
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
			//���˾����潨�����

			Mesh::FaceIter  f_it,f_end(m_pOriginalMesh->faces_end());
			Mesh::FaceVertexIter fv_it2;

			OpenMesh::FPropHandleT<vecint> v_index;  //����ÿ������ڲ���ͱ߽���������
			polymesh.add_property(v_index,"v_index");

			OpenMesh::VPropHandleT<Location_Type> v_location; //��ӵ�����
			m_pOriginalMesh->get_property_handle(v_location,"v_location");

			OpenMesh::FPropHandleT<Location_Type> f_location; //���������
			m_pOriginalMesh->get_property_handle(f_location,"f_location");

			//�ȶ�ÿ����Ի���
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
				//��ɢ���̣����������������ڲ������Σ���Խ�����Σ��ⲿ������
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
					vector<int>  v1;//����һ�ֵĶ���
					v1.push_back(p.idx());
					polymesh.property(v_index,*f2_it).push_back(p.idx());
					vector<int>  v4;//�洢���е��ⲿ�������
					do 
					{
						vector<int> v3;//�洢��һ�ֵĶ������

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
											//�ж�λ��
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
									//�ж������ε�λ��
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
				 cout<<"��Ҫ�����:"<<(*f2_it).idx()<<endl;
				 m_pOriginalMesh->property(v_location,p)=outside;
				 int turn=0;
				 vector<int>  v1;//����һ�ֵĶ���
				 v1.push_back(p.idx());
				 do 
				 {
					 vector<int> v3;//�洢��һ�ֵĶ������
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
										 //�ж�λ��
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
								 //�ж������ε�λ��

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
			//���horizon_index,vertical_index			vertical_index.clear();
			horizon_index.clear();
			vertical_index.clear();
			vertical_index.resize(unum-1);
			horizon_index.resize(vnum-1);
			cout<<"ÿ������������"<<endl;
			for (int i=0;i<unum-1;i++)
			{
				cout<<"��"<<i+1<<"�����Ϊ:"<<endl;
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

			cout<<"ÿ������������"<<endl;
			for (int i=0;i<vnum-1;i++)
			{
				cout<<"��"<<i+1<<"�����Ϊ:"<<endl;
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
			//�����µĽڵ�������/������������εĺ��ڲ���ͱ߽������
			vector<double>  u_delete;
			for (vector<knot_info>::iterator ibegin=uknots_new.begin();ibegin!=uknots_new.end();)
			{

				VIPair P=equal_range(uknots.begin(),uknots.end(),(*ibegin).knot);
				bool stop=false;
				if (P.first!=P.second)//�ҵ��ض�ֵ
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
						 if (vindex[0]==-1)//��������û�а�����
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
				if (P.first!=P.second)//�ҵ��ض�ֵ
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
							if (vindex[0]==-1)//��������û�а�����
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
			//���˾����潨�����

			Mesh::FaceIter  f_it,f_end(m_pOriginalMesh->faces_end());
			Mesh::FaceVertexIter fv_it2;

			OpenMesh::FPropHandleT<vecint> v_index;  //����ÿ������ڲ���ͱ߽���������
			polymesh.add_property(v_index,"v_index");

			OpenMesh::VPropHandleT<Location_Type> v_location; //��ӵ�����
			m_pOriginalMesh->get_property_handle(v_location,"v_location");

			OpenMesh::FPropHandleT<Location_Type> f_location; //���������
			m_pOriginalMesh->get_property_handle(f_location,"f_location");

			//�ȶ�ÿ����Ի���
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
				//��ɢ���̣����������������ڲ������Σ���Խ�����Σ��ⲿ������
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
					vector<int>  v1;//����һ�ֵĶ���
					v1.push_back(p.idx());
					polymesh.property(v_index,*f2_it).push_back(p.idx());
					vector<int>  v4;//�洢���е��ⲿ�������
					do 
					{
						vector<int> v3;//�洢��һ�ֵĶ������

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
											//�ж�λ��
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
									//�ж������ε�λ��
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
				 cout<<"��Ҫ�����:"<<(*f2_it).idx()<<endl;
				 m_pOriginalMesh->property(v_location,p)=outside;
				 int turn=0;
				 vector<int>  v1;//����һ�ֵĶ���
				 v1.push_back(p.idx());
				 do 
				 {
					 vector<int> v3;//�洢��һ�ֵĶ������
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
										 //�ж�λ��
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
								 //�ж������ε�λ��

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
			//���horizon_index,vertical_index			vertical_index.clear();
			horizon_index.clear();
			vertical_index.clear();
			vertical_index.resize(unum-1);
			horizon_index.resize(vnum-1);
			cout<<"ÿ������������"<<endl;
			for (int i=0;i<unum-1;i++)
			{
				cout<<"��"<<i+1<<"�����Ϊ:"<<endl;
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

			cout<<"ÿ������������"<<endl;
			for (int i=0;i<vnum-1;i++)
			{
				cout<<"��"<<i+1<<"�����Ϊ:"<<endl;
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
			//�����µĽڵ�������/������������εĺ��ڲ���ͱ߽������
			vector<double>  utemp;
			vector<double>  vtemp;//��ɾ����Ԫ��
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
						if (vindex[0]==-1)//��������û�а�����
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
	//����kd-tree
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
			//���˾����潨�����

			Mesh::FaceIter  f_it,f_end(m_pOriginalMesh->faces_end());
			Mesh::FaceVertexIter fv_it2;

			OpenMesh::FPropHandleT<vecint> v_index;  //����ÿ������ڲ���ͱ߽���������
			polymesh.add_property(v_index,"v_index");

			OpenMesh::VPropHandleT<Location_Type> v_location; //��ӵ�����
			m_pOriginalMesh->add_property(v_location,"v_location");

			OpenMesh::FPropHandleT<Location_Type> f_location; //���������
			m_pOriginalMesh->add_property(f_location,"f_location");

			//�ȶ�ÿ����Ի���
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
				//��ɢ���̣����������������ڲ������Σ���Խ�����Σ��ⲿ������
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
					vector<int>  v1;//����һ�ֵĶ���
					v1.push_back(p.idx());
					polymesh.property(v_index,*f2_it).push_back(p.idx());
					vector<int>  v4;//�洢���е��ⲿ�������
					do 
					{
						vector<int> v3;//�洢��һ�ֵĶ������
					
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
											//�ж�λ��
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
									//�ж������ε�λ��
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
				 cout<<"��Ҫ�����:"<<(*f2_it).idx()<<endl;
				 m_pOriginalMesh->property(v_location,p)=outside;
				 int turn=0;
				 vector<int>  v1;//����һ�ֵĶ���
				 v1.push_back(p.idx());
				 do 
				 {
					 vector<int> v3;//�洢��һ�ֵĶ������
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
										 //�ж�λ��
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
								 //�ж������ε�λ��

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
			//���horizon_index,vertical_index			vertical_index.clear();
			horizon_index.clear();
			vertical_index.clear();
			vertical_index.resize(unum-1);
			horizon_index.resize(vnum-1);
			cout<<"ÿ������������"<<endl;
			for (int i=0;i<unum-1;i++)
			{
				cout<<"��"<<i+1<<"�����Ϊ:"<<endl;
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
			cout<<"ÿ������������"<<endl;
			for (int i=0;i<vnum-1;i++)
			{
				cout<<"��"<<i+1<<"�����Ϊ:"<<endl;
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
	//		cout<<"���ε��������"<<max_err_real<<endl;
	//		return false;
 //       }
 //       compute_error();
	//	cout<<"���ε��������"<<max_err_real<<endl;
	//	label++;
	//}
 // 

	//cout<<"���յĽ����"<<"p:"<<parameter1->getp()<<"q:"<<parameter1->getq()<<"m:"<<parameter1->getm()<<"n:"<<parameter1->getn()<<endl;
    return true;
}
bool CSurfaceData::add_horizon_knots()
{

if (!error_compute_ok||kdTree==NULL)
	   return false;	

	 adjust_horizon_knots();

	//cout<<"���յĽ����"<<"p:"<<parameter1->getp()<<"q:"<<parameter1->getq()<<"m:"<<parameter1->getm()<<"n:"<<parameter1->getn()<<endl;
    return true;


}
void CSurfaceData::adjust_horizon_knots()
{
	if(!error_compute_ok)
		return;
	//��ʱ��ʼ
	_LARGE_INTEGER time_start;    /*��ʼʱ��*/
	_LARGE_INTEGER time_over;        /*����ʱ��*/
	double dqFreq;                /*��ʱ��Ƶ��*/
	LARGE_INTEGER f;            /*��ʱ��Ƶ��*/
	QueryPerformanceFrequency(&f);
	dqFreq=(double)f.QuadPart;
	QueryPerformanceCounter(&time_start);
	if(add_horizon_num==0)
	{
		vknots_new.clear();//��ʹ��֮ǰ������
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
		cout<<"����ƽ�����:"<<mean_err<<" "<<"������:"<<maxerr<<"��Ȩ���"<<cur1<<endl;
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
			cout<<"��"<<i+1<<"��ˮƽ��������û�е�����������ڴ������в����ӽڵ���"<<endl;
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
		cout<<"��"<<i+1<<"ˮƽ���ε�ƽ�����:"<<meanerr<<" "<<"������:"<<maxerr.error<<"��Ȩ���"<<cur1<<endl;
		errorvec.push_back(ErrorData(i,cur1));
		horizon_max_index.push_back(maxerr.index);
	}


	//�������������������С���İٷֱ�

	sort(errorvec.begin(),errorvec.end(),greater<ErrorData>());
	double minerror=(*(max_element(errorvec.begin(),errorvec.end(),greater<ErrorData>()))).error;

	for(int i=0;i<errorvec.size()-1;i++)
	{
		double per=(errorvec[i].error-minerror)/minerror*100;
		cout<<"��"<<errorvec[i].index<<"�����������Ϊ��"<<per<<"%"<<endl;
	}
	int sum_=0;
	int i_j=0;
	for(int i=0;i<errorvec.size()-1&sum_<add_horizon_num;i++,i_j++)
	{
#if 0
		
		if (abs(vknots[errorvec[i].index-unum+2]-vknots[errorvec[i].index-unum+1])<=EPSILON2)//����ڵ���֮�������խ�������ӽڵ���
		{
			knotvec.push_back(KnotData(errorvec[i].index,0));
			continue;
		}

		
#endif
		double per=(errorvec[i].error-minerror)/minerror*100;
		if (errorvec[i].error>=1e8)//�쳣ֵ
		{
			knotvec.push_back(KnotData(errorvec[i].index,0));
			continue;
		}
		//����һ��ֻ����һ��

		int n=(add_horizon_num-sum_>=1?1:0);
		knotvec.push_back(KnotData(errorvec[i].index,n));
		sum_+=n;

		//������
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
	//���ø�����Ӧ�����ӵĽڵ��߸���
	knotvec.push_back(KnotData(errorvec[errorvec.size()-1].index,0));//������С�����������ʱ���Žڵ���
	int num_sum=sum_;
	cout<<"�����Ƽ�����"<<num_sum<<"���ڵ���"<<endl;
	while(num_sum<add_horizon_num)
	{
		for (int i=0;i<knotvec.size()&&num_sum<add_horizon_num;i++)
		{
			if (errorvec[i].error>=90000)//�쳣ֵ
			{
				continue;
			}
#if 0
			
		if (abs(vknots[errorvec[i].index-unum+2]-vknots[errorvec[i].index-unum+1])<=EPSILON2)//����ڵ���֮�������խ�������ӽڵ���
		{
			continue;
		}

			
#endif
			knotvec[i].num++;
			num_sum++;
		}
	}
	cout<<"����ʵ������"<<add_horizon_num<<"���ڵ���"<<endl;
	sort(knotvec.begin(),knotvec.end(),less<KnotData>());
	//���ӽڵ���
	vknots_new.clear();//��ʹ��֮ǰ������
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
			if (abs(y1-vknots[j])<=EPSILON2||abs(y1-vknots[1+j])<=EPSILON2)//������ڽڵ����ϣ���ֱ��ƽ������
			{
				y1=(vknots[j]+vknots[j+1])/2;

			}
			vknots_new.push_back(knot_info(0,y1));
			
		}
		else if (knotvec[j].num>1)
		{//������ӵĽڵ��߸�������2�����Ȳ��õ�һ���ڵ���Ϊ��Ȩ�ڵ���
			double y1=0;
			double  err_sum=0;
			for (vector<int>::iterator p=horizon_index[j].begin();p!=horizon_index[j].end();++p)
			{
				Mesh::VertexHandle v=m_pFittedMesh->vertex_handle(*p);
				y1+=m_pFittedMesh->property(verror,v)*m_pFittedMesh->texcoord2D(v)[1];
				err_sum+=m_pFittedMesh->property(verror,v);
			}
			y1/=err_sum;
			if (abs(y1-vknots[j])<=EPSILON2||abs(y1-vknots[j+1])<=EPSILON2)//������ڽڵ����ϣ���ֱ��ƽ������
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
		
			//�ٴ��������Ľڵ���
			TexCoord QP=m_pFittedMesh->texcoord2D(m_pFittedMesh->vertex_handle(horizon_max_index[j]));
			if (QP[1]<y1)//����Ľڵ��߷�����������ڵ��߽��ĵط���ƽ������
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
//��ʱ����
	QueryPerformanceCounter(&time_over); 
	
	cout<<"����ˮƽ�ڵ��ߺ�ʱ:"<<((time_over.QuadPart-time_start.QuadPart)/dqFreq)<<"s"<<endl;//��λΪ�룬����Ϊ1000 000/��cpu��Ƶ��΢��

}
void CSurfaceData::adjust_vertical_knots()
{
	if(!error_compute_ok)
		return;
	//��ʱ��ʼ
	_LARGE_INTEGER time_start;    /*��ʼʱ��*/
	_LARGE_INTEGER time_over;        /*����ʱ��*/
	double dqFreq;                /*��ʱ��Ƶ��*/
	LARGE_INTEGER f;            /*��ʱ��Ƶ��*/
	QueryPerformanceFrequency(&f);
	dqFreq=(double)f.QuadPart;
	QueryPerformanceCounter(&time_start);
	if(add_num==0)
	{
		uknots_new.clear();//��ʹ��֮ǰ������
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
		cout<<"����ƽ�����:"<<mean_err<<" "<<"������:"<<maxerr<<"��Ȩ���"<<cur1<<endl;
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
			cout<<"��"<<i+1<<"��ֱ��������û�е�����������ڴ������в����ӽڵ���"<<endl;
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
		cout<<"��"<<i+1<<"����ֱ���ε�ƽ�����:"<<meanerr<<" "<<"������:"<<maxerr.error<<"��Ȩ���"<<cur1<<endl;
		errorvec.push_back(ErrorData(i,cur1));
		vertical_max_index.push_back(maxerr.index);
	}



	//�������������������С���İٷֱ�

	sort(errorvec.begin(),errorvec.end(),greater<ErrorData>());
	double minerror=(*(max_element(errorvec.begin(),errorvec.end(),greater<ErrorData>()))).error;

	for(int i=0;i<errorvec.size()-1;i++)
	{
		double per=(errorvec[i].error-minerror)/minerror*100;
		cout<<"��"<<errorvec[i].index<<"�����������Ϊ��"<<per<<"%"<<endl;
	}
	int sum_=0;
	int i_j=0;
	for(int i=0;i<errorvec.size()-1&sum_<add_num;i++,i_j++)
	{
#if 0
		
			if (abs(uknots[errorvec[i].index+1]-uknots[errorvec[i].index])<=EPSILON2)//����ڵ���֮�������խ�������ӽڵ���
			{
				knotvec.push_back(KnotData(errorvec[i].index,0));
				continue;
			}
		
	
#endif
		double per=(errorvec[i].error-minerror)/minerror*100;
		if (errorvec[i].error>=1e8)//�쳣ֵ
		{
			knotvec.push_back(KnotData(errorvec[i].index,0));
			continue;
		}
		//����һ��ֻ����һ��

		int n=(add_num-sum_>=1?1:0);
		knotvec.push_back(KnotData(errorvec[i].index,n));
		sum_+=n;

		//������
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
	//���ø�����Ӧ�����ӵĽڵ��߸���
	knotvec.push_back(KnotData(errorvec[errorvec.size()-1].index,0));//������С�����������ʱ���Žڵ���
	int num_sum=sum_;
	cout<<"�����Ƽ�����"<<num_sum<<"���ڵ���"<<endl;
	while(num_sum<add_num)
	{
		for (int i=0;i<knotvec.size()&&num_sum<add_num;i++)
		{
			if (errorvec[i].error>=90000)//�쳣ֵ
			{
				continue;
			}
#if 0
		if (abs(uknots[errorvec[i].index+1]-uknots[errorvec[i].index])<=EPSILON2)//����ڵ���֮�������խ�������ӽڵ���
		{
			continue;
		}
			
#endif
			knotvec[i].num++;
			num_sum++;
		}
	}
	cout<<"����ʵ������"<<add_num<<"���ڵ���"<<endl;
	sort(knotvec.begin(),knotvec.end(),less<KnotData>());
	//���ӽڵ���
	uknots_new.clear();//��ʹ��֮ǰ������
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
			if (abs(x1-uknots[i])<=EPSILON2||abs(x1-uknots[1+i])<=EPSILON2)//������ڽڵ����ϣ���ֱ��ƽ������
			{
				x1=(uknots[i]+uknots[i+1])/2;

			}
			uknots_new.push_back(knot_info(0,x1));
		}
		else if (knotvec[i].num>1)
		{//������ӵĽڵ��߸�������2�����Ȳ��õ�һ���ڵ���Ϊ��Ȩ�ڵ���
			double x1=0;
			double  err_sum=0;
			for (vector<int>::iterator p=vertical_index[i].begin();p!=vertical_index[i].end();++p)
			{
				Mesh::VertexHandle v=m_pFittedMesh->vertex_handle(*p);
				x1+=m_pFittedMesh->property(verror,v)*m_pFittedMesh->texcoord2D(v)[0];
				err_sum+=m_pFittedMesh->property(verror,v);
			}
			x1/=err_sum;
			if (abs(x1-uknots[i])<=EPSILON2||abs(x1-uknots[i+1])<=EPSILON2)//������ڽڵ����ϣ���ֱ��ƽ������
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
		
			//�ٴ��������Ľڵ���
			TexCoord QP=m_pFittedMesh->texcoord2D(m_pFittedMesh->vertex_handle(vertical_max_index[i]));
			if (QP[0]<x1)//����Ľڵ��߷�����������ڵ��߽��ĵط���ƽ������
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
//��ʱ����
	QueryPerformanceCounter(&time_over); 
	cout<<"������ֱ�ڵ��ߺ�ʱ:"<<((time_over.QuadPart-time_start.QuadPart)/dqFreq)<<"s"<<endl;//��λΪ�룬����Ϊ1000 000/��cpu��Ƶ��΢��

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
		return false;//����յĻ�����Ҫ����ԭʼ�������domain�������Ȳ�����domain,�������ǵ�����һ��Ҫ���в���������
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

	QFile file1(files.takeFirst()); //takeFirstȥ����һ����ر�ȥ������
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
		m_pFittedMesh->property(verror,*v_it2)=vec.norm()/max_length;//��һ������������λ��ɵ����
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
	//����ÿ����������ƽ�����
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
	//ɾ�����ܵĽڵ���
	double  epsi=pow(sqrt(double(m_pOriginalMesh->n_vertices())),-1);
	viterator uiter=uknots.begin();
	advance(uiter,1);
	//u[1]ȷ��
	while(*uiter<epsi&&uiter!=uknots.end())
	{
		uiter=uknots.erase(uiter);
	}
	viterator  upre=uiter;
	viterator  ulast=upre+1;
	while(upre!=uknots.end()-1&&ulast!=uknots.end()-1)//upre!=uknots.end()-1���������Ϊ�˷�ֹu����ֻʣ��0��1�ڵ��ߵ������
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
		//�������ڶ���Ԫ��
		if(*ulast-*upre<epsi)
		{
			uknots.erase(upre);
		}
	}

	//����vknots

	viterator viter=vknots.begin();
	advance(viter,1);
	//u[1]ȷ��
	while(*viter<epsi &&viter!=vknots.end())
	{
		viter=vknots.erase(viter);
	}
	viterator  vpre=viter;
	viterator  vlast=vpre+1;
	while(vpre!=vknots.end()-1&&vlast!=vknots.end()-1)//vpre!=vknots.end()-1���������Ϊ�˷�ֹu����ֻʣ��0��1�ڵ��ߵ������
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
		//�������ڶ���Ԫ��
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
	
	/* Retrieves(�һ�) the handle to a named property by it's name.

	/* Parameters:
	_ph	A property handle. On success the handle is valid else invalid.
	_name	Name of wanted property.
	Returns
	true if such a named property is available, else false.
	
	*/
	/************************************************************************/
	 OpenMesh::FPropHandleT<double> f2_mean_curvature;
	bool p_result=m_pOriginalMesh->get_property_handle(f2_mean_curvature,"f_mean_curvature"); //p_resultָ����f2_mean_curvature�Ƿ���Ч


	 //��ʱ��ʼ
	 _LARGE_INTEGER time_start;    /*��ʼʱ��*/
	 _LARGE_INTEGER time_over;        /*����ʱ��*/
	 double dqFreq;                /*��ʱ��Ƶ��*/
	 LARGE_INTEGER f;            /*��ʱ��Ƶ��*/
	 QueryPerformanceFrequency(&f);
	 dqFreq=(double)f.QuadPart;
	 QueryPerformanceCounter(&time_start);

	
	//���ƽ�����ʻ����ܺ�
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
	  cout<<"ƽ�����ʻ���Ϊ:"<<curaverage_<<endl;

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

	  //����ԭʼ�������������
	  OpenMesh::VPropHandleT<Location_Type> v_location; //��ӵ�����
	  m_pOriginalMesh->add_property(v_location,"v_location");

	  OpenMesh::FPropHandleT<Location_Type> f_location; //���������
	  m_pOriginalMesh->add_property(f_location,"f_location");

	  for (v_it=m_pOriginalMesh->vertices_begin();v_it!=v_end;++v_it)
	  {
		  m_pOriginalMesh->property(v_location,*v_it)=unknown;

	  }
	  for (f_it=m_pOriginalMesh->faces_begin(); f_it!=f_end; ++f_it)
	  {
		  m_pOriginalMesh->property(f_location,*f_it)=unknown;
	  }

//-------------------------------------------------------------------- ����

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
   cout<<"��ʼ�Ľ����"<<"p:"<<parameter1->getp()<<"q:"<<parameter1->getq()<<"m:"<<parameter1->getm()<<"n:"<<parameter1->getn()<<endl;
   result<<"��ʼ�Ľ����"<<"p:"<<parameter1->getp()<<"q:"<<parameter1->getq()<<"m:"<<parameter1->getm()<<"n:"<<parameter1->getn()<<endl;
   //��ʱ����
   QueryPerformanceCounter(&time_over); 
   cout<<"���������޶��ڵ��ߺ�ʱ:"<<((time_over.QuadPart-time_start.QuadPart)/dqFreq)<<"s"<<endl;//��λΪ�룬����Ϊ1000 000/��cpu��Ƶ��΢��
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
	//��ʱ��ʼ
	_LARGE_INTEGER time_start;    /*��ʼʱ��*/
	_LARGE_INTEGER time_over;        /*����ʱ��*/
	double dqFreq;                /*��ʱ��Ƶ��*/
	LARGE_INTEGER f;            /*��ʱ��Ƶ��*/
	QueryPerformanceFrequency(&f);
	dqFreq=(double)f.QuadPart;
	QueryPerformanceCounter(&time_start);
	
//���ýڵ�
	vector<double>  temporary_uknot(uknots);//����ɵĽڵ�
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
 
 //���������ڵ����ܵĽڵ��е����
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
	//���������ڵ����ܵĽڵ��е����
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
	if (iter_times!=-1)//һ����������ȡ-1����������Ҫͨ�����ַ�����½ڵ�
	{
	//���ƽ���������ܺ�
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
	cout<<"ƽ��������Ϊ:"<<erroraverage_<<endl;
	//����ԭʼ�������������
	OpenMesh::VPropHandleT<Location_Type> v_location; //��ӵ�����
	m_pFittedMesh->add_property(v_location,"v_location_fit");

	OpenMesh::FPropHandleT<Location_Type> f_location; //���������
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
	//����
	int  i=0;
	while(i<iter_times)
	{
		update_knots2(i);
		i++;
	}
	sort(uknots.begin(),uknots.end());
	sort(vknots.begin(),vknots.end());
//���������ӵĽڵ�λ��,���־ɵĽڵ�λ��

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

	//����¼ӵĽڵ�����ԭ���Ľڵ����غϣ���ȥ��
	vector<knot_info> ni;//���Ӧ��ɾ���Ľڵ�
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
	range_query();//ɾѡ�����ʵĽڵ���
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
	cout<<"���յĽ����"<<"p:"<<parameter1->getp()<<"q:"<<parameter1->getq()<<"m:"<<parameter1->getm()<<"n:"<<parameter1->getn()<<endl;
	result<<"���յĽ����"<<"p:"<<parameter1->getp()<<"q:"<<parameter1->getq()<<"m:"<<parameter1->getm()<<"n:"<<parameter1->getn()<<endl;
//��ʱ����
	QueryPerformanceCounter(&time_over); 
	cout<<"��������޶��ڵ��ߺ�ʱ:"<<((time_over.QuadPart-time_start.QuadPart)/dqFreq)<<"s"<<endl;//��λΪ�룬����Ϊ1000 000/��cpu��Ƶ��΢��
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
			//���˾����潨�����
			polymesh.request_face_colors(); 
			if (!polymesh.has_face_colors())
				return;
			Mesh::FaceIter  f_it,f_end(m_pOriginalMesh->faces_end());
			Mesh::FaceVertexIter fv_it2;

			OpenMesh::FPropHandleT<double> f_area;  //����ÿ����������������
			polymesh.add_property(f_area,"f_area");

			OpenMesh::FPropHandleT<double> f_error;  //����ÿ����������������
			polymesh.add_property(f_error,"f_error");

			OpenMesh::VPropHandleT<Location_Type> v_location; //��ӵ�����
			m_pOriginalMesh->get_property_handle(v_location,"v_location");

			OpenMesh::FPropHandleT<Location_Type> f_location; //���������
			m_pOriginalMesh->get_property_handle(f_location,"f_location");


			for (MyMesh::FaceIter f2_it=polymesh.faces_begin();f2_it!=polymesh.faces_end();++f2_it)
			{
				polymesh.property(f_area,*f2_it)=0.0;
			}

			OpenMesh::FPropHandleT<double> f_mean_curvature2;
			m_pOriginalMesh->get_property_handle(f_mean_curvature2,"f_mean_curvature");

			//�ȶ�ÿ����Ի���
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
				//��ɢ���̣����������������ڲ������Σ���Խ�����Σ��ⲿ������
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
					vector<int>  v1;//����һ�ֵĶ���
					v1.push_back(p.idx());
					int turn=0;
                    vector<int>  v4;//�洢���е��ⲿ�������
					do 
					{
						vector<int> v3;//�洢��һ�ֵĶ������
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
											//�ж�λ��
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
									//�ж������ε�λ��
									assert(outside_sum+boundary_sum+inside_sum==3);
									double t22=m_pOriginalMesh->property(f_mean_curvature2,*vf_it);
									//������������
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
										else //0-2-1,0-1-2��0-0-3
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

					vector<int>  v1;//����һ�ֵĶ���
					v1.push_back(p.idx());

					int turn=0;//���ӵĴ���
					do 
					{
						vector<int> v3;//�洢��һ�ֵĶ������
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
											//�ж�λ��
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
									//�ж������ε�λ��
									assert(outside_sum+boundary_sum+inside_sum==3);
									double t22=m_pOriginalMesh->property(f_mean_curvature2,*vf_it);
									//������������
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

			//������С���ε����
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
					v2 + qRound( ratio * ( v1 - v2 ) ) );//��ȡhsv

				int color[3];
				c.getRgb(&color[0], &color[1], &color[2]);//ת����rgb
				//vcolor[0] = color[0]/255.0;//ת����double
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
	//������������εĽ���
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
	 //���˾����潨�����
//--------------------------------------------------------------------  end 
			Mesh::FaceIter  f_it,f_end(m_pOriginalMesh->faces_end());
			Mesh::FaceVertexIter fv_it2;

			OpenMesh::FPropHandleT<double> f_area;  //����ÿ����������������
			polymesh.add_property(f_area,"f_area");

			OpenMesh::EPropHandleT<double> e_area;  //����ÿ���ߵ����ʻ�������
			polymesh.add_property(e_area,"e_area");

			OpenMesh::VPropHandleT<Location_Type> v_location; //��ӵ�����
			m_pOriginalMesh->get_property_handle(v_location,"v_location");

			OpenMesh::FPropHandleT<Location_Type> f_location; //���������
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


//-------------------------------------------------------------  �ȶ�ÿ����Ի���
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
				//��ɢ���̣����������������ڲ������Σ���Խ�����Σ��ⲿ������
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
					vector<int>  v1;//����һ�ֵĶ���
					v1.push_back(p.idx());
					vector<int>  f_boundary;//�����������ε����
					vector<int>  v4;//�洢���е��ⲿ�������
					int turn=0;
					do 
					{
						vector<int> v3;//�洢��һ�ֵĶ������
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
											//�ж�λ��
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
									//�ж������ε�λ��
									assert(outside_sum+boundary_sum+inside_sum==3);
									double t22=m_pOriginalMesh->property(f_mean_curvature2,*vf_it);
									//������������
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

					//�����������ı߻���
					//ͨ��ʶ��һ�����Ƿ������а���������棬�������Ƿ������������������߻���
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

					vector<int>  v1;//����һ�ֵĶ���
					v1.push_back(p.idx());
					vector<int>  f_boundary;//�����������ε����
					int turn=0;//���ӵĴ���
					do 
					{
						vector<int> v3;//�洢��һ�ֵĶ������
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
											//�ж�λ��
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
									//�ж������ε�λ��
									assert(outside_sum+boundary_sum+inside_sum==3);
									double t22=m_pOriginalMesh->property(f_mean_curvature2,*vf_it);
									//������������
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

					//�����������ı߻���
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

			//���Ŀ��ֵ
			MyMesh::FaceIter face_iter,face_end(polymesh.faces_end());
			double g=0;
			for (face_iter=polymesh.faces_begin();face_iter!=face_end;++face_iter)
			{
				g+=pow(polymesh.property(f_area,*face_iter)-curaverage_,2);
			}
			error_sum.push_back(g);
			std::cout<<"��"<<" "<<iter_num++<<" "<<"�κ���ֵ:"<<g<<" ";

			//׼���ݶ�
			vector<double> uknot_grad(unum-2),vknot_grad(vnum-2);//�ֱ�洢ÿ���ڵ��ߵ��ݶ�
			{
				//������
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

				//�ٱ������еı�
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
			std::cout<<"�ݶȷ���"<<sqrt(gradnorm)<<std::endl;
			//�ڵ����
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
			//���˾����潨�����
			Mesh::FaceIter  f_it,f_end(m_pFittedMesh->faces_end());
			Mesh::FaceVertexIter fv_it2;

			OpenMesh::FPropHandleT<double> f_area;  //����ÿ����������������
			polymesh.add_property(f_area,"f_area");

			OpenMesh::EPropHandleT<double> e_area;  //����ÿ���ߵ����ʻ�������
			polymesh.add_property(e_area,"e_area");

			OpenMesh::VPropHandleT<Location_Type> v_location; //��ӵ�����
			m_pFittedMesh->get_property_handle(v_location,"v_location_fit");

			OpenMesh::FPropHandleT<Location_Type> f_location; //���������
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

			//�ȶ�ÿ����Ի���
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
				//��ɢ���̣����������������ڲ������Σ���Խ�����Σ��ⲿ������
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
					vector<int>  v1;//����һ�ֵĶ���
					v1.push_back(p.idx());
					vector<int>  f_boundary;//�����������ε����
					int turn=0;
					vector<int>  v4;//�洢���е��ⲿ�������
					do 
					{
						vector<int> v3;//�洢��һ�ֵĶ������
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
											//�ж�λ��
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
									//�ж������ε�λ��
									assert(outside_sum+boundary_sum+inside_sum==3);
									double t22=m_pFittedMesh->property(f_mean_error,*vf_it);
									//������������
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

					//�����������ı߻���
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
					vector<int>  v1;//����һ�ֵĶ���
					v1.push_back(p.idx());
					vector<int>  f_boundary;//�����������ε����
					int turn=0;//���ӵĴ���
					do 
					{
						vector<int> v3;//�洢��һ�ֵĶ������
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
											//�ж�λ��
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
									//�ж������ε�λ��
									assert(outside_sum+boundary_sum+inside_sum==3);
									double t22=m_pFittedMesh->property(f_mean_error,*vf_it);
									//������������
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

					//�����������ı߻���
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

			//���Ŀ��ֵ
			MyMesh::FaceIter face_iter,face_end(polymesh.faces_end());
			double g=0;
			for (face_iter=polymesh.faces_begin();face_iter!=face_end;++face_iter)
			{
				g+=pow(polymesh.property(f_area,*face_iter)-erroraverage_,2);
			}
			error_sum.push_back(g);
			std::cout<<"��"<<" "<<iter_num++<<" "<<"�κ���ֵ:"<<g<<" ";

			//׼���ݶ�
			vector<double> uknot_grad(unum-2),vknot_grad(vnum-2);//�ֱ�洢ÿ���ڵ��ߵ��ݶ�
			{
				//������
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

				//�ٱ������еı�
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
			std::cout<<"�ݶȷ���"<<sqrt(gradnorm)<<std::endl;
			//�ڵ����
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
            v2 + qRound( ratio * ( v1 - v2 ) ) );//��ȡhsv

			int color[3];
			c.getRgb(&color[0], &color[1], &color[2]);//ת����rgb
			//vcolor[0] = color[0]/255.0;//ת����double
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


