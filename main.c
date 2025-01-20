#include <avr/io.h>

#define PWM0 PB0
#define PWM1 PB1
#define BTN_RUN PB2

#define CUT_OFF_PWM (8<<7)

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
 
//При малых значениях регулятора скважности RV1 inc_pwm < 1.
//16 битные переменные для того, чтобы не использовать float.
//Чтобы inc_pwm > 0 умножаем всё на 128 (<<7). 
 unsigned int pwm_target = 0;
 unsigned int pwm_current = 0;
 unsigned int pwm_cycles_cnt = 0;
 unsigned int inc_pwm = 0;

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
	  pwm_target = adc_read(3)<<7;//"Читаем" резистор когда ШИМ не "шумит"

	  inc_pwm = pwm_target/90;//Цикл while(1) порядка 11мс. Плавный пуск около 1000мс. 1000/11=90.
	  //Плавный пуск.   
	  pwm_current += inc_pwm;
	  if(pwm_current > pwm_target)
		pwm_current = pwm_target;

      pwm_cycles_cnt = (pwm_current>>7)<< 2;//pwm_current * (10 Делим на 2.5, т.к. pwm 255 уровней, а не 100)
      if(!(PINB &(1<<BTN_RUN)))
      {
		 if(pwm_target > CUT_OFF_PWM)
			PORTB |= (1<<PWM0);//Ставим один пин ШИМ в 1, иначе будут синфазно работать
		//Цикл заполнения (duty)   
		 for(i = 0; i < pwm_cycles_cnt;i++)
		 {
#endif
			if(pwm_target > CUT_OFF_PWM)
			   PORTB ^= pwm_bits_mask;//инверсия всех битов ШИМ сразу или будет задержка
			asm("nop");//_delay_us(1);
#ifndef PWM_SPEED_TEST	    
		 }
      }
      else//"Добиваем" до нужной задержки всего цикла
      {
		 pwm_current = 0;
		 for(i = 0;i < pwm_cycles_cnt;i++)//Фукции _delay_us нельзя передать переменную,а только константу
		 {
			asm("nop"); //_delay_us(1);
		 }
      }
      PORTB &= ~pwm_bits_mask; //ШИМ вЫкл
	  //Делим на 128 (>>7) - нормализуем значения
      pwm_cycles_cnt = (255 - (pwm_current>>7))*11>>1;//10/2.5*K
      //Цикл релакса  
      for(i = 0;i<pwm_cycles_cnt;i++)
      {
		asm("nop");//_delay_us(1);
      }
#endif
   }
}