#
# Copyright (C) 2012 Xilinx, Inc.
#
# This file is part of the port for FreeRTOS made by Xilinx to allow FreeRTOS
# to operate with Xilinx Zynq devices.
#
# This file is free software; you can redistribute it and/or modify it under
# the terms of the GNU General Public License (version 2) as published by the
# Free Software Foundation AND MODIFIED BY the FreeRTOS exception
# (see text further below).
#
# This file is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
# or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
# more details.
#
# You should have received a copy of the GNU General Public License; if not it
# can be viewed here: <http://www.gnu.org/licenses/>
#
# The following exception language was found at
# http://www.freertos.org/a00114.html on May 8, 2012.
#
# GNU General Public License Exception
#
# Any FreeRTOS source code, whether modified or in its original release form,
# or whether in whole or in part, can only be distributed by you under the
# terms of the GNU General Public License plus this exception. An independent
# module is a module which is not derived from or based on FreeRTOS.
#
# EXCEPTION TEXT:
#
# Clause 1
#
# Linking FreeRTOS statically or dynamically with other modules is making a
# combined work based on FreeRTOS. Thus, the terms and conditions of the
# GNU General Public License cover the whole combination.
#
# As a special exception, the copyright holder of FreeRTOS gives you permission
# to link FreeRTOS with independent modules that communicate with FreeRTOS
# solely through the FreeRTOS API interface, regardless of the license terms
# of these independent modules, and to copy and distribute the resulting
# combined work under terms of your choice, provided that 
#
# Every copy of the combined work is accompanied by a written statement that
# details to the recipient the version of FreeRTOS used and an offer by
# yourself to provide the FreeRTOS source code (including any modifications
# you may have  made) should the recipient request it.
# The combined work is not itself an RTOS, scheduler, kernel or related product.
# The independent modules add significant and primary functionality to FreeRTOS
# and do not merely extend the existing functionality already present 
# in FreeRTOS.
#
# Clause 2
#
# FreeRTOS may not be used for any competitive or comparative purpose,
# including the publication of any form of run time or compile time metric,
# without the express permission of Real Time Engineers Ltd. (this is the norm
# within the industry and is intended to ensure information accuracy).
#


# standalone bsp version. set this to the latest "ACTIVE" version.
#set standalone_version standalone_v3_10_a
set standalone_version standalone_v4_2

proc FreeRTOS_drc {os_handle} {

    global env

    set sw_proc_handle [get_sw_processor]
    set hw_proc_handle [get_cells [get_property HW_INSTANCE $sw_proc_handle] ]
    set proctype [get_property IP_NAME $hw_proc_handle]
    set procname [get_property NAME    $hw_proc_handle]

}

