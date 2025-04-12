# SMbus_by_Arduino
Arduino goes to SMbus through ATCMD

AT+READ=<addr>,<len>,<data>,<size>
addr-从机地址
len -发送数据长度
data-读数据之前写的数据
size-要读数据的长度
例如AT+READ=11,2,0053,2			//读PF状态(从机地址0x0B)

AT+WIRE=<addr>,<len>,<data>
addr-从机地址
len -数据长度
data-数据
例如AT+WIRE=11,4,11223344    //向0x0B写0x11223344四字节数据

