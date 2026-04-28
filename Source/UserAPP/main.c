#include <SN32F400.h>
#include <SN32F400_Def.h>
#include "..\Driver\GPIO.h"
#include "..\Driver\CT16B0.h"
#include "..\Driver\WDT.h"
#include "..\Driver\Utility.h"
#include "..\Driver\Flash.h" 
#include "..\Module\KeyScan.h"
#include "..\Module\Segment.h"

/*_____ D E C L A R A T I O N S ____________________________________________*/
void PFPA_Init(void);
void NotPinOut_GPIO_init(void);
void Buzzer_Control(uint8_t state);
void LED_D6_Control(uint8_t state);
void Save_Alarm_To_Flash(void);
void Load_Alarm_From_Flash(void);

/*_____ D E F I N I T I O N S ______________________________________________*/
#ifndef SN32F407
	#error Please install SONiX.SN32F4_DFP.0.0.18.pack or version >= 0.0.18
#endif
#define PKG SN32F407

//switch
#define KEY_SW3   0x11  // Nut Setup
#define KEY_SW16  0x81  // N�t hen gio
#define KEY_SW6   0x18  // N�t (+)
#define KEY_SW10  0x28  // N�t (-)

// EEPROM
// Thay d?i C?C K? QUAN TR?NG
#define EEPROM_START_ADDRESS  0x00003E00 
#define MAGIC_WORD 0xAA550000
#define MAX_SAVE_COUNT 128

// LED 7 Thanh
#define SEG_DARK  0
#define SEG_H     0x80
extern uint8_t segment_buff[4];
extern const uint8_t SEGMENT_TABLE[];

typedef enum {
    STATE_NORMAL = 0,
    STATE_SET_HOUR,
    STATE_SET_MINUTE,
    STATE_SET_ALARM_HOUR,
    STATE_SET_ALARM_MINUTE
} ClockState;

/*_____ G L O B A L   V A R I A B L E S ___________________________________*/
ClockState currentState = STATE_NORMAL;

uint8_t hour = 0, minute = 0, second = 0;
uint8_t alarm_hour = 0, alarm_minute = 0;

uint16_t ms_tick = 0;             
uint16_t timeout_counter = 0;     
uint16_t buzzer_beep_timer = 0;   
uint16_t alarm_ring_timer = 0;    

uint8_t blink_flag = 1;           
uint16_t read_key = 0;
uint8_t alarm_triggered = 0; 

/*_____ F U N C T I O N S __________________________________________________*/

// Hanh vi cua led 7 thanh
void Display_Time(uint8_t current_hour, uint8_t current_minute, ClockState state, uint8_t blink) {
    uint8_t digit_hour_tens = current_hour / 10;
    uint8_t digit_hour_ones = current_hour % 10;
    uint8_t digit_minute_tens = current_minute / 10;
    uint8_t digit_minute_ones = current_minute % 10;
    

    if ((state == STATE_SET_HOUR || state == STATE_SET_ALARM_HOUR) && !blink) {
        segment_buff[0] = SEG_DARK; 
        segment_buff[1] = SEG_DARK | SEG_H; // T?t s? nhung V?N GI? D?U CH?M ph�n c�ch
    } else {
        segment_buff[0] = SEGMENT_TABLE[digit_hour_tens]; 
        segment_buff[1] = SEGMENT_TABLE[digit_hour_ones] | SEG_H; // Hi?n th? s? v� b?t d?u ch?m
    }

    if ((state == STATE_SET_MINUTE || state == STATE_SET_ALARM_MINUTE) && !blink) {
        segment_buff[2] = SEG_DARK; 
        segment_buff[3] = SEG_DARK; 
    } else {
        segment_buff[2] = SEGMENT_TABLE[digit_minute_tens]; 
        segment_buff[3] = SEGMENT_TABLE[digit_minute_ones]; 
    }
}

