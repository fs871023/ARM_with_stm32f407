/**
 ******************************************************************************
 * @file           : main.c
 * @author         : Auto-generated by STM32CubeIDE
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2022 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */

#include <stdint.h>
#include <stdio.h>
#include "main.h"
#include "led.h"

#if !defined(__SOFT_FP__) && defined(__ARM_FP)
  #warning "FPU is not initialized, but the project is compiling for an FPU. Please initialize the FPU before use."
#endif

//psp of different tasks
//uint32_t psp_of_tasks[MAX_TASKS] = {T1_STACK_START,  T2_STACK_START,  T3_STACK_START,  T4_STACK_START};
//uint32_t task_handler[MAX_TASKS];

typedef struct
{
	uint32_t psp_value;
	uint32_t block_count;
	uint8_t current_state;
	void (*task_handler)(void);
}TCB_t;
TCB_t user_task[MAX_TASKS];

uint8_t current_task = 1; //task is running
uint32_t g_tick_count = 0;
void schedule(void)
{
	//pend the pendSV
	uint32_t *pICSR = (uint32_t*)0xE000ED04;
	*pICSR |= (1 << 28);
}
void task_delay(uint32_t tick_count)
{
	//Warning: The global var can be accessed from the fault handler and thread mode code.
	//Since the exception and interrupt handlers are asynchronous(非同步), which may cause race condition.
	//So, it is better to disable interrupt before we access this global var here.
	//But in some hard operation system, the interrupt disabling must be concerned, which would make the system fail.

	//disable interrupt
	//use PRIMASK reg
	INTERRUPT_DISABLE();

	if(current_task)//do nothing in idle task
	{
		//tick system maintained globally + tick task sent = block count of a task
		user_task[current_task].block_count = g_tick_count + tick_count;
		//the task will not be scheduled due to the block state
		user_task[current_task].current_state = TASK_BLOCK_STATE;
		schedule();
	}

	//enable interrupt
	INTERRUPT_ENABLE();
}

void idle_task(void)
{
	while(1);
}
void task1_handler(void)
{
	while(1)
	{
		led_on(LED_GREEN);
		task_delay(1000);
//		delay(DELAY_COUNT_1S);
		led_off(LED_GREEN);
		task_delay(1000);
//		delay(DELAY_COUNT_1S);
	}
}
void task2_handler(void)
{
	while(1)
	{
		led_on(LED_ORANGE);
		task_delay(500);
//		delay(DELAY_COUNT_500MS);
		led_off(LED_ORANGE);
		task_delay(500);
//		delay(DELAY_COUNT_500MS);
	}
}
void task3_handler(void)
{
	while(1)
	{
		led_on(LED_RED);
		task_delay(250);
//		delay(DELAY_COUNT_250MS);
		led_off(LED_RED);
		task_delay(250);
//		delay(DELAY_COUNT_250MS);
	}
}
void task4_handler(void)
{
	while(1)
	{
		led_on(LED_BLUE);
		task_delay(125);
//		delay(DELAY_COUNT_125MS);
		led_off(LED_BLUE);
		task_delay(125);
//		delay(DELAY_COUNT_125MS);
	}
}

