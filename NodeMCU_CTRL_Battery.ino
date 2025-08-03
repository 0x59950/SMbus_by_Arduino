#include "Wire.h"

#define AT_ARGV_COUNT 5
#define AT_ARGV_VALUE 65
#define AT_COMMAND_MAX 20

typedef unsigned char uint8_t;
typedef unsigned int uint23_t;



// 定义存储 AT 指令的最大长度
const int maxCommandLength = 64;
char command[maxCommandLength];
int commandIndex = 0;



// 封装的 print 函数
void u_printf(const char *format, ...)
{
	char buffer[100];
	va_list args;
	va_start(args, format);
	vsnprintf(buffer, sizeof(buffer), format, args);
	va_end(args);
	Serial.print(buffer);
}

void bin_print(uint8_t data)
{
	for(uint8_t i = 7; ; i--)
	{
		if((data>>i) & 1)
			Serial.print("1");
		else
			Serial.print("0");
		if(i == 0)
			return;
	}
}

/*******
* 按16进制打印数据
* 参数:: head:打印数据前提示; data:要打印的数据; len:数据长度; tail: 打印结束提示
********/
#define LENGTH 16
void do_print_data_hex(char *head, unsigned char *data, int len, char *tail)
{
	int i;
	int line;
	unsigned char a;

	u_printf("%s::len=%d\n", head,len);
	line=0;
	for (i=0;i<len;i++)
	{
		u_printf("%02X ", *(data+i));
		line++;
		if(line>=LENGTH || i==(len-1))       //已经打印LENGTH个hex数据 或者 数据已经打完了
		{
			u_printf("  ");               //在16进制后面打印字符(两个 隔开)
			line--;
			if(i==(len-1))					//数据打完了用 补齐hex部分
			{
				for(a=0; a<(LENGTH-((len%LENGTH)?(len%LENGTH):LENGTH)); a++)
				{
					u_printf("   ");
				}
			}
			for( ; line>=0; line--)			//打印ascll字符
			{
				if((*(data+i-line))>=33 && (*(data+i-line))<=126)		//is ASCLL
				{
					u_printf("%c", *(data+i-line));
				}
				else
				{														//not ASCLL
					u_printf("."/*, *(data+i-line)*/);
				}
			}
			u_printf("\n");
			line=0;
		}
	}
	u_printf("\n%s\n", tail);
}

int char_to_hex(char H, char L)
{
	int ret = 0;
	if(H >= '0' && H <= '9')
		ret = (H - '0');
	else if(H >= 'A' && H <= 'F')
		ret = (H - 'A' + 10);
	else if(H >= 'a' && H <= 'f')
		ret = (H - 'a' + 10);
	else
		return -1;

	ret *= 16;

	if(L >= '0' && L <= '9')
		ret += (L - '0');
	else if(L >= 'A' && L <= 'F')
		ret += (L - 'A' + 10);
	else if(L >= 'a' && L <= 'f')
		ret += (L - 'a' + 10);
	else
		return -1;

	return ret;
}

static int at_read(int argc, char *argv[])
{
	if(argc != 4)
		return -2;

	// do_print_data_hex("READ_ARGV[0]", (uint8_t *)argv[0], strlen(argv[0]), "==== END ====\n");		//addr
	// do_print_data_hex("READ_ARGV[1]", (uint8_t *)argv[1], strlen(argv[1]), "==== END ====\n");		//len
	// do_print_data_hex("READ_ARGV[2]", (uint8_t *)argv[2], strlen(argv[2]), "==== END ====\n");		//data
	// do_print_data_hex("READ_ARGV[3]", (uint8_t *)argv[3], strlen(argv[3]), "==== END ====\n");		//len

	int len = atoi(argv[1]);
	uint8_t addr = atoi(argv[0]);

	u_printf("READ: address[0x%02X],len[%d],cmd[0x", addr, len);

	Wire.beginTransmission(addr);			// 向地址x写地址
	for(int i = 0; i < len; i++)
	{
		int data = char_to_hex(argv[2][i*2], argv[2][i*2+1]);
		if(data < 0)
			break;
		u_printf("%02X", data);
		if(i < (len-1))
			Serial.print(" ");
		Wire.write(data);
	}
	Wire.endTransmission(0);

	len = atoi(argv[3]);
	u_printf("].len[%d],data:\r\n==========\r\n", len);

	Wire.requestFrom(addr, len);	// 向地址x请求n个字节
	while (Wire.available())
	{
		byte data = Wire.read();	// 读取数据
		u_printf("0x%02X\r\n", data);
		bin_print(data);
		Serial.print("\n==========\n");
	}
	// Wire.endTransmission();

	return 0;
}

static int at_wire(int argc, char *argv[])
{
	if(argc != 4)
		return -2;

	// do_print_data_hex("ARGV[0]", (uint8_t *)argv[0], strlen(argv[0]), "==== END ====\n");	//addr
	// do_print_data_hex("ARGV[1]", (uint8_t *)argv[1], strlen(argv[1]), "==== END ====\n");	//cmd
	// do_print_data_hex("ARGV[2]", (uint8_t *)argv[2], strlen(argv[2]), "==== END ====\n");	//len
	// do_print_data_hex("ARGV[3]", (uint8_t *)argv[3], strlen(argv[3]), "==== END ====\n");	//data

	int addr = atoi(argv[0]);
	int len = atoi(argv[2]);
	int cmd = atoi(argv[1]);

	u_printf("WRITE: address[0x%02X],cmd[0x%02X].len[%d],data[", addr, cmd, len);
	Wire.beginTransmission(addr);
	Wire.write(cmd);

	for(int i = 0; i < len; i++)
	{
		int data = char_to_hex(argv[3][i*2], argv[3][i*2+1]);
		if(data < 0)
			break;
		u_printf("%02X ", data);
		Wire.write(data);
	}
	u_printf("]END\r\n");

	Wire.endTransmission();

	return 0;
}



