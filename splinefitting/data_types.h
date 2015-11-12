/*************************************************
* Copyright (c) 2014-至今,厦门大学计算机图形学实验室Graphics@XMU
* All rights reserved.
*
* 文件名称：data_types.h 
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

#ifndef _DATA_TYPES_
#define _DATA_TYPES_

#include <functional>
//#include <CGAL/Cartesian.h>
//#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
//#include <CGAL/Polygon_2_algorithms.h>
////#include "enriched_polyhedron.h"
// #include <vector>
//#include <list>
//
//#define EPSILON 10e-16
//
//typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
//typedef K::Point_2                                          Point_2;
//typedef K::Point_3                                          Point_3;
//typedef K::Vector_2                                         Vector_2;
//typedef K::Vector_3                                         Vector_3;
//typedef CGAL::Cartesian<double>  Kernel3;
////typedef Enriched_polyhedron<Kernel3,Enriched_items> Polyhedron;
//
//using namespace std;
//
//typedef struct _Principal_Direction
//{
//	Point_2 pt;
//	Vector_2 direction;
//}Principal_Direction;
//
//typedef struct _Edge_Cost
//{
//	unsigned index1;
//	unsigned index2;
//	double Cost1;
//	double Cost2;
//}Edge_Cost;
//
//typedef struct _knot
//{
//	Point_2 pt;
//	Point_3 vertex;
//	bool is_border;
//	double density;
//}Knot;

enum Mesh_Type
{
	Curvature_Mesh,
	Density_Mesh,
	Fitted_Mesh,
	Error_Mesh,
	Control_Edges,
	Control_Mesh,
	Para_Mesh,
	None,
};
//
//struct CV_Correspondence
//{
//	int centroid_id;
//	int basis_id;
//};
//
//struct Edge
//{
//	unsigned first;
//	unsigned second;
//};
//
//class Index_Density
//{
//public:
//	int index;
//	double density;
//	bool operator<(const Index_Density &id)
//	{
//		return density>id.density;
//	}
//};
//
//class ID_Vec
//{
//public:
//	void  sort()
//	{
//		std::sort(vec.begin(), vec.end());
//	}
//
//	vector<Index_Density> vec;
//};
//
//typedef struct BarycenterCoordinate_
//{
//	double bc[3];
//}BarycenterCoordinate;
//
//typedef struct _Insert_Knot
//{
//	double total_error;
//	Point_2 pt;
//	Point_3 vertex;
//	double curvature;
//	vector<unsigned> ori_triangle;
//	vector<unsigned> triangle;
//	bool is_border;
//}Insert_Knot;
//
//struct Knot_Data
//{
//	Point_2 pt;
//	Point_3 vertex;
//	bool is_border;
//	unsigned index;
//	unsigned nn_id;
//	double curvature;
//	Vector_3 normal;
//	bool is_feature;
//	double color[3];
//};
//
//struct High_Density_Data
//{
//	unsigned index;
//	Point_2 pt;
//	double density;
//};
//
struct Max_Error
{
	double error;
	int index;
};
class knot_info//存储新增节点的大小和在总的节点序列中的序号。
{
public:
	int index;
	double knot;
	knot_info(int i,double k)
	{
		index=i;
		knot=k;
	}
 bool operator==(const knot_info& rhs)const
 {
	 if (index==rhs.index)
	 {
		 return true;
	 }
	 else
		 return false;
 }

};

class ErrorData
{
public:
	int index;
	double error;	
	ErrorData(int index_,double error_):index(index_),error(error_){}
};
template<> struct std::greater<ErrorData>
{
	bool operator()( ErrorData &elem1, ErrorData &elem2) const // 重载运算符
	{
		if (elem1.error>elem2.error)
			return true;
		return false;
	}
};

class KnotData
{
public:
	int index;
	int  num;	
	KnotData(int index_,int num_):index(index_),num(num_){}

};
template<> struct std::less<KnotData>
{
	bool operator()( KnotData &elem1, KnotData &elem2) const // 重载运算符
	{
		if (elem1.index<elem2.index)
			return true;
		return false;
	}
};
//struct PointNormal
//{
//	Point_3 point;
//	Vector_3 normal;
//};
//
//struct BoundaryInteriorConfigs
//{
//	vector<unsigned> boundary;
//	vector<unsigned> interior;
//};
//
//typedef struct Link_
//{
//	list<unsigned> sortEdges;
//	list<pair<unsigned, unsigned>> edges;
//	vector<unsigned> interior;
//}Link;
//
//bool isClosed(Link &link)
//{
//	list<unsigned>::iterator start = link.sortEdges.begin();
//	list<unsigned>::iterator last = link.sortEdges.end();
//	last--;
//	if (*start==*last)
//		return true;
//	return false;
//}
//
//bool isSingular(Link &link)
//{
//	if (link.sortEdges.empty())
//		return true;
//	
//	list<unsigned>::iterator iter = link.sortEdges.begin();
//	list<unsigned>::iterator last = link.sortEdges.end();
//	last--;
//	if (link.sortEdges.size()==3 && *iter==*last)
//	{
//		return true;
//	}
//	return false;
//}
//
//int findLinks(vector<Link> &allLinks, vector<unsigned> &interior)
//{
//	for (int i=0; i<allLinks.size(); i++)
//	{
//		int eqncount =0;
//		for (int j=0; j<interior.size(); j++)
//		{
//			if (interior[j]==allLinks[i].interior[j])
//			{
//				eqncount++;
//			}
//		}
//		if(eqncount==interior.size())
//			return i;
//	}
//	return -1;
//}
//
//struct linkofVertices
//{
//	unsigned first;
//	unsigned second;
//	linkofVertices *prev;
//	linkofVertices *next;
//};
//
//class CentroidTriangulation
//{
//public:
//	Point_2 pt;
//	vector<unsigned> indices;
//	bool operator ==(const CentroidTriangulation &ct) const
//	{
//		for(int i=0; i<indices.size(); i++)
//		{
//			if(indices[i]!=ct.indices[i])
//				return false;
//		}
//		return true;
//	}
//	bool operator <(const CentroidTriangulation &ct) const
//	{
//		for(int i=0; i<indices.size(); i++)
//		{
//			if(indices[i]<ct.indices[i])
//				return true;
//		}
//		return false;
//	}
//};
//
//struct Facet
//{
//	unsigned first;
//	unsigned second;
//	unsigned third;
//};
//
//struct SplineBasis
//{
//	vector<unsigned> interior;
//	vector<vector<unsigned>> boundaries;//collection all the boundaries;
//	vector<unsigned> indices;//record the positions of nonzeros
//	unsigned nPoints_inside;
//	double *basis;
//	bool hasCombinated;
//	bool isBoundaryCorner;
//	/*double *uderivate;
//	double *vderivate;*/
//};
//
//enum Triangulation_type
//{
//	Delaunay,
//	DDT_Mean_Error,
//	DDT_SCO,
//	DDT_Variance,
//};
//
//enum SimulatedAnnealing
//{
//	OneStep,
//	SA,
//	Lop,
//	Combination,
//};
//
//class Edges
//{
//public:
//	Edges(){}
//	unsigned first;
//	unsigned second;
//	void sort()
//	{
//		if(first>second)
//		{
//			unsigned temp = first;
//			first = second;
//			second = first;
//		}
//	}
//	bool operator()(const Edges one, const Edges two)
//	{
//		if(one.first==two.first)
//			return one.second < two.second;
//		return one.first < two.second;
//	}
//	bool operator <(const Edges edge)
//	{
//		if(first==edge.first)
//			return second < edge.second;
//		return first < edge.first;
//	}
//	bool operator ==(const Edges edge)
//	{
//		return first == edge.first && second == edge.second;
//	}
//};
//
//typedef list<linkofVertices> llink;
//typedef llink::iterator link_iter;
//
//class CLink
//{
//public:
//	CLink();
//	CLink(list<linkofVertices> &srclinks);
//	~CLink(){}
//	void preprocessing();//delete the oppostie edges
//	bool sort();
//	void insert(linkofVertices &lk);
//	int size();
//	link_iter &begin();
//	link_iter end();
//private:
//	list<linkofVertices> links;
//	link_iter it;
//};
//
//
////typedef struct ErrorInfo_
////{
////	double error;
////	double sq; //shapequality
////}ErrorInfo;
//
//typedef struct PtInTriangle_
//{
//	Point_3 pt;
//	double error;
//}PtInTriangle;
//
//
//typedef struct FaceInfo_
//{
//	vector<PtInTriangle> pts;
//	unsigned ids[3];
//	Vector_2 normal;
//	double error;
//	double sq; //shapequality
//	bool flag;
//}FaceInfo;
//
//class Edge_IDs
//{
//public:
//	Edge_IDs(unsigned id1_, unsigned id2_)
//	{
//		id1 = id1_;
//		id2 = id2_;
//	}
//	unsigned id1;
//	unsigned id2;
//	bool operator < (const Edge_IDs &edge) const
//	{
//		if(id1<edge.id1)
//			return true;
//		else
//		{
//			if(id1>edge.id1)
//				return false;
//			else
//				return id2<edge.id2;
//		}
//	} 
//};
//
//template < class Gt, class Vb = CGAL::Triangulation_vertex_base_2<Gt> >
//class SimplexSpline_vertex_base : public Vb
//{
//	typedef Vb Base;
//public:
//	typedef typename Vb::Vertex_handle Vertex_handle;
//	typedef typename Vb::Face_handle Face_handle;
//	typedef typename Vb::Point Point;
//	template < typename TDS2 >
//	struct Rebind_TDS 
//	{
//		typedef typename Vb::template Rebind_TDS<TDS2>::Other Vb2;
//		typedef SimplexSpline_vertex_base<Gt,Vb2> Other;
//	};
//private:
//	unsigned index;
//	double density;
//public:
//	SimplexSpline_vertex_base() : Base() {}
//	SimplexSpline_vertex_base(const Point & p) : Base(p) {}
//	SimplexSpline_vertex_base(const Point & p, Face_handle f) : Base(f,p) {}
//	SimplexSpline_vertex_base(Face_handle f) : Base(f) {}
//	void set_associated_index(unsigned ind) { index = ind;}
//	unsigned get_associated_index() { return index; }
//	void set_density(double d){density = d;}
//	double get_density(){return density;}
//};
//
//struct EdgeInfo
//{
//	int Id;
//	double cost;
//	bool operator<(const EdgeInfo &einf) const
//	{
//		return cost < einf.cost;
//	}
//};
//
//struct FeaturePointInfo
//{
//	double x;
//	double y;
//	double z;
//	int eID;
//};
//
//struct FeatureLineInfo
//{
//	double alpha;
//	double beta;
//	double gamma;
//};
//
//struct EFInfo
//{
//	int vId1;
//	int vId2;
//	int fId;
//};
//
//struct FeaturesInfo
//{
//	FeaturePointInfo *fps;
//	int fpNum;
//	FeatureLineInfo *fvs;
//	int lNum;
//	EFInfo *eFids;
//	int fFNum;
//};
//
//struct Error_Index
//{
//	double error;
//	unsigned index;
//};
//
//class SortError
//{
//public:
//	bool operator()(Error_Index a, Error_Index b) const
//	{
//		return a.error > b.error;
//	}
//};


