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
#include <algorithm>//set_intersection
#include <utility>//pair
#include <fstream>
#include <exception>
#include <set>
#include <iterator> //set_intersection
#include <numeric>  //accumulate


#include <windows.h>
#include <ctime>

#include "SurfaceData.h"




#define _READ_ 1 //ûʲô���ã�����
typedef  vector<int> vecint;
typedef  set<int>  setint;
typedef  vector<vector< OpenMesh::Vec3d> > vectex;
typedef vector<double>::iterator   viterator;
typedef  pair<viterator,viterator>  VIPair;
using namespace std;
using namespace Eigen;



//vector<double>  error_sum;//ÿһ�ε���������
int CSurfaceData::iter_num=1;


CSurfaceData::CSurfaceData(void)
{
	m_pOriginalMesh = NULL; /**< ������ΪNULL���� MeshViewer::draw(), MeshViewer::draw_original_mesh()*/
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
	max_err.error = 1000;//��ʼ��,����void KnotsViewer::error_fitting_view(bool kv)����δ�������ʱʹ��
	max_err.index = -1;

	rangequery_vertical=false;
	rangequery_horizon=false;
	
}


CSurfaceData::~CSurfaceData(void)
{
	 clear_data();
	

		
}
void CSurfaceData::setparameter(B_parameter *parameter_2)
{

	parameter1=parameter_2;
}
void CSurfaceData::set_knots_adding_num(int num)
{
	add_vertical_num=num;
}
void CSurfaceData::set_horizon_knots_addnum(int num)
{
	add_horizon_num=num;
}


void CSurfaceData::vertical_range_query()
{
	//���vertical_index

	vertical_index.clear();
	vertical_index.resize(unum-1);
	OpenMesh::FPropHandleT<setint> v_index;  //����ÿ��������Ķ����������
	bool result=polymesh.get_property_handle(v_index,"v_index");

	if(!rangequery_horizon)  //���ˮƽ�Ѿ����ӹ�v_index���ԣ��Ͳ�����;����Ҫ����
	{
		if (result) //�ҵ�������;��������ڵڶ��κ�ᷢ��
		{
			MyMesh::FaceIter fit,fend(polymesh.faces_end());
			for(fit=polymesh.faces_begin();fit!=fend;++fit)
			{
				polymesh.property(v_index,*fit).clear();
			}
		}
		else      //��������,��������ڵ�һ�η���
		{
			 polymesh.add_property(v_index,"v_index");
			 //---------------------------------------------------------------------------		
		}
		//�������

		Mesh::VertexIter          v_it, v_end(m_pOriginalMesh->vertices_end());
		for (v_it=m_pOriginalMesh->vertices_begin(); v_it!=v_end; ++v_it)  
		{

			TexCoord tx=m_pOriginalMesh->texcoord2D(*v_it);
			int index=location(uknots, vknots,tx);
			if (index==-1)
			{
				throw  std::exception("Point Location Exception!");
				return;
			}
			MyMesh::FaceHandle   fh=polymesh.face_handle(index);
			polymesh.property(v_index,fh).insert((*v_it).idx());

		}
		rangequery_vertical=true;
	}
	else
	{
		rangequery_horizon=false;

	}
	

//-----------------------------------------------------------------------��ѯ
	for (int i=0;i<unum-1;i++)
	{
		for (int j=0;j<vnum-1;j++)
		{
			int index=i+(unum-1)*j;  //����������
		
			MyMesh::FaceHandle f_h=polymesh.face_handle(index);

			setint  tindex=polymesh.property(v_index,f_h);

			copy(tindex.begin(),tindex.end(),insert_iterator<setint>(vertical_index[i],vertical_index[i].begin()));
		}
	
	}



}
void CSurfaceData::horizon_range_query()
{
	//���horizon_index

	horizon_index.clear();

	horizon_index.resize(vnum-1);

	OpenMesh::FPropHandleT<setint> v_index;  //����ÿ��������Ķ����������
	bool result=polymesh.get_property_handle(v_index,"v_index");

	if(!rangequery_vertical) //�����ֱ�Ѿ����ӹ�v_index���ԣ��Ͳ�����;����Ҫ����
	{
		if (result) //�ҵ�������
		{
			MyMesh::FaceIter fit,fend(polymesh.faces_end());
				for(fit=polymesh.faces_begin();fit!=fend;++fit)
				{
					polymesh.property(v_index,*fit).clear();
				}
		}
		else      //��������
		{
			polymesh.add_property(v_index,"v_index");
			
		}
		//---------------------------------------------------------------------------
		//�������

		Mesh::VertexIter          v_it, v_end(m_pOriginalMesh->vertices_end());
		for (v_it=m_pOriginalMesh->vertices_begin(); v_it!=v_end; ++v_it)  
		{

			TexCoord tx=m_pOriginalMesh->texcoord2D(*v_it);
			int index=location(uknots, vknots,tx);
			if (index==-1)
			{
				throw  std::exception("Point Location Exception!");
				return;
			}
			MyMesh::FaceHandle   fh=polymesh.face_handle(index);
			polymesh.property(v_index,fh).insert((*v_it).idx());

		}
		rangequery_horizon=true;
	}
	else
	{
		rangequery_vertical=false;

	}

	//-----------------------------------------------------------------------��ѯ
	for (int i=0;i<vnum-1;i++)
	{
		for (int j=0;j<unum-1;j++)
		{
			int index=(unum-1)*i+j;
		
			MyMesh::FaceHandle f_h=polymesh.face_handle(index);

			setint  tindex=polymesh.property(v_index,f_h);

			copy(tindex.begin(),tindex.end(),insert_iterator<setint>(horizon_index[i],horizon_index[i].begin()));
		}
	}

}

