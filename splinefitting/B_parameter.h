/*************************************************
* Copyright (c) 2014-至今,厦门大学计算机图形学实验室Graphics@XMU
* All rights reserved.
*
* 文件名称：B_parameter.h 
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
#ifndef _BPARAMETER_H_
#define _BPARAMETER_H_

#include <windows.h> 
#include <vector>
using std::vector;

class B_parameter
{
public:
	B_parameter(void);
	~B_parameter(void);
	B_parameter(int p1,int q1,int m1,int n1);
	bool operator ==(B_parameter &c){return (p==c.p)&&(q==c.q)&&(m==c.m)&&(n==c.n);}
	bool compute_knots();
	bool configure_knots();//初始时刻，就是compute_knots,后来迭代时更新节点
	vector<double>  getuknot();
	vector<double>  getvknot();
	void setnum(int a,int b);
	int  getp();
	int  getq();
	int  getm();
	int  getn();
	int  getnum_u();
	int  getnum_v();
	void  setp(int value);
	void  setq(int value);
	void  setm(int value);
	void  setn(int value);
	void  setuknots(vector<double>  uknots);
    void  setvknots(vector<double>  vknots);
	 bool  iteration_ok;//是否迭代
private:
	int p;
	int q;
	int m;
	int n;
	int num_u;
	int num_v;
	vector<double>  uknot;
	vector<double>  vknot;
	
};

//------------------------------------------------------------------------

#endif