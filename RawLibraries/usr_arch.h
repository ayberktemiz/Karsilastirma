#ifndef __USR_ARCH_H
#define __USR_ARCH_H

#ifndef _iar
    #define STM32_L051R8
#endif 

#ifndef _2G
    #define GSM2G
#endif

#ifdef _4G
    #define GSM4G
#endif


#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h> 
#include <math.h>

#define _io  static
#define _iov static volatile

#endif //__USR_ARCH_H


/*
    Usage: 
    IAR -> Project -> Options -> C/C++ Compiler -> Preprocessor 
    Defined Symbols:
        USE_HAL_DRIVER
        STM32L051xx                 --> işlemci tipi
        _iar                        --> bu hiçbir yerde tanımlı değilse aşağıdaki makrou çalıştır
        STM32_L051R8                --> Üsteki makroya göre bunu tanımla işlmeci tipine göre kütüphane
        _2G                         --> bu hiçbir yerde tanımlı değilse aşağıdaki makrou çalıştır
        GSM2G                       --> Üsteki makroya göre bunu tanımla GSM modulüne göre kütüphane
        _acc_module_using           --> accelometre tanımlama kütüphanesi
        DEBUG    

*/