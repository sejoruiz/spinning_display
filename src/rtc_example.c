#define F_CPU 18432000UL

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "uart.h"

#define UART_BAUD_RATE      4800  
#define led_pin 0

//Struct that holds the time
typedef struct{ 
unsigned char second;   //enter the current time
unsigned char minute;
unsigned char hour;                                     
            }time;

time t;
uint8_t led_state = 0;
uint8_t new_tick = 0;  

void init_IO()
{
	DDRB = 0x01;
}

void init_rtc()
{
	//Disable all TC2 interrupts
	TIMSK2 &=~((1<<TOIE2)|(1<<OCIE2A)|(1<<OCIE2B));     
    //Set Timer/Counter2 to be asynchronous from the CPU clock 
    //with a second external clock(32,768kHz)driving it.  
	ASSR |= (1<<AS2);
	//TCNT2 = 0x00;
	//Prescaller = 128
    TCCR2B |= (1<<CS22)|(1<<CS20);               
		
	//while(ASSR&0x07);           //Wait until TC2 is updated
    TIMSK2 |= (1<<TOIE2);        //set 8-bit Timer/Counter2 Overflow Interrupt Enable                             
}                                       
void main(void)  
{   
	//Initialize IOs                           
	init_IO();
	//Initialize the RTC
	init_rtc();
	
	//Initialize the UART
	uart_init( UART_BAUD_SELECT(UART_BAUD_RATE,F_CPU) ); 
	
	//set the Global Interrupt Enable Bit
	sei();                     
                              
    while(1)                     
    {
		//Blink sekonds LED
		if (led_state == 1) { 
			PORTB |= (1<<led_pin); // Turn on LED
		}
		else 
		{ 
			PORTB &= ~(1<<led_pin); // Turn off LED
		}
		
		if (new_tick)
		{
			//Convert and display hours
			char buf [2];
			itoa (t.hour,buf,10);
			uart_puts(buf);
			uart_puts(":");
			
			//Convert and display minutes
			itoa (t.minute,buf,10);
			uart_puts(buf);
			uart_puts(":");
			
			//Convert and display seconds
			itoa (t.second,buf,10);
			uart_puts(buf);
			uart_puts("\r\n");
			
			new_tick = 0;
		}
    }            
}

ISR (TIMER2_OVF_vect) //overflow interrupt vector
{ 
    
    if (++t.second==60)        //keep track of time, date, month, and year
    {
        t.second=0;
        if (++t.minute==60) 
        {
            t.minute=0;
            if (++t.hour==24)
            {
                t.hour=0;                
            }
        }
    }  
	
	//Flip the monitoring LED
	if (led_state == 1)
	{
		led_state = 0;
	}else
	{
		led_state = 1;
	}
	
	//Set the flag that show a new tick
	new_tick = 1;
}  

        
