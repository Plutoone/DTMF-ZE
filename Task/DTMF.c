#include "DTMF.h"
#include "math.h"
#include "string.h"

#define PIx2				6.2831852		//2*pi
#define fs 					8000			//采样频率
#define MAX_BINS			8

/*
	1	2	3	A			1	2	3	4
	4	5	6	B			5	6	7	8
	7	8	9	C			9	10	11	12
	*	0	#	D			13	14	15	16
*/

char code[17] = {'1', '2', '3', 'a', 			//不知道为什么把这个数组放在DTMF.c中，程序将不能正确返回1,2,3其余均可以正常返回
				 '4', '5', '6', 'b',
				 '7', '8', '9', 'c', 
				 '*', '0', '#', 'd',   'H'}; // DTMF表中的16个ASCII码,H表示错误
/*
	1	2	3	A			1	2	3	4
	4	5	6	B			5	6	7	8
	7	8	9	C			9	15	15	12
	*	0	#	D			13	14	15	16	
*/
				 
uint8_t code_number [17] = {1,   2,    3,   4, 			//不知道为什么把这个数组放在DTMF.c中，程序将不能正确返回1,2,3其余均可以正常返回
				       5,   6,    7,   8,
				       9,   10,   11,  12, 
				       13,  14,   15,  16,  17}; // DTMF表中的16个ASCII码,H表示错误	

uint8_t N_decode=1;	            
uint8_t next_code=0;
int n=1;

uint16_t sound_one[16][400] = {0};				//16个DTMF信号
uint16_t freqs[8] = {697,770, 852, 941, 1209, 1336, 1477,1633};

char key1='F' ,key2='F',key3='F';
uint8_t key_number;//录到音的数字
uint8_t key_flag = 0; //录音播音标志位 1的时候录 2的时候播
int8_t key_N=1;
uint8_t key_data[40] = {'F'}; //把录到的音的数字放到数组
char key_data_char[40] = {'F'}; //把录到的音的数字放到字符数组

char key_data_char_COPY[40] = {'F'}; //把录到的音的数字放到播放字符数组





uint8_t data_number =1;//录音数组的位数
uint8_t data_number_COPY =21;//录音数组的位数

float Wf[MAX_BINS]={1.7077,1.6452,1.5686,1.4782,1.1640,0.9963,0.7985,0.5684};// 角频率8000
float Vf[MAX_BINS][3]={0.0,0.0,0.0,
						0.0,0.0,0.0,
						0.0,0.0,0.0,
						0.0,0.0,0.0,
						0.0,0.0,0.0,
						0.0,0.0,0.0,
						0.0,0.0,0.0,
						0.0,0.0,0.0 }; //8个频率的计算数据				
float Af[MAX_BINS];//频谱值


char find_key(float *Af_buffer);



void DTMF_Produc_Init()		//填充需要产生的DTMF，
{
	uint16_t i,j;
	uint8_t tem_row,tem_col;		//用于记录第几个数据。
	float tem_x;
	for(i=0; i<16; i++)
	{
		tem_col = i/4;
		tem_row = i - tem_col*4 + 4;
		for(j=1; j<401; j++)
		{
			tem_x = sin(PIx2*j*freqs[tem_col]/fs)+sin(PIx2*j*freqs[tem_row]/fs); //按DTMF编码生成一个号码的信号
			
			tem_x = tem_x + 2;									//单电源供电
			tem_x = tem_x * 3.3 / 4;							//标度转换，从0-4V转化为0-3.3V
			sound_one[i][j-1] = (uint16_t)(tem_x * 4096 / 3.3);		//从0-3.3V转化为0--4096
		}
	}
	
}


