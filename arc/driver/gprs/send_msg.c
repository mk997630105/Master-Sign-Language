/* ------------------------------------------
 * \version 2016-01
 * \date 2018-04-07
 * \author Yongchen Bai
 * \functionGPS��λģ��
--------------------------------------------- */
#include "arc.h"
#include "arc_builtin.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdio.h>
#include "embARC.h"
#include "embARC_debug.h"
#include "embARC_toolchain.h"
#include "embARC_error.h"
#include "board.h"
#include "dev_uart.h"
#include "dev_iic.h"
#define I2C_SLAVE_ADDR2  0x01
DEV_IIC *dev_iic2=NULL;
uint8_t *phoneNum = "18710890684";
uint8_t DH_id_sep[20];                            //ȫ�ֱ������飬��ദ��20����������
struct   GPS_Information
{
    uint8_t Located;           //��λ��Чλ
    uint8_t UTC_Time[7];       //ʱ��
    uint8_t UTC_Date[7];       //����
    char    Latitude[11];      //γ��
    uint8_t NS_Indicator;      //N=������S=�ϰ���
    char    Longitude[12];     //����
    uint8_t EW_Indicator;      //E=������W=����
    uint8_t Speed[7];          //��������
    uint8_t Course[7];         //���溽��
    uint8_t MSL_Altitude[7];   //MSL���θ߶�
    uint8_t Use_EPH_Sum[2];    //ʹ�õ���������
	char lat_long[100];        //�����ľ�γ��
}*GPS;                                    

struct UTC_Time_Date
{										    
 	uint8_t year;	//���
	uint8_t month;	//�·�
	uint8_t date;	//����
	uint8_t hour; 	//Сʱ
	uint8_t min; 	//����
	uint8_t sec; 	//����
}*UTC;

struct Latitude
{										    
	uint8_t du; 	//��
	uint8_t fen; 	//��
	uint8_t miao; 	//��
}*Lati;

struct  Longitude
{										    
	uint8_t du; 	//��
	uint8_t fen; 	//��
	uint8_t miao; 	//��
}*Long;
static void delay_ms(volatile uint32_t z)//��ʱ����,��λ��ms
{
	volatile uint32_t x,y;
	for (x = 1400 ;x > 0; x --)
		for (y = z ;y > 0; y --);
}

void Creat_DH_Index(uint8_t k ,int8_t* buffer )  //�洢һ֡������ÿ�����ŵĵ�ַ
{
	uint8_t i;
	uint8_t idj = 0;
    for ( i = 0;i < k;i++ )
    {
        if ( buffer[i] == ',' )
        {
            DH_id_sep[idj] = i;
            idj++;
        }
    }
}

int8_t* Real_Process_DH( int8_t* buffer, uint8_t num )  //���ص�num��������һλ���ݵĵ�ַ
{
    if ( num < 1 )
        return  &buffer[0];
	else
        return  &buffer[ DH_id_sep[num - 1] + 1];
}

