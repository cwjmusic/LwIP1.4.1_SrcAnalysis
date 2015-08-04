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

/* ���MEM_LIBC_MALLOC=1����ڴ����ʹ��C���ṩ��DMM������ʹ��lwip���ڲ�DMM */
#if MEM_LIBC_MALLOC

/* ***************��������********************************************** */
                                             /* for size_t */
#include <stddef.h> 

typedef size_t mem_size_t;                  /* ��Ϊ���ڴ����ʹ��C���ṩ��DMM��ʵ�֣�����mem_size_t���;���C���DMM
                                             * ʵ���йأ�C�����ṩ�ľ���size_t������mem_size_t����Ϊsize_t */

/* *****�������Ͷ�Ӧ�ĵ��Թ��ܵĸ�ʽ�����˵����************************ */
                                            /* mem_size_t�������ݸ�ʽ������ĸ�ʽ˵���� */
#define MEM_SIZE_F SZT_F


/* *****ʹ��C��DMMʱ�Ĳ����ӿ�ʵ��************************************** */
                                           /* aliases for C library malloc() */
                                           /* mem_init��ʹ��C��DMMʱ��lwip����ʵ�֡�lwip_init
										    * ʵ���ϲ��õ��ã����Խ��ú������滻�ɿ� */
#define mem_init()
                                           /* ʹ��#ifndef..#endif����Ԥ����ָ���Ŀ���ǣ�
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
                                           * ʹ��C��DMMʱ����ΪC���в�û�ж�Ӧ��shrink memory without moving it�ĺ�����
										   * ����mem_trim��Ҫ���з���ֵ��������������е��ڴ����׵�ַ�����Ա��꺯��
										   * ʵ��ֱ�Ӻ��滻����������е��ڴ���׵�ַ�� */
#ifndef mem_trim
#define mem_trim(mem, size) (mem)
#endif
/* *********����MEM_LIBC_MALLOC=0����ʾʹ��lwip�ڲ���DMM*************************************** */
#else /* MEM_LIBC_MALLOC */
/* ********�������ͺ͸�ʽ�����˵��������********************************* */
/* ��Ϊ   mem_size_t���;����������޷�����������Ϊ���ڴ�ʵ�ʷ���Ĵ�С��MEM_SIZE+���ڶ���
 *        ���ڴ��С���йأ�
 * ����   �����ϵ���MEM_SIZE+���ڶ�����ڴ��С������16�޷����ܱ�ʾ���������65535��ʱ��
 *        mem_size_t����32λ�޷���������ʾ��������16λ2�޷�����������ʾ��
 * ����   ��Ϊ�����ڶ�����ڴ��С����ȷ����
 * ����   ��ʵ��ʱ�����衰���ڶ�����ڴ��С�����ֵ��1535���ֽڣ�����ʹ��MEM_SIZE���бȽϣ�
 *        ���MEM_SIZE>(65535-1535)=64000����ʹ��32λ�޷���������������ʹ��16λ�޷�����������
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

/* *********�����ӿں���ʵ�ּ�ԭ������******************************** */
/* ���lwip�ڲ�DMMʹ�û��ڹ̶��ڴ��ĳ��ڴ���ԣ���mem_init��mem_trim����ʵ�ʲ��õ��ã�
 * ʵ��ʱ��ʹ��Ԥ����ָ�����չΪ�գ�ͬʱע�Ᵽ�ֽӿڲ��䣩���൱��ɾ���ϲ������Щ
 * ����ʱ�Ĵ��� */
#if MEM_USE_POOLS  
                                          /* mem_init is not used when using pools instead of a heap */
#define mem_init()
                                          /* mem_trim is not used when using pools instead of a heap:
                                           * we can't free part of a pool element and don't want to copy the rest */
#define mem_trim(mem, size) (mem)
#else /* MEM_USE_POOLS */ 
/* ���򣬱�ʾlwip�ڲ�DMMʹ�û��ڿɱ��ڴ��Ķ��ڴ���ԣ���mem_init��mem_trim������Ҫʵ�֣�
 * ͷ�ļ���ֻ����ԭ������ */
void  mem_init(void);
void *mem_trim(void *mem, mem_size_t size);
#endif /* MEM_USE_POOLS */  


/* ʣ������ӿں�����ԭ������ */
void *mem_malloc(mem_size_t size);
void *mem_calloc(mem_size_t count, mem_size_t size);
void  mem_free(void *mem);
#endif /* MEM_LIBC_MALLOC */



/* ******************���ڴ����������ʵ�ֲ����޹صĲ����ӿڣ����ݶ��봦��*********************** */

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
