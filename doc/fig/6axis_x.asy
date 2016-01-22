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
triple lookPoint=(-r, r/5, -r/5);
currentprojection = orthographic(lookPoint, up=-Z, target=O);
/* 旋转角 */
real theta=10;
transform3 thetaRotate=rotate(theta, O, X);
real thetaCos=cos(theta/180*pi);
real thetaSin=sin(theta/180*pi);
real theta1 = -theta;
real phi1 = 90;
real theta2 = 0;
real phi2 = 90;
/* 重力矢量 */
real gLength=4*r/5;
triple G=(0, 0, gLength);
pen gColor=blue;
/* 原坐标 */
triple Y1=(0,r,0);
triple Z1=(0,0,r);
pen  Y1Z1Pen=black+dashed+0.1;
/* 旋转后的坐标 */
triple Y2=thetaRotate * Y1;
triple Z2=thetaRotate * Z1;
triple GInY2=gLength*thetaSin*unit(Y2);
triple GInZ2=gLength*thetaCos*unit(Z2);
pen  Y2Z2Pen=red+dashed+0.1;

/* 旋转轴 */
pen  XPen=DarkCyan+dashed+0.1;
triple X12=(r,0,0);

/***************************************** 绘图 *****************************************/
/* 原坐标轴 */
draw(Label("$x,x'$", EndPoint), O--X12, XPen, Arrow3(DefaultHead2));
draw(Label("$y$", EndPoint), O--Y1,  Y1Z1Pen, Arrow3(DefaultHead2));
draw(Label("$z$", EndPoint), O--Z1,  Y1Z1Pen, Arrow3(DefaultHead2));

/* 不变向量 */
draw(Label("$\mathbf{g}$", EndPoint, NE), O--G, gColor, Arrow3(DefaultHead2));

/* 旋转后的坐标轴 */
draw(Label("$y'$", EndPoint), O--Y2,  Y2Z2Pen, Arrow3(DefaultHead2));
draw(Label("$z'$", EndPoint), O--Z2,  Y2Z2Pen, Arrow3(DefaultHead2));
draw(G--GInY2, Y2Z2Pen);
draw(G--GInZ2, Y2Z2Pen);
dot(GInY2, Y2Z2Pen);
draw(Label("$y_2$", EndPoint, SE), GInY2, Y2Z2Pen);
dot(GInZ2, Y2Z2Pen);
draw(Label("$z_2$", EndPoint, NW), GInZ2, Y2Z2Pen);

/* 角度弧线 */
draw(Label("$\theta$"), p=XPen, arc(O, r/2, theta1, phi1, theta2, phi2), Arrow3(DefaultHead2));

