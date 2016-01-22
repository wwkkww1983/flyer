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
triple lookPoint=(r/10, r/5, r);
currentprojection = orthographic(lookPoint, up=Y, target=O);
/* 旋转角 */
real theta=10;
transform3 thetaRotate=rotate(theta, O, Z);
real thetaCos=cos(theta/180*pi);
real thetaSin=sin(theta/180*pi);
real theta1 = 90;
real phi1 = 0;
real theta2 = 90;
real phi2 = theta;
pen  rotateAxisPen=red+dashed+0.1;
/* 不变矢量 */
real pLength=r;
real pAngle=45;
real pCos=cos(pAngle/180*pi);
real pSin=sin(pAngle/180*pi);
triple P=(pCos*pLength, pSin*pLength, 0);
pen pPen=blue;
/* 原坐标 */
triple X1=(r,0,0);
triple Y1=(0,r,0);
triple PInX1=(P.x,0,0);
triple PInY1=(0,P.y,0);
pen  X1Z1Pen=black+dashed+0.1;
/* 旋转后的坐标 */
triple X2=thetaRotate * X1;
triple Y2=thetaRotate * Y1;
triple PInX2=(P.x*thetaCos+P.y*thetaSin)*unit(X2);
triple PInY2=(-P.x*thetaSin+P.y*thetaCos)*unit(Y2);
pen  X2Y2Pen=red+dashed+0.1;

/* 旋转轴 */
pen  Z1Z2Pen=DarkCyan+dashed+0.1;
triple Z1=(0,0,r);
triple Zk=(0,0,r/2);

/***************************************** 绘图 *****************************************/
/* 原坐标轴 */
draw(Label("$x$", EndPoint), O--X1, X1Z1Pen, Arrow3(DefaultHead2));
draw(Label("$y$", EndPoint), O--Y1, X1Z1Pen, Arrow3(DefaultHead2));
/* 投影*/
draw(P--PInX1, X1Z1Pen);
draw(P--PInY1, X1Z1Pen);
dot(PInX1, X1Z1Pen);
draw(Label("$x_1$" ,EndPoint, SE), PInX1, X1Z1Pen, Arrow3(DefaultHead2));
dot(PInY1, X1Z1Pen);
draw(Label("$y_1$", EndPoint, NW), PInY1, X1Z1Pen, Arrow3(DefaultHead2));

/* 旋转后的坐标轴 */
draw(Label("$x'$", EndPoint), O--X2, X2Y2Pen, Arrow3(DefaultHead2));
draw(Label("$y'$", EndPoint), O--Y2, X2Y2Pen, Arrow3(DefaultHead2));
/* 投影*/
draw(P--PInX2, X2Y2Pen);
draw(P--PInY2, X2Y2Pen);
dot(PInX2, X2Y2Pen);
draw(Label("$x_2$", EndPoint, SE), PInX2, X2Y2Pen, Arrow3(DefaultHead2));
dot(PInY2, X2Y2Pen);
draw(Label("$y_2$", EndPoint, NW), PInY2, X2Y2Pen, Arrow3(DefaultHead2));

/* 旋转轴 */
draw(Label("$z,z'$", EndPoint), O--Z1, Z1Z2Pen, Arrow3(DefaultHead2));
draw(Label("$k,k'$", EndPoint, align=E), O--Zk, Z1Z2Pen, Arrow3(DefaultHead2));

/* 不变向量 */
draw(Label("$\mathbf{p}$", EndPoint), O--P, pPen, Arrow3(DefaultHead2));
/* 角度弧线 */
draw("$\psi$", p=rotateAxisPen, arc(O, r/2, theta1, phi1, theta2, phi2), Arrow3(DefaultHead2));

