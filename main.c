#include <avr/io.h>

#define PWM0 PB0
#define PWM1 PB1
#define BTN_RUN PB2

#define CUT_OFF_PWM 8

//#define PWM_SPEED_TEST

#ifndef PWM_SPEED_TEST
void adc_setup (void)
{
    // Set the ADC input to ADMUX |= (1 << MUX1) |(1 << MUX0);
    ADMUX |= (1 << ADLAR);
    // Set the prescaler to clock/128 & enable ADC
    ADCSRA |= (1 << ADPS1) | (1 << ADPS0) | (1 << ADEN);
}

int adc_read(unsigned char channel)
{       
    //clear lower 4 bits of ADMUX and select ADC channel according to argument
    ADMUX &= (0xF0);
    ADMUX |= (channel & 0x0F); //set channel, limit channel selection to lower 4 bits

    //start ADC conversion
    ADCSRA |= (1 << ADSC);

    //wait for conversion to finish 16 bit
   // while(!(ADCSRA & (1 << ADIF)));
    //ADCSRA |= (1 << ADIF); //reset as required      
    //return ADC;
    
      // Wait for it to finish - blocking
    while (ADCSRA & (1 << ADSC));
    return ADCH;
}
#endif
 
 unsigned char pwm = 0;
 unsigned int pwm_cycles_cnt = 0;

int main (void) 
{ 
   DDRB |= (1 << PWM0);
   DDRB |= (1 << PWM1);
   //Маска битов ШИМ
   unsigned char pwm_bits_mask = (1<<PWM0)|(1<<PWM1);
#ifdef PWM_SPEED_TEST
   PORTB |= (1<<PWM0);
   pwm = 255;
   pwm_cycles_cnt = 1000;
#else  
   adc_setup();
   DDRB |= (0 << BTN_RUN);
   PORTB |= (1<<BTN_RUN);//PULL UP  
   int i = 0;
#endif
   while(1)
   { 
#ifndef PWM_SPEED_TEST
      pwm_cycles_cnt = pwm << 2;//10 Делим на 2.5, т.к. pwm 255 уровней, а не 100
      if(!(PINB &(1<<BTN_RUN)))
      {
	 if(pwm > CUT_OFF_PWM)
	    PORTB |= (1<<PWM0);//Ставим один пин ШИМ в 1, иначе будут синфазно работать
		//Цикл заполнения (duty)   
		 for(i = 0; i < pwm_cycles_cnt;i++)
		 {
#endif
			if(pwm > CUT_OFF_PWM)
			   PORTB ^= pwm_bits_mask;//инверсия всех битов ШИМ сразу или будет задержка
			asm("nop");//_delay_us(1);
#ifndef PWM_SPEED_TEST	    
		 }
      }
      else//"Добиваем" до нужной задержки всего цикла
      {
		 for(i = 0;i < pwm_cycles_cnt;i++)//Фукции _delay_us нельзя передать переменную,а только константу
		 {
			asm("nop"); //_delay_us(1);
		 }
      }
      PORTB &= ~pwm_bits_mask; //ШИМ вЫкл
      pwm = adc_read(3);
      pwm_cycles_cnt = (255 - pwm)*11>>1;//10/2.5*K
      //Цикл релакса  
      for(i = 0;i<pwm_cycles_cnt;i++)
      {
		asm("nop");//_delay_us(1);
      }
#endif
   }
}