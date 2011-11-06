/*
 * This file is part of the tumanako_vc project.
 *
 * Copyright (C) 2010 Edward Cheeseman <cheesemanedward@gmail.com>
 * Copyright (C) 2009 Uwe Hermann <uwe@hermann-uwe.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * History:
 * Example from libopenstm32 expanded to make tumanako arm tester
 */

#define STM32F1  //applicable to the STM32F1 series of devices

#include <libopencm3/stm32/f1/rcc.h>
#include <libopencm3/stm32/f1/gpio.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/stm32/f1/adc.h>
#include <libopencm3/stm32/f1/memorymap.h>
#include <STM32MCU.h>

void clock_setup(void)
{
	rcc_clock_setup_in_hse_8mhz_out_72mhz();
	
	/* Enable all present GPIOx clocks. (whats with GPIO F and G?)*/
	rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_IOPAEN);
	rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_IOPBEN);
	rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_IOPCEN);
	rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_IOPDEN);
	rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_IOPEEN);
	
	/* Enable clock for USART1. */
	rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_USART1EN);
}

void usart_setup(void)
{
	/* Setup GPIO pin GPIO_USART3_TX/GPIO10 on GPIO port B for transmit. */
	gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ,
                      GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO_USART1_TX);
	
	/* Setup UART parameters. */
	usart_set_baudrate(USART1, 38400, rcc_ppre2_frequency);         

	//u32 clock = rcc_ppre1_frequency;
        //if (usart == USART1) {
        //      clock = rcc_ppre2_frequency;
        //}

	usart_set_databits(USART1, 8);
	usart_set_stopbits(USART1, USART_STOPBITS_1);
	usart_set_mode(USART1, USART_MODE_TX_RX);
	usart_set_parity(USART1, USART_PARITY_NONE);
	usart_set_flow_control(USART1, USART_FLOWCONTROL_NONE);
	
	/* Finally enable the USART. */
	usart_enable(USART1);
}

void gpio_setup(void)
{
	/* Set LEDs (in GPIO port C) to 'output push-pull'. */
	gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_2_MHZ,
		      GPIO_CNF_OUTPUT_PUSHPULL, red_led | blue_led);
}

void adc_setup(void)
{
	int i;
	rcc_peripheral_enable_clock(&RCC_APB2ENR, RCC_APB2ENR_ADC1EN);
	
	/* make shure it didnt run during config */
	adc_off(ADC1);
	
	/* we configure everything for one single conversion */	
	adc_disable_scan_mode(ADC1);
	adc_set_single_conversion_mode(ADC1);
	adc_enable_discontinous_mode_regular(ADC1);
	adc_disable_external_trigger_regular(ADC1);
	adc_set_right_aligned(ADC1);
	/* we want read out the temperature sensor so we have to enable it */
	adc_enable_temperature_sensor(ADC1);
	adc_set_conversion_time_on_all_channels(ADC1, ADC_SMPR_SMP_28DOT5CYC); 
	
	adc_on(ADC1);
	/* wait for adc starting up*/
        for (i = 0; i < 80000; i++);    /* Wait (needs -O0 CFLAGS). */
	
	adc_reset_calibration(ADC1);
	adc_calibration(ADC1);	
}

u8 adcchfromport(int command_port, int command_bit)
{
	/* 
	 PA0 ADC12_IN0
	 PA1 ADC12_IN1
	 PA2 ADC12_IN2
	 PA3 ADC12_IN3
	 PA4 ADC12_IN4
	 PA5 ADC12_IN5
	 PA6 ADC12_IN6
	 PA7 ADC12_IN7
	 PB0 ADC12_IN8
	 PB1 ADC12_IN9
	 PC0 ADC12_IN10
	 PC1 ADC12_IN11
	 PC2 ADC12_IN12
	 PC3 ADC12_IN13
	 PC4 ADC12_IN14
	 PC5 ADC12_IN15
	 temp ADC12_IN16
	 */
	switch (command_port) {
		case 0: /* port A */
			if (command_bit<8) return command_bit;
			break;
		case 1: /* port B */
			if (command_bit<2) return command_bit+8;
			break;
		case 2: /* port C */
			if (command_bit<6) return command_bit+10;
			break;
	}
	return 16;
}

