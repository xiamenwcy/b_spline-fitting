/*************************************************
* Copyright (c) 2014-至今,厦门大学计算机图形学实验室Graphics@XMU
* All rights reserved.
*
* 文件名称：B_parameter.cpp
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

//STL
#include <iostream>
#include <fstream>
#include <iterator>
#include <algorithm>
#include <limits> 

#include "B_parameter.h"

using namespace std;



B_parameter::B_parameter(void)
{
	p=q=m=n=num_u=num_v=0;
	iteration_ok=false;
}

B_parameter::~B_parameter(void)
{
	 uknot.clear();
	 vknot.clear();
}

B_parameter::B_parameter(int p1,int q1,int m1,int n1)
{
	
	p=p1;
	q=q1;
	m=m1;
	n=n1;
	num_u=num_v=0;
	iteration_ok=false;
}
void B_parameter::setnum(int a,int b)
{
	num_u=a;
	num_v=b;
}
bool B_parameter::compute_knots()
{
	if (p>(m+1))
		return false;
	if (q>(n+1))
		return false;

	double du=1.0/(m+1-p);
	double dv=1.0/(n+1-q);
	uknot.resize(num_u);
	vknot.resize(num_v);
	int i,j;
	uknot[0]=0;
	vknot[0]=0;
	double ep=std::numeric_limits<double>::epsilon();
	for(i=1;i<num_u;i++)
	{
		if (i==num_u-1)
		uknot[i]=1.0;
		else
		uknot[i]=uknot[i-1]+du+ep;
	}
	for (j=1;j<num_v;j++)
	{
		if (j==num_v-1)
		vknot[j]=1.0;
		else
		vknot[j]=vknot[j-1]+dv+ep;
	}
	return true;
}
/*
  如果迭代成功，绘制的是迭代后的节点线或者是后来增加的节点线，
  否则，不管是初次初始化还是迭代失败，均绘制初始化的节点线.
*/
bool B_parameter::configure_knots()
{
  if(iteration_ok)
      return true;
  else
  {
	 if(compute_knots())
	     return true;
	 else
		 return false;
  }
}
vector<double> B_parameter::getuknot()
{
	return uknot;
}
vector<double> B_parameter::getvknot()
{
	return vknot;
}
int B_parameter::getp()
{
	return p;
}
int B_parameter::getq()
{
	return q;
}
int B_parameter::getm()
{
	return m;
}
int B_parameter::getn()
{
	return n;
}
int B_parameter::getnum_u()
{
	return num_u;
}
int B_parameter::getnum_v()
{
	return num_v;
}
void B_parameter::setp(int value)
{
	p=value;
	
}
void B_parameter::setq(int value)
{
	q=value;

}
void B_parameter::setm(int value)
{
	m=value;

}
void B_parameter::setn(int value)
{
	n=value;

}
void B_parameter::setuknots(vector<double> uknots)
{
    uknot=uknots;
}
void B_parameter::setvknots(vector<double> vknots)
{
    vknot=vknots;
}