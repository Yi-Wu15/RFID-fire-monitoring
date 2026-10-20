#include <ioCC2530.h>
#include "OnBoard.h"
#include "IOT.h"
#include "delay.h"
#include "ZComDef.h"
#include "ZGlobals.h"
#include "OnBoard.h"
#include "OSAL.h"
#include "MT.h"
#include "MT_SYS.h"
#include "DebugTrace.h"

/* Hal */
#include "hal_lcd.h"
#include "hal_mcu.h"
#include "hal_timer.h"
#include "hal_key.h"
#include "hal_led.h"
#include "hal_uart.h"
#include "string.h"
#include "stdlib.h"
#include "stdio.h"
/* Allow access macRandomByte() */
#include "mac_radio_defs.h"
char uartbuf2[256]={0x00};

IOT IOT_Status;
extern void LCD_P8x16Str(unsigned char x, unsigned char y,unsigned char ch[]);
extern void HalLcdDisplayPercentBar( char *title, uint8 value );
extern void LCD_P16x16Ch(unsigned char x, unsigned char y, unsigned char N);


void UartInitPort0(void )
{

          halUARTCfg_t uartConfig;//顶一个串口结构体
          uartConfig.configured             =TRUE;//串口配置为真
          uartConfig.baudRate               =HAL_UART_BR_115200;//波特率为115200
          uartConfig.flowControl            =FALSE;//流控制为假
          uartConfig.callBackFunc       =    rxCB;
          HalUARTOpen(HAL_UART_PORT_0,&uartConfig);// 打开串口0
}
//下面这个回调函数是我加的，从串口0读取n个字符，放进uartbuf
static void rxCB(uint8 port,uint8 event)
{

  unsigned char uartbuf[250]={0x00};
  uint8 j=0;
  //HalUARTRead(0,uartbuf,1);
 while (Hal_UART_RxBufLen(0)) //检测串口数据是否接收完成
  {
    HalUARTRead (0,&uartbuf[j], 1);  //把数据接收放到buf中
    j++;                           //记录字符数
  } 
  
}

void UartInitPort1( halUARTCBack_t SerialCallBack )//配置USART1接到IOT模块
{
  halUARTCfg_t uartConfig;

  // configure UART
  uartConfig.configured           = TRUE;
  uartConfig.configured             =TRUE;//串口配置为真
  uartConfig.baudRate               =HAL_UART_BR_115200;//波特率为115200
  uartConfig.flowControl            =FALSE;//流控制为假
  uartConfig.callBackFunc         = SerialCallBack;
  // start UART
  // Note: Assumes no issue opening UART port.
  HalUARTOpen( HAL_UART_PORT_1, &uartConfig );

}


//串口接收函数 大部分的时候都在这里面做 模块的反馈信息
//下面这个回调函数是我加的，从串口1读取n个字符，放进uartbuf
 void SerialCallBack(uint8 port,uint8 event)
{
  unsigned char flag=0;
  uint8 j=0;
  //HalUARTRead(0,uartbuf,1);
  while (Hal_UART_RxBufLen(1)) //检测串口数据是否接收完成
  {
    HalUARTRead (1,(unsigned char *)&uartbuf2[j], 1);  //把数据接收放到buf中
    j++;                           //记录字符数
    flag=1;                         //已经从串口接收到信息
  } 
 
 if(flag)//收到数据了 收据全部收完了
 {
     /*char *strx;
     strx=strstr((const char*)uartbuf2,(const char*)"+MQTTCONNECTED");//ONENET登录成功，可以发布订阅数据了
     if(strx)
     {
       strx=strstr((const char*)uartbuf2,(const char*)"OK");
       if(strx)
       {
          HalUARTWrite(0,"\r\n**CSGSM Connect To ONENET[OK]\r\n",33); //串口提示正在连接模块
          LCD_P8x16Str(0, 4, "CONNECT CLOUD OK");
          IOT_Status.Status=6;
       }
       
    }*/ 
    IOT_RecdataPut();  //对模块接收到的数据进行处理
    HalUARTWrite(0,(unsigned char *)uartbuf2,j); //将接收到的数据打印出来
    IOT_Status.Reclen=j;        //获取接收到的数据长度
     
   }
}
unsigned char isBUSY=0; //是否切换到手动模式       

void IOT_Init(void)//初始化IOT 
{ 
 
  switch(IOT_Status.Status)
  {
    case 0:
      //isBUSY = 1;
      HalUARTWrite(1,"AT\r\n",osal_strlen("AT\r\n"));//配置发AT命令
      HalUARTWrite(0,"\r\n**CSGSM Connect To MODULE...\r\n",32); //串口提示正在连接模块
    break;
     case 1:
      HalUARTWrite(1,"AT+CWMODE=1\r\n",osal_strlen("AT+CWMODE=1\r\n"));//设置模式 設置到STA模式
      //HalUARTWrite(0,"**leigong CARD...\r\n",19);
      HalUARTWrite(0,"Set WiFi ModuleSTA..\r\n",22);
    break;
      case 2:
      HalUARTWrite(1,"AT+CIPMODE=0\r\n",osal_strlen("AT+CIPMODE=0\r\n"));//配置发AT命令 //非透传模式CS
      HalUARTWrite(0,"Set WiFi Module Not transparent\r\n",33); //串口提示正在连接模块
    break;
  }
}


