/************************
Here you have a USART communication example through for the NUCLEO-F767ZI Board
1. On the serial terminal the MCU returns the Byte sent from the console (Echo)
2. When the ASCII of 'a' is received then it toggles the LED connected to PB0

 - USART Configuration at 115200 bauds
 - PLL Configuration
 - GPIO Initialization
************************/

#include <stdio.h>
#include <string.h>
#include "stm32f7xx.h"                  
static void SystemClock_Config(void);
static void GPIO_Init (void);
static void USART3_Init (void);

short rec_flag = 0;
void USART3_IRQHandler(void){      						// Data received Interrupt
	if (USART3->ISR & USART_ISR_RXNE ) {  			// Check on RXNE flag (if active)
		char data = USART3->RDR;									// Save received byte on variable data	
		USART3->TDR = data; 											// Echo		
		while ((USART3->ISR &= 0x80)==0);   			// Wait until TXE flag is 1, indicating shifting to TDR
		while ((USART3->ISR &= 0x40)==0);					// Wait until TC flag is 1, indicating Transfer complete
		
		if (data == 'a'){
			rec_flag = 1;
		}
	}
}


void GPIO_Init(void){								// GPIO Configuration
	// LED GPIO Config
	RCC->AHB1ENR  |= 0x00000002; 			// Enable clock for GPIOB
  GPIOB->MODER	|= 0x10004001;			// Configure PB0, PB7 and PB14 as Outputs Green, Blue and Red LEDs
	
	// USART_3 GPIO Config
  RCC->AHB1ENR  |=  0x00000008; 			// Enable clock for GPIOD
  GPIOD->AFR[1] |=  0x00000077; 			// select AF7 (USART_3) for PD8,PD9
  GPIOD->MODER  |=  0x000A0000; 			// PD8,PD9 => alternate functions
	GPIOD->OTYPER |=  0x0000;						// Push-pull function on the pins	
	GPIOD->PUPDR	|=	0x000A0000; 			// Pull-up resistors on pins	
	GPIOD->OSPEEDR	|=	0x000F0000;			// High frequency function on PD8 and PD9
}

void USART3_Init(void){								// USART Configuration	
	RCC->APB1ENR	|= 0x40000; 					// Enable clock for USART_3
	RCC->DCKCFGR2 |= 0x00000000;				// USART3 CLK Source as APB1 CLK = 48 MHz	
	
	USART3->CR1		|= 0x0;								// Default configuration of CR1 register
	USART3->CR1 	|= 0x2C;      				// Tx, Rx enabled, Interrupt Rx enabled
	
	/* USART oversampling by 8, fclk = 48 MHz
				-> USARTDIV = 2*48e6/BAUDRATE = 2*48e6/9600 = 0x2710 = dec(10000)
		 USART oversampling by 16, fclk = 48 MHz
				-> USARTDIV = 48e6/BAUDRATE = 48e6/9600 = 0x1388 = dec(5000)
		 ex.-> USART3->BRR = 0x1388;      // 9600 Baudios, fclk=48MHz
		 ex.-> USART3->BRR = 0xD0;        // 230400 Baudios, fclk=48MHz*/
	
	USART3->BRR = 0x1A0;      					// 115200 Baudios, fclk=48MHz	
	//USART3->CR2 |= 0x00100000;				// Autobaudrate detection	
	USART3->CR1 |= 0x1;       					// USART, UE=1 	
	NVIC_EnableIRQ(USART3_IRQn);      	// Interrupt function enable USART3 
}



long Cont;
int main () 
{
	SystemClock_Config();
	SystemCoreClockUpdate();
	GPIO_Init();	
	USART3_Init();
  while(1) {
		GPIOB->ODR = 0x0001 & GPIOB->ODR;		// LEDs turned off
		for(Cont=0;Cont<100000;Cont++);			// Wastes some time
		GPIOB->ODR = 0x4080 | GPIOB->ODR;		// LEDs turned on
		for(Cont=0;Cont<100000;Cont++);		  // Wastes some time
		
		if (rec_flag == 1){
			rec_flag = 0;
			GPIOB->ODR = (0xFFFE & GPIOB->ODR) | ~(0x0001 & GPIOB->ODR);		// Output state and Toggle of LED1 
		}
	}
}


static void SystemClock_Config(void)
{	
	FLASH->ACR |= 0x01;__NOP();																//Sets the wait states to 1	
	RCC->APB1ENR |= (RCC_APB1ENR_PWREN);__NOP();							// Enables power interface 
		
	PWR->CR1 &= ~(PWR_CR1_VOS);																// Clear bits for voltage scaling				
	PWR->CR1 |= PWR_CR1_VOS_0;__NOP();												//Sets the voltage scaling mode to 3, VOS = 0x1 = b1
	
	// Configuration at SYSCLK = 192 MHz
	// Change PLL_N at the value you need to configure the fout at your desired frequency
	// In this specific configuration PLL_N = fout 
	RCC->PLLCFGR = ( 8ul                   |                 // PLL_M =  8 		fM = 16MHz/M = 2
                 (192ul <<  6)            |                // PLL_N = 192		VCO = N*fM = 384 MHz
                 (  0ul << 16)            |                // PLL_P =   2 	fout = VCO/PLLP = 216 MHz
                 (RCC_PLLCFGR_PLLSRC_HSI) |                // PLL_SRC = HSI 
                 (  8ul << 24)            |                // PLL_Q =   8
                 (  7ul << 28)             );              // PLL_R =   7
	
	RCC->CFGR |= (RCC_CFGR_HPRE_DIV1  |                      // HCLK = SYSCLK -> 192 MHz
                RCC_CFGR_PPRE1_DIV4 |                      // APB1 = HCLK/4 -> 48 MHz
                RCC_CFGR_PPRE2_DIV2	  );                   // APB2 = HCLK/2 -> 96 MHz
	
	FLASH->ACR |= 0x07;__NOP();
	
	RCC->CR |= RCC_CR_PLLON;                                 // Enable PLL
  while(!((RCC->CR & RCC_CR_PLLRDY) == RCC_CR_PLLRDY));    // Wait until PLL is ready
	
	RCC->CFGR |=  RCC_CFGR_SW_PLL;
  while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL);  // Wait until PLL is system clock src	
}
