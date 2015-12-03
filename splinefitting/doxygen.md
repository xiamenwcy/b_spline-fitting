B spline fitting                            {#mainpage}
=================

介绍
------------

本项目始于2014年9月1日，终于2015年12月3日。主要使用B样条曲面去拟合一个现有的三维网格曲面。由于B样条曲面有几个参数：
U,V节点，控制顶点，参数化。我们的文章固定了参数化，转而自适应地设置节点线，开始依据曲率，使得节点线依照曲率分布，后来依照误差添加
节点线，最终使曲面拟合到我们需要的精度。

使用方法
---------------------

本程序使用C++编写，使用Qt4做界面，中间使用了CGAL求解三角形与线段的交集，使用Eigen+suitsparse求解稀疏方程组([Eigen+suitsparse安装指南](http://blog.csdn.net/xiamentingtao/article/details/50100549))
，使用openmesh作为三角网格的半边数据结构库，使用了libqglviewer库用于三维几何体的显示等，所以需要安装这些库，才可以使用。

输入三维数据
---------------------

三维网格和相应的曲率文件，见百度云盘：[http://pan.baidu.com/s/1kTAQTgV](http://pan.baidu.com/s/1kTAQTgV)

操作视频
---------------------

见百度云盘:[http://pan.baidu.com/s/1bnoM7SV](http://pan.baidu.com/s/1bnoM7SV)


