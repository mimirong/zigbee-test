#include "hal_defs.h"
#include "hal_cc8051.h"
#include "hal_int.h"
#include "hal_mcu.h"
#include "hal_board.h"
#include "hal_led.h"
#include "hal_rf.h"
#include "basic_rf.h"
#include "hal_uart.h" 


#define MAX_SEND_BUF_LEN  128
#define MAX_RECV_BUF_LEN  128
static uint8 pTxData[MAX_SEND_BUF_LEN]; //定义无线发送缓冲区的大小
static uint8 pRxData[MAX_RECV_BUF_LEN]; //定义无线接收缓冲区的大小

#define MAX_UART_SEND_BUF_LEN  128
#define MAX_UART_RECV_BUF_LEN  128
uint8 uTxData[MAX_UART_SEND_BUF_LEN]; //定义串口发送缓冲区的大小
uint8 uTxData1[MAX_UART_SEND_BUF_LEN]; //定义串口发送缓冲区的大小
uint8 uRxData[MAX_UART_RECV_BUF_LEN]; //定义串口接收缓冲区的大小
uint16 uTxlen = 0;
uint16 uRxlen = 0;

 
/*****点对点通讯地址设置******/
#define RF_CHANNEL                16           // 频道 11~26
#define PAN_ID                    0x0303      //网络id 
#define MY_ADDR                   0x3804      // 本机模块地址
#define SEND_ADDR3                0x3803     //LED
#define SEND_ADDR5                0x3805     //风扇
#define SEND_ADDR6                0x3806    //灯
#define SEND_ADDR7                0x3807    //告警灯



/**************************************************/
static basicRfCfg_t basicRfConfig;
/******************************************/
void MyByteCopy(uint8 *dst, int dststart, uint8 *src, int srcstart, int len)
{
    int i;
    for(i=0; i<len; i++)
    {
        *(dst+dststart+i)=*(src+srcstart+i);
    }
}
/****************************************************/
uint16 RecvUartData(void)
{   
    uint16 r_UartLen = 0;
    uint8 r_UartBuf[128]; 
    uRxlen=0; 
    r_UartLen = halUartRxLen();
    while(r_UartLen > 0)
    {
        r_UartLen = halUartRead(r_UartBuf, sizeof(r_UartBuf));
        MyByteCopy(uRxData, uRxlen, r_UartBuf, 0, r_UartLen);
        uRxlen += r_UartLen;
        halMcuWaitMs(5);   //这里的延迟非常重要，因为这是串口连续读取数据时候需要有一定的时间间隔
        r_UartLen = halUartRxLen();       
    }   
    return uRxlen;
}
/**************************************************/
// 无线RF初始化
void ConfigRf_Init(void)
{
    basicRfConfig.panId       =   PAN_ID;        //zigbee的ID号设置
    basicRfConfig.channel     =   RF_CHANNEL;    //zigbee的频道设置
    basicRfConfig.myAddr      =  MY_ADDR;   //设置本机地址
    basicRfConfig.ackRequest  =   TRUE;          //应答信号
    while(basicRfInit(&basicRfConfig) == FAILED); //检测zigbee的参数是否配置成功
    basicRfReceiveOn();                // 打开RF
}

