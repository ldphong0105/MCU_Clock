#include <SN32F400.h>
#include <SN32F400_Def.h>
#include "..\Driver\GPIO.h"
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

// --- Khai bao phim ---
#define KEY_SW3   0x11  // Nut Setup
#define KEY_SW16  0x81  // Nut hen gio
#define KEY_SW6   0x18  // Nut (+)
#define KEY_SW10  0x28  // Nut (-)

// --- EEPROM (FLASH) ---
#define EEPROM_START_ADDRESS  0x00003E00 
#define MAGIC_WORD            0xAA550000
#define MAX_SAVE_COUNT        128
#define FLASH_EMPTY_VAL       0xFFFFFFFF
#define ALARM_DATA_SIZE       4

// --- LED 7 THANH ---
#define SEG_DARK  0
#define SEG_H     0x80

// =========================================================================
// === THAM SO ===
// =========================================================================

// --- GIOI HAN THOI GIAN ---
#define MAX_SECOND            60
#define MAX_MINUTE            60
#define MAX_HOUR              24

// --- THOI GIAN TIMER (Ðon vi: ms) ---
#define TICK_1_SEC_MS         1000  // 1000ms = 1 giây
#define BLINK_HALF_SEC_MS     500   // 500ms nhay LED 1 lan
#define ALARM_RING_DUR_MS     5000  // Chuong bao thuc keu trong 5 giay
#define SETUP_TIMEOUT_MS      30000 // Tu dong thoat cai dat sau 30 giay
#define KEY_BEEP_DUR_MS       300   // Coi keu 300ms khi bam phim

// --- CAU HINH CHÂN GPIO ---
#define BUZZER_PIN_BIT        0     // Coi (Buzzer) chan P3.0
#define LED_D6_PIN_BIT        8     // LED D6 chan P3.8

// --- THAM SA ÐIEU KHIEN COI (BUZZER) ---
#define BUZZER_CYCLES         2     // So lan lap tao xung còi
#define BUZZER_ON_DELAY_10US  3    // Thoi gian coi bat (x 10us)
#define BUZZER_OFF_DELAY_10US 100   // Thoi gian còi tat (x 10us)

// =========================================================================

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
volatile ClockState currentState = STATE_NORMAL;

volatile uint8_t hour = 0, minute = 0, second = 0;
volatile uint8_t alarm_hour = 0, alarm_minute = 0;

volatile uint16_t ms_tick = 0;             
volatile uint16_t timeout_counter = 0;     
volatile uint16_t buzzer_beep_timer = 0;   
volatile uint16_t alarm_ring_timer = 0;    

volatile uint8_t blink_flag = 1;           
volatile uint8_t alarm_triggered = 0; 
uint16_t read_key = 0;

/*_____ F U N C T I O N S __________________________________________________*/

// Hanh vi cua led 7 thanh
void Display_Time(uint8_t current_hour, uint8_t current_minute, ClockState state, uint8_t blink) {
    uint8_t digit_hour_tens = current_hour / 10;
    uint8_t digit_hour_ones = current_hour % 10;
    uint8_t digit_minute_tens = current_minute / 10;
    uint8_t digit_minute_ones = current_minute % 10;
    
    if ((state == STATE_SET_HOUR || state == STATE_SET_ALARM_HOUR) && !blink) {
        segment_buff[0] = SEG_DARK; 
        segment_buff[1] = SEG_DARK | SEG_H; 
    } else {
        segment_buff[0] = SEGMENT_TABLE[digit_hour_tens]; 
        segment_buff[1] = SEGMENT_TABLE[digit_hour_ones] | SEG_H; 
    }

    if ((state == STATE_SET_MINUTE || state == STATE_SET_ALARM_MINUTE) && !blink) {
        segment_buff[2] = SEG_DARK; 
        segment_buff[3] = SEG_DARK; 
    } else {
        segment_buff[2] = SEGMENT_TABLE[digit_minute_tens]; 
        segment_buff[3] = SEGMENT_TABLE[digit_minute_ones]; 
    }
}

void SysTick_Handler(void) {
    ms_tick++;
    
    if (ms_tick >= TICK_1_SEC_MS) {
        ms_tick = 0;
        second++;
        if (second >= MAX_SECOND) {
            second = 0;
            minute++;
            if (minute >= MAX_MINUTE) {
                minute = 0;
                hour++;
                if (hour >= MAX_HOUR) hour = 0;
            }
        }
        if (currentState == STATE_NORMAL && hour == alarm_hour && minute == alarm_minute) {
            if (alarm_triggered == 0) { 
                alarm_ring_timer = ALARM_RING_DUR_MS; 
                alarm_triggered = 1;
            }
        } else {
            alarm_triggered = 0; 
        }
    }

    if (ms_tick == BLINK_HALF_SEC_MS || ms_tick == 0) {
        blink_flag = !blink_flag;
    }

    if (currentState != STATE_NORMAL) {
        timeout_counter++;
        if (timeout_counter >= SETUP_TIMEOUT_MS) { 
            currentState = STATE_NORMAL;
            timeout_counter = 0;
            buzzer_beep_timer = KEY_BEEP_DUR_MS; 
        }
    }

    if (buzzer_beep_timer > 0) buzzer_beep_timer--;
    if (alarm_ring_timer > 0) alarm_ring_timer--;
	
	Digital_Scan(); 
}