void Real_GPS_Command_Process(uint8_t k, char *data )
{
	char *rd_pt;
	uint8_t i,t;

	for(i=0;i<k;i++)
	{
			rd_pt = Real_Process_DH( data, 1 );//��1������ΪUTCʱ�䣬ʱ����
			if ( ( *rd_pt >= '0' ) && ( *rd_pt <= '9' ) )//����������0��9
			{
				for(t=0;t<6;t++)
				{
					if( *rd_pt == ',' )  break;
					GPS->UTC_Time[t] = *rd_pt;
					rd_pt++;
			    }
				GPS->UTC_Time[t] = '\0';
                printf("GPS->UTC_Time=%s\r\n",GPS->UTC_Time);
			}
			rd_pt = Real_Process_DH( data, 2 ); //��3������Ϊγ��
			if ( ( *rd_pt >= '0' ) && ( *rd_pt <= '9' ) )
			{
				for(t=0;t<9;t++)
				{
					if( *rd_pt == ',' )  break;
					GPS->Latitude[t] = *rd_pt;
					rd_pt++;
				}
			    GPS->Latitude[t] = '\0';  
				printf("GPS->Latitude=%s\r\n",GPS->Latitude);
			}
			
			rd_pt = Real_Process_DH( data, 3 );    //��4������Ϊγ�Ȱ��򣬱�γ����γ
			if ( ( *rd_pt == 'N' ) || ( *rd_pt == 'S' ) )
			{
				GPS->NS_Indicator = *rd_pt;
				EMBARC_PRINTF("GPS->NS_Indicator=%c\r\n",GPS->NS_Indicator);
			}
			
			rd_pt = Real_Process_DH( data, 4 );    //��5������Ϊ����
			if ( ( *rd_pt >= '0' ) && ( *rd_pt <= '9' ) )
			{
				for(t=0;t<9;t++)
				{
					if( *rd_pt == ',' )  break;
					GPS->Longitude[t] = *rd_pt;
					rd_pt++;
				}
				GPS->Longitude[t] = '\0';
				EMBARC_PRINTF("GPS->Longitude=%s\r\n",GPS->Longitude);
			}
			
			rd_pt = Real_Process_DH( data, 5 );    //��6������Ϊ���Ȱ��򣬶���������
			if ( ( *rd_pt == 'E' ) || ( *rd_pt == 'W' ) )
			{
				GPS->EW_Indicator = *rd_pt;
				EMBARC_PRINTF("GPS->EW_Indicator=%c\r\n",GPS->EW_Indicator);
			}
			
			rd_pt = Real_Process_DH( data, 7 );     //��7������Ϊ��������
			if ( ( *rd_pt >= '0' ) && ( *rd_pt <= '9' ) )
			{
				for(t=0;t<7;t++)
				{
					if( *rd_pt == ',' )  break;
					GPS->Speed[t] = *rd_pt;
					rd_pt++;
				}
				GPS->Speed[t] = '\0';
				EMBARC_PRINTF("GPS->Speed=%s\r\n",GPS->Speed);
			}
			
			rd_pt = Real_Process_DH( data, 8 );     //��8������Ϊ���溽��
			if ( ( *rd_pt >= '0' ) && ( *rd_pt <= '9' ) )
			{
				for(t=0;t<7;t++)
				{
					if( *rd_pt == ',' )  break;
					GPS->Course[t] = *rd_pt;
					rd_pt++;
				}
				GPS->Course[t] = '\0';
				EMBARC_PRINTF("GPS->Course=%s\r\n",GPS->Course);
			}
			
			rd_pt = Real_Process_DH( data, 9 );     //��9������ΪUTCʱ�䣬������
			if ( ( *rd_pt >= '0' ) && ( *rd_pt <= '9' ) )
			{
				for(t=0;t<6;t++)
				{
					if( *rd_pt == ',' )  break;
					GPS->UTC_Date[t] = *rd_pt;
					rd_pt++;
			    }
				GPS->UTC_Date[t] = '\0';
				EMBARC_PRINTF("GPS->UTC_Date=%s\r\n",GPS->UTC_Date);
			}
			rd_pt = Real_Process_DH( data, 7 );      //��7������Ϊ����ʹ�õ����ڶ�λ����������
			if ( ( *rd_pt >= '0' ) && ( *rd_pt <= '9' ) )
			{
				for(t=0;t<2;t++)
				{
					if( *rd_pt == ',' )  break;
					GPS->Use_EPH_Sum[t] = *rd_pt;
					rd_pt++;
			    }
				GPS->Use_EPH_Sum[t] = '\0';
				EMBARC_PRINTF("GPS->Use_EPH_Sum=%s\r\n",GPS->Use_EPH_Sum);
			}
			
			rd_pt = Real_Process_DH( data, 9 );        //��9������Ϊ���θ߶�
			if ( ( *rd_pt >= '0' ) && ( *rd_pt <= '9' ) )
			{
				for(t=0;t<6;t++)
				{
					if( *rd_pt == ',' )  break;
					GPS->MSL_Altitude[t] = *rd_pt;
					rd_pt++;
			    }
				GPS->MSL_Altitude[t] = '\0';
				EMBARC_PRINTF("GPS->MSL_Altitude=%s\r\n",GPS->MSL_Altitude);
			}
			
		
    }
}



uint8_t Calc_GPS_Sum( const int8_t* Buffer )
{
    uint8_t i, j, k, sum;
    sum = 0;

    for ( i = 0; i < 100; i++ ) //i��1��ʼ������$��ʼ��
    {
        if ( Buffer[i] != '*' ) //�жϽ�����
            sum ^= Buffer[i];   //GPSУ����㷨ΪXOR
        else
            break;
    }
	
    j = Buffer[i + 1]; //ȡ����������λ�ַ�
    k = Buffer[i + 2]; //ȡ����������λ�ַ�

    if ( isalpha( j ) ) //�ж��ַ��Ƿ�ΪӢ����ĸ��ΪӢ����ĸʱ���ط���ֵ�����򷵻���
    {
        if ( isupper( j ) ) //�ж��ַ�Ϊ��дӢ����ĸʱ�����ط���ֵ�����򷵻���
            j -= 0x37;//ǿ��ת��Ϊ16����
        else
            j -= 0x57;//ǿ��ת��Ϊ16����
    }
    else
    {
        if ( ( j >= 0x30 ) && ( j <= 0x39 ) )
            j -= 0x30;//ǿ��ת��Ϊ16����
    }

    if ( isalpha( k ) ) //�ж��ַ��Ƿ�ΪӢ����ĸ��ΪӢ����ĸʱ���ط���ֵ�����򷵻���
    {
        if ( isupper( k ) ) //�ж��ַ�Ϊ��дӢ����ĸʱ�����ط���ֵ�����򷵻���
            k -= 0x37;//ǿ��ת��Ϊ16����
        else
            k -= 0x57;//ǿ��ת��Ϊ16����
    }
    else
    {
        if ( ( k >= 0x30 ) && ( k <= 0x39 ) )
            k -= 0x30;//ǿ��ת��Ϊ16����
    }

    j = ( j << 4 ) + k; //ǿ�ƺϲ�Ϊ16����

    if ( sum == j )
        return 1; //У�������
    else
        return 0; //У��ʹ���
}

