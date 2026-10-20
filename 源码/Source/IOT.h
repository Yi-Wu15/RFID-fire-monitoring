#ifndef __IOT_H__
#define __IOT_H__

#define uchar unsigned char
#define LED1  P1_0
#define LED2  P1_1
//////////////////必须改的四要素/////////////////////////////

#define  ENDNUM 3 //定义设备号 


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//需要修改的是下面5行

#define  token    "YOUR_ONENET_TOKEN" //token工具计算值
#define  devid    "terminal_device"       //新版本onenet平台设备id
#define  proid     "z4xxxywGMH"           //新版本onenet平台产品id
#define  LYSSID    "YOUR_WIFI_SSID"               // 修改你路由器的SSId
#define  LYPASSWD  "YOUR_WIFI_PASSWORD"             // 修改你路由器的密码
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#define MQTTCONN "AT+MQTTCONN=0,\"mqtts.heclouds.com\",1883,1\r\n"//连接mqtt服务器




void UartInitPort0(void );
void rxCB(uint8 port,uint8 event);//
void UartInitPort1( halUARTCBack_t SerialCallBack );
void SerialCallBack(uint8 port,uint8 event);
void IOT_Init(void);
void IOT_RecdataPut(void);
void IOT_MQTTInit(void);
void IOT_ONENETMQTTSenddata( char temp, char humi);
void aliyunMQTT_PUBdata(uchar id,uchar temp,uchar humi,uchar sensor1,uchar sensor2,uchar sensor3);
typedef struct
{
    unsigned char Status;    
    unsigned char Recflag;   //收到数据标志
    uint8 Reclen;  
    unsigned char Recbuffer[20];
} IOT;

#endif