proc generate {os_handle} {

    variable standalone_version

    set sw_proc_handle [get_sw_processor]
    set hw_proc_handle [get_cells [get_property HW_INSTANCE $sw_proc_handle] ]
    set proctype [get_property IP_NAME $hw_proc_handle]
    set procname [get_property NAME    $hw_proc_handle]

    set enable_sw_profile [get_property CONFIG.enable_sw_intrusive_profiling $os_handle]
    set need_config_file "false"

    # proctype should be "microblaze" or "ppc405" or "ppc405_virtex4" or "ppc440" or ps7_cortexa9
    set armsrcdir "../${standalone_version}/src/cortexa9"
    set armccdir "../${standalone_version}/src/cortexa9/armcc"
    set ccdir "../${standalone_version}/src/cortexa9/gcc"
    set commonsrcdir "../${standalone_version}/src/common"
    set mbsrcdir "../${standalone_version}/src/microblaze"
    set ppcsrcdir "../${standalone_version}/src/ppc405"
    set ppc440srcdir "../${standalone_version}/src/ppc440"

    foreach entry [glob -nocomplain [file join $commonsrcdir *]] {
        file copy -force $entry [file join ".." "${standalone_version}" "src"]
    }

    # proctype should be "ps7_cortexa9"
    switch $proctype {

    "ps7_cortexa9"  {
        foreach entry [glob -nocomplain [file join $armsrcdir *]] {
            file copy -force $entry [file join ".." "${standalone_version}" "src"]
        }

        foreach entry [glob -nocomplain [file join $ccdir *]] {
            file copy -force $entry [file join ".." "${standalone_version}" "src"]
        }

        file delete -force "../${standalone_version}/src/asm_vectors.S"
        file delete -force "../${standalone_version}/src/armcc"
        file delete -force "../${standalone_version}/src/gcc"

        set need_config_file "true"

        set file_handle [xopen_include_file "xparameters.h"]
        puts $file_handle "#include \"xparameters_ps.h\""
        puts $file_handle ""
        close $file_handle
        }

    "default" {puts "processor type $proctype not supported\n"}
    }

    # Write the Config.make file
    set makeconfig [open "../${standalone_version}/src/config.make" w]

    if { $proctype == "ps7_cortexa9" } {
        puts $makeconfig "LIBSOURCES = *.c *.s *.S"
        puts $makeconfig "LIBS = standalone_libs"
    }

    close $makeconfig

    # Remove arm directory...
    file delete -force $armsrcdir

    # copy required files to the main src directory
    file copy -force [file join src Source tasks.c] ./src
    file copy -force [file join src Source queue.c] ./src
    file copy -force [file join src Source list.c] ./src
    file copy -force [file join src Source timers.c] ./src
    file copy -force [file join src Source croutine.c] ./src
    file copy -force [file join src Source event_groups.c] ./src
    file copy -force [file join src Source portable MemMang heap_4.c] ./src
    file copy -force [file join src Source portable GCC Zynq FreeRTOS_asm_vectors.S] ./src
    file copy -force [file join src Source portable GCC Zynq FreeRTOS_tick_config.c] ./src
    file copy -force [file join src Source portable GCC Zynq FreeRTOSConfig.h] ./src
    file copy -force [file join src Source portable GCC ARM_CA9 port.c] ./src
    file copy -force [file join src Source portable GCC ARM_CA9 portASM.S] ./src
    file copy -force [file join src Source portable GCC ARM_CA9 portmacro.h] ./src

    # copy all the headers
    set headers [glob -join ./src/Source/include *.\[h\]]
    foreach header $headers {
        file copy -force $header src
    }

    file delete -force [file join src Source]
    file delete -force [file join src Source]

    # Remove microblaze, ppc405, ppc440, cortexa9 and common directories...
    file delete -force $mbsrcdir
    file delete -force $ppcsrcdir
    file delete -force $ppc440srcdir
    file delete -force $armsrcdir
    file delete -force $ccdir
    file delete -force $commonsrcdir
    file delete -force $armccdir

    # Handle stdin and stdout
    handle_stdin $os_handle
    handle_stdout $os_handle

    set config_file [open "./src/FreeRTOSConfig.h" w]
    puts $config_file "\#ifndef FREERTOS_CONFIG_H"
    puts $config_file "\#define FREERTOS_CONFIG_H"
    puts $config_file ""

    puts $config_file "\#include \"xparameters.h\""
    puts $config_file ""

    puts $config_file "/*
* The FreeRTOS Cortex-A port implements a full interrupt nesting model.
*
* Interrupts that are assigned a priority at or below
* configMAX_API_CALL_INTERRUPT_PRIORITY (which counter-intuitively in the ARM
* generic interrupt controller \[GIC\] means a priority that has a numerical
* value above configMAX_API_CALL_INTERRUPT_PRIORITY) can call FreeRTOS safe API
* functions and will nest.
*
* Interrupts that are assigned a priority above
* configMAX_API_CALL_INTERRUPT_PRIORITY (which in the GIC means a numerical
* value below configMAX_API_CALL_INTERRUPT_PRIORITY) cannot call any FreeRTOS
* API functions, will nest, and will not be masked by FreeRTOS critical
* sections (although it is necessary for interrupts to be globally disabled
* extremely briefly as the interrupt mask is updated in the GIC).
*
* FreeRTOS functions that can be called from an interrupt are those that end in
* \"FromISR\".  FreeRTOS maintains a separate interrupt safe API to enable
* interrupt entry to be shorter, faster, simpler and smaller.
*
* The Zynq implements 256 unique interrupt priorities.  For the purpose of
* setting configMAX_API_CALL_INTERRUPT_PRIORITY 255 represents the lowest
* priority.
*/"

    set max_api_call_interrupt_priority [get_property CONFIG.max_api_call_interrupt_priority $os_handle]
    puts $config_file "\#define configMAX_API_CALL_INTERRUPT_PRIORITY $max_api_call_interrupt_priority \n"

    set cpu_clock_hz [get_property CONFIG.cpu_clock_hz $os_handle]
    puts $config_file "\#define configCPU_CLOCK_HZ ${cpu_clock_hz}UL"

    set use_port_optimised_task_selection [expr [get_property CONFIG.use_port_optimised_task_selection $os_handle] == true]
    puts $config_file "\#define configUSE_PORT_OPTIMISED_TASK_SELECTION $use_port_optimised_task_selection"

    set use_tickless_idle [expr [get_property CONFIG.use_tickless_idle $os_handle] == true]
    puts $config_file "\#define configUSE_TICKLESS_IDLE $use_tickless_idle"

    set tick_rate_hz [get_property CONFIG.tick_rate_hz $os_handle]
    puts $config_file "\#define configTICK_RATE_HZ ( (TickType_t) $tick_rate_hz )"

    set peripheral_clock_hz [get_property CONFIG.peripheral_clock_hz $os_handle]
    puts $config_file "\#define configPERIPHERAL_CLOCK_HZ ${peripheral_clock_hz}UL"

    set use_preemption [expr [get_property CONFIG.use_preemption $os_handle] == true]
    puts $config_file "\#define configUSE_PREEMPTION $use_preemption"

    set use_idle_hook [expr [get_property CONFIG.use_idle_hook $os_handle] == true]
    set use_tick_hook [expr [get_property CONFIG.use_tick_hook $os_handle] == true]
    puts $config_file "\#define configUSE_IDLE_HOOK $use_idle_hook"
    puts $config_file "\#define configUSE_TICK_HOOK $use_tick_hook"

    set max_priorities [get_property CONFIG.max_priorities $os_handle]
    puts $config_file "\#define configMAX_PRIORITIES ( $max_priorities )"

    set minimal_stack_size [get_property CONFIG.minimal_stack_size $os_handle]
    puts $config_file "\#define configMINIMAL_STACK_SIZE ( (unsigned short) $minimal_stack_size )"

    set total_heap_size [get_property CONFIG.total_heap_size  $os_handle]
    puts $config_file "\#define configTOTAL_HEAP_SIZE ( $total_heap_size * 1024 )"

    set max_task_name_len [get_property CONFIG.max_task_name_len  $os_handle]
    puts $config_file "\#define configMAX_TASK_NAME_LEN ( $max_task_name_len )"

    set use_trace_facility [expr [get_property CONFIG.use_trace_facility $os_handle] == true]
    puts $config_file "\#define configUSE_TRACE_FACILITY $use_trace_facility"

    set use_16_bit_ticks [expr [get_property CONFIG.use_16_bit_ticks $os_handle] == true]
    puts $config_file "\#define configUSE_16_BIT_TICKS $use_16_bit_ticks"

    set idle_should_yield [expr [get_property CONFIG.idle_should_yield  $os_handle] == true]
    puts $config_file "\#define configIDLE_SHOULD_YIELD $idle_should_yield"
    puts $config_file ""

    # kernel features
    set use_mutexes [expr [get_property CONFIG.use_mutexes $os_handle] == true]
    puts $config_file "\#define configUSE_MUTEXES $use_mutexes"

    set queue_registry_size [get_property CONFIG.queue_registry_size $os_handle]
    puts $config_file "\#define configQUEUE_REGISTRY_SIZE $queue_registry_size"

    set check_for_stack_overflow [get_property CONFIG.check_for_stack_overflow $os_handle]
    puts $config_file "\#define configCHECK_FOR_STACK_OVERFLOW $check_for_stack_overflow"

    set use_recursive_mutexes [expr [get_property CONFIG.use_recursive_mutexes $os_handle] == true]
    puts $config_file "\#define configUSE_RECURSIVE_MUTEXES $use_recursive_mutexes"

    set use_stats_formatting_functions [expr [get_property CONFIG.use_stats_formatting_functions $os_handle] == true]
    puts $config_file "\#define configUSE_STATS_FORMATTING_FUNCTIONS $use_stats_formatting_functions"
    puts $config_file ""

    # hooks
    set use_malloc_failed_hook [expr [get_property CONFIG.use_malloc_failed_hook $os_handle] == true]
    puts $config_file "\#define configUSE_MALLOC_FAILED_HOOK $use_malloc_failed_hook"

    set use_application_task_tag [expr [get_property CONFIG.use_application_task_tag $os_handle] == true]
    puts $config_file "\#define configUSE_APPLICATION_TASK_TAG $use_application_task_tag"

    set use_counting_semaphores [expr [get_property CONFIG.use_counting_semaphores $os_handle] == true]
    puts $config_file "\#define configUSE_COUNTING_SEMAPHORES $use_counting_semaphores"

    set use_queue_sets [expr [get_property CONFIG.use_queue_sets $os_handle] == true]
    puts $config_file "\#define configUSE_QUEUE_SETS $use_queue_sets"
    puts $config_file ""

    # co-routine
    set use_co_routines [expr [get_property CONFIG.use_co_routines $os_handle] == true]
    puts $config_file "\#define configUSE_CO_ROUTINES $use_co_routines"

    set max_co_routine_priorities [get_property CONFIG.max_co_routine_priorities $os_handle]
    puts $config_file "\#define configMAX_CO_ROUTINE_PRIORITIES ( $max_co_routine_priorities )"
    puts $config_file ""

    # timers
    set use_timers [expr [get_property CONFIG.use_timers $os_handle] == true]
    puts $config_file "\#define configUSE_TIMERS $use_timers"

    set timer_task_priority [get_property CONFIG.timer_task_priority $os_handle]
    set timer_queue_length [get_property CONFIG.timer_queue_length $os_handle]
    set timer_task_stack_depth [get_property CONFIG.timer_task_stack_depth $os_handle]
    puts $config_file "\#define configTIMER_TASK_PRIORITY $timer_task_priority"
    puts $config_file "\#define configTIMER_QUEUE_LENGTH $timer_queue_length"
    puts $config_file "\#define configTIMER_TASK_STACK_DEPTH $timer_task_stack_depth"
    puts $config_file ""

    set generate_run_time_stats [expr [get_property CONFIG.generate_run_time_stats $os_handle] == true]
    if {$generate_run_time_stats == 1} {
        puts $config_file "\#define configGENERATE_RUN_TIME_STATS $generate_run_time_stats"
        puts $config_file ""
        puts $config_file "/* use watchdog in timer mode */"
        puts $config_file "\#include \"xscuwdt.h\""
        puts $config_file "extern XScuWdt xWatchDogInstance;"
        puts $config_file "extern void vInitialiseTimerForRunTimeStats( void );"
        puts $config_file ""
        puts $config_file "\#define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS() vInitialiseTimerForRunTimeStats()"
        puts $config_file "\#define portGET_RUN_TIME_COUNTER_VALUE() ( ( 0xffffffffUL - XScuWdt_ReadReg( xWatchDogInstance.Config.BaseAddr, XSCUWDT_COUNTER_OFFSET ) ) >> 1 )"
        puts $config_file ""
    }

    set command_int_max_output_size [get_property CONFIG.command_int_max_output_size $os_handle]
    puts $config_file "\#define configCOMMAND_INT_MAX_OUTPUT_SIZE $command_int_max_output_size"
    puts $config_file ""

    # Normal assert() semantics without relying on the provision of an assert.h
    # header file.
    set config_assert [expr [get_property CONFIG.config_assert $os_handle] == true]
    if {$config_assert == 1} {
        puts $config_file "void vAssertCalled( const char * pcFile, unsigned long ulLine );"
        puts $config_file "\#define configASSERT( x ) if( ( x ) == 0 ) vAssertCalled( __FILE__, __LINE__ );"
    }

    # If configTASK_RETURN_ADDRESS is not defined then a task that attempts to
    # return from its implementing function will end up in a "task exit error"
    # function - which contains a call to configASSERT().  However this can give GCC
    # some problems when it tries to unwind the stack, as the exit error function has
    # nothing to return to.  To avoid this define configTASK_RETURN_ADDRESS to 0.
    puts $config_file "\#define configTASK_RETURN_ADDRESS NULL"
    puts $config_file ""

    # API functions
    puts $config_file "\#define INCLUDE_vTaskPrioritySet         1"
    puts $config_file "\#define INCLUDE_uxTaskPriorityGet        1"
    puts $config_file "\#define INCLUDE_vTaskDelete              1"
    puts $config_file "\#define INCLUDE_vTaskCleanUpResources    1"
    puts $config_file "\#define INCLUDE_vTaskSuspend             1"
    puts $config_file "\#define INCLUDE_vTaskDelayUntil          1"
    puts $config_file "\#define INCLUDE_vTaskDelay               1"
    puts $config_file "\#define INCLUDE_xTimerPendFunctionCall   1"
    puts $config_file "\#define INCLUDE_eTaskGetState            1"
    puts $config_file ""

    # hardware options"
    puts $config_file "/* Hardware specific settings */"
    puts $config_file "void vConfigureTickInterrupt( void );"
    puts $config_file "\#define configSETUP_TICK_INTERRUPT() vConfigureTickInterrupt()"
    puts $config_file "void vClearTickInterrupt( void );"
    puts $config_file "\#define configCLEAR_TICK_INTERRUPT() vClearTickInterrupt()"
    puts $config_file ""

    set interrupt_controller_base_address [get_property CONFIG.interrupt_controller_base_address $os_handle]
    set interrupt_controller_cpu_interface_offset [get_property CONFIG.interrupt_controller_cpu_interface_offset $os_handle]
    set unique_interrupt_priorities [get_property CONFIG.unique_interrupt_priorities $os_handle]
    puts $config_file "\#define configINTERRUPT_CONTROLLER_BASE_ADDRESS $interrupt_controller_base_address"
    puts $config_file "\#define configINTERRUPT_CONTROLLER_CPU_INTERFACE_OFFSET $interrupt_controller_cpu_interface_offset"
    puts $config_file "\#define configUNIQUE_INTERRUPT_PRIORITIES $unique_interrupt_priorities"
    puts $config_file ""

    # network options
    puts $config_file "/* Network configuration settings - only used when the lwIP example is built. */"

    set mac_input_task_priority [get_property CONFIG.mac_input_task_priority $os_handle]
    set lwip_task_priority [get_property CONFIG.lwip_task_priority $os_handle]
    set cli_task_priority [get_property CONFIG.cli_task_priority $os_handle]
    puts $config_file "\#define configMAC_INPUT_TASK_PRIORITY $mac_input_task_priority"
    puts $config_file "\#define configLWIP_TASK_PRIORITY $lwip_task_priority"
    puts $config_file "\#define configCLI_TASK_PRIORITY $cli_task_priority"
    puts $config_file ""

    # implement a typical 7-level priority system
    puts $config_file "/* Implement a typical 7-level priority systems */"
    puts $config_file "\#define REALTIME_PRIORITY (6)"
    puts $config_file "\#define HIGHEST_PRIORITY  (5)"
    puts $config_file "\#define HIGH_PRIORITY     (4)"
    puts $config_file "\#define NORMAL_PRIORITY   (3)"
    puts $config_file "\#define LOW_PRIORITY      (2)"
    puts $config_file "\#define LOWEST_PRIORITY   (1)"
    puts $config_file "\#define IDLE_PRIORITY     (0)"
    puts $config_file ""

    set mac_addr0 [get_property CONFIG.mac_addr0 $os_handle]
    set mac_addr1 [get_property CONFIG.mac_addr1 $os_handle]
    set mac_addr2 [get_property CONFIG.mac_addr2 $os_handle]
    set mac_addr3 [get_property CONFIG.mac_addr3 $os_handle]
    set mac_addr4 [get_property CONFIG.mac_addr4 $os_handle]
    set mac_addr5 [get_property CONFIG.mac_addr5 $os_handle]
    puts $config_file "\#define configMAC_ADDR0 $mac_addr0"
    puts $config_file "\#define configMAC_ADDR1 $mac_addr1"
    puts $config_file "\#define configMAC_ADDR2 $mac_addr2"
    puts $config_file "\#define configMAC_ADDR3 $mac_addr3"
    puts $config_file "\#define configMAC_ADDR4 $mac_addr4"
    puts $config_file "\#define configMAC_ADDR5 $mac_addr5"
    puts $config_file ""

    set ip_addr0 [get_property CONFIG.ip_addr0 $os_handle]
    set ip_addr1 [get_property CONFIG.ip_addr1 $os_handle]
    set ip_addr2 [get_property CONFIG.ip_addr2 $os_handle]
    set ip_addr3 [get_property CONFIG.ip_addr3 $os_handle]
    puts $config_file "\#define configIP_ADDR0 $ip_addr0"
    puts $config_file "\#define configIP_ADDR1 $ip_addr1"
    puts $config_file "\#define configIP_ADDR2 $ip_addr2"
    puts $config_file "\#define configIP_ADDR3 $ip_addr3"
    puts $config_file ""

    set net_mask0 [get_property CONFIG.net_mask0 $os_handle]
    set net_mask1 [get_property CONFIG.net_mask1 $os_handle]
    set net_mask2 [get_property CONFIG.net_mask2 $os_handle]
    set net_mask3 [get_property CONFIG.net_mask3 $os_handle]
    puts $config_file "\#define configNET_MASK0 $net_mask0"
    puts $config_file "\#define configNET_MASK1 $net_mask1"
    puts $config_file "\#define configNET_MASK2 $net_mask2"
    puts $config_file "\#define configNET_MASK3 $net_mask3"
    puts $config_file ""

    # complete the header protectors
    puts $config_file "\#endif"
    close $config_file
}

proc xopen_new_include_file { filename description } {
    set inc_file [open $filename w]
    xprint_generated_header $inc_file $description
    set newfname [string map {. _} [lindex [split $filename {\/}] end]]
    puts $inc_file "\#ifndef _[string toupper $newfname]"
    puts $inc_file "\#define _[string toupper $newfname]\n\n"
    return $inc_file
}

proc xput_define { config_file parameter param_value } {
    puts $config_file "#define $parameter $param_value\n"
}