double resolve(char *buf)
{	
	double t;
	int  d;
	t=atof(buf);
	d=(int)(t/100);
	t=d+(t-d*100)/60;
	return t;
}


void send(uint8_t *phoneNum, uint8_t *msg)
{	
	dev_iic2 = iic_get_dev(DW_IIC_1_ID	);
    dev_iic2->iic_open(DEV_MASTER_MODE, IIC_SPEED_STANDARD);
	dev_iic2->iic_control(IIC_CMD_MST_SET_TAR_ADDR, CONV2VOID(I2C_SLAVE_ADDR2));
	uint8_t i;
	uint8_t pnum[64];
	int8_t *cmd[5]={"AT+CMGF=1\r\n", "AT+CSCS=\"GSM\"\r\n",
				  "AT+CSCA?\r\n",  "AT+CSMP=17,167,0,240\r\n",
				   };
	snprintf(pnum, 64, "AT+CMGS=\"%s\"\r\n", phoneNum);
	for(i = 0; i < 4; i++)
	{
		dev_iic2->iic_write(cmd[i], strlen(cmd[i]));
		printf("cmd\r\n");
		delay_ms(1000);
	}	
	dev_iic2->iic_write(pnum, strlen(pnum));
	printf("pnum\r\n");
	delay_ms(1000);
	dev_iic2->iic_write(msg, strlen(msg));
	printf("msg\r\n");
	delay_ms(1000);
	uint8_t end[]={0x1a,0x00};
	dev_iic2->iic_write(end, strlen(end));printf("end\r\n");
	
}

void send_msg(void)
{
	DEV_UART *dev_uart2 = NULL;
	uint8_t rcv_buf[10];
	int8_t data[100];
	uint32_t rcv_cnt;
	uint32_t k;
	uint32_t length;
	uint32_t baudrate = 9600;
	uint32_t rd_avail = 0;
	char  Longitude[]={"11540.5393"};
	char  Latitude[]={"3743.5044"};
	cpu_lock();
	board_init(); 
	cpu_unlock();
	
	dev_uart2 = uart_get_dev(DW_UART_2_ID);
	dev_uart2->uart_open(baudrate);

	while (1) 
	{
	
		k = 0;
		dev_uart2->uart_control(UART_CMD_GET_RXAVAIL, (void *)(&rd_avail));
		if(rd_avail > 0)
		{
			rcv_cnt = dev_uart2->uart_read(rcv_buf, 1);
			rcv_buf[rcv_cnt] = '\0';
			if (rcv_buf[0]=='$')         //Ѱ��֡������ʼλ
			{
				while (1) 
				{
					dev_uart2->uart_control(UART_CMD_GET_RXAVAIL, (void *)(&rd_avail));
					if(rd_avail > 0)
					{
						rcv_cnt = dev_uart2->uart_read(rcv_buf, 1);
                        
						rcv_buf[rcv_cnt] = '\0';
						printf("rcv_buf[0]=%c\r\n",rcv_buf[0]);
						if ((rcv_cnt) && (rcv_buf[0] != '\r'))  //�ɹ����յ��ֽ���Ϊ1���ҽ��յ��ֽڲ���֡������־(CR)
							data[k++]= rcv_buf[0];		        //�����յ��ֽڷ���������					
						
					    if(rcv_buf[0]=='\r')      //���յ��ֽ���֡������־(CR)�����֡����
							break;
					}
				}
				data[k]='\0';
                if( Calc_GPS_Sum( data ) == 1 )
				    if ( ( data[3] == 'G' ) && ( data[4] == 'A' ))       //$GPGGA
				{
					Creat_DH_Index( k,data );
					Real_GPS_Command_Process( k,data );
					            double jingdu, weidu;
			                    char mymsg[32]="the location of your family is:";
			                             {
											
			                                jingdu=resolve(GPS->Longitude);
				                            weidu=resolve(GPS->Latitude);
				                            snprintf(GPS->lat_long,100,"%s longitude:%lf latitude:%lf\r\n",mymsg, jingdu, weidu);
				                            EMBARC_PRINTF("lat_long=%s",GPS->lat_long);
				                            EMBARC_PRINTF("\nyouxiao\n ");//
			                             }
					break;
				}
			}  
		}
	}
	send(phoneNum,GPS->lat_long);
	
}

			
	
