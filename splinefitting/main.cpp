/*************************************************
* Copyright (c) 2014-����,���Ŵ�ѧ�����ͼ��ѧʵ����Graphics@XMU
* All rights reserved.
*
* �ļ����ƣ�main.cpp
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

/*! Default main. ������һ��
 *  QtӦ�ã�������windows�ڲ��������˼�ʱ.
 */

int main(int argc, char *argv[])
{
	//����QtӦ��
	QApplication app(argc, argv);
	app.setApplicationName("B-spline fitting");

	std::cout.precision(10);
	//std::cout << std::scientific;
	std::cout<<"B-spline fitting begin ..."<<std::endl;

	
	//Timing begins.

	_LARGE_INTEGER time_start;    /* start time. */  
	_LARGE_INTEGER time_over;     /* over time.  */
	double dqFreq;                /* ��ʱ��Ƶ��.  */
	LARGE_INTEGER f;              /* ��ʱ��Ƶ��.  */
	QueryPerformanceFrequency(&f);
	dqFreq=(double)f.QuadPart;
	QueryPerformanceCounter(&time_start);
	
	//��������

	mainwindow w; 
	w.show();
	int r=app.exec();

	//Timing over.
	QueryPerformanceCounter(&time_over); 
	cout<<"ȫ�̺�ʱ:"<<((time_over.QuadPart-time_start.QuadPart)/dqFreq)<<endl;//��λΪ�룬����Ϊ1000 000/��cpu��Ƶ��΢��
  
	return r;
}
