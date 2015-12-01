# 更新日志
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

- 2015年11月19日

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
- 2015年11月21日
  + 修复错误 
  void  CSurfaceData::update_knots(int k)中计算面积分和线积分 
  polymesh.property(f_area,*f_it)+= m_pOriginalMesh->property(f_mean_curvature2,*f_it)*Area;改为
  polymesh.property(f_area,*f_it)+= m_pOriginalMesh->property(f_mean_curvature2,fh3)*Area;
  + 增加tri_integral属性，直接计算原始网格的三角形面积分,并相应修改update_knots()函数
  + 增加函数： void   build_polymesh_with_query(MyMesh& polymesh_new);
  与  MyMesh build_polymesh();使得原来在update_knots中建立和查询polymesh的功能分离开来。
  + 去掉GetFirstk，改在location中使用lower_bound(),简化代码
- 2015年11月23日
  + 修复错误void CSurfaceData::clear_data()中m_pFittedMesh的删除;
   CBSplineSurfaceView ::~CBSplineSurfaceView()添加m_pFittedMesh = NULL;
  + 增加函数 void  query_isolated_range(); 
  + void mainwindow::input_parameter()中添加推荐m,n值功能，在输入m,n的时候。主要依据网格点数多少   
- 2015年11月24日
  + 修改线性方程组的系数矩阵的填充方法，改为先填充三元组，再填充稀疏矩阵
  + 改造void mainwindow::adjust_knots_by_fitting_error()，去掉迭代，改为直接添加合适的节点线
  + 增加函数void  vertical_range_query(); 与 void  horizon_range_query(); 
  + 去除kd tree.
- 2015年11月30日
  + 改openmesh中迭代器返回句柄的方法由 ``iter.handle()---->*+iter``
  + 将拟合过程中的SparseQR方法改为SPQR方法，后者由SuiteSparse package支持，安装方法见http://blog.csdn.net/xiamentingtao/article/details/50100549
  + ``bool CBSplineSurfaceView::solvecontrolpoint(Mesh *mesh)`` 中改 
  ``tripletList.reserve((m+1)*(n+1)*p_num);``为``tripletList.reserve(p_num);``否则后者随着m,n放大，会超过vector支持的最大分配内存容量.最好的方法是不加这句话，改用C++11的emplace_back代替push_back 
- 2015年12月1日
  + 改造线积分
  + 使用vs2008的Release的exe运行更快







  [1]: http://7xohdy.com1.z0.glb.clouddn.com/bspline_img/01/h11.jpg
  [2]: http://7xohdy.com1.z0.glb.clouddn.com/bspline_img/3-3-7-6-2/01/h14.jpg
  [3]:
  http://7xohdy.com1.z0.glb.clouddn.com/bspline_img/01/h1.jpg?imageView2/1/w/406/h/432/q/75
  [4]: http://7xohdy.com1.z0.glb.clouddn.com/01.jpg