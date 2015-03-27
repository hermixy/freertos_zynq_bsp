#

proc swapp_get_name {} {
    return "FreeRTOS gpio !";
}

proc swapp_get_description {} {
    return "This demo can use both AXI GPIO or PS GPIO to control LED (output) or Buttons (input) ! It is based on the 'hello, world !' demo we had create before and also uses a software timer. The driver are in the 'freertos_gpio.c' and it has support on both AXI- or PS- GPIO, but if you want to use AXI GPIO, you should re-create the block design. PS GPIO is the default, LED is at pin (47) while button is at pin (51), modify the settings the way you like.";
}

proc check_os {} {
    set oslist [get_os];

    if { [llength $oslist] != 1 } {
        return 0;
    }
    set os [lindex $oslist 0];

    if { $os != "freertos_zynq" } {
        error "This application is supported only on FreeRTOS based Board Support Packages.";
    }
}

proc get_stdout {} {
    set os [get_os]
    if { $os == "" } {
        error "No Operating System specified in the Board Support Package.";
    }
    set stdout [get_property CONFIG.STDOUT $os];
    return $stdout;
}

proc check_stdout_hw {} {
}

proc check_stdout_sw {} {
}

proc swapp_is_supported_hw {} {
    return 1;
}

proc swapp_is_supported_sw {} {
    return 1;
}

proc swapp_generate {} {

}

