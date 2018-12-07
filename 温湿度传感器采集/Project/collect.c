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
static uint8 pTxData[MAX_SEND_BUF_LEN]; //�������߷��ͻ������Ĵ�С
static uint8 pRxData[MAX_RECV_BUF_LEN]; //�������߽��ջ������Ĵ�С

#define MAX_UART_SEND_BUF_LEN  128
#define MAX_UART_RECV_BUF_LEN  128
uint8 uTxData[MAX_UART_SEND_BUF_LEN]; //���崮�ڷ��ͻ������Ĵ�С
uint8 uTxData1[MAX_UART_SEND_BUF_LEN]; //���崮�ڷ��ͻ������Ĵ�С
uint8 uRxData[MAX_UART_RECV_BUF_LEN]; //���崮�ڽ��ջ������Ĵ�С
uint16 uTxlen = 0;
uint16 uRxlen = 0;

 
/*****��Ե�ͨѶ��ַ����******/
#define RF_CHANNEL                16           // Ƶ�� 11~26
#define PAN_ID                    0x0303      //����id 
#define MY_ADDR                   0x3804      // ����ģ���ַ
#define SEND_ADDR3                0x3803     //LED
#define SEND_ADDR5                0x3805     //����
#define SEND_ADDR6                0x3806    //��
#define SEND_ADDR7                0x3807    //�澯��



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
        halMcuWaitMs(5);   //������ӳٷǳ���Ҫ����Ϊ���Ǵ���������ȡ����ʱ����Ҫ��һ����ʱ����
        r_UartLen = halUartRxLen();       
    }   
    return uRxlen;
}
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

/********************MAIN START************************/
void main(void)
{
    uint16 len = 0;
    int i=0;
    int k1=0;
    halBoardInit();  //ģ�������Դ�ĳ�ʼ��
    ConfigRf_Init(); //�����շ����������ó�ʼ�� 
    halLedSet(1);
    halLedSet(2);
    while(1)
    {
        
      len = RecvUartData();    // ���մ�������    
      if(len > 0)
      {
        halLedToggle(3);       // �̵�ȡ�������߷���ָʾ
        if(uRxData[0]=='1'){basicRfSendPacket(SEND_ADDR5, uRxData,len);}//����
        else if(uRxData[0]=='2')//led��
        {
          uRxData[0]='1';
          basicRfSendPacket(SEND_ADDR6, uRxData,len);
        }
       
        else if(uRxData[0]=='7')//�澯��
        {
          uRxData[0]='1';
          basicRfSendPacket(SEND_ADDR7, uRxData,len);
        }
        else
        {
          basicRfSendPacket(SEND_ADDR3, uRxData,len);
        }
        if(uRxData[0]=='0'){basicRfSendPacket(SEND_ADDR7, uRxData,len);}
        
            //�Ѵ�������ͨ��zigbee���ͳ�ȥ            
      }
      
      if(basicRfPacketIsReady())   //��ѯ��û�յ������ź�
        {
            halLedToggle(4);   // ���ȡ�������߽���ָʾ
            //������������
            len = basicRfReceive(pRxData, MAX_RECV_BUF_LEN, NULL);
            ////////////////////////////////��������//01XXXX02XXXX03XXXX
            if(pRxData[1]=='1')//11����ǿ��
            {
              uTxData[0]='A';
              for(i=1; i<5; i++)
              {
                  uTxData[i]=pRxData[i+3];
              }
            }
            else///////////////////////�����ǰ��������ն����ݣ���copy��һ��
            {
              for(i=0; i<5; i++)
              {
                  uTxData[i]=uTxData1[i];
              }
            }
            if(pRxData[1]=='2')//20
            { 
              uTxData[5]='B';
              for(i=6; i<10; i++)//�¶�
              {
                  uTxData[i]=pRxData[i-2];
              }
              uTxData[10]='C';
              for(i=11; i<15; i++)//ʪ��
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
            ////////////////////////////////��������//AXXXXBXXXXCXXXX///15�ַ�
            //////////////////////////////////////////////////////���ݱ���
            for(i=0; i<15; i++)
            {
             uTxData1[i]=uTxData[i];
            }
            /////////////////////////////////////////////////////���ݱ���
            /////////////////////////////////////////////////�Ƶķ����Զ�����
            if((uTxData[3]<'5')&(uTxData[1]<'1'))//����ǿ��С��0.5���򿪵�
            {
              uRxData[0]='1';uRxData[1]='1';
              basicRfSendPacket(SEND_ADDR6, uRxData,2);
            }
            else
            {
              uRxData[0]='1';uRxData[1]='0';
              basicRfSendPacket(SEND_ADDR6, uRxData,2);
            }
            //////////////////////////////////////////////////�Ƶķ����Զ�����
            

            
          /*  if((uTxData[6]>='1')&(uTxData[7]>='5'))//�¶ȶȴ���15���򿪷���
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
            //////////////////////////////////////////////////�Ƶķ����Զ�����
            k1=k1+1;
            if(k1==20)
            {  
              
              
            uRxData[0]=0xCE;
            uRxData[1]=0xC2;
            uRxData[2]=0xB6;
            uRxData[3]=0xC8;
            uRxData[4]=uTxData[6];
            uRxData[5]=uTxData[7];
            uRxData[6]=uTxData[8];            ///led�¶���ʾ
            uRxData[7]=uTxData[9];
            uRxData[8]=0xB6;
            uRxData[9]=0xC8;
            basicRfSendPacket(SEND_ADDR3, uRxData,10);
                if((uTxData[6]>='1')&(uTxData[7]>='5'))//�¶ȶȴ���15���򿪷���
            {
              //uRxData[0]='1';uRxData[1]='1';
                uRxData[0]=0xCE ;
                uRxData[1]=0xC2 ;
                uRxData[2]=0xB6 ;
                uRxData[3]=0xC8 ;
                uRxData[4]=0xB9 ;    ///�¶ȹ���
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
                uRxData[16]=uTxData[8];            ///led�¶���ʾ
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
            //���ݷ��͵�PC��
            halUartWrite(uTxData,15);
        } 
    }
}
/************************MAIN END ****************************/