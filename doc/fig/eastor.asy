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
triple lookPoint=(-15, -30, -15);
currentprojection = orthographic(lookPoint, up=-Z, target=O);
/* 重力矢量 */
triple G=(0, 0, 4*r/5);
pen GColor=red;
/* 磁矢量 */
real mLength=4*r/5;
real mAngle=30;
real mZ=-sin(mAngle/180*pi)*mLength;
real mX=cos(mAngle/180*pi)*mLength;
triple M=(mX, 0, mZ);
pen MColor=green;
/* 角度弧 */
real theta1 = 0;
real phi1 = 0;
real theta2 = mAngle + 90;
real phi2 = 0;
/* 指东针 */
triple EPointer=(0, 4*r/5, 0);
pen EColor=blue;
/* 坐标轴 */
pen  XYZPen=gray+dashed+0.1;

/***************************************** 绘图 *****************************************/
/* 原坐标轴 */
draw(Label("$x$", EndPoint), O--X, XYZPen, Arrow3(DefaultHead2));
draw(Label("$y$", EndPoint), O--Y, XYZPen, Arrow3(DefaultHead2));
draw(Label("$z$", EndPoint), O--Z, XYZPen, Arrow3(DefaultHead2));

/* 重力矢量 */
draw(Label("$\mathbf{g}$", EndPoint, E), O--G, GColor, Arrow3(DefaultHead2));

/* 磁矢量 */
draw(Label("$\mathbf{m}$", EndPoint, SW), O--M, MColor, Arrow3(DefaultHead2));

/* 指东针 */
draw(Label("$\mathbf{e}$", EndPoint, E), O--EPointer, EColor, Arrow3(DefaultHead2));

/* 角度弧线 */
draw("$\theta$", p=black, arc(O, r/2, theta1, phi1, theta2, phi2), Arrow3(DefaultHead2));