int main(void)
{
	SystemInit();
	SystemCoreClockUpdate();
	PFPA_Init();
	NotPinOut_GPIO_init();
	
	SN_SYS0->EXRSTCTRL_b.RESETDIS = 0;
	GPIO_Init();
	
	SN_GPIO3->MODE |= (1 << 0) | (1 << 8);  
	Buzzer_Control(0);                      
	LED_D6_Control(0);                      
	
	Load_Alarm_From_Flash();

	//WDT_Init(); 
	
	while (1)
	{
		__WDT_FEED_VALUE;
		
		read_key = KeyScan();
		if(read_key)
		{
			buzzer_beep_timer = 300; 
			timeout_counter = 0;     
			alarm_ring_timer = 0;    
			//State machine
			switch(currentState) {
				case STATE_NORMAL:
					if (read_key == KEY_SW3) currentState = STATE_SET_HOUR;
					else if (read_key == KEY_SW16) currentState = STATE_SET_ALARM_HOUR;
					break;
					
				case STATE_SET_HOUR:
					if (read_key == KEY_SW3) currentState = STATE_SET_MINUTE;
					else if (read_key == KEY_SW6) { hour++; if(hour > 23) hour = 0; }
					else if (read_key == KEY_SW10){ if(hour == 0) hour = 23; else hour--; }
					break;
					
				case STATE_SET_MINUTE:
					if (read_key == KEY_SW3) currentState = STATE_NORMAL;
					else if (read_key == KEY_SW6) { minute++; if(minute > 59) minute = 0; }
					else if (read_key == KEY_SW10){ if(minute == 0) minute = 59; else minute--; }
					break;
					
				case STATE_SET_ALARM_HOUR:
					if (read_key == KEY_SW16) currentState = STATE_SET_ALARM_MINUTE;
					else if (read_key == KEY_SW6) { alarm_hour++; if(alarm_hour > 23) alarm_hour = 0; }
					else if (read_key == KEY_SW10){ if(alarm_hour == 0) alarm_hour = 23; else alarm_hour--; }
					break;
					
				case STATE_SET_ALARM_MINUTE:
					if (read_key == KEY_SW16) {
						currentState = STATE_NORMAL;
						Save_Alarm_To_Flash(); 
					}
					else if (read_key == KEY_SW6) { alarm_minute++; if(alarm_minute > 59) alarm_minute = 0; }
					else if (read_key == KEY_SW10){ if(alarm_minute == 0) alarm_minute = 59; else alarm_minute--; }
					break;
			}
		}

		ms_tick++;
		if (ms_tick >= 1000) {
			ms_tick = 0;
			second++;
			
			if (second >= 60) {
				second = 0;
				minute++;
				if (minute >= 60) {
					minute = 0;
					hour++;
					if (hour >= 24) hour = 0;
				}
			}
			
			if (currentState == STATE_NORMAL && hour == alarm_hour && minute == alarm_minute) {
				if (alarm_triggered == 0) { 
					alarm_ring_timer = 5000; 
					alarm_triggered = 1;
				}
			} else {
				alarm_triggered = 0; 
			}
		}
		
		if (ms_tick == 500 || ms_tick == 0) {
			blink_flag = !blink_flag;
		}

		if (currentState != STATE_NORMAL) {
			timeout_counter++;
			if (timeout_counter >= 30000) {
				currentState = STATE_NORMAL;
				timeout_counter = 0;
				buzzer_beep_timer = 300; 
			}
		}

		if (currentState == STATE_NORMAL || currentState == STATE_SET_HOUR || currentState == STATE_SET_MINUTE) {
		    Display_Time(hour, minute, currentState, blink_flag);
		} else {
		    Display_Time(alarm_hour, alarm_minute, currentState, blink_flag);
		}
		
		Digital_Scan(); 

		if (buzzer_beep_timer > 0) {
			buzzer_beep_timer--;
			Buzzer_Control(1); 
		} 
		else if (alarm_ring_timer > 0) {
			alarm_ring_timer--;
			if (ms_tick < 500) Buzzer_Control(1);
			else Buzzer_Control(0);
		}
		else {
			Buzzer_Control(0); 
		}

		if (currentState == STATE_SET_ALARM_HOUR || currentState == STATE_SET_ALARM_MINUTE) {
			LED_D6_Control(blink_flag); 
		} else {
			LED_D6_Control(0);          
		}

	}
}


void Buzzer_Control(uint8_t state) {
    if (state) {
        for (uint8_t i = 0; i < 2; i++) {
            SN_GPIO3->BSET = (1 << 0);      
            UT_DelayNx10us(3);             
            
            SN_GPIO3->BCLR = (1 << 0);     
            UT_DelayNx10us(3);             
        }
    } else {
        SN_GPIO3->BCLR = (1 << 0);
        UT_DelayNx10us(100);                
    }
}

void LED_D6_Control(uint8_t state) {
	if (state) SN_GPIO3->BCLR = (1 << 8); 
	else       SN_GPIO3->BSET = (1 << 8);
}

void Save_Alarm_To_Flash(void) {
    static uint32_t page_buffer[128];
    
    for (int i = 0; i < 128; i++) {
        page_buffer[i] = 0xFFFFFFFF;
    }

    page_buffer[0] = MAGIC_WORD | (alarm_hour << 8) | alarm_minute;

    __WDT_FEED_VALUE; 
    __disable_irq();  
    
    FLASH_EraseSector(EEPROM_START_ADDRESS);
    FLASH_ProgramPage(EEPROM_START_ADDRESS, FLASH_PAGE_SIZE, (uint8_t*)page_buffer);
    
    __enable_irq();   
}

void Load_Alarm_From_Flash(void) {
    uint32_t *flash_ptr = (uint32_t *)EEPROM_START_ADDRESS;
    uint32_t valid_data = flash_ptr[0]; 

    if ((valid_data & 0xFFFF0000) == MAGIC_WORD) {
        alarm_hour = (valid_data >> 8) & 0xFF;
        alarm_minute = valid_data & 0xFF;
        
        if(alarm_hour > 23) alarm_hour = 0;
        if(alarm_minute > 59) alarm_minute = 0;
    } else {
        alarm_hour = 0;
        alarm_minute = 0;
    }
}

void NotPinOut_GPIO_init(void)
{
#if (PKG == SN32F407)
#endif
}

void HardFault_Handler(void)
{
	NVIC_SystemReset();
}