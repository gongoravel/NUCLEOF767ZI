/************************
Here you have a Blinking led example using the NUCLEO-F767ZI Board
 - PLL Configuration
 - GPIO Initialization
************************/

#include <stdio.h>
#include <string.h>
#include "stm32f7xx.h"                  
static void SystemClock_Config(void);
static void GPIO_Init (void);


short rec_flag = 0;
int8_t Dato;

void GPIO_Init(void){		
	// LED GPIO Config
	RCC->AHB1ENR  |= 0x00000002; 			// Enable clock for GPIOB
  GPIOB->MODER	|= 0x10004001;			// Configure PB0, PB7 and PB14 as Outputs Green, Blue and Red LEDs
}


long Cont;
int main () 
{
	SystemClock_Config();
	SystemCoreClockUpdate();
	GPIO_Init();	
  while(1) {
		GPIOB->ODR = 0x0000;								// LEDs turned off
		for(Cont=0;Cont<100000;Cont++);			// Wastes some time
		GPIOB->ODR = 0x4081;								// LEDs turned on
		for(Cont=0;Cont<100000;Cont++);		  // Wastes some time
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
