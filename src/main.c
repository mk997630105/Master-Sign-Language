#include "string.h"
#include "stdio.h"
#include "embARC.h"
#include "embARC_debug.h"
#include "embARC_error.h"

#include "u8g.h"

#define I2C_SLAVE_ADDR1  0x02
#define uchar unsigned char
#define uint unsigned int
#define HEADLEN       5  //���ݰ�ͷ�ĳ���
DEV_IIC *dev_iicyu=NULL;
//static DEV_GPIO_PTR port_call;//��绰
static DEV_GPIO_PTR port_swt1;//����ȷ���Ƿ�������﷭��
static DEV_GPIO_PTR port_swt2;//��Ӣ�л�
static DEV_GPIO_PTR port_swt3;//�Զ�����
static DEV_GPIO_PTR port_call;//һ������
static DEV_GPIO_PTR port_PA4;
static DEV_GPIO_PTR port_PA5;
static DEV_GPIO_PTR port_PA6;
static DEV_GPIO_PTR port_PA7;
int adc[4];
double aValue[3];
double wValue[3];
double AValue[3];
int cmd;
u8g_t u8g;
static void delay_ms(volatile int z) //1ms
{
	volatile uint32_t x,y;
	for(x=1400;x>0;x--)
		for(y=z;y>0;y--);
}
void  yuyin(char text[]) 
{    
    dev_iicyu = iic_get_dev(DW_IIC_1_ID	);
    dev_iicyu->iic_open(DEV_MASTER_MODE, IIC_SPEED_STANDARD);
	dev_iicyu->iic_control(IIC_CMD_MST_SET_TAR_ADDR, CONV2VOID(I2C_SLAVE_ADDR1));
/****************��Ҫ���͵��ı�**********************************/ 
	uint8_t  length=0x00;  
    char  ecc  = 0x00;  			//����У���ֽ�
	uint8_t i=0; 
    //length =sizeof(text); 			//��Ҫ�����ı��ĳ���
  while(text[length++]);
/*****************���͹���**************************************/           
		// headOfFrame[0] = 0xFD ; 			//����֡ͷFD
		// headOfFrame[1] = 0x00 ; 			//�������������ȵĸ��ֽ�
		// headOfFrame[2] = length + 3; 		//�������������ȵĵ��ֽ�
		// headOfFrame[3] = 0x01 ; 			//���������֣��ϳɲ�������
/*****************����������**************************************/  		 
		// headOfFrame[4] = 0x00 ; 			//������������������ʽΪGBK
       char head[HEADLEN] = {0xfd,0x00,0x00,0x01,0x00};
/*****************������������**************************************/  		 
		//headOfFrame[4] = 0x01 ; 			//������������������ʽΪGBK
      
		
		
		head[2] = length + 3;
		
		 for(i = 0; i<5; i++)   				//���η��͹���õ�5��֡ͷ�ֽ�
	     {  
	         ecc=ecc^(head[i]); 		//�Է��͵��ֽڽ������У��	
	         //SBUF = headOfFrame[i]; 
			// printf("head[%d]=%d\r\n",i,head[i]);
			 
		 }   
		    // printf("length=%d\r\n",length);
	     
             dev_iicyu->iic_write(head, 5);
	   	 
		 
		 for(i = 0; i<length+1; i++)   		//���η��ʹ��ϳɵ��ı�����
	     {  
	          				//�Է��͵��ֽڽ������У��	
	         //SBUF = text[i]; 
			 if(i==length) 
				 text[i] = ecc;
			 else 
				 ecc=ecc^(text[i]);
			//printf("text[%d]=%d\r\n",i,text[i]);
	      
	     }
		   //printf("ecc=%d\r\n",ecc);
            dev_iicyu->iic_write(text, length+1);
			
            delay_ms(200);			
		   }