void init_systick_timer(uint32_t tick_hz)
{
	uint32_t count_val = (SYSTICK_TIM_CLK / tick_hz)-1;
	//see the user guide to see systick values
	//see systick reload register
	//systick reload reg : the reg which specifies the start value
	//to load into the systick current value(CVR) reg.
	//When the timer enabled, the value of RVR will copied to CVR
	//When the CVR count to 0, the value of RVR will copied to CVR, too
	uint32_t *pSRVR = (uint32_t*)0xE000E014;
	//see systick control and status register(CSR)
	uint32_t *pSCSR = (uint32_t*)0xE000E010;

	//clear the value of RVR
	*pSRVR &= ~(0x00FFFFFFFF);
	//load the value in to RVR
	*pSRVR |= count_val;

	//set CSR
	*pSCSR |= (1 << 1);//enabled systick exception request (counting down to 0 assert systick exception request)
	*pSCSR |= (1 << 2);//indicates the clock source, processor clock source
	*pSCSR |= (1 << 0);//enabled the counter
}
__attribute__((naked)) void init_scheduler_stack(uint32_t sched_top_of_stack)
{
	__asm volatile("MSR MSP, %0": : "r" (sched_top_of_stack) : );
	__asm volatile("BX LR");//go back to main
	//BX: branch indirect
	//LR: link reg, which stores the return addr
}
//store dummy sf1(stack frame)& sf2 in stack memory of each task
void init_tasks_stack(void)
{
	//use ptr to access stack memory
	user_task[0].current_state = TASK_READY_STATE;
	user_task[0].psp_value = IDLE_STACK_START;
	user_task[0].task_handler = idle_task;

	user_task[1].current_state = TASK_READY_STATE;
	user_task[1].psp_value = T1_STACK_START;
	user_task[1].task_handler = task1_handler;

	user_task[2].current_state = TASK_READY_STATE;
	user_task[2].psp_value = T2_STACK_START;
	user_task[2].task_handler = task2_handler;

	user_task[3].current_state = TASK_READY_STATE;
	user_task[3].psp_value = T3_STACK_START;
	user_task[3].task_handler = task3_handler;

	user_task[4].current_state = TASK_READY_STATE;
	user_task[4].psp_value = T4_STACK_START;
	user_task[4].task_handler = task4_handler;

	uint32_t  *pPSP;
	for(int i = 0; i < MAX_TASKS; i ++)
	{
//		pPSP = (uint32_t*)psp_of_tasks[i];
		pPSP = (uint32_t*) user_task[i].psp_value;

		//decrement first then store value
		pPSP --;
		*pPSP = DUMMY_XPSR;//0x01000000

		pPSP --;//PC  return addr
//		*pPSP = task_handler[i];//make sure it is odd
		*pPSP = (uint32_t)user_task[i].task_handler;

		pPSP --;//LR
		*pPSP = 0xFFFFFFFD;
		//return to thread mode, exception return uses non-floatint state
		//from the PSP, and execution uses PSP after return

		//all registers are zero (13 zeros)
		for(int j = 0; j < 13; j ++)
		{
			pPSP --;
			*pPSP = 0;
		}
		//preserve the value of psp
//		psp_of_tasks[i] = (uint32_t)pPSP;
		user_task[i].psp_value = (uint32_t)pPSP;

	}
}

void enable_processor_fault(void)
{
	uint32_t *pSHCSR = (uint32_t*) 0xE000ED24;
	*pSHCSR |= (1 << 16); //mem manage fault
	*pSHCSR |= (1 << 17); //bus fault
	*pSHCSR |= (1 << 18); //usage fault
}
void HardFault_Handler(void)
{
	printf("Exception: HardFault\n");
	while(1);
}
void MemManage_Handler(void)
{
	printf("Exception: MemManage\n");
	while(1);
}
void BusFault_Handler(void)
{
	printf("Exception: BusFault\n");
	while(1);
}

uint32_t get_psp_value(void)
{
	return user_task[current_task].psp_value;
}
void save_psp_value(uint32_t current_psp_value)
{
	user_task[current_task].psp_value = current_psp_value;
}
void update_next_task(void)
{
	int state = TASK_BLOCK_STATE;

	for(int i = 0; i < MAX_TASKS; i ++)
	{
		current_task ++;
		current_task %= MAX_TASKS;
		state = user_task[current_task].current_state;// get its current state
		if((state == TASK_READY_STATE) && (current_task != 0) )
			break; // if it is ready, then schedule it
	}
	if(state != TASK_READY_STATE)//all tasks are blocked
		current_task = 0;
}
__attribute__((naked)) void switch_sp_to_psp(void)
{
	// 1. initialize the PSP with task1 stack start addr
	//SP = MSP here
	//save the LR first, or the value of LR will lost, which is come from main()
	__asm volatile("PUSH {LR}");
	__asm volatile("BL get_psp_value");//get the value of psp of current task
	//Branch with Link because we will come back to this func
	__asm volatile("MSR PSP, R0");// initialize PSP reg
	__asm volatile("POP {LR}");//pop back LR value

	// 2. change SP to PSP using CONTROL register
	__asm volatile("MOV R0, #0X02");// set first bit of CONTROL reg
	__asm volatile("MSR CONTROL, R0");//change sp reg addr from MSP to PSP
	__asm volatile("BX LR");
	//LR: reg of capturing the return addr. it knows where to go back when the func finished.
}

