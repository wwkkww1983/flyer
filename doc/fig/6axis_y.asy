import graph3;
import geometry;
import x11colors;

settings.tex = "xelatex";
usepackage("xeCJK");
texpreamble("\setCJKmainfont{AR PL UKai CN}");

/***************************************** 定义 *****************************************/
/* 尺寸 */ 
real picSize=150;
size(picSize);
/* 半径 */
real r=picSize/1.5;
real perpendicularSize = picSize/20;
/* 原点 */ 
triple O=(0, 0, 0);
triple X=(r, 0, 0);
triple Y=(0, r, 0);
triple Z=(0, 0, r);
/* 观察点 */
triple lookPoint=(r/5, -r, -r/5);
currentprojection = orthographic(lookPoint, up=-Z, target=O);
/* 旋转角 */
real phi=10;
transform3 phiRotate=rotate(phi, O, Y);
real phiCos=cos(phi/180*pi);
real phiSin=sin(phi/180*pi);
real theta1=phi;
real phi1=0;
real theta2=0;
real phi2=0;
/* 重力矢量 */
real gLength=4*r/5;
triple G=(0, 0, gLength);
pen gColor=blue;
/* 原坐标 */
triple X1=(r,0,0);
triple Z1=(0,0,r);
pen  X1Z1Pen=black+dashed+0.1;
/* 旋转后的坐标 */
triple X2=phiRotate * X1;
triple Z2=phiRotate * Z1;
triple GInX2=gLength*phiSin*unit(-X2);
triple GInZ2=gLength*phiCos*unit(Z2);
pen  X2Z2Pen=red+dashed+0.1;
/* 旋转轴 */
triple Y12=(0,r,0);
pen  YPen=DarkCyan+dashed+0.1;

/***************************************** 绘图 *****************************************/
/* 原坐标轴 */
draw(Label("$x$", EndPoint), O--X1,  X1Z1Pen, Arrow3(DefaultHead2));
draw(Label("$z$", EndPoint), O--Z1,  X1Z1Pen, Arrow3(DefaultHead2));
draw(Label("$y,y'$", EndPoint), O--Y12, YPen, Arrow3(DefaultHead2));

/* 不变向量 */
draw(Label("$\mathbf{g}$", EndPoint, NE), O--G, gColor, Arrow3(DefaultHead2));

/* 旋转后的坐标轴 */
draw(Label("$x'$", EndPoint), O--X2,  X2Z2Pen, Arrow3(DefaultHead2));
draw(Label("$z'$", EndPoint), O--Z2,  X2Z2Pen, Arrow3(DefaultHead2));
draw(G--GInX2, X2Z2Pen);
draw(O--GInX2, X2Z2Pen); /* 负半轴 辅助线 */
draw(G--GInZ2, X2Z2Pen);
dot(GInX2, X2Z2Pen);
draw(Label("$x_2$", EndPoint, SE), GInX2, X2Z2Pen);
dot(GInZ2, X2Z2Pen);
draw(Label("$z_2$", EndPoint, NW), GInZ2, X2Z2Pen);

/* 角度弧线 */
draw(Label("$\phi$"), p=YPen, arc(O, r/2, theta1, phi1, theta2, phi2, true), Arrow3(DefaultHead2));