void CSurfaceData::query_isolated_range()
{
	//-----------------------------------------------���ҹ����Ĳ���һ��������ľ����򣬲�ɾ�����ڵĽڵ���
	static int index=0;
	OpenMesh::FPropHandleT<setint> v_index;  //��ȡÿ��������Ķ����������
	bool result=polymesh.get_property_handle(v_index,"v_index");
	MyMesh::FaceIter fi(polymesh.faces_begin()),fend(polymesh.faces_end());
	advance(fi,index);    //��0��ʼ��index����
	for (;fi!=fend;++fi)
	{
		setint result=polymesh.property(v_index,*fi);
		if (result.empty())
		{
			break;
		}
	}
	if (fi==fend)
	{
		query_success=true;
		return;
	}
	  int k=(*fi).idx();      //������ľ��������
	  int j=k/(unum-1);       //����v���������
	  int i=k%(unum-1);       //����u���������
	  bool delete_vertical_knots=(uknots[i+1]-uknots[i])<=(vknots[j+1]-vknots[j]);
	  bool delete_left_down=false;
	  bool delete_right_up=false;
	  int k2;                 //ԭ���������¾���������,Ҳ��δ���µĿ�ʼ��ѯ���
	  if (delete_vertical_knots)   //ɾ��u�ڵ�
	  {
	   if ((i>=1&&((i+2)<=(unum-1))&&(uknots[i]-uknots[i-1])<=(uknots[i+2]-uknots[i+1]))||((i+2)>(unum-1)))
	   {
	         delete_left_down=true;//ɾ����ߵĽڵ���uknots[i]
			 uknots.erase(uknots.begin()+i);
			 k2=k-(j+1);

	   }
	   else
	   {
		   delete_right_up=true;//ɾ���ұߵĽڵ���uknots[i+1]
	        uknots.erase(uknots.begin()+i+1);
		    k2=k-j;
	   }
	  }
	  else
	  {
	   if ((j>=1&&((j+2)<=(vnum-1))&&(vknots[j]-vknots[j-1])<=(vknots[j+2]-vknots[j+1]))||((j+2)>(vnum-1)))
	   {
		   delete_left_down=true;//ɾ���±ߵĽڵ���vknots[j]
	       vknots.erase(vknots.begin()+j);
		   k2=k-unum+1;

	   }
	   else
	   {
		   delete_right_up=true;//ɾ���ϱߵĽڵ���vknots[j+1]
	       vknots.erase(vknots.begin()+j+1);
		   k2=k;
	   }
	  }
	  index=k2;
	  unum=uknots.size();
	  vnum=vknots.size();
	  //��ʱuknots,vknots�������.
//--------------------------------------------------------------------------------------����polymesh2���滻ԭ����polymesh
	MyMesh polymesh2=build_polymesh();
	OpenMesh::FPropHandleT<setint> v_index2;  //����ÿ����������������������
	polymesh2.add_property(v_index2,"v_index");
	   if (delete_vertical_knots)
	   {
		if (delete_left_down)
		{
            MyMesh::FaceIter fi2(polymesh2.faces_begin());
			for (advance(fi2,k2);fi2!=polymesh2.faces_end();++fi2,++fi)
			{
				int k3=(*fi2).idx();
				if (k3==k2)      //��ʼ������
				{
                    fi--;
					setint left1=polymesh.property(v_index,*fi);
					fi++;
                    polymesh2.property(v_index2,*fi2)=left1;

				}
				else if (k3%(unum-1)!=k2%(unum-1))//fi2���ڴ��ϲ����������,��fi2��u���겻��k2���ڵ��������ͬ
				{
					polymesh2.property(v_index2,*fi2)=polymesh.property(v_index,*fi);
				}
				else
				{
					setint left1=polymesh.property(v_index,*fi); 
				    fi++;
					setint right1=polymesh.property(v_index,*fi); 
					setint sum;
					set_union(left1.begin(),left1.end(),right1.begin(),right1.end(),insert_iterator<setint>(sum,sum.begin()));
					polymesh2.property(v_index2,*fi2)=sum;
				}

			}
		}
		else
		{
			MyMesh::FaceIter fi2(polymesh2.faces_begin());
			for (advance(fi2,k2);fi2!=polymesh2.faces_end();++fi2,++fi)
			{
				int k3=(*fi2).idx();
				if (k3==k2)      //��ʼ������
				{
					fi++;
					setint right1=polymesh.property(v_index,*fi);
					polymesh2.property(v_index2,*fi2)=right1;


				}
				else if (k3%(unum-1)!=k2%(unum-1))//fi2���ڴ��ϲ����������,��fi2��u���겻��k2���ڵ��������ͬ
				{
					polymesh2.property(v_index2,*fi2)=polymesh.property(v_index,*fi);
				}
				else
				{
					setint left1=polymesh.property(v_index,*fi); 
					fi++;
					setint right1=polymesh.property(v_index,*fi); 
					setint sum;
					set_union(left1.begin(),left1.end(),right1.begin(),right1.end(),insert_iterator<setint>(sum,sum.begin()));
					polymesh2.property(v_index2,*fi2)=sum;
				}

			}
		
		 }
	   }
	else
	{
		if (delete_left_down)
		{
			MyMesh::FaceIter fi2(polymesh2.faces_begin());
			for (advance(fi2,k2);fi2!=polymesh2.faces_end();++fi2,++fi)
			{
				int k3=(*fi2).idx();
				
			    if (k3/(unum-1)!=k2/(unum-1))//fi2���ڴ��ϲ����������,��fi2��v���겻��k2���ڵ��������ͬ
				{
					polymesh2.property(v_index2,*fi2)=polymesh.property(v_index,*fi);
				}
				else
				{
					MyMesh::FaceIter fdown(polymesh.faces_begin());
					advance(fdown,k3); 
					setint left1=polymesh.property(v_index,*fdown); 
					setint right1=polymesh.property(v_index,*fi); 
					setint sum;
					set_union(left1.begin(),left1.end(),right1.begin(),right1.end(),insert_iterator<setint>(sum,sum.begin()));
					polymesh2.property(v_index2,*fi2)=sum;
				}

			}

		}
		else
		{
			MyMesh::FaceIter fi2(polymesh2.faces_begin());
			for (advance(fi2,k2);fi2!=polymesh2.faces_end();++fi2,++fi)
			{
				int k3=(*fi2).idx();
                if (k3==k2)
                {
					advance(fi,unum-1); 
					setint upper1=polymesh.property(v_index,*fi); 
					polymesh2.property(v_index2,*fi2)=upper1;
                }
				else if (k3/(unum-1)!=k2/(unum-1))//fi2���ڴ��ϲ����������,��fi2��v���겻��k2���ڵ��������ͬ
				{
					polymesh2.property(v_index2,*fi2)=polymesh.property(v_index,*fi);
				}
				else
				{
					setint upper1=polymesh.property(v_index,*fi); 
					MyMesh::FaceIter fdown(polymesh.faces_begin());
					advance(fdown,k3); 
					setint down1=polymesh.property(v_index,*fdown); 
					setint sum;
					set_union(upper1.begin(),upper1.end(),down1.begin(),down1.end(),insert_iterator<setint>(sum,sum.begin()));
					polymesh2.property(v_index2,*fi2)=sum;
				
				}

			}

		}

	}
	polymesh.clear();
	polymesh=polymesh2;


}