void command_parse(int user_command)
{
	static int command_port = 0,	command_bit = 0,	command_action = ' ';
	int analog_data = 0;
	static u8 channel_array[16] = {16,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0};
	static u32 command_portr = GPIOA;
	switch (user_command) {
		case 'A': command_port = 0;	command_portr = GPIOA;	break;
		case 'B': command_port = 1;	command_portr = GPIOB;	break;
		case 'C': command_port = 2;	command_portr = GPIOC;	break;
		case 'D': command_port = 3;	command_portr = GPIOD;	break;
		case 'E': command_port = 4;	command_portr = GPIOE;	break;
		case '0': command_bit = 0;	break;
		case '1': command_bit = 1;	break;
		case '2': command_bit = 2;	break;
		case '3': command_bit = 3;	break;
		case '4': command_bit = 4;	break;
		case '5': command_bit = 5;	break;
		case '6': command_bit = 6;	break;
		case '7': command_bit = 7;	break;
		case '8': command_bit = 8;	break;
		case '9': command_bit = 9;	break;
		case 'a': command_bit = 10;	break;
		case 'b': command_bit = 11;	break;
		case 'c': command_bit = 12;	break;
		case 'd': command_bit = 13;	break;
		case 'e': command_bit = 14;	break;
		case 'f': command_bit = 15;	break;
		case 'o': gpio_set_mode(command_portr, GPIO_MODE_OUTPUT_2_MHZ,
					GPIO_CNF_OUTPUT_PUSHPULL, 1 << command_bit);
			command_action = 'o';
			break;
		case 'i': gpio_set_mode(command_portr, GPIO_MODE_INPUT,
					GPIO_CNF_INPUT_FLOAT, 1 << command_bit);
			command_action = 'i';
			break;
		case 'g': gpio_set_mode(command_portr, GPIO_MODE_INPUT,
					GPIO_CNF_INPUT_ANALOG, 1 << command_bit);
			command_action = 'g';
			channel_array[0] = adcchfromport(command_port,command_bit); /* address adc to current pin*/
			break;
		case 's': gpio_set(command_portr, 1 << command_bit);
			command_action = 's';
			break;
		case 'r': gpio_clear(command_portr, 1 << command_bit);
			command_action = 'r';
			break;
	}
	usart_send(USART1, 'A' + command_port );			/* send port */
	if(command_bit<10)	usart_send(USART1, '0' + command_bit ); /* send bit  */
	else			usart_send(USART1, 'a' - 10 + command_bit );
	usart_send(USART1, command_action );				/* send command */
	usart_send(USART1, ' ');
	if(channel_array[0]<10)	usart_send(USART1, '0' + channel_array[0]); /* send adc ch*/
	else			usart_send(USART1, 'a' - 10 + channel_array[0]);
	adc_set_regular_sequence(ADC1, 1, channel_array);
	adc_on(ADC1);	/* If the ADC_CR2_ON bit is already set -> setting it another time starts the conversion */
	while (!(ADC_SR(ADC1) & ADC_SR_EOC));	/* Waiting for end of conversion */
	analog_data = ADC_DR(ADC1) & 0xFFF;		/* read adc data */
	usart_send(USART1, ' ');
	if(analog_data > 1000)	usart_send(USART1, analog_data/1000 + '0');
				/* send adc val*/
	else usart_send(USART1, ' ');
	analog_data -= ((u8) (analog_data/1000)) * 1000;
	usart_send(USART1, analog_data/100 + '0');
	analog_data -= ((u8) (analog_data/100))  * 100;
	usart_send(USART1, analog_data/10 + '0');
	analog_data -= ((u8) (analog_data/10))   * 10;
	usart_send(USART1, analog_data + '0');
}

int main(void)
{
	int i, j = 0, c = 0;

	clock_setup();
	gpio_setup();
	usart_setup();
	adc_setup();		/* todo: check setup of analog peripheral*/
	
	/* Blink the LED on the board with every transmitted line refresh */
	while (1) {
		gpio_toggle(GPIOC, red_led);	/* LED on/off */
		for (j = 0; j < 5; j++) {
			usart_send(USART1, 'A' + j);
			switch (j) {
				case 0:	c = GPIOA_IDR;	break;
				case 1:	c = GPIOB_IDR;	break;
				case 2:	c = GPIOC_IDR;	break;
				case 3:	c = GPIOD_IDR;	break;
				case 4:	c = GPIOE_IDR;	break;
			}
			for (i = 15; i > -1; i--) {
				usart_send(USART1, '0'+ ((c & (1 << i)) >> i));
			}
			usart_send(USART1, ' ');
		}
		if (USART1_SR & USART_SR_RXNE) { /* if we have received something */
			command_parse(usart_recv(USART1));	/* execute the command */
			gpio_toggle(GPIOC, blue_led);
		}
		usart_send(USART1, '\r');
		for (i = 0; i < 80000; i++);	/* Wait (needs -O0 CFLAGS). */
	}
	return 0;
}




