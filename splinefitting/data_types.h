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




#endif