void DTMF_Decode(float decode_data)		   //只采样计数110个点计算2次，N_decode控制采样计算的点数
{  										   //13.8ms计算出一个点
	int i,j,k;	

	if(N_decode>110)	
	{
		N_decode++;

		return;
	}
	
	for(k=0;k<8;k++)
	{
		Vf[k][0]=Wf[k]*Vf[k][1]-Vf[k][2] + decode_data;
		if(N_decode != 110)	
		{
			Vf[k][2]=Vf[k][1];
			Vf[k][1]=Vf[k][0];
		} 
	}
	
	if(next_code == 1)	
	{
		key3='H';		  //由每20ms判断是否有静音，通个变量next_code来控制key3控制是否输出码值
		next_code=0;	  //当没有静音时key3不会置‘H’，从而控制了多输出码值
	}		//静音时key3 = 'H'
	

	
	if(N_decode==110) 
	{
		for(k=0;k<8;k++)
		Af[k]=Vf[k][0]*Vf[k][0]+Vf[k][1]*Vf[k][1]-Wf[k]*Vf[k][0]*Vf[k][1]; //计算频谱值

		for(i=0;i<8;i++)	 //为下一个计数初始化Vf[i][j
			for(j=0;j<3;j++)	
				Vf[i][j]=0.0;
		
		if(key_N==1)
		{
			key1 = find_key(Af);			
			key_number = find_key_number(Af);
			key_N++;

			
			N_decode++;		//每进行一次GOERTZEL，都要加一次N_decode
		
			return;
		}
		else 
		{
			key2=find_key(Af);
			key_N=1;

	
			if(key1==key2 && key1!='H' && key2!='H' && key1!=key3)
			{					
				key3=key1;
				printf("实时检测音的字符：");
				printf("%c\r\n", key1);//打印录到的音的数字字符
//				printf("%d\n\r", key_number);//打印录到的音的数字
				if(key_flag == 3)
				{
					key_data_char_COPY[n]=key1;
					printf("%c\r\n", key_data_char_COPY[n]);
					printf("%d\r\n", n);
					n++;
					if(n==18)
					{
						n=1;
						memset(key_data_char_COPY,'\0',sizeof(key_data_char_COPY));
					}
					
				}
								
				if( key_flag == 1	)
				{
				key_data[data_number] = key_number -1;	
				key_data_char[data_number] = key1;
				data_number ++;
						
					

	//printf("%d\n\r",key_number);
	//printf("我是keynumber");
				}
				/*
				else if( key_flag == 3	)
				{
				key_data[data_number_COPY] = key_number -1;
				//printf("%d",key_data[data_number_COPY]);
				key_data_char[data_number_COPY] = key1;
				data_number_COPY ++;	
				}
				*/
				
				
		
				key1='H';
				key2='H';
				N_decode++;			
				return;
			}
			else
			{
				N_decode++;
				return;
			}
		}
	}
	N_decode++;
}

uint8_t test_DTMF(float *Af_buffer,int low,int high)
{
	uint8_t i;
	
	for (i=0;i<=3;i++)						//低高频段检测,主能量大于其它的5倍以上
		if(i!=low)
			if(Af_buffer[i]*5.0>=Af_buffer[low])	 
				return 1;  // 无效DTMF信号
	
	for (i=4;i<=7;i++)
		if(i!=high)
			if(Af_buffer[i]*5.0>=Af_buffer[high])
				return 1;
	
	return 0;//有效信号
}



char find_key(float *Af_buffer)
{   
	int8_t i,low_freq,high_freq,test_signal=1;
	int8_t tem;

	// 在低频找max
	for (low_freq = 0, i = 1; i < 4; i ++)
	{
		if (Af_buffer[i] > Af_buffer[low_freq])
		{
			low_freq = i;
		}
	}

	// 在高频找max
	for (high_freq = 4, i = 5; i < 8; i ++)
	{
		if (Af_buffer[i] > Af_buffer[high_freq])
		{
			high_freq = i;
		}
	}
	
	test_signal=test_DTMF(Af_buffer,low_freq,high_freq);
	tem = low_freq*4 + high_freq - 4;
		
	if(test_signal==1)   return 'H';	  //终止判定返回H表示错信号	   test_signal==1 
	else if(tem>-1 && tem<16 && test_signal == 0)

	{
		return code[tem];
	}
	
	return	'H';	 //H表示错了
}

 
uint8_t find_key_number (float *Af_buffer)
{   
	int8_t i,low_freq,high_freq,test_signal=1;
	int8_t tem;

	// 在低频找max
	for (low_freq = 0, i = 1; i < 4; i ++)
	{
		if (Af_buffer[i] > Af_buffer[low_freq])
		{
			low_freq = i;
		}
	}

	// 在高频找max
	for (high_freq = 4, i = 5; i < 8; i ++)
	{
		if (Af_buffer[i] > Af_buffer[high_freq])
		{
			high_freq = i;
		}
	}
	
	test_signal=test_DTMF(Af_buffer,low_freq,high_freq);
	tem = low_freq*4 + high_freq - 4;
		
	if(test_signal==1)   return 17;	  //终止判定返回17表示错信号	   test_signal==1 
	else if(tem>-1 && tem<16 && test_signal == 0)

	{
		return code_number [tem];
	}
	
	return	17;	 //17表示错了
}




