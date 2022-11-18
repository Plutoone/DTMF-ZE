#include "task.h"
#include "oled.h"
#include "stdio.h"


GPIO_TypeDef* key_ports[] = {GPIOB, GPIOB, GPIOB};
uint16_t key_pins[] = {GPIO_PIN_12, GPIO_PIN_13, GPIO_PIN_14};
uint8_t key_value = 0;

void IsKeyPressed()
{
	for(int i=0; i<3; i++)
	{
		if(HAL_GPIO_ReadPin(key_ports[i], key_pins[i]) == 0)
		{
			HAL_Delay(5);	
			if(HAL_GPIO_ReadPin(key_ports[i], key_pins[i]) == 0)
			{
				OLED_Clear();
				key_value = i;
			}
		}
	}
}


char show_mode[16];
char show_char[16];
char show_num[16];

void oled_show()
{
	switch(key_value)
	{
		case 0:
		{
			
			sprintf(show_mode, "Detection...");
			sprintf(show_char, "Detected num is");
			sprintf(show_num, "       N       ");
			OLED_ShowString(2,0,(unsigned char *)show_mode,16);
			OLED_ShowString(2,2,(unsigned char *)show_char,16);
			OLED_ShowString(2,6,(unsigned char *)show_num,16);
		}break;
		
		case 1:
		{
			sprintf(show_mode, "Record Mode...");
			OLED_ShowString(2,0,(unsigned char *)show_mode,16);
		}break;
		
		case 2:
		{
			sprintf(show_mode, "Play Mode...");
			OLED_ShowString(2,0,(unsigned char *)show_mode,16);
		}break;
		
		default:
			break;
	}
	  
}

void AllTask()
{
	IsKeyPressed();
	oled_show();
}