bool CSurfaceData::add_vertical_knots()
{

	if (!error_compute_ok)
	   return false;	
	//��ʱ��ʼ
	_LARGE_INTEGER time_start;    /*��ʼʱ��*/
	_LARGE_INTEGER time_over;        /*����ʱ��*/
	double dqFreq;                /*��ʱ��Ƶ��*/
	LARGE_INTEGER f;            /*��ʱ��Ƶ��*/
	QueryPerformanceFrequency(&f);
	dqFreq=(double)f.QuadPart;
	QueryPerformanceCounter(&time_start);
	//--------------------------------------------------------------------------------------------
	vertical_range_query();   //��ѯ��ֱ���ΰ��������������
	//--------------------------------------------------------------ͳ��ÿ�����εļ�Ȩ���
	vector<double> errorvec;
	errorvec.reserve(unum-1);

	OpenMesh::VPropHandleT<double>  verror; 
	m_pFittedMesh->get_property_handle(verror,"verror");

	vector<int> vertical_max_index;
	vertical_max_index.reserve(unum-1);

	for (int i=0;i<unum-1;i++)
	{
		double  meanerr=0;
		int num=vertical_index[i].size();     //ÿ����ֱ���ΰ����ĵ���
		Max_Error maxerr={0,0};

		for (setint::iterator p=vertical_index[i].begin();p!=vertical_index[i].end();++p)
		{
			Mesh::VertexHandle v=m_pFittedMesh->vertex_handle(*p);

			meanerr+=m_pFittedMesh->property(verror,v);

			if(m_pFittedMesh->property(verror,v)>maxerr.error)
			{
				maxerr.error=m_pFittedMesh->property(verror,v);
				maxerr.index=v.idx();
			}
		}
		meanerr/=num;
		double cur1=0.2*meanerr+0.8*maxerr.error;
		cout<<"��"<<i+1<<"����ֱ���ε�ƽ�����:"<<meanerr<<" "<<"������:"<<maxerr.error<<"��Ȩ���"<<cur1<<endl;
		errorvec.push_back(cur1);
		vertical_max_index.push_back(maxerr.index);
	}


	/************************************************************************/
	/* �������ӽڵ��ߵķ��������ڼ�Ȩ���ƽ��ֵ���ϵ���������Ҫ����һ���ڵ���                                                                      */
	/************************************************************************/
	//---------------------------------------------------�����Ȩ����ƽ��ֵ

	double error_avg=accumulate(errorvec.begin(),errorvec.end(),0.)/(errorvec.size());
	double error_max=*max_element(errorvec.begin(),errorvec.end());

	cout<<"������ֱ���εļ�Ȩ���ƽ��ֵΪ:"<<" "<<error_avg<<endl;

	double error_avg2=(error_avg+error_max)/2; 

	cout<<"������ֱ���εļ�Ȩ���1/4��ֵΪ:"<<" "<<error_avg2<<endl;

	//-----------------------------------------------------��ʼ����
	uknots_new.clear();//��ʹ��֮ǰ������

	for (int i=0;i<unum-1;i++)
	{
		if (errorvec[i]>=error_avg2)   //��1/4��ֵ���������ӽڵ���
		{
			double x1=0;
			double  err_sum=0;
			for (setint::iterator p=vertical_index[i].begin();p!=vertical_index[i].end();++p)
			{
				Mesh::VertexHandle v=m_pFittedMesh->vertex_handle(*p);
				x1+=m_pFittedMesh->property(verror,v)*m_pFittedMesh->texcoord2D(v)[0];
				err_sum+=m_pFittedMesh->property(verror,v);
			}
			x1/=err_sum;
			uknots_new.push_back(x1);
		}
	}
	//��ʱ����
	QueryPerformanceCounter(&time_over); 
	cout<<"������ֱ�ڵ��ߺ�ʱ:"<<((time_over.QuadPart-time_start.QuadPart)/dqFreq)<<"s"<<endl;//��λΪ�룬����Ϊ1000 000/��cpu��Ƶ��΢��

    return true;
}
bool CSurfaceData::add_horizon_knots()
{

   if (!error_compute_ok)
	   return false;	
   //��ʱ��ʼ
   _LARGE_INTEGER time_start;    /*��ʼʱ��*/
   _LARGE_INTEGER time_over;        /*����ʱ��*/
   double dqFreq;                /*��ʱ��Ƶ��*/
   LARGE_INTEGER f;            /*��ʱ��Ƶ��*/
   QueryPerformanceFrequency(&f);
   dqFreq=(double)f.QuadPart;
   QueryPerformanceCounter(&time_start);
   //---------------------------------------------------------------------------------------------
   horizon_range_query();  //��ѯˮƽ���ΰ��������������
   //--------------------------------------------------------------ͳ��ÿ�����εļ�Ȩ���
   vector<double > errorvec;
   errorvec.reserve(vnum-1);

   OpenMesh::VPropHandleT<double>  verror; 
   m_pFittedMesh->get_property_handle(verror,"verror");

   vector<int>  horizon_max_index;
   horizon_max_index.reserve(vnum-1);

   for (int i=0;i<vnum-1;i++)
   {
	   double  meanerr=0;
	   int num=horizon_index[i].size(); //ÿ��ˮƽ���ΰ����ĵ���
	   Max_Error maxerr={0,0};

	   for (setint::iterator p=horizon_index[i].begin();p!=horizon_index[i].end();++p)
	   {
		   Mesh::VertexHandle v=m_pFittedMesh->vertex_handle(*p);

		   meanerr+=m_pFittedMesh->property(verror,v);

		   if(m_pFittedMesh->property(verror,v)>maxerr.error)
		   {
			   maxerr.error=m_pFittedMesh->property(verror,v);
			   maxerr.index=v.idx();
		   }
	   }
	   meanerr/=num;
	   double cur1=0.2*meanerr+0.8*maxerr.error;
	   cout<<"��"<<i+1<<"ˮƽ���ε�ƽ�����:"<<meanerr<<" "<<"������:"<<maxerr.error<<"��Ȩ���"<<cur1<<endl;
	   errorvec.push_back(cur1);
	   horizon_max_index.push_back(maxerr.index);
   }

   /************************************************************************/
   /* �������ӽڵ��ߵķ��������ڼ�Ȩ���ƽ��ֵ���ϵ�ˮƽ��������Ҫ����һ���ڵ���                                                                      */
   /************************************************************************/
   //---------------------------------------------------�����Ȩ����ƽ��ֵ

   double error_avg=accumulate(errorvec.begin(),errorvec.end(),0.)/(errorvec.size());

   double error_max=*max_element(errorvec.begin(),errorvec.end());

   cout<<"����ˮƽ���εļ�Ȩ���ƽ��ֵΪ:"<<" "<<error_avg<<endl;

   double error_avg2=(error_avg+error_max)/2; 

   cout<<"����ˮƽ���εļ�Ȩ���1/4��ֵΪ:"<<" "<<error_avg2<<endl;

   //-----------------------------------------------------��ʼ����
   vknots_new.clear();//��ʹ��֮ǰ������

   for (int j=0;j<vnum-1;j++)
   {

	   if (errorvec[j]>error_avg2)   //��ƽ�����������ӽڵ���
	   {
		   double y1=0;
		   double  err_sum=0;
		   for (setint::iterator p=horizon_index[j].begin();p!=horizon_index[j].end();++p)
		   {
			   Mesh::VertexHandle v=m_pFittedMesh->vertex_handle(*p);
			   y1+=m_pFittedMesh->property(verror,v)*m_pFittedMesh->texcoord2D(v)[1];
			   err_sum+=m_pFittedMesh->property(verror,v);
		   }
		   y1/=err_sum;
		   vknots_new.push_back(y1);

	   }

   }
   //��ʱ����
   QueryPerformanceCounter(&time_over); 

   cout<<"����ˮƽ�ڵ��ߺ�ʱ:"<<((time_over.QuadPart-time_start.QuadPart)/dqFreq)<<"s"<<endl;//��λΪ�룬����Ϊ1000 000/��cpu��Ƶ��΢��

    return true;


}


