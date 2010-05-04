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

#include <libopenstm32/rcc.h>
#include <libopenstm32/gpio.h>
#include <libopenstm32/usart.h>
#include <STM32MCU.h>

void clock_setup(void)
{
	rcc_clock_setup_in_hse_8mhz_out_72mhz();

	/* Enable all present GPIOx clocks. (whats with GPIO F and G?)*/
	rcc_peripheral_enable_clock(&RCC_APB2ENR, IOPAEN);
	rcc_peripheral_enable_clock(&RCC_APB2ENR, IOPBEN);
	rcc_peripheral_enable_clock(&RCC_APB2ENR, IOPCEN);
	rcc_peripheral_enable_clock(&RCC_APB2ENR, IOPDEN);
	rcc_peripheral_enable_clock(&RCC_APB2ENR, IOPEEN);
	
	/* Enable clock for USART1. */
	rcc_peripheral_enable_clock(&RCC_APB2ENR, USART1EN);
}

void usart_setup(void)
{
	/* Setup GPIO pin GPIO_USART3_TX/GPIO10 on GPIO port B for transmit. */
	gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ,
                      GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO_USART1_TX);

	/* Setup UART parameters. */
	usart_set_baudrate(USART1, 38400);
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

void command_parse(int user_command)
{
	static int command_port = 0,	command_bit = 0,	command_action = ' ';
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
			command_action = 'g'; /* todo: turn on analog peripheral*/ 
			break;
		case 's': gpio_set(command_portr, 1 << command_bit);
			command_action = 's';
			break;
		case 'r': gpio_clear(command_portr, 1 << command_bit);
			command_action = 'r';
			break;
		
	}
	usart_send(USART1, 'A' + command_port );
	if(command_bit<10)	usart_send(USART1, '0' + command_bit );
	else			usart_send(USART1, 'a' - 10 + command_bit );
	usart_send(USART1, command_action );
}

int main(void)
{
	int i, j = 0, c = 0;

	clock_setup();
	gpio_setup();
	usart_setup();

	/* Blink the LED on the board with every transmitted byte. */
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
//			v = usart_recv(USART1);	/* find out what */
//			usart_send(USART1, v );	/* echo it back */
			command_parse(usart_recv(USART1));	/* execute the command */
//			j++;
			gpio_toggle(GPIOC, blue_led);
		}
		usart_send(USART1, '\r');
		for (i = 0; i < 80000; i++);	/* Wait (needs -O0 CFLAGS). */
	}
	return 0;
}