int main(void)
{
	enable_processor_fault();//copy code from fault_gen
	//initialize the scheduler stack
	init_scheduler_stack(SCHED_STACK_START);
	//capture the addr of different task handler
//	task_handler[0] = (uint32_t)task1_handler;
//	task_handler[1] = (uint32_t)task2_handler;
//	task_handler[2] = (uint32_t)task3_handler;
//	task_handler[3] = (uint32_t)task4_handler;
	//task stack init
	init_tasks_stack();

	led_init_all();

	//generate systick timer exception
	init_systick_timer(TICK_HZ);

	switch_sp_to_psp();

	task1_handler();//trap here
    /* Loop forever */
	for(;;);
}
//context switch here
__attribute__((naked)) void PendSV_Handler(void)
{
	/*Save the context of current task*/
	//1. get current running task's PSP value
	__asm volatile("MRS R0, PSP");

	//2. using that PSP value store SF2 (R4 to R11)
	//**we can't use PUSH here, because it will affect the MSP
	//we store the reg value to task private stack
	__asm volatile("STMDB R0!, {R4-R11}");
	//!: what is loaded from or stored to is written back into Rn
	// so Rn will update when each store
	//STMDB: store multiple reg, decrement before each access

	//!!save LR value first
	__asm volatile("PUSH {LR}");
	//3. save the current value of PSP
	__asm volatile("BL save_psp_value");

	/*Retrieve the context of next task*/
	//1. Decide next task to run
	__asm volatile("BL update_next_task");
	//2. get its past PSP value
	__asm volatile("BL get_psp_value");

	//3. using that PSP value retrieve SF2(R4 to R11)
	__asm volatile("LDMIA R0!, {R4-R11}");
	//LDMIA: load multiple reg and increment after

	//4. update PSP and exit
	__asm volatile("MSR PSP, R0");
	__asm volatile("POP {LR}");
	//because it is naked now
	__asm volatile("BX LR");
}
void update_global_tick_count(void)
{
	g_tick_count ++;
}
void unblock_tasks(void)
{
	for(int i = 1; i < MAX_TASKS; i ++)//we don't need to check the idle task
	{// user task is not running
		if(user_task[i].current_state != TASK_READY_STATE)
		{// the delay is elapsed
			if(user_task[i].block_count == g_tick_count)
			{// change state
				user_task[i].current_state = TASK_READY_STATE;
			}
		}
	}
}
void SysTick_Handler(void)
{
	uint32_t *pICSR = (uint32_t*)0xE000ED04;
	update_global_tick_count();
	//check if there is any blocked task which can change to ready state
	//by comparing block_tick_count or block period of tasks
	//with the global_tick_count value
	unblock_tasks();
	//pend the pendSV exception
	*pICSR |= (1 << 28);
}

//__attribute__((naked)) void SysTick_Handler(void)
//{
//	__asm volatile("MRS R0, PSP");
//	__asm volatile("STMDB R0!, {R4-R11}");
//	__asm volatile("PUSH {LR}");
//	__asm volatile("BL save_psp_value");

//	__asm volatile("BL update_next_task");
//	__asm volatile("BL get_psp_value");
//	__asm volatile("LDMIA R0!, {R4-R11}");
//	__asm volatile("MSR PSP, R0");

//	__asm volatile("POP {LR}");

//	__asm volatile("BX LR");
//}
