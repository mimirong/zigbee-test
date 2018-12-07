#include "hal_defs.h"
#include "hal_cc8051.h"
#include "hal_int.h"
#include "hal_mcu.h"
#include "hal_board.h"
#include "hal_led.h"
#include "hal_rf.h"
#include "basic_rf.h"
#include "hal_uart.h" 
#include "UART_PRINT.h" 
#include "TIMER.h"
#include "get_adc.h"
#include "sh10.h"
#include <string.h>

#define MAX_SEND_BUF_LEN  128  
#define MAX_RECV_BUF_LEN  128
static uint8 pTxData[MAX_SEND_BUF_LEN]; //定义无线发送缓冲区的大小
static uint8 pRxData[MAX_RECV_BUF_LEN]; //定义无线接收缓冲区的大小

#define MAX_UART_SEND_BUF_LEN  128
#define MAX_UART_RECV_BUF_LEN  128
uint8 uTxData[MAX_UART_SEND_BUF_LEN]; //定义串口发送缓冲区的大小
uint8 uRxData[MAX_UART_RECV_BUF_LEN]; //定义串口接收缓冲区的大小
uint16 uTxlen = 0;
uint16 uRxlen = 0;

#define JD P2_0    // P2_0定义为P2.0
#define LED P1_0

#define ON 1          //定义开继电器
#define OFF 0         //定义关继电器


/*****点对点通讯地址设置******/
#define RF_CHANNEL                16           // 频道 11~26
#define PAN_ID                    0x0303      //网络id 
#define MY_ADDR                   0x3807      // 本机模块地址
#define SEND_ADDR                 0x3804   //发送地址
/**************************************************/
static basicRfCfg_t basicRfConfig;
uint8   APP_SEND_DATA_FLAG;
/******************************************/

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

/********************MAIN************************/
void main(void)    
{   uint16 sensor_val,sensor_tem;
    uint16  len = 0;
    halBoardInit();  //模块相关资源的初始化
    ConfigRf_Init(); //无线收发参数的配置初始化 
    halLedSet(1);
    halLedSet(2);  
    Timer4_Init(); //定时器初始化
    //Timer4_On();  //打开定时器
    JD = 0;               
    while(1)
    {   
        uint16 len = 0;
        if(basicRfPacketIsReady())   //查询有没收到无线信号
        {
            halLedToggle(4);   // 红灯取反，无线接收指示
            //接收无线数据
            len = basicRfReceive(pRxData, MAX_RECV_BUF_LEN, NULL);
            /* if((pRxData[0]=='1')&&(pRxData[1]=='1'))
            {
             JD=ON;
             LED=1;              
            }
            else if((pRxData[0]=='1')&&(pRxData[1]=='0'))
            {
             JD=OFF;
              LED=0;            
            }*/
            
           if((pRxData[0]=='1')&&(pRxData[1]=='1'))
            {
              JD=ON;
              LED=1;
              
              uTxData[0]=0x01;
              uTxData[1]=0x05;
              uTxData[2]=0x00;
              uTxData[3]=0x11;
              uTxData[4]=0xff;
              uTxData[5]=0x00;
              uTxData[6]=0xdc;
              uTxData[7]=0x3f;
              halUartWrite(uTxData,8);
            }else if((pRxData[0]=='1')&&(pRxData[1]=='0'))
            {
              JD=OFF;
              LED=0;  
             
              uTxData[0]=0x01;
              uTxData[1]=0x05;
              uTxData[2]=0x00;
              uTxData[3]=0x11;
              uTxData[4]=0x00;
              uTxData[5]=0x00;
              uTxData[6]=0x9d;
              uTxData[7]=0xcf;
              halUartWrite(uTxData,8);
            }
            
            //把接收到的无线发送到串口
           // halUartWrite(pRxData,len);
              
        } 
      
    }
}
/************************main end ****************************/