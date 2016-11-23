# 四轴飞行器说明文档
四轴飞行器的电子器件主要分为(飞控动力板)、WiFi无线模块、功能板四部分。
1. 飞控板是飞行器的核心和基础,它有3大功能:姿态控制、运动控制、动力控制
   1.1 姿态控制是飞控板的核心,用于控制飞行器的姿态。
   2.2 运动控制用于控制飞行器的运动,主要包括,前后左右上下旋转
   2.3 动力控制用于放大控制信号驱动空心杯电机
2. wifi命令模块\(STA模式\)用于接收主机命令控制飞控模块,暂时采用ESP8266模块

# 工作计划
0. 实现三轴融合(陀螺),加计数据上传
1. flyer下位机while(TRUE)改为复位
2. 焊接新板子(重量分布均衡),集成测试(测试飞行时间) 
3. 制定改板计划

## 优化
1. 移植cmos,实现多任务并行
2. 使用SPI读取传感器
3. 硬件MPU9250倾斜45度(与机翼对齐)
4. 捕获控件中将串口接收监控/解析等代码封装到一个管理器类
## 策略
1. bsp中多个设备共有则移入drv目录中实现
## 分析
1. 分析磁力计数据异常的原因

# 参数
## 计划测试
1. 四820+55ma电池压降,(测试esp8266输入电压 )
2. 使用720空心杯配55mm桨电流/电压/拉力
## 已测试
1. cpu裸电流                        1.4ma
2. 单820+55mm电流                   1.7a
3. 四820+55ma电池压降               0.5v(4.05v下降到3.5v)

# 展望
## 远期计划
1. PC手机等周边工具开发
2. 设计并实现pid算法时间
3. 使用实物测试pid四元数求PWM值算法
## 优化计划
1. 更换CPU为QFN48(降成本,省空间)
2. ESP8266板载
# 注意:
1. eMD6.12 DMP仅可6轴或3轴融合(无法9轴融合)
2. DCDC(LM27313)必须使用CD系列功率电感
3. 通信串口帧同步方法:
   a. 设置同步位,比如第一个字节固定为0xaa,以后的数据才是正确的数据,否则丢弃(数据中需要避免0xaa);
   b. 正确设置接收的数据量和超时时间,一般这一帧(6字节为例)的数据是一起发送的,然后隔比较长的时间才是后一帧6字节数据,设置接收数据量为帧长,超时时间看你的发送设置,差不多就行了.设置好以后,如果是同一组的6个字节,接收时间短,在超时内就接收完了,不会出现超时错误,如果不是同一组的6个字节,则在超时以后还没接受完6个 字节,会有超时错误,相应处理就行了.
## wifi连接
1. 关闭上位机防火墙
2. 上位机:192.168.1.2:8080
## 飞机方向
1. 机前 晶体顺时针方向第1个 顺时针
2. 机右 晶体顺时针方向第2个 逆时针
3. 机后 晶体顺时针方向第3个 顺时针
4. 机左 晶体顺时针方向第4个 逆时针