int main (void){
//uint32_t call_read;
	uint32_t DIP1,DIP2,DIP3;
	uint32_t call_read,PA4,PA5,PA6,PA7;
	uint32_t Y[4];
	cpu_lock();	/* lock cpu to do initializations */

	board_init(); /* board level init */

	cpu_unlock();	/* unlock cpu to let interrupt work */
	//initialize the uart0 which is on PMOD1 J3 for RXD J4 for TXD
    //dev_uart2 = uart_get_dev(DW_UART_2_ID);
	//dev_uart2->uart_open(baudrate1);
	u8g_InitComFn(&u8g, &u8g_dev_ssd1306_128x64_2x_i2c, U8G_COM_SSD_I2C); /* create a new interface to a graphics display */
	u8g_Begin(&u8g); /* reset display and put it into default state */
	u8g_prepare();
	     port_swt1 = gpio_get_dev(DW_GPIO_PORT_A);
	     port_swt1->gpio_open(0x100);
	     port_swt1->gpio_control(GPIO_CMD_SET_BIT_DIR_INPUT, (void *)0x100);
	     port_PA4 = gpio_get_dev(DW_GPIO_PORT_C);  
         port_PA4 -> gpio_open(0x10000);	
	     port_PA4 -> gpio_control(GPIO_CMD_SET_BIT_DIR_INPUT, (void *)0x10000);		
		 port_PA5 = gpio_get_dev(DW_GPIO_PORT_C);  
         port_PA5 -> gpio_open(0x20000);	
	     port_PA5 -> gpio_control(GPIO_CMD_SET_BIT_DIR_INPUT, (void *)0x20000);	
		 port_PA6 = gpio_get_dev(DW_GPIO_PORT_C);  
         port_PA6 -> gpio_open(0x40000);	
	     port_PA6 -> gpio_control(GPIO_CMD_SET_BIT_DIR_INPUT, (void *)0x40000);		
		 port_PA7 = gpio_get_dev(DW_GPIO_PORT_C);  
         port_PA7 -> gpio_open(0x80000);	
	     port_PA7 -> gpio_control(GPIO_CMD_SET_BIT_DIR_INPUT, (void *)0x80000);		
		 
while(1)
 {
	  
	   port_swt1->gpio_read(&DIP1, 0x100);
	   printf("DIP1=0x%x\r\n",DIP1);
	   if(DIP1==0x0)
	   {
		   char textstop[]={"stop"};
		   u8g_FirstPage(&u8g); /* marks the beginning of the picture loop; it cannot be used inside the picture loop */
		           do 
			       {
			          draw(textstop);
		            } while (u8g_NextPage(&u8g)); 
	   }
 else  if(DIP1==0x100)
	{   
        port_call = gpio_get_dev(DW_GPIO_PORT_A);  
	     port_call -> gpio_open(0x800);	
	     port_call -> gpio_control(GPIO_CMD_SET_BIT_DIR_INPUT, (void *)0x800);		
	     port_call -> gpio_read(&call_read, 0x800);	
		 printf("call_read=0x%x\r\n",call_read);
		if(call_read==0x800)
		  {	
			  delay_ms(400);
			  send_msg();
		   printf("ABCD\r\n");
		   }	
		
	  port_PA4 -> gpio_read(&PA4, 0x10000);	
	  port_PA5 -> gpio_read(&PA5, 0x20000);	
	  port_PA6 -> gpio_read(&PA6, 0x40000);	
	  port_PA7 -> gpio_read(&PA7, 0x80000);	
	  Y[0]=PA7;
	  Y[1]=PA6;
	  Y[2]=PA5;
	  Y[3]=PA4;
	  printf("PA7=0x%x\r\n",PA7);
	  printf("PA6=0x%x\r\n",PA6);
	  printf("PA5=0x%x\r\n",PA5);
	  printf("PA4=0x%x\r\n",PA4);
	  printf("Y[0]=0x%x\r\n",Y[0]);
	  printf("Y[1]=0x%x\r\n",Y[1]);
	  printf("Y[2]=0x%x\r\n",Y[2]);
	  printf("Y[3]=0x%x\r\n",Y[3]);
     if(Y[0]==0x0&Y[1]==0x0&Y[2]==0x0&Y[3]==0x0)	
         {printf("���\r\n"); 
	      u8g_FirstPage(&u8g);
		  do 
			  {
			        drawBhello();
		      } while (u8g_NextPage(&u8g)); 
		  delay_ms(1);
		  }
	     
	    
	 if(Y[0]==0x80000&Y[1]==0x0&Y[2]==0x0&Y[3]==0x0)		
	    {printf("���ʲô\r\n");	
	     u8g_FirstPage(&u8g); 
		  do 
			  {
			        drawByourname();
		      } while (u8g_NextPage(&u8g));
		delay_ms(1);
		}	
		 
	     
	 if(Y[0]==0x0&Y[1]==0x40000&Y[2]==0x0&Y[3]==0x0)	
	    {printf("�ҽ�����\r\n");	
	     u8g_FirstPage(&u8g); 
		  do 
			  {
			        drawBlisi();
		      } while (u8g_NextPage(&u8g));
			  delay_ms(1);
		}	
		
	     
	 if(Y[0]==0x80000&Y[1]==0x40000&Y[2]==0x0&Y[3]==0x0)		
	    {printf("��22\r\n");
	     u8g_FirstPage(&u8g); 
		  do 
			  {
			        drawBmyage();
		      } while (u8g_NextPage(&u8g));	
			  delay_ms(1);
		}	
		 
	 if(Y[0]==0x0&Y[1]==0x0&Y[2]==0x20000&Y[3]==0x0)	
	    { printf("����\r\n");
	      u8g_FirstPage(&u8g); 
		  do 
			  {
			       drawBnine()	;
		      } while (u8g_NextPage(&u8g));	
		 delay_ms(1);
		 }
		
	     
	 if(Y[0]==0x80000&Y[1]==0x0&Y[2]==0x20000&Y[3]==0x0)		
	    {printf("��Ҳ��\r\n");	
	     u8g_FirstPage(&u8g); 
		  do 
			  {
			         drawBmetoo();
		      } while (u8g_NextPage(&u8g));
		 delay_ms(1);
			  }
			
	  
	 if(Y[0]==0x0&Y[1]==0x40000&Y[2]==0x20000&Y[3]==0x0)		
        {printf("��Ҫȥ̨��\r\n");
	    u8g_FirstPage(&u8g); 
		  do 
			  {
			         drawBgotw() ;
		      } while (u8g_NextPage(&u8g));
		delay_ms(1);
		}
		 
	    
	  if(Y[0]==0x80000&Y[1]==0x40000&Y[2]==0x20000&Y[3]==0x0)	
         {printf("�ҽ�������\r\n");
	      u8g_FirstPage(&u8g); 
		  do 
			  {
			        drawBruwei();
		      } while (u8g_NextPage(&u8g));
		 delay_ms(1);
		 }
		 
	    
	 if(Y[0]==0x0&Y[1]==0x0&Y[2]==0x0&Y[3]==0x10000)		
        {printf("��֪��̨����ʲô��ʤ\r\n");
	     u8g_FirstPage(&u8g);
		  do 
			  {
			         drawBchat1();
		      } while (u8g_NextPage(&u8g));	
		delay_ms(1);
			  } 
		
	    
	  if(Y[0]==0x80000&Y[1]==0x0&Y[2]==0x0&Y[3]==0x10000)	
	     {printf("�ð�\r\n");	
	      u8g_FirstPage(&u8g); 
		  do 
			  {
			         drawBhaoba();
		      } while (u8g_NextPage(&u8g));
			  delay_ms(1);
		}
		
	   
	  if(Y[0]==0x0&Y[1]==0x40000&Y[2]==0x0&Y[3]==0x10000)	
       {printf("лл\r\n");		
        u8g_FirstPage(&u8g); 
		  do 
			  {
			         drawBthank();
		      } while (u8g_NextPage(&u8g));
	     delay_ms(1);
	 } 
	   port_swt3 = gpio_get_dev(DW_GPIO_PORT_A);
	   port_swt3->gpio_open(0x400);
	   port_swt3->gpio_control(GPIO_CMD_SET_BIT_DIR_INPUT, (void *)0x400);
	   port_swt3->gpio_read(&DIP3, 0x400);
	   printf("DIP3=0x%x\r\n",DIP3);
	  if(DIP3==0x400)
		  {
			EMBARC_PRINTF("�Զ�����\r\n");
			data(adc,aValue,wValue,AValue);
		    cmd=judge(aValue,wValue,AValue,adc,cmd);
			if(cmd==1)
		        {
		          char texta1[]={"��Һã�������������������������������������о���"};
                   yuyin(texta1);
		            cmd=0;
		        }
		        else if(cmd==2)
		        {
			      char textb1[]={"���ã�����·�ˣ������԰���һ����"};
			      yuyin(textb1);
			        cmd=0;
		        }
		         else if(cmd==3)
		        {
		            char textc1[]={"�ܸ�����ʶ��"};
			        yuyin(textc1);
			        cmd=0;
			    }
		        else if(cmd==4)
		        {
			        char textd1[]={"����һ���ȥ�Ķ�"};
			        yuyin(textd1);
			        cmd=0;
			    }
			     else if(cmd==5)
		        {
			        char textE1[]={"����������ð�"};
			        yuyin(textE1);
			         cmd=0;
		         }
				 else if(cmd==6)
		        {
			        char textf1[]={"f"};
			    
			         cmd=0;
		         }
			
		   }
		  else
		   {
		     port_swt2 = gpio_get_dev(DW_GPIO_PORT_A);
	         port_swt2->gpio_open(0x200);
	         port_swt2->gpio_control(GPIO_CMD_SET_BIT_DIR_INPUT, (void *)0x200);
	         port_swt2->gpio_read(&DIP2, 0x200);
			 printf("DIP2=0x%x\r\n",DIP2);
		     if(DIP2==0x200)
			 {
		       data(adc,aValue,wValue,AValue);
		       cmd=judge(aValue,wValue,AValue,adc,cmd);
			   if(cmd==1)
		        {
		          char texta[]={"A"};
                   yuyin(texta);
			       u8g_FirstPage(&u8g); /* marks the beginning of the picture loop; it cannot be used inside the picture loop */
		           do 
			       {
			          draw(texta);
		            } while (u8g_NextPage(&u8g)); 
		            cmd=0;
		        }
		        else if(cmd==2)
		        {
			      char textb[]={"B"};
			      yuyin(textb);
			      u8g_FirstPage(&u8g); /* marks the beginning of the picture loop; it cannot be used inside the picture loop */
		           do 
			       {
			         draw(textb);
		            } while (u8g_NextPage(&u8g));  
			        cmd=0;
		        }
		         else if(cmd==3)
		        {
		            char textc[]={"C"};
			        yuyin(textc);
			        u8g_FirstPage(&u8g); /* marks the beginning of the picture loop; it cannot be used inside the picture loop */
		            do 
			         {
			           draw(textc);
		              } while (u8g_NextPage(&u8g));  
			        cmd=0;
			    }
		        else if(cmd==4)
		        {
			        char textd[]={"D"};
			        yuyin(textd);
			        u8g_FirstPage(&u8g); /* marks the beginning of the picture loop; it cannot be used inside the picture loop */
		            do
			         {
			            draw(textd);
		              } while (u8g_NextPage(&u8g)); 
			        cmd=0;
			    }
			     else if(cmd==5)
		        {
			        char textE[]={"E"};
			        yuyin(textE);
			        u8g_FirstPage(&u8g); /* marks the beginning of the picture loop; it cannot be used inside the picture loop */
		            do 
			        {
			          draw(textE);
		            } while (u8g_NextPage(&u8g));  
			         cmd=0;
		         }
		          else if(cmd==6)
		         {
		              char textF[]={"F"};
			           yuyin(textF);
			           u8g_FirstPage(&u8g); /* marks the beginning of the picture loop; it cannot be used inside the picture loop */
		               do 
			           {
			              draw(textF);
		                } while (u8g_NextPage(&u8g));  
			            cmd=0;
			      }
		          else if(cmd==7)
		          {
			            char textG[]={"G"};
			            yuyin(textG);
			            u8g_FirstPage(&u8g); /* marks the beginning of the picture loop; it cannot be used inside the picture loop */
		                do
			            {
			               draw(textG);
		                } while (u8g_NextPage(&u8g)); 
			             cmd=0;
			       }
			       else if(cmd==8)
		           {
			           char textH[]={"H"};
			           yuyin(textH);
			           u8g_FirstPage(&u8g); /* marks the beginning of the picture loop; it cannot be used inside the picture loop */
		               do 
			          {
			             draw(textH);
		               } while (u8g_NextPage(&u8g));  
			          cmd=0;
		            }
		             else if(cmd==9)
		            {
		               char textI[]={"I"};
			            yuyin(textI);
			            u8g_FirstPage(&u8g); /* marks the beginning of the picture loop; it cannot be used inside the picture loop */
		                do 
			            {
			               draw(textI);
		                 } while (u8g_NextPage(&u8g));  
			              cmd=0;
			        }
		            else if(cmd==10)
		           {
			            char textJ[]={"J"};
			             yuyin(textJ);
			             u8g_FirstPage(&u8g); /* marks the beginning of the picture loop; it cannot be used inside the picture loop */
		                 do
			             {
			                draw(textJ);
		                  } while (u8g_NextPage(&u8g)); 
			              cmd=0;
			        }
			        else if(cmd==11)
		            {
			            char textK[]={"K"};
			             yuyin(textK);
			             u8g_FirstPage(&u8g); /* marks the beginning of the picture loop; it cannot be used inside the picture loop */
		                 do 
			              {
			                 draw(textK);
		                   } while (u8g_NextPage(&u8g));  
			              cmd=0;
		            }
		             else if(cmd==12)
	             	{
	                 	char textL[]={"L"};
			            yuyin(textL);
			            u8g_FirstPage(&u8g); /* marks the beginning of the picture loop; it cannot be used inside the picture loop */
		                do 
			            {
			                 draw(textL);
		                 } while (u8g_NextPage(&u8g));  
			             cmd=0;
			        }
		             else if(cmd==13)
		            {
			           char textM[]={"M"};
			           yuyin(textM);
			           u8g_FirstPage(&u8g); /* marks the beginning of the picture loop; it cannot be used inside the picture loop */
		                do
			           {
			             draw(textM);
		                } while (u8g_NextPage(&u8g)); 
			           cmd=0;
			        }
			         else if(cmd==14)
		            {
			           char textN[]={"N"};
			           yuyin(textN);
			          u8g_FirstPage(&u8g); /* marks the beginning of the picture loop; it cannot be used inside the picture loop */
		              do 
			           {
			             draw(textN);
		               } while (u8g_NextPage(&u8g));  
			           cmd=0;
		            }
		            else if(cmd==15)
		            {
		              char textO[]={"O"};
		   	          yuyin(textO);
			          u8g_FirstPage(&u8g); /* marks the beginning of the picture loop; it cannot be used inside the picture loop */
		               do 
			          {
			               draw(textO);
		              } while (u8g_NextPage(&u8g));  
			           cmd=0;
			        }
		            else if(cmd==16)
		           {
			           char textP[]={"P"};
			           yuyin(textP);
			           u8g_FirstPage(&u8g); /* marks the beginning of the picture loop; it cannot be used inside the picture loop */
		                do
			             {
			                draw(textP);
		                } while (u8g_NextPage(&u8g)); 
			           cmd=0;
			        }
			        else if(cmd==17)
		            {
			            char textQ[]={"Q"};
			             yuyin(textQ);
			             u8g_FirstPage(&u8g); /* marks the beginning of the picture loop; it cannot be used inside the picture loop */
		                 do 
			             {
			                draw(textQ);
		                  } while (u8g_NextPage(&u8g));  
			             cmd=0;
		            }
		            else if(cmd==18)
		             {
		                 char textR[]={"R"};
			              yuyin(textR);
		                	u8g_FirstPage(&u8g); /* marks the beginning of the picture loop; it cannot be used inside the picture loop */
		                  do 
		                  	{
			                  draw(textR);
		                    } while (u8g_NextPage(&u8g));  
			               cmd=0;
			         }
		            else
		                 {
		    //char text5[]={"0"};
				//yuyin(text5);
				
		                  }
		       //delay_ms(1500);	
	}
	          else
	         {
		       data(adc,aValue,wValue,AValue);
		       cmd=judge1(aValue,wValue,AValue,adc,cmd);
			   if(cmd==1)
		        {
		          char text1[]={"���"};
                   yuyin(text1);
			       u8g_FirstPage(&u8g); /* marks the beginning of the picture loop; it cannot be used inside the picture loop */
		           do 
			       {
			          drawhowold();
		            } while (u8g_NextPage(&u8g)); 
		            cmd=0;
		        }
		        else if(cmd==2)
		        {
			      char text2[]={"����"};
			      yuyin(text2);
			      u8g_FirstPage(&u8g); /* marks the beginning of the picture loop; it cannot be used inside the picture loop */
		           do 
			       {
			         drawkeyi();
		            } while (u8g_NextPage(&u8g));  
			        cmd=0;
		        }
		         else if(cmd==3)
		        {
		            char text3[]={"��"};
			        yuyin(text3);
			        drawyou(); /* marks the beginning of the picture loop; it cannot be used inside the picture loop */
		            do 
			         {
			           draw(text3);
		              } while (u8g_NextPage(&u8g));  
			        cmd=0;
			    }
		        else if(cmd==4)
		        {
			        char text4[]={"����"};
			        yuyin(text4);
			        u8g_FirstPage(&u8g); /* marks the beginning of the picture loop; it cannot be used inside the picture loop */
		            do
			         {
			            drawclever();
		              } while (u8g_NextPage(&u8g)); 
			        cmd=0;
			    }
			     else if(cmd==5)
		        {
			        char text5[]={"��"};
			        yuyin(text5);
			        u8g_FirstPage(&u8g); /* marks the beginning of the picture loop; it cannot be used inside the picture loop */
		            do 
			        {
			          drawzhang();
		            } while (u8g_NextPage(&u8g));  
			         cmd=0;
		         }
		          else if(cmd==6)
		         {
		              char text6[]={"ʹ��"};
			           yuyin(text6);
			           u8g_FirstPage(&u8g); /* marks the beginning of the picture loop; it cannot be used inside the picture loop */
		               do 
			           {
			              drawuse();
		                } while (u8g_NextPage(&u8g));  
			            cmd=0;
			      }
		          else if(cmd==7)
		          {
			            char text7[]={"ȥ"};
			            yuyin(text7);
			            u8g_FirstPage(&u8g); /* marks the beginning of the picture loop; it cannot be used inside the picture loop */
		                do
			            {
			               drawgo();
		                } while (u8g_NextPage(&u8g)); 
			             cmd=0;
			       }
			       else if(cmd==8)
		           {
			           char text8[]={"23"};
			           yuyin(text8);
			           u8g_FirstPage(&u8g); /* marks the beginning of the picture loop; it cannot be used inside the picture loop */
		               do 
			          {
			             draw(text8);
		               } while (u8g_NextPage(&u8g));  
			          cmd=0;
		            }
		             else if(cmd==9)
		            {
		               char text9[]={"��"};
			            yuyin(text9);
			            u8g_FirstPage(&u8g); /* marks the beginning of the picture loop; it cannot be used inside the picture loop */
		                do 
			            {
			               drawhen();
		                 } while (u8g_NextPage(&u8g));  
			              cmd=0;
			        }
		            else if(cmd==10)
		           {
			            char text10[]={"Ư��"};
			             yuyin(text10);
			             u8g_FirstPage(&u8g); /* marks the beginning of the picture loop; it cannot be used inside the picture loop */
		                 do
			             {
			                drawpiaoliang();
		                  } while (u8g_NextPage(&u8g)); 
			              cmd=0;
			        }
			        else if(cmd==11)
		            {
			            char text11[]={"��"};
			             yuyin(text11);
			             u8g_FirstPage(&u8g); /* marks the beginning of the picture loop; it cannot be used inside the picture loop */
		                 do 
			              {
			                 drawni();
		                   } while (u8g_NextPage(&u8g));  
			              cmd=0;
		            }
		             else if(cmd==12)
	             	{
	                 	char text12[]={"��"};
			            yuyin(text12);
			            u8g_FirstPage(&u8g); /* marks the beginning of the picture loop; it cannot be used inside the picture loop */
		                do 
			            {
			                 drawhao();
		                 } while (u8g_NextPage(&u8g));  
			             cmd=0;
			        }
		             else if(cmd==13)
		            {
			           char text13[]={"ף"};
			           yuyin(text13);
			           u8g_FirstPage(&u8g); /* marks the beginning of the picture loop; it cannot be used inside the picture loop */
		                do
			           {
			             drawzhu();
		                } while (u8g_NextPage(&u8g)); 
			           cmd=0;
			        }
			         else if(cmd==14)
		            {
			           char text14[]={"��;"};
			           yuyin(text14);
			          u8g_FirstPage(&u8g); /* marks the beginning of the picture loop; it cannot be used inside the picture loop */
		              do 
			           {
			             drawlvtu();
		               } while (u8g_NextPage(&u8g));  
			           cmd=0;
		            }
		            else if(cmd==15)
		            {
		              char text15[]={"��ʶ"};
		   	          yuyin(text15);
			          u8g_FirstPage(&u8g); /* marks the beginning of the picture loop; it cannot be used inside the picture loop */
		               do 
			          {
			               drawrenshi();
		              } while (u8g_NextPage(&u8g));  
			           cmd=0;
			        }
		            else if(cmd==16)
		           {
			           char text16[]={"��"};
			           yuyin(text16);
			           u8g_FirstPage(&u8g); /* marks the beginning of the picture loop; it cannot be used inside the picture loop */
		                do
			             {
			                drawsan();
		                } while (u8g_NextPage(&u8g)); 
			           cmd=0;
			        }
			        else if(cmd==17)
		            {
			            char text17[]={"����"};
			             yuyin(text17);
			             u8g_FirstPage(&u8g); /* marks the beginning of the picture loop; it cannot be used inside the picture loop */
		                 do 
			             {
			                drawhappy();
		                  } while (u8g_NextPage(&u8g));  
			             cmd=0;
		            }
		            else if(cmd==18)
		             {
		                 char text18[]={"���"};
			              yuyin(text18);
		                	u8g_FirstPage(&u8g); /* marks the beginning of the picture loop; it cannot be used inside the picture loop */
		                  do 
		                  	{
			                  drawyukuai();
		                    } while (u8g_NextPage(&u8g));  
			               cmd=0;
			         }
		        else if(cmd==19)
		        {
			      char text19[]={"��"};
			      yuyin(text19);
			      u8g_FirstPage(&u8g); /* marks the beginning of the picture loop; it cannot be used inside the picture loop */
		           do 
			       {
			         drawbu();
		            } while (u8g_NextPage(&u8g));  
			        cmd=0;
		        }
		         else if(cmd==20)
		        {
		            char text20[]={"���"};
			        yuyin(text20);
			        u8g_FirstPage(&u8g); /* marks the beginning of the picture loop; it cannot be used inside the picture loop */
		            do 
			         {
			           drawhello();
		              } while (u8g_NextPage(&u8g));  
			        cmd=0;
			    }
		        else if(cmd==21)
		        {
			        char text21[]={"ʲô"};
			        yuyin(text21);
			        u8g_FirstPage(&u8g); /* marks the beginning of the picture loop; it cannot be used inside the picture loop */
		            do
			         {
			            drawwhat();
		              } while (u8g_NextPage(&u8g)); 
			        cmd=0;
			    }
			     else if(cmd==22)
		        {
			        char text22[]={"��ʤ"};
			        yuyin(text22);
			        u8g_FirstPage(&u8g); /* marks the beginning of the picture loop; it cannot be used inside the picture loop */
		            do 
			        {
			          drawscene();
		            } while (u8g_NextPage(&u8g));  
			         cmd=0;
		         }
		          else if(cmd==23)
		         {
		              char text23[]={"лл"};
			           yuyin(text23);
			           u8g_FirstPage(&u8g); /* marks the beginning of the picture loop; it cannot be used inside the picture loop */
		               do 
			           {
			              drawthank();
		                } while (u8g_NextPage(&u8g));  
			            cmd=0;
			      }
		          else if(cmd==24)
		          {
			            char text24[]={"���"};
			            yuyin(text24);
			            u8g_FirstPage(&u8g); /* marks the beginning of the picture loop; it cannot be used inside the picture loop */
		                do
			            {
			               drawdajia();
		                } while (u8g_NextPage(&u8g)); 
			             cmd=0;
			       }
			       else if(cmd==25)
		           {
			           char text25[]={"��"};
			           yuyin(text25);
			           u8g_FirstPage(&u8g); /* marks the beginning of the picture loop; it cannot be used inside the picture loop */
		               do 
			          {
			             drawshi();
		               } while (u8g_NextPage(&u8g));  
			          cmd=0;
		            }
		             else if(cmd==26)
		            {
		               char text26[]={"Ϊʲô"};
			            yuyin(text26);
			            u8g_FirstPage(&u8g); /* marks the beginning of the picture loop; it cannot be used inside the picture loop */
		                do 
			            {
			               drawwhy();
		                 } while (u8g_NextPage(&u8g));  
			              cmd=0;
			        }
		            else if(cmd==27)
		           {
			            char text27[]={"��绰"};
			             yuyin(text27);
			             u8g_FirstPage(&u8g); /* marks the beginning of the picture loop; it cannot be used inside the picture loop */
		                 do
			             {
			                drawcall();
		                  } while (u8g_NextPage(&u8g)); 
			              cmd=0;
			        }
			        else if(cmd==28)
		            {
			            char text28[]={"֪��"};
			             yuyin(text28);
			             u8g_FirstPage(&u8g); /* marks the beginning of the picture loop; it cannot be used inside the picture loop */
		                 do 
			              {
			                 drawzhidao();
		                   } while (u8g_NextPage(&u8g));  
			              cmd=0;
		            }
		             else if(cmd==29)
	             	{
	                 	char text29[]={"�Բ���"};
			            yuyin(text29);
			            u8g_FirstPage(&u8g); /* marks the beginning of the picture loop; it cannot be used inside the picture loop */
		                do 
			            {
			                 drawsorry();
		                 } while (u8g_NextPage(&u8g));  
			             cmd=0;
			        }
		             else if(cmd==30)
		            {
			           char text30[]={"��"};
			           yuyin(text30);
			           u8g_FirstPage(&u8g); /* marks the beginning of the picture loop; it cannot be used inside the picture loop */
		                do
			           {
			             drawwo();
		                } while (u8g_NextPage(&u8g)); 
			           cmd=0;
			        }
			         else if(cmd==31)
		            { 
			           char text30[]={"��"};
			           yuyin(text30);
			           u8g_FirstPage(&u8g); /* marks the beginning of the picture loop; it cannot be used inside the picture loop */
		                do
			           {
			             drawjiao();
		                } while (u8g_NextPage(&u8g)); 
			           cmd=0;
		            }
		          
	          }
            }
    }
                      
 }
	return E_SYS;
	
	
}