# 四轴飞行器说明文档
四轴飞行器的电子器件主要分为(飞控动力板)、WiFi无线模块、功能板四部分。
1. 飞控板是飞行器的核心和基础,它有3大功能:姿态控制、运动控制、动力控制
   1.1 姿态控制是飞控板的核心,用于控制飞行器的姿态。
   2.2 运动控制用于控制飞行器的运动,主要包括,前后左右上下旋转
   2.3 动力控制用于放大控制信号驱动空心杯电机
2. wifi命令模块\(AP模式\)用于接收主机命令控制飞控模块,也可以解析主机命令后发送回功能模块\(也可以不使用\),暂时采用ESP8266模块
3. 功能板目前可以实现摄像、跟踪、GPS。跟踪模块用于跟踪某一固定目标\(暂时不开发\)



# 工作计划
## 调试计划
1. 实现I2C3简单读写MPU9250/BMP280
2. 等待MPU9250/BMP280焊接完成
3. 测试I2C3,MPU9250中断
## 采购计划
序号 条件          动作                               作用
1    v2.x改板完成  采购v2.x bom                       测试v2.x
2    v2.x调试尾声  采购720配套55mm浆                  测试拉力/电流/电压
3    v3.x改版完成  采购stm32f401qfn48/MPU9250/BMP280  测试v3.x
## 改版计划
1. v2.x版本近期(原型机器)
   1.1 led并联电机
   1.2 恢复TF卡电路
   1.6 小芯片扇出6mil(PCB板厂极限工艺)
   1.7 普通线宽宽10mil
   1.8 V3D3/V3D3R线宽20mil
   1.9 四电机两两共一根线:80mil两电机,40mil一电机
2. v3.x版本(小型化)
   2.0 完成参数测试,分析可行性
   2.1 修复v2.0检查出的BUG
   2.2 更换CPU为 QFN48
   2.3 移除tf卡
   2.4 优化复用
   2.5 布局(30mm*30mm)
   2.5 修改板框,小型化



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
4. 优化融合算法(实在不行可以考虑accel gyro compass中断读冗余整理)
5. ESP8266板载

## 注意:
1. eMD6.12 DMP仅可6轴融合(无法9轴融合)
2. DCDC(LM27313)必须使用CD系列功率电感

