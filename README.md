# VultureEgg141
Based on TI BLE-CC254x-1.4.1.43908 stack

http://www.geek-workshop.com/thread-10164-1-1.html

VultureEgg project based on 'SensorTag' project.

==========
Service UUID : FFF0

Command UUID : F0C1

Trans   UUID : FFF6

==========

Enable FFF6 notify.

Write F0C1 following command

Protocol between phone and VultureEgg:

AB0101 -- Start LM75A and measurement period is the default value 20s.

AB01010A -- Start LM75A and measurement period is 10s

AB0100 -- Stop LM75A


AB0201 -- Start humidity(SHT21) and measurement period is the default value 70s.

AB02010F -- Start humidity(SHT21) and measurement period is 15s

AB0200 -- Stop humidity(SHT21)


AB0301 -- Start MPU6050 and measurement period is the default value 2s.

AB03010001 -- Start MPU6050 and measurement period is 1ms
AB03010002 -- Start MPU6050 and measurement period is 2ms
AB0301012C -- Start MPU6050 and measurement period is 300ms

AB0300 -- Stop MPU6050


==========
When disconnected, all measurement will be stopped.

==========
About soft IIC:
#define SDA_GPIO P0_4
#define SCL_GPIO P0_1
