# 四轴飞行器说明文档
四轴飞行器的电子器件主要分为(飞控动力板)、WiFi无线模块、功能板四部分。
1. 飞控板是飞行器的核心和基础,它有3大功能:姿态控制、运动控制、动力控制
   1.1 姿态控制是飞控板的核心,用于控制飞行器的姿态。
   2.2 运动控制用于控制飞行器的运动,主要包括,前后左右上下旋转
   2.3 动力控制用于放大控制信号驱动空心杯电机
2. wifi命令模块\(STA模式\)用于接收主机命令控制飞控模块,暂时采用ESP8266模块

# 工作计划
1. 3轴gyro,accel裸数据读取,融合(上位机发送融合算法参数,参考pid参数实现,实现融合算法参数设置)
2. 调整pid参数,测试飞行时间
3. 考虑优化内容,制定改板计划
## 优化
1. PCB机翼固定桨的结构
2. 排针过孔绘制尺寸小一点
3. 复位按键考虑MPU9250等外围芯片
4. 分流采样数据:
    a. I2C总线1用于加计陀螺;
    b. I2C总线2用于磁力计,气压计等
    c. 磁力计,气压计原始数据读取(包括代码准备)
5. 是否需要硬件MPU9250倾斜45度(与机翼对齐)
6. 实现绘图的缩放控制(非config.py配置)
7. esp8266链接时,调试器无法连接
## 策略
1. 新设备首先加入到bsp中,bsp中多个设备共有则移入drv目录中实现
## 分析
1. 分析磁力计数据异常的原因

# 展望
## 远期计划
1. 移植cmos,实现多任务并行
2. 改板使用SPI读取传感器,提升采样速度
3. PC手机等周边工具开发
## 优化计划
1. 更换CPU为QFN48(降成本,省空间)
2. ESP8266板载

# 注意:
1. mpu9250的加计容易损坏
2. eMD6.12 DMP:
   a. 仅可6轴或3轴融合,无法9轴融合,
   b. DMP解算不便于滤波,纯C滤波
   c. DMP的3轴陀螺融合算法,不便于加计补偿
3. esp8266模块初始化链接时(蓝灯闪),单片机调试器无法链接,esp8266初始化结束后调试器正常,猜测与电池电压有关
4. esp8266无法低于20ms的采样周期采样

# 参数
## wifi连接
1. 关闭上位机防火墙
2. 上位机:192.168.1.2:8080
## 飞机方向(需要修正)
1. 机头正东向
2. NE1 晶体顺时针方向第1个 顺时针
3. SE2 晶体顺时针方向第2个 逆时针
4. SW1 晶体顺时针方向第3个 顺时针
5. NW4 晶体顺时针方向第4个 逆时针