void IOT_MQTTInit(void)//初始化MQTT
{
  unsigned char SendATData[256];
    
  switch(IOT_Status.Status)
  {
     case 3:
      memset(SendATData,0,256);
      sprintf((char *)SendATData,"AT+CWJAP=\"%s\",\"%s\"\r\n",LYSSID,LYPASSWD);//链接到路由器
      HalUARTWrite(1,SendATData,osal_strlen((char *)SendATData));
      HalUARTWrite(0,SendATData,osal_strlen((char *)SendATData));
      HalUARTWrite(0,"Connect To WiFi Route...\r\n",26);
      break;
    
  case 4:
       memset(SendATData,0,256);
       sprintf((char *)SendATData,"AT+MQTTUSERCFG=0,1,\"%s\",\"%s\",\"%s\",0,0,\"\"\r\n",devid,proid,token);
       HalUARTWrite(1,(unsigned char*)SendATData, strlen((char const *)SendATData));
       break;
       
    case 5:
       HalUARTWrite(1,MQTTCONN, strlen(MQTTCONN));//设置登录到MQTT服务器
      //IOT_Status.Status=5;//等待数据返回，调整到串口接收函数处理 获取到6
      break;
      
    case 6:
     memset(SendATData,0,256);
     sprintf((char *)SendATData,"AT+MQTTSUB=0,\"$sys/%s/%s/thing/property/post/reply\",1\r\n",proid,devid);
     HalUARTWrite(1,(unsigned char*)SendATData, strlen((char const *)SendATData));
      break;  
  }
}


void IOT_RecdataPut(void)//对IOT返回的数据内容进行解析
{ 
  char *strx;
  
  switch(IOT_Status.Status)
  {
    case 0:
      strx=strstr((const char*)uartbuf2,(const char*)"OK");//返回OK，表明通讯正常c s t x
       if(strx)
       {
         LCD_P8x16Str(0, 4, "CONNET MODULE OK");
         IOT_Status.Status=1;
         HalUARTWrite(0,"\r\n**CSGSM Connect To MODULE[OK]\r\n",33); //串口提示正在连接模块
       }
      break;
      
      case 1:
      strx=strstr((const char*)uartbuf2,(const char*)"OK");//设置下模式1
       if(strx)
       {
         LCD_P8x16Str(0, 4, "SET MODE 1  [OK]");
         IOT_Status.Status=2;
       }
       break;
         
      case 2:
      strx=strstr((const char*)uartbuf2,(const char*)"OK");//设置模块为 透传模式 发送json到oennet
       if(strx)
       {
         LCD_P8x16Str(0, 4, "NOT TRANSPARNET ");
         IOT_Status.Status=3;
       } 
      break;  
    
       case 3://判断登录服务器情况
        strx=strstr((const char*)uartbuf2,(const char*)"OK");//CSTX链接到路由器反馈，要上外网必须让WiFi去链接到路由器
       if(strx)
       {
         HalUARTWrite(0,"\r\n**CSGSM Connect To WIFI NET OK\r\n",34); //串口提示正在连接模块
         LCD_P8x16Str(0, 4, "CONNECT Route OK");
         IOT_Status.Status=4;//此时需要发布登录
       } 
      break;   
    
       case 4://查看登录情况返回
       strx=strstr((const char*)uartbuf2,(const char*)"OK");//mqtt登录信息设置成功
       if(strx)
       {
          LCD_P8x16Str(0, 4, "MQTT CONFIG OK");
          IOT_Status.Status=5;//此时就可以发数据了
       } 
      break;
    
      case 5:
        strx=strstr((const char*)uartbuf2,(const char*)"MQTTCONNECTED");//ONENET登录成功，可以发布订阅数据了
       if(strx)
       {
          LCD_P8x16Str(0, 4, "CONNECT CLOUD OK");
          IOT_Status.Status=6;//此时就可以发数据了
       } 
      break;
      
     case 6://查看发布数据情况
     strx=strstr((const char*)uartbuf2,(const char*)"OK");//消息订阅成功
     if(strx)
     {
        LCD_P8x16Str(0, 4, "SEND CLOUD DATA.");
        IOT_Status.Status=7;//此时就可以发数据了
        //isBUSY = 0;
     }
     else
     {
        LCD_P8x16Str(0, 4, "SEND CLOUD DATA*");
        IOT_Status.Status=7;//此时就可以发数据了
        //isBUSY = 0;
     }
     
    break;
  }
  
       
}