B_parameter* CSurfaceData::getparameter()
{
	return parameter1;
}

void CSurfaceData::clear_data()
{
	if(m_pOriginalMesh!=NULL)
	{
		m_pOriginalMesh->clear();
		delete m_pOriginalMesh;
		m_pOriginalMesh = NULL;
	}
	if(m_pFittedMesh!=NULL) //m_pFittedMesh�������������ģ�������bsurface����ģ���Ӧ��������
	{
		m_pFittedMesh->clear();
		m_pFittedMesh = NULL;
	}
	if(m_pErrorMesh!=NULL)
	{
		m_pErrorMesh->clear();
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
QString  CSurfaceData::get_filename()
{
	return fileName;
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
	/************************************************************************/
	/* �������ְ취��ȡ��׺����
	    ��һ�ְ취��ȡ�����ұ�4���ַ������ַ����������У���Ϊһ����ļ���ʽ��
	   .off,.obj,.stl...����4���ַ������ǲ�³����һ�����������ĸ��ַ��ĺ�׺���ͻ�������⡣
	   �ڶ��ַ���ʹ����������ʽ��ȡ�˺�׺���������򵥡���ݡ���Ч��
	 */
	/************************************************************************/
	//����һ
	// Returns a substring that contains the n rightmost characters of the string.
	   //QString extension = fileName.right(4);
    //������
	//ʹ��������ʽƥ���ļ���

	QRegExp rx("[^/]+(\\..+)"); 
	 int pos=rx.indexIn(fileName);
	 if (-1==pos)
	 {
		 return false; //ƥ��ʧ��
	 }
    QString extension=rx.cap(1);

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
		return sample_num;
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
	//OpenMesh::FPropHandleT<double> f_mean_error;
	//m_pFittedMesh->add_property(f_mean_error,"f_mean_error");
	//m_pFittedMesh->property(f_mean_error).set_persistent(true);

	//Mesh::FaceIter fit = m_pFittedMesh->faces_begin();
	//Mesh::FaceVertexIter    fv_it;
	//for(; fit!=m_pFittedMesh->faces_end(); ++fit)
	//{ 
	//	fv_it=m_pFittedMesh->fv_iter( *fit );
	//	m_pFittedMesh->property(f_mean_error,*fit)=m_pFittedMesh->property(verror,*fv_it);
	//	++fv_it;
	//	m_pFittedMesh->property(f_mean_error,*fit)+=m_pFittedMesh->property(verror,*fv_it);
	//	++fv_it;
	//	m_pFittedMesh->property(f_mean_error,*fit)+=m_pFittedMesh->property(verror,*fv_it);
	//	m_pFittedMesh->property(f_mean_error,*fit)/=3;

	//}
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
	//-------------------------------------------------------------ɾ�����ܵĽڵ���
	//------------------------------------------------------����uknots
	double  epsi=/*(sample_num<10000?1:2)**/pow(sqrt(double(sample_num)),-1);
	viterator uiter=uknots.begin();
	advance(uiter,1);
	//u[1]ȷ��
	while(uiter!=uknots.end()&&*uiter<epsi)
	{
		uiter=uknots.erase(uiter);
	}
	viterator  upre=uiter;     //�õĽڵ���
	viterator  ulast=upre+1;   //�����Ľڵ���
	while(upre!=uknots.end()-1&&ulast!=uknots.end()-1)//��������upre��ulast���ڣ�0,1���������
	{
		while(ulast!=uknots.end()-1 && *ulast-*upre<epsi )
		{
			double utemp=(*upre+*ulast)/2;
			upre=uknots.erase(upre);
			upre=uknots.erase(upre);
			upre=uknots.insert(upre,utemp);
			ulast=upre+1;
		}
		if (ulast!=uknots.end()-1) 
		{
			upre=ulast; //ulast��Ϊ�õĽڵ���
			ulast=upre+1;
		}

	}

	if(upre!=uknots.end()-1) //��������upre���ڵ����ڶ���Ԫ�ص����
	{ 
		assert(ulast==uknots.end()-1);
		//�������ڶ���Ԫ��
		if(*ulast-*upre<epsi)
		{
			uknots.erase(upre);
		}
	}

	//--------------------------------------------------------����vknots

	viterator viter=vknots.begin();
	advance(viter,1);
	//v[1]ȷ��
	while(viter!=vknots.end()&& *viter<epsi)
	{
		viter=vknots.erase(viter);
	}
	viterator  vpre=viter;
	viterator  vlast=vpre+1;
	while(vpre!=vknots.end()-1&&vlast!=vknots.end()-1)//��������vpre��vlast���ڣ�0,1���������
	{
		while(vlast!=vknots.end()-1 && *vlast-*vpre<epsi)
		{
			double vtemp=(*vpre+*vlast)/2;
			vpre=vknots.erase(vpre);
			vpre=vknots.erase(vpre);
			vpre=vknots.insert(vpre,vtemp);
			vlast=vpre+1;
		}
		if (vlast!=vknots.end()-1)
		{
			vpre=vlast; //vlast��Ϊ�õĽڵ���
			vlast=vpre+1;
		}

	}

	if(vpre!=vknots.end()-1) //��������vpre���ڵ����ڶ���Ԫ�ص����
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
	unum=uknots.size();
	vnum=vknots.size();
	//------------------------------------------------------�ҵ�����һ��������ľ������򣬲�ɾ�����ڵĽڵ���
	update_polymesh_and_query_point();
	query_success=false;
	while(!query_success)
	{
		query_isolated_range();
	}
}
bool   CSurfaceData::adjust_knots_by_curvature()
{
	add_horizon_num=1;
	add_vertical_num=1;

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

	 //�������������ʻ�������
	 OpenMesh::FPropHandleT<double> tri_integral; //������������������
	 m_pOriginalMesh->add_property(tri_integral,"tri_integral"); 

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
		  double integral=ts2*abs(CGAL::area(Point_2(A[0],A[1]),Point_2(B[0],B[1]),Point_2(C[0],C[1])));
		  m_pOriginalMesh->property(tri_integral,*f_it)=integral;
		  sum+=integral;
	  }
	  double num3=(unum-1)*(vnum-1);
	  double  fa=sum/num3;
	  curaverage_=fa;
	  cout<<"ƽ�����ʻ���Ϊ:"<<curaverage_<<endl;

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

	  parameter1->setnum(uknots.size(),vknots.size());
	  parameter1->setm(uknots.size()+parameter1->getp()-2);
	  parameter1->setn(vknots.size()+parameter1->getq()-2);
	  parameter1->setuknots(uknots);
	  parameter1->setvknots(vknots);
	  //-------------------------------------------------------------������ʻ��ֵ�������ʾ��knotsSubWindow��
	  update_polymesh();
	  parameter1->iteration_ok=true;

      cout<<"��ʼ�Ľ����"<<"p:"<<parameter1->getp()<<"q:"<<parameter1->getq()<<"m:"<<parameter1->getm()<<"n:"<<parameter1->getn()<<endl;

   //��ʱ����
    QueryPerformanceCounter(&time_over); 
    cout<<"���������޶��ڵ��ߺ�ʱ:"<<((time_over.QuadPart-time_start.QuadPart)/dqFreq)<<"s"<<endl;//��λΪ�룬����Ϊ1000 000/��cpu��Ƶ��΢��
	return true;
}
bool CSurfaceData::adjust_knots_by_fitting_error()
{
	add_horizon_num=1;
	add_vertical_num=1;
	if (m_pFittedMesh==NULL||parameter1==NULL)
		return false;

	//��ʱ��ʼ
	_LARGE_INTEGER time_start;    /*��ʼʱ��*/
	_LARGE_INTEGER time_over;        /*����ʱ��*/
	double dqFreq;                /*��ʱ��Ƶ��*/
	LARGE_INTEGER f;            /*��ʱ��Ƶ��*/
	QueryPerformanceFrequency(&f);
	dqFreq=(double)f.QuadPart;
	QueryPerformanceCounter(&time_start);
	
	//-------------------------------------�����µĽڵ���
	if (uknots_new.empty()&&vknots_new.empty())
	{
		cout<<"û���µĽڵ��߿������"<<endl;
		return false;
	}
	vector<double>::iterator knot_iter;
	for (knot_iter=uknots_new.begin();knot_iter!=uknots_new.end();++knot_iter)
	{
		uknots.push_back(*knot_iter);
	}
    for (knot_iter=vknots_new.begin();knot_iter!=vknots_new.end();++knot_iter)
    {
		vknots.push_back(*knot_iter);
    }

	sort(uknots.begin(),uknots.end());
	sort(vknots.begin(),vknots.end());
	unum=uknots.size();
	vnum=vknots.size();

	//����֮ǰ�����Ľڵ��ߣ���Ϊ����û���ˡ�
	uknots_new.clear();
	vknots_new.clear(); 

    modification();  //������еĽڵ���


	parameter1->setnum(uknots.size(),vknots.size());
	parameter1->setm(uknots.size()+parameter1->getp()-2);
	parameter1->setn(vknots.size()+parameter1->getq()-2);
	parameter1->setuknots(uknots);
	parameter1->setvknots(vknots);
	parameter1->iteration_ok=true;

	cout<<"���յĽ����"<<"p:"<<parameter1->getp()<<"q:"<<parameter1->getq()<<"m:"<<parameter1->getm()<<"n:"<<parameter1->getn()<<endl;

//��ʱ����
	QueryPerformanceCounter(&time_over); 
	cout<<"��������޶��ڵ��ߺ�ʱ:"<<((time_over.QuadPart-time_start.QuadPart)/dqFreq)<<"s"<<endl;//��λΪ�룬����Ϊ1000 000/��cpu��Ƶ��΢��
	return true;

}

void CSurfaceData::update_polymesh()
{
	
			update_polymesh_and_query_tri();
			//---------------------------------------------------------------------------------------  end 
			polymesh.request_face_colors(); 
			if (!polymesh.has_face_colors())
				return;

			OpenMesh::FPropHandleT<double> f_area;  //����ÿ����������������
			polymesh.add_property(f_area,"f_area");

			OpenMesh::FPropHandleT<double> f_error;  //����ÿ����������������
			polymesh.add_property(f_error,"f_error");

			for (MyMesh::FaceIter f2_it=polymesh.faces_begin();f2_it!=polymesh.faces_end();++f2_it)
			{
				polymesh.property(f_area,*f2_it)=0.0;
			}

			OpenMesh::FPropHandleT<setint> f_index;  //����ÿ����������������������
			polymesh.get_property_handle(f_index,"f_index");


			OpenMesh::FPropHandleT<double> tri_integral; //��ȡ���������������
			m_pOriginalMesh->get_property_handle(tri_integral,"tri_integral"); 

			
			//-------------------------------------------------------------------------------------------------
			//���������
			MyMesh::FaceIter f_it,f_end(polymesh.faces_end());
			for (f_it=polymesh.faces_begin();f_it!=f_end;++f_it)
			{
				setint tri_set=polymesh.property(f_index,*f_it);
				for (setint::iterator s_it=tri_set.begin();s_it!=tri_set.end();++s_it)
				{
					Mesh::FaceHandle  fh3=m_pOriginalMesh->face_handle(*s_it);
					polymesh.property(f_area,*f_it)+= m_pOriginalMesh->property(tri_integral,fh3);

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
//�������κ��߶ε��ཻ���
bool CSurfaceData::intersection_st( Triangle_2 tri,Segment_2 seg,double *length)
{
//------------------------------------------------------------��һ�ַ���
	/*CGAL::Object result;
	Point_2 ipoint;
	Segment_2 iseg;

	result = CGAL::intersection(seg, tri);
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
		*length=0;
		return false;
	}*/
//-------------------------------------------------------------end
//-------------------------------------------------------------�ڶ��ַ���
	CGAL::Object result;
	const Point_2* ipoint;
	const Segment_2*  iseg;

	if (CGAL::do_intersect(seg,tri))
	{
		result = CGAL::intersection(seg, tri);
		if (ipoint = CGAL::object_cast<Point_2>( &result))
			*length=0;
		else if (iseg = CGAL::object_cast<Segment_2>( &result))
			*length=sqrt(iseg->squared_length());
		return true;

	} else
	{
		*length=0;
		return false;
	}

//-------------------------------------------------------------end
}


inline int CSurfaceData::location(vector<double>& uknots,vector<double>& vknots,TexCoord& p)
{
	if (uknots.empty() || vknots.empty())
	{
		return -1;
	}
	int unum = uknots.size();

	vector<double>::iterator ui = lower_bound(uknots.begin(), uknots.end(),p[0]);//���ҵ�һ�����ڵ���p[0]��ֵ
	int i = distance(uknots.begin(), ui);
	i =( i == 0 ? 0 : i - 1);
	vector<double>::iterator vi = lower_bound(vknots.begin(), vknots.end(),p[1]);//���ҵ�һ�����ڵ���p[1]��ֵ
	int j = distance(vknots.begin(), vi);
	j= (j == 0 ? 0 : j - 1);
	int local = j*(unum - 1) + i;
	return local;


}
MyMesh CSurfaceData::build_polymesh()
{
	MyMesh polymesh;
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
	return polymesh;


}
void    CSurfaceData::update_polymesh_and_query_point()
{
//-----------------------------------------------------update_polymesh
	        polymesh.clear();
            polymesh=build_polymesh();
//---------------------------------------------------------------------------------------  end 

			OpenMesh::FPropHandleT<setint> v_index;  //����ÿ��������Ķ����������
			polymesh.add_property(v_index,"v_index");
	
   //---------------------------------------------------------------------------
			//����λ�Ͳ�ѯ

			Mesh::VertexIter          v_it, v_end(m_pOriginalMesh->vertices_end());
			for (v_it=m_pOriginalMesh->vertices_begin(); v_it!=v_end; ++v_it)  
			{

				TexCoord tx=m_pOriginalMesh->texcoord2D(*v_it);
                int index=location(uknots, vknots,tx);
			    if (index==-1)
				   {
				   	   throw  std::exception("Point Location Exception!");
				   	   return;
				    }
		       MyMesh::FaceHandle   fh=polymesh.face_handle(index);
               polymesh.property(v_index,fh).insert((*v_it).idx());
               
			}
		
		
}

void   CSurfaceData::update_polymesh_and_query_tri()
{
//-----------------------------------------------------update_polymesh
	        polymesh.clear();
            polymesh=build_polymesh();
//---------------------------------------------------------------------------------------  end 

			OpenMesh::FPropHandleT<setint> f_index;  //����ÿ����������������������
			polymesh.add_property(f_index,"f_index");
	
   //---------------------------------------------------------------------------
			//����λ�Ͳ�ѯ

			Mesh::VertexIter          v_it, v_end(m_pOriginalMesh->vertices_end());
			for (v_it=m_pOriginalMesh->vertices_begin(); v_it!=v_end; ++v_it)  
			{

				TexCoord tx=m_pOriginalMesh->texcoord2D(*v_it);
                int index=location(uknots, vknots,tx);
			    if (index==-1)
				   {
				   	   throw  std::exception("Point Location Exception!");
				   	   return;
				    }
		       MyMesh::FaceHandle   fh=polymesh.face_handle(index);
			   Mesh::VertexFaceIter vf_it;
			   for ( vf_it=m_pOriginalMesh->vf_iter(*v_it); vf_it.is_valid(); ++vf_it)
			   {
                  polymesh.property(f_index,fh).insert((*vf_it).idx());
			   }
               
			}
		
		/*	MyMesh::FaceHandle   fh3=polymesh.face_handle(7);
			test2=polymesh.property(f_index,fh3);
			MyMesh::FaceHandle   fh4=polymesh.face_handle(8);
			test3=polymesh.property(f_index,fh4);*/

}
void  CSurfaceData::update_knots(int k)
{
//-------------------------------------------------------------------------------------begin build polymesh
         //����polymesh������ѯ�������������������

           	sort(uknots.begin(),uknots.end());
	        sort(vknots.begin(),vknots.end());

	        update_polymesh_and_query_tri();
//---------------------------------------------------------------------------------------  end 

			OpenMesh::FPropHandleT<double> f_area;  //����ÿ����������������
			polymesh.add_property(f_area,"f_area");

			OpenMesh::EPropHandleT<double> e_area;  //����ÿ���ߵ����ʻ�������
			polymesh.add_property(e_area,"e_area");

			/*for (MyMesh::EdgeIter  ed=polymesh.edges_begin();ed!=polymesh.edges_end();++ed)
			{
				polymesh.property(e_area,*ed)=0.0;
			}*/

			for (MyMesh::FaceIter f2_it=polymesh.faces_begin();f2_it!=polymesh.faces_end();++f2_it)
			{
				polymesh.property(f_area,*f2_it)=0.0;
			}

			OpenMesh::FPropHandleT<setint> f_index;  //����ÿ����������������������
			polymesh.get_property_handle(f_index,"f_index");

			OpenMesh::FPropHandleT<double> f_mean_curvature2; //��ȡ��������
			m_pOriginalMesh->get_property_handle(f_mean_curvature2,"f_mean_curvature");

			OpenMesh::FPropHandleT<double> tri_integral; //��ȡ���������������
			m_pOriginalMesh->get_property_handle(tri_integral,"tri_integral"); 
		
//-------------------------------------------------------------------------------------------------
               //��������ֺ��߻���
			MyMesh::FaceIter f_it,f_end(polymesh.faces_end());
			for (f_it=polymesh.faces_begin();f_it!=f_end;++f_it)
			{
				setint tri_set=polymesh.property(f_index,*f_it);
				for (setint::iterator s_it=tri_set.begin();s_it!=tri_set.end();++s_it)
				{
					Mesh::FaceHandle  fh3=m_pOriginalMesh->face_handle(*s_it);
				   polymesh.property(f_area,*f_it)+= m_pOriginalMesh->property(tri_integral,fh3);

				}
			}
			//�����߻���
             MyMesh::EdgeIter e_it,e_end(polymesh.edges_end());
			 for (e_it=polymesh.edges_begin();e_it!=polymesh.edges_end();++e_it)
			 {
				 if (!polymesh.is_boundary(*e_it))
 				{
 					MyMesh::HalfedgeHandle  he1=polymesh.halfedge_handle(*e_it,0);
					MyMesh::HalfedgeHandle  he2=polymesh.halfedge_handle(*e_it,1);
 					MyMesh::FaceHandle   f1=polymesh.face_handle(he1);
					MyMesh::FaceHandle   f2=polymesh.face_handle(he2);
					setint c;
                    setint l_set=polymesh.property(f_index,f1);
					setint r_set=polymesh.property(f_index,f2);
					set_intersection(l_set.begin(),l_set.end(),r_set.begin(),r_set.end(),insert_iterator<setint>(c,c.begin()));
                 /*  if((*e_it).idx()==21)
				   {
					test1=c;
				   }*/
					//�ཻ�����е������ο�Խ��������������������ּӶ��ˣ���ȥһ��ոպ�
					double sums=0.,avg;
					for (setint::iterator iter=c.begin();iter!=c.end();++iter)
                    {
						Mesh::FaceHandle  fh3=m_pOriginalMesh->face_handle(*iter);
						double t23=m_pOriginalMesh->property(f_mean_curvature2,fh3);
						sums+=t23;
						/*Mesh::FaceVertexIter  fv_it2;
						TexCoord A,B,C;
						fv_it2=m_pOriginalMesh->fv_iter(fh3);
						A=m_pOriginalMesh->texcoord2D(*fv_it2);
						++fv_it2;
						B=m_pOriginalMesh->texcoord2D(*fv_it2);
						++fv_it2;
						C=m_pOriginalMesh->texcoord2D(*fv_it2);*/
						double  result=m_pOriginalMesh->property(tri_integral,fh3)/2; //���ֵ�һ��
						polymesh.property(f_area,f1)-= result;   
						polymesh.property(f_area,f2)-= result;  


						/*Triangle_2  tri(Point_2 (A[0],A[1]),Point_2 (B[0],B[1]),Point_2 (C[0],C[1]));
						MyMesh::VertexHandle     fv=polymesh.from_vertex_handle(he1),tv=polymesh.to_vertex_handle(he1);
						MyMesh::Point     p1=polymesh.point(fv),p2=polymesh.point(tv);
						double length=0;
						bool is=intersection_st(tri,Segment_2(Point_2(p1[0],p1[1]),Point_2(p2[0],p2[1])),&length);*/
					/*	assert(is==true);
						if (is)
						{
							assert(length!=0.);
							polymesh.property(e_area,*e_it)+=t23*length;
						}*/
                    }
					avg=sums/c.size();
					MyMesh::VertexHandle     fv=polymesh.from_vertex_handle(he1),tv=polymesh.to_vertex_handle(he1);
					MyMesh::Point     p1=polymesh.point(fv),p2=polymesh.point(tv);
					double length= (p1-p2).length(); 
					polymesh.property(e_area,*e_it)=avg*length;

 					
 				}

			 }
	/*		 static int nu=1;
			 
       if (nu==1)
	   {
		   nu++;*/
			//���Ŀ��ֵ
			MyMesh::FaceIter face_iter,face_end(polymesh.faces_end());
			double g=0;
			for (face_iter=polymesh.faces_begin();face_iter!=face_end;++face_iter)
			{
				g+=pow(polymesh.property(f_area,*face_iter)-curaverage_,2);
			}
		/*	error_sum.push_back(g);*/
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
				double  dis2=uh*(uknot_grad[i-1]==0.?0.:uknot_grad[i-1]/abs(uknot_grad[i-1]));
				uknots[i]-=dis2;
			}
			for (int j=1;j<=vnum-2;j++)
			{
				double  dis2=vh*(vknot_grad[j-1]==0.?0.:vknot_grad[j-1]/abs(vknot_grad[j-1]));
				vknots[j]-=dis2;

			}
  /*  }*/
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


