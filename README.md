A renderer written in C++11.

#main features

自定义矩阵类（带移动构造，高斯约旦求逆）  
三角形光栅化绘制 triangle rasterization  
深度测试 z-buffer  
变换矩阵MVP  
透视正确插值 perspective-correct interpolation  
齐次空间裁剪 homogeneous clipping  
阴影映射 shadow mapping
背面剔除 back-face culling
切线空间法线映射 tangent space normal mapping
Blinn-Phong shading
PBR shading
Image-based lighting
Shader Interface

#preview

PBR
![preview](https://github.com/aihara-mei/Project_Renderer/raw/master/img/pbr.png?raw=true)

Blinn-Phong
![preview](https://github.com/aihara-mei/Project_Renderer/raw/master/img/blin-phong.png?raw=true)

shadow mapping
![preview](https://github.com/aihara-mei/Project_Renderer/raw/master/img/shadowmap.png?raw=true)

透视不正确插值
![preview](https://github.com/aihara-mei/Project_Renderer/raw/master/img/before_correct.png?raw=true)

透视正确的插值
![preview](https://github.com/aihara-mei/Project_Renderer/raw/master/img/after_correct.png?raw=true)

视锥体直接剔除
![preview](https://github.com/aihara-mei/Project_Renderer/raw/master/img/culling.png?raw=true)

视锥体裁剪
![preview](https://github.com/aihara-mei/Project_Renderer/raw/master/img/clipping.png?raw=true)

#Acknowledgment

gun model from Dennis Haupt (3dhaupt): https://free3d.com/3d-model/sniper-rifle-ksr-29-new-34178.html  