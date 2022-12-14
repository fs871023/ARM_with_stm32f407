/**
  ******************************************************************************
  * @file    main.c
  * @author  Auto-generated by STM32CubeIDE
  * @version V1.0
  * @brief   Default main function.
  ******************************************************************************
*/

int main(void)
{
// inline assembly statement
#if 0
	//volatile: to instruct the compiler not to optimize the assembler code
	__asm volatile("LDR R1,=#0x20001000"); //init R1
	__asm volatile("LDR R2,=#0x20001004");
	__asm volatile("LDR R0,[R1]"); //read
	__asm volatile("LDR R1,[R2]");
	__asm volatile("ADD R0,R0,R1"); //operation
	__asm volatile("STR R0, [R2]"); //store
	//[something]: means value in this address
	//see window->register
	//play with window->memory->monitor, and reset the value in memory
#endif
	//c value to ARM register
	//__asm volatile(code:output operand list : input operand list : clobber list)
	//use ',' to separate in list
	int val = 25;
	__asm volatile("MOV R0,%0": :"r"(val));
	//%0 means first value in operand
	//"r":constraint string = constraint char + constraint modifier
	//r: the compiler user general registers r0,...,r15
	//go through a progress like "value->register R3->R0"
	//see inline_1/debug/inline_1.list
	__asm volatile("MOV R1,%0": :"i"(0x50));
	//i: give an immediate value
	//then the compiler will go through a progress like "0x50->R0" instead of above one

	// read CONTROL register to control_reg
	int control_reg;
	__asm volatile("MRS %0,CONTROL":"=r"(control_reg));
	//MRS: move value from special reg to general reg
	//=: write-only operand, always used for all output operands

	//implement "var2=var1;"
	int var1=15, var2;
	__asm volatile("MOV %0,%1": "=r"(var2): "r"(var1));
	// implement p1 = *p2
	int p1, *p2;
	p2 = (int*)0x20000008;
	__asm volatile("LDR %0,[%1]": "=r"(p1): "r"(p2));
}
