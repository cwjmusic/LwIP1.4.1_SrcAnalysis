/**
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
 * All rights reserved. 
 * 
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED 
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, 
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT 
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING 
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY 
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 * 
 * Author: Adam Dunkels <adam@sics.se>
 *
 */
#ifndef __LWIP_MEM_H__
#define __LWIP_MEM_H__

#include "lwip/opt.h"

#ifdef __cplusplus
extern "C" {
#endif

/* 如果MEM_LIBC_MALLOC=1则堆内存管理使用C库提供的DMM，否则使用lwip的内部DMM */
#if MEM_LIBC_MALLOC

/* ***************数据类型********************************************** */
                                             /* for size_t */
#include <stddef.h> 

typedef size_t mem_size_t;                  /* 因为堆内存管理使用C库提供的DMM来实现，所以mem_size_t类型就与C库的DMM
                                             * 实现有关，C库中提供的就是size_t。所以mem_size_t定义为size_t */

/* *****数据类型对应的调试功能的格式化输出说明符************************ */
                                            /* mem_size_t类型数据格式化输出的格式说明符 */
#define MEM_SIZE_F SZT_F


/* *****使用C库DMM时的操作接口实现************************************** */
                                           /* aliases for C library malloc() */
                                           /* mem_init在使用C库DMM时，lwip不用实现。lwip_init
										    * 实际上不用调用，所以将该函数宏替换成空 */
#define mem_init()
                                           /* 使用#ifndef..#endif条件预编译指令的目的是：
										    * in case C library malloc() needs extra protection,
                                            * allow these defines to be overridden. */
#ifndef mem_free
#define mem_free free
#endif
#ifndef mem_malloc
#define mem_malloc malloc
#endif
#ifndef mem_calloc
#define mem_calloc calloc
#endif
                                          /* Since there is no C library allocation function to shrink memory without
                                           * moving it, define this to nothing.
                                           * 使用C库DMM时，因为C库中并没有对应的shrink memory without moving it的函数，
										   * 另外mem_trim又要求有返回值且仍是输入参数中的内存块的首地址，所以本宏函数
										   * 实现直接宏替换成输入参数中的内存块首地址。 */
#ifndef mem_trim
#define mem_trim(mem, size) (mem)
#endif
/* *********否则MEM_LIBC_MALLOC=0，表示使用lwip内部的DMM*************************************** */
#else /* MEM_LIBC_MALLOC */
/* ********数据类型和格式化输出说明符定义********************************* */
/* 因为   mem_size_t类型具体是哪种无符号整型数与为堆内存实际分配的大小（MEM_SIZE+用于对齐
 *        的内存大小）有关；
 * 所以   理论上当（MEM_SIZE+用于对齐的内存大小）大于16无符号能表示的最大数（65535）时，
 *        mem_size_t就用32位无符号整数表示，否则用16位2无符号整型数表示；
 * 但是   因为“用于对齐的内存大小”不确定，
 * 所以   在实现时，假设“用于对齐的内存大小”最大值是1535个字节，并且使用MEM_SIZE进行比较，
 *        如果MEM_SIZE>(65535-1535)=64000，则使用32位无符号整型数，否则使用16位无符号整型数。
 */
/* MEM_SIZE would have to be aligned, but using 64000 here instead of
 * 65535 leaves some room for alignment...
 */
										
#if MEM_SIZE > 64000L
typedef u32_t mem_size_t;
#define MEM_SIZE_F U32_F
#else
typedef u16_t mem_size_t;
#define MEM_SIZE_F U16_F
#endif /* MEM_SIZE > 64000 */

/* *********操作接口函数实现及原型声明******************************** */
/* 如果lwip内部DMM使用基于固定内存块的池内存策略，则mem_init和mem_trim函数实际不用调用，
 * 实现时就使用预处理指令，宏扩展为空（同时注意保持接口不变），相当于删除上层调用这些
 * 函数时的代码 */
#if MEM_USE_POOLS  
                                          /* mem_init is not used when using pools instead of a heap */
#define mem_init()
                                          /* mem_trim is not used when using pools instead of a heap:
                                           * we can't free part of a pool element and don't want to copy the rest */
#define mem_trim(mem, size) (mem)
#else /* MEM_USE_POOLS */ 
/* 否则，表示lwip内部DMM使用基于可变内存块的堆内存策略，则mem_init和mem_trim函数需要实现，
 * 头文件中只进行原型声明 */
void  mem_init(void);
void *mem_trim(void *mem, mem_size_t size);
#endif /* MEM_USE_POOLS */  


/* 剩余操作接口函数的原型声明 */
void *mem_malloc(mem_size_t size);
void *mem_calloc(mem_size_t count, mem_size_t size);
void  mem_free(void *mem);
#endif /* MEM_LIBC_MALLOC */



/* ******************堆内存管理器的与实现策略无关的操作接口：数据对齐处理*********************** */

/* Calculate memory size for an aligned buffer - returns the next highest
 * multiple of MEM_ALIGNMENT (e.g. LWIP_MEM_ALIGN_SIZE(3) and
 * LWIP_MEM_ALIGN_SIZE(4) will both yield 4 for MEM_ALIGNMENT == 4).
 */
#ifndef LWIP_MEM_ALIGN_SIZE
#define LWIP_MEM_ALIGN_SIZE(size) (((size) + MEM_ALIGNMENT - 1) & ~(MEM_ALIGNMENT-1))
#endif

/* Calculate safe memory size for an aligned buffer when using an unaligned
 * type as storage. This includes a safety-margin on (MEM_ALIGNMENT - 1) at the
 * start (e.g. if buffer is u8_t[] and actual data will be u32_t*)
 */
#ifndef LWIP_MEM_ALIGN_BUFFER
#define LWIP_MEM_ALIGN_BUFFER(size) (((size) + MEM_ALIGNMENT - 1))
#endif

/* Align a memory pointer to the alignment defined by MEM_ALIGNMENT
 * so that ADDR % MEM_ALIGNMENT == 0
 */
#ifndef LWIP_MEM_ALIGN
#define LWIP_MEM_ALIGN(addr) ((void *)(((mem_ptr_t)(addr) + MEM_ALIGNMENT - 1) & ~(mem_ptr_t)(MEM_ALIGNMENT-1)))
#endif

#ifdef __cplusplus
}
#endif

#endif /* __LWIP_MEM_H__ */
