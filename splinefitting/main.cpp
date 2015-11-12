/*************************************************
* Copyright (c) 2014-至今,厦门大学计算机图形学实验室Graphics@XMU
* All rights reserved.
*
* 文件名称：main.cpp
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

#include <QApplication>
#include <QtGui>
#include "mainwindow.h"
#include <windows.h>
#include <iostream>
using namespace std;

/*! \file
 *  \brief Main entrance for Qt Application.
 *
 *  This file contains main()
 */

/*! Default main. 定义了一个
 *  Qt应用，并且用windows内部函数做了计时.
 */

int main(int argc, char *argv[])
{
	//建立Qt应用
	QApplication app(argc, argv);
	app.setApplicationName("B-spline fitting");

	std::cout.precision(10);
	//std::cout << std::scientific;
	std::cout<<"B-spline fitting begin ..."<<std::endl;

	
	//Timing begins.

	_LARGE_INTEGER time_start;    /* start time. */  
	_LARGE_INTEGER time_over;     /* over time.  */
	double dqFreq;                /* 计时器频率.  */
	LARGE_INTEGER f;              /* 计时器频率.  */
	QueryPerformanceFrequency(&f);
	dqFreq=(double)f.QuadPart;
	QueryPerformanceCounter(&time_start);
	
	//打开主窗口

	mainwindow w; 
	w.show();
	int r=app.exec();

	//Timing over.
	QueryPerformanceCounter(&time_over); 
	cout<<"全程耗时:"<<((time_over.QuadPart-time_start.QuadPart)/dqFreq)<<endl;//单位为秒，精度为1000 000/（cpu主频）微秒
  
	return r;
}