typedef struct at_type
{
	const char * name;
	int (*RUN)(int, char**);
}at_t;

static int at_scan(int argc, char *argv[])
{
	scan_iic();
	return 0;
}

const at_t at_cmd_reg[] = {
	{"READ", at_read},
	{"WIRE", at_wire},
	{"SCAN", at_scan},
	{NULL, NULL},
	{NULL, NULL}
};

/*
AT error code
-1		指令问题
-2		参数问题
-3		执行问题
-4		其他问题

*/

int ret_min(int a, int b)
{
	return a < b ? a : b;
}

void at_return(int r)
{
	if(r == 0)
		Serial.println("OK");
	else if(r > 0)
		;
	else if(r == -1)
		Serial.println("ERROR=-1");
	else if(r == -2)
		Serial.println("ERROR=-2");
	else if(r == -3)
		Serial.println("ERROR=-3");
	else
		Serial.println("ERROR=-4");
}

int parseATCommand_arg(const char *in, char **arg)
{
	int count = 0;
	const char *p = in;
	const char *v;
	// const char *max = in + strlen(in);		//'0'

	while(1)
	{
		v = strchr(p , ',');
		if(v && strlen(v) > 1)
		{
			memcpy(arg[count], p, ret_min(v-p, AT_ARGV_VALUE-1));
			p = v+1;
			count++;
		}
		else
		{
			memcpy(arg[count], p, ret_min(strlen(p), AT_ARGV_VALUE-1));
			count++;
			break;
		}
		if(count >= AT_ARGV_COUNT)
			break;
	}
	return count;
}

// 解析 AT 指令
void parseATCommand(const char* command) {
	const char *p = NULL;
	const char *b = NULL;
	const char *v = strstr(command, "AT");

	if(v == NULL)
	{
		at_return(-1);
		return;
	}

	if (strcmp(v, "AT") == 0)
	{
		at_return(0);
		return;
	}

	b = strchr(v, '=');

	if(strlen(v) < 4 || memcmp(v, "AT+", 3) != 0)
	{
		at_return(-1);
		return;
	}

	p = v+3;

	char at_cmd[AT_COMMAND_MAX] = {0};
	char argv[AT_ARGV_COUNT][AT_ARGV_VALUE] = {0};
	char *argv_p[AT_ARGV_COUNT] = {argv[0], argv[1], argv[2], argv[3], argv[4]};

	if(b)
		memcpy(at_cmd, p, b-p);
	else
		strncpy(at_cmd, p, AT_COMMAND_MAX);

	// u_printf("==== AT CMD:[%s] ====\r\n", at_cmd);

	for(unsigned int i = 0; i < (sizeof(at_cmd_reg) / sizeof(at_t)); i++)
	{
		// u_printf("==== I:[%d] ====\r\n", i);
		if((at_cmd_reg[i].name != NULL) && (strcasecmp(at_cmd, at_cmd_reg[i].name) == 0))
		{
			if(b && strlen(b) > 1)
			{
				int count = parseATCommand_arg(b+1, argv_p);
				at_return(at_cmd_reg[i].RUN(count, argv_p));
				return;
			}
			else
			{
				at_return(at_cmd_reg[i].RUN(0, NULL));
				return;
			}
		}
	}

	at_return(-4);
	return;




	#if 0		// ===============================
	if (strcmp_l(command, "AT", 2) == 0)
	{
		if()
		else
		{
			if(command[2])
		}
		else if (strcmp(command, "AT+TEST") == 0)
		{
			Serial.println("TEST RESPONSE");
		}
		else
		{
			Serial.println("ERROR");
		}
	}
	else
	{
		Serial.println("ERROR=-1");
	}
	#endif		// ===============================
}



// 串口事件函数，当有数据到达串口时自动调用
void serialEvent()
{
	while(Serial.available() > 0)
	{
		char c = Serial.read();
		if (c == '\r' || c == '\n')
		{
			if (commandIndex > 0)
			{
				command[commandIndex] = '\0';		// 字符串结束符
				// Serial.print("接收到指令: ");
				Serial.println(command);
				parseATCommand(command);
				commandIndex = 0;					// 重置指令索引
			}
		}
		else if (commandIndex < maxCommandLength - 1)
		{
			command[commandIndex++] = c;
		}
	}
}

void scan_iic(void)
{
	byte error, address;
	int nDevices;

	Serial.println("Scanning...");

	nDevices = 0;
	for (address = 1; address < 127; address++)
	{
		Wire.beginTransmission(address);
		error = Wire.endTransmission();

		if (error == 0)
		{
			Serial.print("I2C device found at address 0x");
			if (address < 16)
				Serial.print("0");
			Serial.print(address, HEX);
			Serial.println("  !");

			nDevices++;
		}
		else if (error == 4)
		{
			Serial.print("Unknown error at address 0x");
			if (address < 16)
				Serial.print("0");
			Serial.println(address, HEX);
		}
	}

	if (nDevices == 0)
		Serial.println("No I2C devices found\n");
	else
		Serial.println("done\n");
}



void setup() {
	// Wire.stretchClock(true);
	Wire.setClock(80000);
	Wire.begin();

	Serial.begin(74880);	//bootload 波特率74880
	while (!Serial)
		;			//用来等待串口监视器初始化完成
	Serial.println("\nI2C Scanner");
}


void loop() {
#if 0					//循环扫描
	if((millis() % (long unsigned int)5000) == 0)
	{
		scan_iic();
	}
#endif

}