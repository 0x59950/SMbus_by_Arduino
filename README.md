# SMbus_by_Arduino
Arduino goes to SMbus through ATCMD

# 💻AT+READ=\<addr>,\<len>,\<data>,\<size>

addr-从机地址

len -发送数据长度

data-读数据之前写的数据

size-要读数据的长度

例如AT+READ=11,2,0053,2			//读PF状态(从机地址0x0B)

先发0053指令然后读取2字节数据

```bash
AT+READ=11,2,0053,2
READ: address[0x0B],len[2],cmd[0x00 53].len[4],data:
\==========
0x53
01010011
\==========
0x73
01110011
\==========
0xD9
11011001
\==========
0xD9
11011001
\==========
OK
```



# 🚀AT+WIRE=\<addr>,\<cmd>,\<len>,\<data>
addr-从机地址

cmd -寄存器address

len -数据长度

data-数据

*//例如AT+WIRE=11,4,11223344    //向0x0B写0x11223344四字节数据*

例如AT+WIRE=11,00,2,0041    //\[重启]向0x0B从机的00寄存器写0x0041

```bash
AT+WIRE=11,00,2,0041
WRITE: address[0x0B],cmd[0x00].len[2],data[00 41 ]END
OK
```