//bool computeBarycenterCoordinates(Polyhedron *mesh, int index, Point_2 &pt, vector<Point_3> &pts, vector<double> &barycoords)
//{
//	bool bFind = false;
//	Polyhedron::Vertex_iterator vit= mesh->vertices_begin();
//	std::advance(vit, index);
//	Polyhedron::Halfedge_around_vertex_circulator pHalfEdge = vit->vertex_begin(), start = pHalfEdge;
//
//	do 
//	{
//		Polyhedron::Facet_handle f = pHalfEdge->facet();
//		if (f==NULL)
//			continue;
//
//		Polyhedron::Halfedge_around_facet_circulator he = f->facet_begin(), hs = he;
//		vector<Point_2> pts_2;
//		do 
//		{
//			pts_2.push_back(Point_2(he->vertex()->vtx, he->vertex()->vty));
//		} while (++he!=hs);
//
//		int ret = CGAL::bounded_side_2(pts_2.begin(), pts_2.end(), pt, K());
//		if(ret!=CGAL::ON_UNBOUNDED_SIDE)
//		{
//			Point_3 pt3 = Point_3(0, 0, 0);
//			double area = CGAL::area(pts_2[0], pts_2[1], pts_2[2]);
//
//			double area0 = CGAL::area(pts_2[1], pts_2[2], pt);
//			double area1 = CGAL::area(pts_2[2], pts_2[0], pt);
//			double barycoord[3];
//			barycoord[0] = area0/area;
//			barycoord[1] = area1/area;
//			barycoord[2] = 1-barycoord[0]-barycoord[1];
//			for (int i=0; i<3; i++)
//			{
//				barycoords.push_back(barycoord[i]);
//			}
//			Polyhedron::Halfedge_around_facet_circulator he = f->facet_begin(), hs = he;
//			do 
//			{
//				pts.push_back(Point_3(he->vertex()->point().x(), he->vertex()->point().y(), he->vertex()->point().z()));
//			} while (++he!=hs);
//
//			bFind = true;
//			break;
//		}
//	} while (++pHalfEdge!=start);
//	return bFind;
//}
//
//bool computeBarycenterCoordinates(Polyhedron *mesh, int index, Point_2 &pt, vector<Polyhedron::Vertex_handle> &vertices, vector<double> &barycoords)
//{
//	bool bFind = false;
//	Polyhedron::Vertex_iterator vit= mesh->vertices_begin();
//	std::advance(vit, index);
//	Polyhedron::Halfedge_around_vertex_circulator pHalfEdge = vit->vertex_begin(), start = pHalfEdge;
//
//	do 
//	{
//		Polyhedron::Facet_handle f = pHalfEdge->facet();
//		if (f==NULL)
//			continue;
//
//		Polyhedron::Halfedge_around_facet_circulator he = f->facet_begin(), hs = he;
//		vector<Point_2> pts_2;
//		do 
//		{
//			pts_2.push_back(Point_2(he->vertex()->vtx, he->vertex()->vty));
//		} while (++he!=hs);
//
//		int ret = CGAL::bounded_side_2(pts_2.begin(), pts_2.end(), pt, K());
//		if(ret!=CGAL::ON_UNBOUNDED_SIDE)
//		{
//			Point_3 pt3 = Point_3(0, 0, 0);
//			double area = CGAL::area(pts_2[0], pts_2[1], pts_2[2]);
//
//			double area0 = CGAL::area(pts_2[1], pts_2[2], pt);
//			double area1 = CGAL::area(pts_2[2], pts_2[0], pt);
//			double barycoord[3];
//			barycoord[0] = area0/area;
//			barycoord[1] = area1/area;
//			barycoord[2] = 1-barycoord[0]-barycoord[1];
//			for (int i=0; i<3; i++)
//			{
//				barycoords.push_back(barycoord[i]);
//			}
//			Polyhedron::Halfedge_around_facet_circulator he = f->facet_begin(), hs = he;
//			do 
//			{
//				vertices.push_back(he->vertex());
//			} while (++he!=hs);
//
//			bFind = true;
//			break;
//		}
//	} while (++pHalfEdge!=start);
//	return bFind;
//}
//
//void computeBarycenterCoordinates(vector<Point_2> &pts, Point_2 &pt, BarycenterCoordinate &bc)
//{
//	double area = CGAL::area(pts[0], pts[1], pts[2]);
//	double area0 = CGAL::area(pts[1], pts[2], pt);
//	double area1 = CGAL::area(pts[0], pt, pts[2]);
//	bc.bc[0] = area0/area;
//	bc.bc[1] = area1/area;
//	bc.bc[2] = 1- bc.bc[0]-bc.bc[1];
//}


#endif