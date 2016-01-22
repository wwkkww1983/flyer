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
triple lookPoint=(-r/5, r/5, r/2);
currentprojection = orthographic(lookPoint, up=-Z, target=O);
/* 旋转角 */
real theta=-15; /* FIXME:负号 修复旋转方向有误  */
transform3 thetaRotate=rotate(theta, O, Z);
real thetaCos=cos(theta/180*pi);
real thetaSin=sin(theta/180*pi);
/* 正东矢量 */
real eLength=4*r/5;
triple EPointer=(0, eLength, 0);
pen eColor=blue;
/* 原坐标 */
triple X1=(r,0,0);
triple Y1=(0,r,0);
pen  X1Y1Pen=black+dashed+0.1;
/* 旋转后的坐标 */
triple X2=thetaRotate*X1;
triple Y2=thetaRotate*Y1;
triple EInX2=eLength*thetaSin*unit(X2);
triple EInY2=eLength*thetaCos*unit(Y2);
pen  X2Y2Pen=red+dashed+0.1;

/* 旋转轴 */
pen  ZPen=DarkCyan+dashed+0.1;
triple Z12=(0,0,r);

/***************************************** 绘图 *****************************************/
/* 原坐标轴 */
draw(Label("$x$", EndPoint), O--X1,  X1Y1Pen, Arrow3(DefaultHead2));
draw(Label("$y$", EndPoint), O--Y1, X1Y1Pen, Arrow3(DefaultHead2));
draw(Label("$z,z'$", EndPoint), O--Z12,  ZPen, Arrow3(DefaultHead2));

/* 不变向量 */
draw(Label("$\mathbf{e}$", EndPoint, N), O--EPointer, eColor, Arrow3(DefaultHead2));

/* 旋转后的坐标轴 */
draw(Label("$x'$", EndPoint), O--X2,  X2Y2Pen, Arrow3(DefaultHead2));
draw(Label("$y'$", EndPoint), O--Y2, X2Y2Pen, Arrow3(DefaultHead2));
draw(EPointer--EInX2, X2Y2Pen);
draw(EPointer--EInY2, X2Y2Pen);
draw(O--EInX2, X2Y2Pen); /* 补全X2负半轴 */
dot(EInX2, X2Y2Pen);
draw(Label("$x_2$", EndPoint, NW), EInX2, X2Y2Pen);
dot(EInY2, X2Y2Pen);
draw(Label("$y_2$", EndPoint, SE), EInY2, X2Y2Pen);

/* 角度弧线 */
real theta1 = 90;
real phi1 = 90+theta;
real theta2 = 90;
real phi2 = 90;
draw(Label("$\psi$"), p=X2Y2Pen, arc(O, r/2, theta1, phi1, theta2, phi2), Arrow3(DefaultHead2));

