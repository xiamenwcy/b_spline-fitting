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

/** Mesh type */
enum Mesh_Type
{
	Curvature_Mesh,   /**<  curvature mesh */
	Fitted_Mesh,      /**<  fitted mesh  */
	Error_Mesh,       /**<  fitted error mesh */
	None              /**<  other */
};

struct Max_Error
{
	double error;
	int index;
};
/** 存储新增节点的大小和在总的节点序列中的序号。*/
class knot_info
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


#endif