//*****************************************************************************
//
//! @file hello_world.c
//!
//! @brief A simple "Hello World" example.
//!
//! This example prints a "Hello World" message with some device info
//! over SWO at 1M baud. To see the output of this program, use J-Link
//! SWO Viewer (or similar viewer appl) and configure SWOClock for 1000.
//! The example sleeps after it is done printing.
//
//*****************************************************************************

//*****************************************************************************
//
// Copyright (c) 2023, Ambiq Micro, Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice,
// this list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright
// notice, this list of conditions and the following disclaimer in the
// documentation and/or other materials provided with the distribution.
//
// 3. Neither the name of the copyright holder nor the names of its
// contributors may be used to endorse or promote products derived from this
// software without specific prior written permission.
//
// Third party software included in this distribution is subject to the
// additional license terms as defined in the /docs/licenses directory.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// This is part of revision release_sdk_4_4_1-7498c7b770 of the AmbiqSuite Development Package.
//
//*****************************************************************************

#include "am_mcu_apollo.h"
#include "am_bsp.h"
#include "am_util.h"

//*****************************************************************************
//
// Insert compiler version at compile time.
//
//*****************************************************************************
#define STRINGIZE_VAL(n)                    STRINGIZE_VAL2(n)
#define STRINGIZE_VAL2(n)                   #n

#ifdef __GNUC__
#define COMPILER_VERSION                    ("GCC " __VERSION__)
#elif defined(__ARMCC_VERSION)
#define COMPILER_VERSION                    ("ARMCC " STRINGIZE_VAL(__ARMCC_VERSION))
#elif defined(__KEIL__)
#define COMPILER_VERSION                    "KEIL_CARM " STRINGIZE_VAL(__CA__)
#elif defined(__IAR_SYSTEMS_ICC__)
#define COMPILER_VERSION                    __VERSION__
#else
#define COMPILER_VERSION                    "Compiler unknown"
#endif

#define ENABLE_DEBUGGER

//constants
#define N 4
#define WORD 4

#define _nv __align(16) __attribute__((section("_nv")))

#define WRITE_MRAM(array_ptr, start_address, size)                                			\
    do                                                                         	 				\
    {                                                                           				\
        if (((uint32_t)(start_address) % 16) != 0)                              				\
        {                                                                       				\
            am_util_stdio_printf("The starting address is not alligned to 16 byte.\n"); \
        }                                                                       				\
        else                                                                    				\
        {                                                                       				\
            returnCode = am_hal_mram_main_program(AM_HAL_MRAM_PROGRAM_KEY,  				    \
                                                      (array_ptr),                  		\
                                                      (start_address),          				\
                                                      (size));                  				\
            am_util_stdio_printf("MRAM program returnCode = %d\n", returnCode); 				\
        }                                                                       				\
    } while (0)

//global variables
int returnCode = 0;
static uint32_t mram_data[N];
_nv uint32_t x = 0;
_nv uint32_t x_cpy = 256;

//*****************************************************************************
//
// Main
//
//*****************************************************************************
int
main(void)
{
		uint32_t *x_addr = &x;
		uint32_t *x_cpy_addr = &x_cpy;
	
    //
    // Set the default cache configuration
    //
    am_hal_cachectrl_config(&am_hal_cachectrl_defaults);
    am_hal_cachectrl_enable();

    //
    // Configure the board for low power operation.
    //
    am_bsp_low_power_init();

    //
    // Initialize the printf interface for ITM output
    //
    if (am_bsp_debug_printf_enable())
    {
        // Cannot print - so no point proceeding
        while(1);
    }
		
		am_util_stdio_terminal_clear();
		
		mram_data[0] = x_cpy;
		uint32_t *ptr = mram_data;

		WRITE_MRAM(ptr, x_addr, WORD);
		
		am_util_stdio_printf("x = 0x%08x = 0x%08x\n", x, *x_addr);

    am_util_stdio_printf("x_addr = 0x%08x\n\n", x_addr);
		
		//
    // We are done printing.
    // Disable debug printf messages on ITM.
    //
    am_util_stdio_printf("Done with prints. Entering While loop\n");
    am_bsp_debug_printf_disable();
		
		while (1)
    {
        // sleep
        am_hal_sysctrl_sleep(AM_HAL_SYSCTRL_SLEEP_DEEP);
    }
}
