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
static uint8 pTxData[MAX_SEND_BUF_LEN]; //�������߷��ͻ������Ĵ�С
static uint8 pRxData[MAX_RECV_BUF_LEN]; //�������߽��ջ������Ĵ�С

#define MAX_UART_SEND_BUF_LEN  128
#define MAX_UART_RECV_BUF_LEN  128
uint8 uTxData[MAX_UART_SEND_BUF_LEN]; //���崮�ڷ��ͻ������Ĵ�С
uint8 uRxData[MAX_UART_RECV_BUF_LEN]; //���崮�ڽ��ջ������Ĵ�С
uint16 uTxlen = 0;
uint16 uRxlen = 0;

#define JD P2_0    // P2_0����ΪP2.0
#define LED P1_0

#define ON 1          //���忪�̵���
#define OFF 0         //����ؼ̵���


/*****��Ե�ͨѶ��ַ����******/
#define RF_CHANNEL                16           // Ƶ�� 11~26
#define PAN_ID                    0x0303      //����id 
#define MY_ADDR                   0x3807      // ����ģ���ַ
#define SEND_ADDR                 0x3804   //���͵�ַ
/**************************************************/
static basicRfCfg_t basicRfConfig;
uint8   APP_SEND_DATA_FLAG;
/******************************************/

/**************************************************/
// ����RF��ʼ��
void ConfigRf_Init(void)
{
    basicRfConfig.panId       =   PAN_ID;        //zigbee��ID������
    basicRfConfig.channel     =   RF_CHANNEL;    //zigbee��Ƶ������
    basicRfConfig.myAddr      =  MY_ADDR;   //���ñ�����ַ
    basicRfConfig.ackRequest  =   TRUE;          //Ӧ���ź�
    while(basicRfInit(&basicRfConfig) == FAILED); //���zigbee�Ĳ����Ƿ����óɹ�
    basicRfReceiveOn();                // ��RF
}

/********************MAIN************************/
void main(void)    
{   uint16 sensor_val,sensor_tem;
    uint16  len = 0;
    halBoardInit();  //ģ�������Դ�ĳ�ʼ��
    ConfigRf_Init(); //�����շ����������ó�ʼ�� 
    halLedSet(1);
    halLedSet(2);  
    Timer4_Init(); //��ʱ����ʼ��
    //Timer4_On();  //�򿪶�ʱ��
    JD = 0;               
    while(1)
    {   
        uint16 len = 0;
        if(basicRfPacketIsReady())   //��ѯ��û�յ������ź�
        {
            halLedToggle(4);   // ���ȡ�������߽���ָʾ
            //������������
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
            
            //�ѽ��յ������߷��͵�����
           // halUartWrite(pRxData,len);
              
        } 
      
    }
}
/************************main end ****************************/