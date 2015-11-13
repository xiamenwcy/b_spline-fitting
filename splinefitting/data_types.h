/*************************************************
* Copyright (c) 2014-����,���Ŵ�ѧ�����ͼ��ѧʵ����Graphics@XMU
* All rights reserved.
*
* �ļ����ƣ�data_types.h 
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
/** �洢�����ڵ�Ĵ�С�����ܵĽڵ������е���š�*/
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
	bool operator()( ErrorData &elem1, ErrorData &elem2) const // ���������
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
	bool operator()( KnotData &elem1, KnotData &elem2) const // ���������
	{
		if (elem1.index<elem2.index)
			return true;
		return false;
	}
};


#endif