#ifndef _DTMF_H
#define _DTMF_H	

//#include "stm32f10x.h"
#include "main.h"
#include "stdio.h"

extern uint8_t N_decode;	            
extern uint8_t next_code;

extern uint16_t sound_one[16][400];
extern char code[17];
extern uint8_t code_number [17];
extern uint8_t key_data[40];
void DTMF_Produc_Init(void);
uint8_t find_key_number (float *Af_buffer);
void DTMF_Decode(float decode_data);		   //只采样计数110个点计算2次，N_decode控制采样计算的点数
//float filter(float b[],int Length_b,float ADC_CovnData);

extern char key1,key2,key3;
extern uint8_t key_flag;
extern uint8_t key_number;
extern uint8_t data_number ;
extern uint8_t data_number_COPY ;
extern char key_data_char[40] ; //把录到的音的数字放到字符数组
extern char key_data_char_COPY[40];
#endif