/********************MAIN START************************/
void main(void)
{
    uint16 len = 0;
    int i=0;
    int k1=0;
    halBoardInit();  //模块相关资源的初始化
    ConfigRf_Init(); //无线收发参数的配置初始化 
    halLedSet(1);
    halLedSet(2);
    while(1)
    {
        
      len = RecvUartData();    // 接收串口数据    
      if(len > 0)
      {
        halLedToggle(3);       // 绿灯取反，无线发送指示
        if(uRxData[0]=='1'){basicRfSendPacket(SEND_ADDR5, uRxData,len);}//风扇
        else if(uRxData[0]=='2')//led灯
        {
          uRxData[0]='1';
          basicRfSendPacket(SEND_ADDR6, uRxData,len);
        }
       
        else if(uRxData[0]=='7')//告警灯
        {
          uRxData[0]='1';
          basicRfSendPacket(SEND_ADDR7, uRxData,len);
        }
        else
        {
          basicRfSendPacket(SEND_ADDR3, uRxData,len);
        }
        if(uRxData[0]=='0'){basicRfSendPacket(SEND_ADDR7, uRxData,len);}
        
            //把串口数据通过zigbee发送出去            
      }
      
      if(basicRfPacketIsReady())   //查询有没收到无线信号
        {
            halLedToggle(4);   // 红灯取反，无线接收指示
            //接收无线数据
            len = basicRfReceive(pRxData, MAX_RECV_BUF_LEN, NULL);
            ////////////////////////////////数据整理//01XXXX02XXXX03XXXX
            if(pRxData[1]=='1')//11光照强度
            {
              uTxData[0]='A';
              for(i=1; i<5; i++)
              {
                  uTxData[i]=pRxData[i+3];
              }
            }
            else///////////////////////如果当前不是这个终端数据，就copy上一次
            {
              for(i=0; i<5; i++)
              {
                  uTxData[i]=uTxData1[i];
              }
            }
            if(pRxData[1]=='2')//20
            { 
              uTxData[5]='B';
              for(i=6; i<10; i++)//温度
              {
                  uTxData[i]=pRxData[i-2];
              }
              uTxData[10]='C';
              for(i=11; i<15; i++)//湿度
              {
                  uTxData[i]=pRxData[i+3];
              }
            }
            else
            {
              for(i=5; i<15; i++)
              {
                  uTxData[i]=uTxData1[i];
              }
            }
            ////////////////////////////////数据整理//AXXXXBXXXXCXXXX///15字符
            //////////////////////////////////////////////////////数据备份
            for(i=0; i<15; i++)
            {
             uTxData1[i]=uTxData[i];
            }
            /////////////////////////////////////////////////////数据备份
            /////////////////////////////////////////////////灯的反馈自动控制
            if((uTxData[3]<'5')&(uTxData[1]<'1'))//光照强度小于0.5，打开灯
            {
              uRxData[0]='1';uRxData[1]='1';
              basicRfSendPacket(SEND_ADDR6, uRxData,2);
            }
            else
            {
              uRxData[0]='1';uRxData[1]='0';
              basicRfSendPacket(SEND_ADDR6, uRxData,2);
            }
            //////////////////////////////////////////////////灯的反馈自动控制
            

            
          /*  if((uTxData[6]>='1')&(uTxData[7]>='5'))//温度度大于15，打开风扇
            {
              //uRxData[0]='1';uRxData[1]='1';
                uRxData[0]=0xCE ;
                uRxData[1]=0xC2 ;
                uRxData[2]=0xB6 ;
                uRxData[3]=0xC8 ;
                uRxData[4]=0xB9 ;
                uRxData[5]=0xFD ;
                uRxData[6]=0xB8 ;
                uRxData[7]=0xDF ;
              basicRfSendPacket(SEND_ADDR3, uRxData,8);
              
              basicRfSendPacket(SEND_ADDR3, uRxData,len);
                uRxData[0]='1';
                uRxData[1]='1';
              basicRfSendPacket(SEND_ADDR5, uRxData,2);
                uRxData[0]='1';
                uRxData[1]='1';
              basicRfSendPacket(SEND_ADDR7, uRxData,2);
              
            }
            else
            {
              uRxData[0]='1';uRxData[1]='0';
              basicRfSendPacket(SEND_ADDR7, uRxData,2);
              basicRfSendPacket(SEND_ADDR5, uRxData,2);
            }*/
            //////////////////////////////////////////////////灯的反馈自动控制
            k1=k1+1;
            if(k1==20)
            {  
              
              
            uRxData[0]=0xCE;
            uRxData[1]=0xC2;
            uRxData[2]=0xB6;
            uRxData[3]=0xC8;
            uRxData[4]=uTxData[6];
            uRxData[5]=uTxData[7];
            uRxData[6]=uTxData[8];            ///led温度显示
            uRxData[7]=uTxData[9];
            uRxData[8]=0xB6;
            uRxData[9]=0xC8;
            basicRfSendPacket(SEND_ADDR3, uRxData,10);
                if((uTxData[6]>='1')&(uTxData[7]>='5'))//温度度大于15，打开风扇
            {
              //uRxData[0]='1';uRxData[1]='1';
                uRxData[0]=0xCE ;
                uRxData[1]=0xC2 ;
                uRxData[2]=0xB6 ;
                uRxData[3]=0xC8 ;
                uRxData[4]=0xB9 ;    ///温度过高
                uRxData[5]=0xFD ;
                uRxData[6]=0xB8 ;
                uRxData[7]=0xDF ;
                
                uRxData[8]=0x2D;  //-
                uRxData[9]=0x2D;  //-
                
                uRxData[10]=0xCE;
                uRxData[11]=0xC2;
                uRxData[12]=0xB6;
                uRxData[13]=0xC8;
                uRxData[14]=uTxData[6];
                uRxData[15]=uTxData[7];
                uRxData[16]=uTxData[8];            ///led温度显示
                uRxData[17]=uTxData[9];
                uRxData[18]=0xB6;
                uRxData[19]=0xC8;
               basicRfSendPacket(SEND_ADDR3, uRxData,20);
              
              
                uRxData[0]='1';
                uRxData[1]='1';
              basicRfSendPacket(SEND_ADDR5, uRxData,2);
                uRxData[0]='1';
                uRxData[1]='1';
              basicRfSendPacket(SEND_ADDR7, uRxData,2);
              
            }
            else
            {
              uRxData[0]='1';uRxData[1]='0';
              basicRfSendPacket(SEND_ADDR7, uRxData,2);
              basicRfSendPacket(SEND_ADDR5, uRxData,2);
            }
              
              
          
            k1=0;
            }
            //数据发送到PC机
            halUartWrite(uTxData,15);
        } 
    }
}
/************************MAIN END ****************************/