/***************************************************************************
***********************************************/









/***************************************************************************
***********************************************/





#if 0
 
 

float filter(float b[],int Length_b,float ADC_CovnData)
{
	float temp=0.0; 
	int i=0;
	
	temp=0.0;
	for(i=Length_b-1;i>=1;i--)
	{					//在计算length_b个数据后才正确滤波
		temp=temp+b[i]*ADC_CovnDataBuffer[i];
		ADC_CovnDataBuffer[i]=ADC_CovnDataBuffer[i-1];	//
	}
	
	temp=temp+b[0]*ADC_CovnDataBuffer[0];
	ADC_CovnDataBuffer[0]=ADC_CovnData;	//filter后的值要比采样慢一个数据
	FilterData=temp;
	
	return FilterData;
}

 
void post_testing()
{
  int         row, col, see_digit;
  int         peak_count, max_index;
  double      maxval, t;
  int         i;
  char *  row_col_ascii_codes[4][4] = {	{"1", "2", "3", "A"},
										{"4", "5", "6", "B"},
										{"7", "8", "9", "C"},
										{"*", "0", "#", "D"}};
 
 
  /* Find the largest in the row group. */
  row = 0;
  maxval = 0.0;
  for ( i=0; i<4; i++ )
  {
    if ( r[i] > maxval )
    {
      maxval = r[i];
      row = i;
    }
  }
 
  /* Find the largest in the column group. */
  col = 4;
  maxval = 0.0;
  for ( i=4; i<8; i++ )
  {
    if ( r[i] > maxval )
    {
      maxval = r[i];
      col = i;
    }
  }
 
 
  /* Check for minimum energy */
 
  if ( r[row] < 4.0e5 )   /* 2.0e5 ... 1.0e8 no change */
  {
    /* energy not high enough */
  }
  else if ( r[col] < 4.0e5 )
  {
    /* energy not high enough */
  }
  else
  {
    see_digit = 1;
 
    /* Twist check
     * CEPT => twist < 6dB
     * AT&T => forward twist < 4dB and reverse twist < 8dB
     *  -ndB < 10 log10( v1 / v2 ), where v1 < v2
     *  -4dB < 10 log10( v1 / v2 )
     *  -0.4  < log10( v1 / v2 )
     *  0.398 < v1 / v2
     *  0.398 * v2 < v1
     */
    if ( r[col] > r[row] )
    {
      /* Normal twist */
      max_index = col;
      if ( r[row] < (r[col] * 0.398) )    /* twist > 4dB, error */
        see_digit = 0;
    }
    else /* if ( r[row] > r[col] ) */
    {
      /* Reverse twist */
      max_index = row;
      if ( r[col] < (r[row] * 0.158) )    /* twist > 8db, error */
        see_digit = 0;
    }
 
    /* Signal to noise test
     * AT&T states that the noise must be 16dB down from the signal.
     * Here we count the number of signals above the threshold and
     * there ought to be only two.
     */
    if ( r[max_index] > 1.0e9 )
      t = r[max_index] * 0.158;
    else
      t = r[max_index] * 0.010;
 
    peak_count = 0;
    for ( i=0; i<8; i++ )
    {
      if ( r[i] > t )
        peak_count++;
    }
    if ( peak_count > 2 )
      see_digit = 0;
 
    if ( see_digit )
    {
      printf( "%s\n", row_col_ascii_codes[row][col-4] );
    }
  }
}

void DTMF_Decode( float sample )
{
  double	q0;
  u32   	i;
 
	if(N_decode>110)	
	{
		N_decode++;
		return;
	}	
	
	sample_count++;
	/*q1[0] = q2[0] = 0;*/
	for ( i=0; i<MAX_BINS; i++ )
	{
		q0 = coefs[i] * q1[i] - q2[i] + sample;
		q2[i] = q1[i];
		q1[i] = q0;
	}
 
  if (sample_count == GOERTZEL_N)
  {
    for ( i=0; i<MAX_BINS; i++ )
    {
      r[i] = (q1[i] * q1[i]) + (q2[i] * q2[i]) - (coefs[i] * q1[i] * q2[i]);
      q1[i] = 0.0;
      q2[i] = 0.0;
    }
    post_testing();
    sample_count = 0;
  }
}


#endif


