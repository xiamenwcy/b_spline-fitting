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




#endif