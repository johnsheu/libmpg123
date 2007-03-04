/*
	getcpucpuflags: get cpuflags for ia32

	copyright ?-2006 by the mpg123 project - free software under the terms of the LGPL 2.1
	see COPYING and AUTHORS files in distribution or http:#mpg123.de
	initially written by KIMURA Takuhiro (for 3DNow!)
	extended for general use by Thomas Orgis

	 extern int getcpuid(struct cpuflags*)
	or just 
	 extern int getcpuid(unsigned int*)
	where there is memory for 4 ints
	 -> the first set of idflags (basic cpu family info)
	    and the idflags, stdflags, std2flags, extflags written to the parameter
	 -> 0x00000000 (CPUID instruction not supported)
*/

#include "mangle.h"

.text
	.align 4

.globl ASM_NAME(getcpuflags)
	.type ASM_NAME(getcpuflags),@function
ASM_NAME(getcpuflags):
	pushl %ebp
	movl %esp,%ebp
	pushl %edx
	pushl %ecx
	pushl %ebx
	pushl %esi
/* get the int pointer for storing the flags */
	movl 8(%ebp), %esi
/* does that one make sense? */
	movl $0x80000000,%eax
/* now save the flags and do a check for cpuid availability */
	pushfl
	pushfl
	popl %eax
	movl %eax,%ebx
/* set that bit... */
	xorl $0x00200000,%eax
	pushl %eax
	popfl
/* ...and read back the flags to see if it is understood */
	pushfl
	popl %eax
	popfl
	cmpl %ebx,%eax
	je .Lnocpuid
/* now get the info, first extended */
	movl $0x80000001,%eax
	cpuid
	movl %edx,12(%esi)
/* then the other ones, called last to get the id flags in %eax for ret */
	movl $0x00000001,%eax
	cpuid
	movl %eax, (%esi)
	movl %ecx, 4(%esi)
	movl %edx, 8(%esi)
	jmp .Lend
	.align 4
.Lnocpuid:
/* error: set everything to zero */
	movl $0, %eax
	movl $0, (%esi)
	movl $0, 4(%esi)
	movl $0, 8(%esi)
	movl $0, 12(%esi)
	.align 4
.Lend:
/* return value are the id flags, still stored in %eax */
	popl %esi
	popl %ebx
	popl %ecx
	popl %edx
	movl %ebp,%esp
	popl %ebp
	ret
