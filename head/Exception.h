//
// Created by bin.zhang on 2017/12/19.
//

#ifndef MUSICDR_EXCEPTION_H
#define MUSICDR_EXCEPTION_H

#include <setjmp.h>

jmp_buf Jump_Buffer;

#define try if(setjmp(Jump_Buffer)==0)

#define catch else

#define throw(n) longjmp(Jump_Buffer, n)

#endif //MUSICDR_EXCEPTION_H
