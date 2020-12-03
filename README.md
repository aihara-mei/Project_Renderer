A renderer written in C++11.

#main features

自定义矩阵类（带移动构造，高斯约旦求逆）  
三角形光栅化绘制 triangle rasterization  
深度测试 z-buffer  
变换矩阵MVP  
透视正确插值 perspective-correct interpolation  
齐次空间裁剪 homogeneous clipping  

#preview

透视不正确插值
![preview](https://github.com/aihara-mei/Project_Renderer/raw/master/img/before_correct.png?raw=true)

透视正确的插值
![preview](https://github.com/aihara-mei/Project_Renderer/raw/master/img/after_correct.png?raw=true)

视锥体直接剔除
![preview](https://github.com/aihara-mei/Project_Renderer/raw/master/img/culling.png?raw=true)

视锥体裁剪
![preview](https://github.com/aihara-mei/Project_Renderer/raw/master/img/clipping.png?raw=true)
