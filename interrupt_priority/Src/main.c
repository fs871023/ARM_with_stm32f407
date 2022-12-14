/**
  ******************************************************************************
  * @file    main.c
  * @author  Auto-generated by STM32CubeIDE
  * @version V1.0
  * @brief   Default main function.
  ******************************************************************************
*/
#define IRQNO_TIMER2	28
#define IRQNO_I2C1	31

#include<stdio.h>
#include<stdint.h>

//get addr from NVIC reg summary
uint32_t *pNVIC_IPRBase = (uint32_t*)0xE000E400; //priority
uint32_t *pNVIC_ISERBase = (uint32_t*)0xE000E100; //enable
uint32_t *pNVIC_ISPRBase = (uint32_t*)0xE000E200; //pending

void config_prior_for_irqs(uint8_t irq_no, uint8_t prior_val)
{
	//1. find out IPRx
	uint8_t iprx = irq_no / 4;
	uint32_t *ipr = pNVIC_IPRBase + iprx;

	//2. find out position in IPRx
	uint8_t pos = (irq_no % 4) * 8;

	//3. configure the priority
	*ipr &= ~(0xFF << pos);//clear priority bit
	*ipr |= (prior_val << pos); //placing the priority level
}

int main(void)
{
	//1. configure the priority for the peripherals
	// always do priority configuration before enable interrupt
	config_prior_for_irqs(IRQNO_TIMER2, 0x80);
	config_prior_for_irqs(IRQNO_I2C1, 0x70);

	//2. set the interrupt pending bit in the NVIC PR
	*pNVIC_ISPRBase |= (1 << IRQNO_TIMER2); //set TIMER2 pending bit
	//only trigger TIMER2

	//3. enable the IRQs in NVIC ISER
	*pNVIC_ISERBase |= (1 << IRQNO_I2C1);
	*pNVIC_ISERBase |= (1 << IRQNO_TIMER2);
//	for(;;);
}
//window -> show view ->SFRs
//IPR7(IRQ priority)
void TIM2_IRQHandler(void)
{
	printf("[TIM2_IRQHandler]\n");
	*pNVIC_ISPRBase |= (1 << IRQNO_I2C1);
	//while hit resume again, because the I2C1 has higher priority,
	//so it jump to the I2C1 handler
	while(1);
}
void I2C1_EV_IRQHandler(void)
{
	printf("[I2C1_EV_IRQHandler]\n");
}