int main(void)
{
    SystemInit();
    SystemCoreClockUpdate();
    PFPA_Init();
    NotPinOut_GPIO_init();
    
    SN_SYS0->EXRSTCTRL_b.RESETDIS = 0;
    GPIO_Init();
    
    SN_GPIO3->MODE |= (1 << BUZZER_PIN_BIT) | (1 << LED_D6_PIN_BIT);  
    Buzzer_Control(0);                     
    LED_D6_Control(0);                     
    
    Load_Alarm_From_Flash();

    SysTick_Config(SystemCoreClock / TICK_1_SEC_MS); 

    //WDT_Init(); 
    
    while (1)
    {
        __WDT_FEED_VALUE;
        
        read_key = KeyScan();
        if(read_key)
        {
            buzzer_beep_timer = KEY_BEEP_DUR_MS; 
            timeout_counter = 0;     
            alarm_ring_timer = 0;    
            
            // State machine
            switch(currentState) {
                case STATE_NORMAL:
                    if (read_key == KEY_SW3) currentState = STATE_SET_HOUR;
                    else if (read_key == KEY_SW16) currentState = STATE_SET_ALARM_HOUR;
                    break;
                    
                case STATE_SET_HOUR:
                    if (read_key == KEY_SW3) currentState = STATE_SET_MINUTE;
                    else if (read_key == KEY_SW6) { hour++; if(hour >= MAX_HOUR) hour = 0; }
                    else if (read_key == KEY_SW10){ if(hour == 0) hour = (MAX_HOUR - 1); else hour--; }
                    break;
                    
                case STATE_SET_MINUTE:
                    if (read_key == KEY_SW3) currentState = STATE_NORMAL;
                    else if (read_key == KEY_SW6) { minute++; if(minute >= MAX_MINUTE) minute = 0; }
                    else if (read_key == KEY_SW10){ if(minute == 0) minute = (MAX_MINUTE - 1); else minute--; }
                    break;
                    
                case STATE_SET_ALARM_HOUR:
                    if (read_key == KEY_SW16) currentState = STATE_SET_ALARM_MINUTE;
                    else if (read_key == KEY_SW6) { alarm_hour++; if(alarm_hour >= MAX_HOUR) alarm_hour = 0; }
                    else if (read_key == KEY_SW10){ if(alarm_hour == 0) alarm_hour = (MAX_HOUR - 1); else alarm_hour--; }
                    break;
                    
                case STATE_SET_ALARM_MINUTE:
                    if (read_key == KEY_SW16) {
                        currentState = STATE_NORMAL;
                        Save_Alarm_To_Flash(); 
                    }
                    else if (read_key == KEY_SW6) { alarm_minute++; if(alarm_minute >= MAX_MINUTE) alarm_minute = 0; }
                    else if (read_key == KEY_SW10){ if(alarm_minute == 0) alarm_minute = (MAX_MINUTE - 1); else alarm_minute--; }
                    break;
            }
        }

        if (currentState == STATE_NORMAL || currentState == STATE_SET_HOUR || currentState == STATE_SET_MINUTE) {
            Display_Time(hour, minute, currentState, blink_flag);
        } else {
            Display_Time(alarm_hour, alarm_minute, currentState, blink_flag);
        }

        if (buzzer_beep_timer > 0) {
            Buzzer_Control(1); 
        } 
        else if (alarm_ring_timer > 0) {
            if (ms_tick < BLINK_HALF_SEC_MS) Buzzer_Control(1);
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
        for (uint8_t i = 0; i < BUZZER_CYCLES; i++) {
            SN_GPIO3->BSET = (1 << BUZZER_PIN_BIT);      
            UT_DelayNx10us(BUZZER_ON_DELAY_10US);              
            
            SN_GPIO3->BCLR = (1 << BUZZER_PIN_BIT);   
            UT_DelayNx10us(BUZZER_ON_DELAY_10US);              
        }
    } else {
        SN_GPIO3->BCLR = (1 << BUZZER_PIN_BIT);
        UT_DelayNx10us(BUZZER_OFF_DELAY_10US);                
    }
}

void LED_D6_Control(uint8_t state) {
	if (state) SN_GPIO3->BCLR = (1 << LED_D6_PIN_BIT); 
	else       SN_GPIO3->BSET = (1 << LED_D6_PIN_BIT);
}

void Save_Alarm_To_Flash(void) {
	uint32_t data_to_write = (alarm_hour << 8) | alarm_minute;
	FLASH_EraseSector(EEPROM_START_ADDRESS);
	FLASH_ProgramPage(EEPROM_START_ADDRESS, ALARM_DATA_SIZE, (uint8_t*)&data_to_write);
}

void Load_Alarm_From_Flash(void) {
	uint32_t read_data = *((volatile uint32_t*)EEPROM_START_ADDRESS);
	
	if (read_data != FLASH_EMPTY_VAL) {
		alarm_hour = (read_data >> 8) & 0xFF;
		alarm_minute = read_data & 0xFF;
		
		if(alarm_hour >= MAX_HOUR) alarm_hour = 0;
		if(alarm_minute >= MAX_MINUTE) alarm_minute = 0;
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