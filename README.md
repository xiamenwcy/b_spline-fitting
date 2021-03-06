﻿# 更新日志
---
- 2015年11月13日 

修复Warning: A paint device can only be painted by one painter at a time.
            办法：在paintevent()里去掉下面的painter->begin(this);因为出现了两次。
            参考：bool QPainter::begin ( QPaintDevice * device )的说明

- 2015年11月14日 

去掉类中无关函数，设置节点线网格初始参数化不可见，待参数化后可见。

- 2015年11月15日 

定义B_parameter ==的函数，用在
  void KnotsViewer::set_knots_view(bool kv) 

-  2015年11月18日 

使用正则表达式准确识别了载入网格文件的后缀名、正确的曲率文件，提出了新的查找矩形区域三角系的方法

- 2015年11月20日

 找到两种方法替代原始的确定矩形区域包含的三角形，以及如何求解面积分和线积分。
  + 遍历Mesh的三角形，以重心落在矩形区域里面确定三角形也落在矩形区域里面，这其中需要用折半查找法确定重心所在的矩形区域。
  + 遍历Mesh的顶点，以顶点落在矩形区域内部，就说明与顶点相关联的所有三角形都落在内部（**需要用到set的概念**），这样就可以确定了矩形区域包含的三角形，然后就可以求解**面积分**。至于**线积分**，也就是两个矩形相交的节点线的积分，我想可以这样：遍历这些非边界边，每条边都有两个半边，每个半边属于一个矩形，这样就找到了这条边相邻的两个矩形，然后计算矩形所包含的三角形的序号的交集(**set_intersection**)，即为与这条边相交的三角形的序号。然后将这些三角形与边求交，取长即可求解线积分。
 *注意：*无论是面积分还是线积分，都带有一些误差，因为有些三角形与矩形相交，但是三角形的三个点都不在矩形中。可以参考如下图片：
  参数设置：
    p=3,q=3,m=7,n=6
    从0开始数，第2个矩形面包含的三角形：
     ![3-3-7-6-2][1]
     
    局部放大图：
     ![][2]

    从0开始数，第3个矩形面包含的三角形：
     ![3-3-7-6-3][3]

    第8条节点线（也就是第2个矩形面和第3个矩形面相交的线段）所关联的三角形：
    ![][4]





  [1]: http://7xohdy.com1.z0.glb.clouddn.com/bspline_img/01/h11.jpg
  [2]: http://7xohdy.com1.z0.glb.clouddn.com/bspline_img/3-3-7-6-2/01/h14.jpg
  [3]:
  http://7xohdy.com1.z0.glb.clouddn.com/bspline_img/01/h1.jpg?imageView2/1/w/406/h/432/q/75
  [4]: http://7xohdy.com1.z0.glb.clouddn.com/01.jpg