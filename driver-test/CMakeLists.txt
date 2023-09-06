# cmake -DCMAKE_TOOLCHAIN_FILE=../Toolchain_ticomp.cmake ..
# in order to run this, use the following command on the command line: 
# cmake -DCMAKE_TOOLCHAIN_FILE=../Toolchain_ticomp.cmake ..

cmake_minimum_required(VERSION 3.10)

set (CMAKE_CONFIGURATION_TYPES "Debug;Release")


project(ENC28J60-DRIVER)
# Example ENC28J60 application
set(ENC28J60_DIR "/Users/sramnath/WirelessMicPrototyping/enc28j60-driver/driver-test")
set(ENC28J60_DRIVER_DIR "/Users/sramnath/WirelessMicPrototyping/enc28j60-driver/driver-files")

set (ENC28J60_DEFINITIONS ENC28J60_DEBUG=1)

# set(CMAKE_VERBOSE_MAKEFILE ON)

# Generate docs before creating source package
# include(${ENC28J60_DIR}/contrib/ports/CMakeCommon.cmake)


set(TIRTOS_XDC_RT "/Applications/ti_ccs9/xdctools_3_32_00_06_core")
set(TIRTOS_BIOS_RT "/Applications/ti_ccs9/tirtos_cc13xx_cc26xx_2_21_01_08/products/bios_6_46_01_38/packages/ti/sysbios")
set(TIRTOS_BIOS_INC "/Applications/ti_ccs9/tirtos_cc13xx_cc26xx_2_21_01_08/products/bios_6_46_01_38/packages")
set(TIRTOS_TARGET_INC "/Applications/ti_ccs9/tirtos_cc13xx_cc26xx_2_21_01_08/products/bios_6_46_01_38/packages") 
set(TIRTOS_SDK "/Applications/ti_ccs9/simplelink_cc13x2_26x2_sdk_3_20_00_68")

set(CMAKE_COMPILER_PATH "/Applications/ti_ccs9/ccs9/ccs/tools/compiler/ti-cgt-arm_18.12.2.LTS")

set(ENC28J60_TIRTOS_SRCS
	${TIRTOS_XDC_RT}/packages
	${TIRTOS_BIOS_RT}
	${TIRTOS_BIOS_INC}
	${TIRTOS_BIOS_INC}/gnu/targets
	${TIRTOS_TARGET_INC}
	${TIRTOS_COMPILER}
	${TIRTOS_SDK}/source
	${TIRTOS_SDK}/source/ti/posix/ccs
	${TIRTOS_SDK}/source/ti/display
)

set (ENC28J60_INCLUDE_DIRS
    "${ENC28J60_DIR}"
    "${ENC28J60_DRIVER_DIR}"
    "${ENC28J60_TIRTOS_SRCS}"		
)


set (ENC28J60_COMPILE_OPTIONS
        "-Dxdc_target_types__=${TIRTOS_BIOS_INC}/ti/targets/arm/std.h"
        "-Dxdc_target_name__=C64P"
	"-DDeviceFamily_CC13X2"
)


set (ENC28J60_CCOPTS
 	"-mv7M4 --code_state=16 --float_support=fpv4spd16 --endian=little --abi=eabi -q -ms --opt_for_speed=0 --program_level_compile -O0  --cmd_file=/Users/sramnath/ccs_workspaces/sdk_3_2/spiEthernet/tirtos_builds_CC1352P1_LAUNCHXL_release_ccs/Debug/configPkg/compiler.opt"  )


#set (ENC28J60_CCOPTS
#	"-mv7M4 --code_state=16 --float_support=fpv4spd16 --endian=little -me  -g --optimize_with_debug --cmd_file=/Users/sramnath/ccs_workspaces/ccs9_ittiam_audio/tirtos_builds_CC1352P1_LAUNCHXL_release_ccs/Debug/configPkg/compiler.opt"  
#)

set (ENC28J60_BIOS_OPTS
	"-Dti_sysbios_knl_Task_minimizeLatency__D=FALSE" 
	"-Dti_sysbios_family_arm_cc26xx_Boot_driverlibVersion=2" 
	"-Dti_sysbios_knl_Clock_stopCheckNext__D=TRUE" 
	"-Dti_sysbios_family_arm_m3_Hwi_enableException__D=TRUE" 
	"-Dti_sysbios_family_arm_m3_Hwi_disablePriority__D=32U" 	
	"-Dti_sysbios_family_arm_m3_Hwi_numSparseInterrupts__D=0U"
	"-Dti_sysbios_BIOS_swiEnabled__D=TRUE"
	"-Dti_sysbios_BIOS_taskEnabled__D=TRUE"
	"-Dti_sysbios_BIOS_clockEnabled__D=TRUE"
	"-Dti_sysbios_BIOS_runtimeCreatesEnabled__D=TRUE"
	"-Dti_sysbios_hal_Hwi_DISABLE_ALL_HOOKS"
	"-Dti_sysbios_knl_Swi_DISABLE_ALL_HOOKS"
	"-Dti_sysbios_BIOS_smpEnabled__D=FALSE"
	"-Dti_sysbios_Build_useHwiMacros"
	"-Dti_sysbios_knl_Swi_numPriorities__D=6"
	"-Dti_sysbios_knl_Task_deleteTerminatedTasks__D=FALSE"
	"-Dti_sysbios_knl_Task_numPriorities__D=16"
	"-Dti_sysbios_knl_Task_checkStackFlag__D=FALSE"
	"-Dti_sysbios_knl_Task_initStackFlag__D=TRUE"
	"-Dti_sysbios_knl_Task_DISABLE_ALL_HOOKS"
	"-Dti_sysbios_knl_Clock_TICK_SOURCE=ti_sysbios_knl_Clock_TickSource_TIMER"
	"-Dti_sysbios_knl_Clock_TICK_MODE=ti_sysbios_knl_Clock_TickMode_DYNAMIC"
	"-Dti_sysbios_hal_Core_delegate_getId=ti_sysbios_hal_CoreNull_getId__E"
	"-Dti_sysbios_hal_Core_delegate_interruptCore=ti_sysbios_hal_CoreNull_interruptCore__E"
	"-Dti_sysbios_hal_Core_delegate_lock=ti_sysbios_hal_CoreNull_lock__E"
	"-Dti_sysbios_hal_Core_delegate_unlock=ti_sysbios_hal_CoreNull_unlock__E"
	"-Dti_sysbios_hal_Core_numCores__D=1"
	"-Dti_sysbios_hal_CoreNull_numCores__D=1"
	"-Dti_sysbios_utils_Load_taskEnabled__D=TRUE"
	"-Dti_sysbios_utils_Load_swiEnabled__D=FALSE"
	"-Dti_sysbios_utils_Load_hwiEnabled__D=FALSE"
	"-Dti_sysbios_family_arm_m3_Hwi_dispatcherSwiSupport__D=TRUE"
	"-Dti_sysbios_family_arm_m3_Hwi_dispatcherTaskSupport__D=TRUE"
	"-Dti_sysbios_family_arm_m3_Hwi_dispatcherAutoNestingSupport__D=TRUE"
	"-Dti_sysbios_knl_Semaphore_supportsEvents__D=FALSE"
	"-Dti_sysbios_knl_Semaphore_supportsPriority__D=FALSE"
	"-Dxdc_runtime_Assert_DISABLE_ALL"
	"-Dxdc_runtime_Log_DISABLE_ALL"
)

set(cc1352p1_board_SRCS
#        ${ENC28J60_DIR}/CC1352P1_LAUNCHXL_TIRTOS.cmd
        ${ENC28J60_DIR}/CC1352P1_LAUNCHXL_fxns.c
        ${ENC28J60_DIR}/CC1352P1_LAUNCHXL.c
	${ENC28J60_DRIVER_DIR}/enc_ethernet.c
	${ENC28J60_DRIVER_DIR}/spimaster.c
        ${ENC28J60_DIR}/ccfg.c
)

set( cc1352p1_libs
	${TIRTOS_SDK}/source/ti/display/lib/display.aem4f
	${TIRTOS_SDK}/source/ti/grlib/lib/ccs/m4f/grlib.a
	${TIRTOS_SDK}/source/third_party/spiffs/lib/ccs/m4f/spiffs.a
	${TIRTOS_SDK}/source/ti/drivers/rf/lib/rf_multiMode_cc13x2.aem4f
	${TIRTOS_SDK}/source/ti/drivers/lib/drivers_cc13x2.aem4f
	${TIRTOS_SDK}/kernel/tirtos/packages/ti/dpl/lib/dpl_cc13x2.aem4f
	${TIRTOS_SDK}/source/ti/devices/cc13x2_cc26x2/driverlib/bin/ccs/driverlib.lib
	${TIRTOS_SDK}/source/third_party/fatfs/lib/ccs/m4/fatfs.a
	/Applications/ti_ccs9/ccs9/ccs/tools/compiler/ti-cgt-arm_18.12.2.LTS/lib/rtsv7M4_T_le_v4SPD16_eabi.lib
	${ENC28J60_DIR}/linker.cmd
)

set( cc1352p1_lib_dirs
	${TIRTOS_SDK}/source/ti/display/lib
	${TIRTOS_SDK}/source/ti/grlib/lib/ccs/m4f
	${TIRTOS_SDK}/source/third_party/spiffs/lib/ccs/m4f
	${TIRTOS_SDK}/source/ti/drivers/rf/lib
	${TIRTOS_SDK}/source/ti/drivers/lib
	${TIRTOS_SDK}/kernel/tirtos/packages/ti/dpl/lib
	${TIRTOS_SDK}/source/ti/devices/cc13x2_cc26x2/driverlib/bin/ccs
	${TIRTOS_SDK}/source/third_party/fatfs/lib/ccs/m4
	/Applications/ti_ccs9/ccs9/ccs/tools/compiler/ti-cgt-arm_18.12.2.LTS/lib
)

# Compiler options

set(CMAKE_C_FLAGS " ${ENC28J60_CCOPTS} -I ${CMAKE_COMPILER_PATH}/include -I ${TIRTOS_SDK}/kernel/tirtos/packages" CACHE STRING "" FORCE)

add_library(enc28j60board EXCLUDE_FROM_ALL ${cc1352p1_board_SRCS})
target_include_directories(enc28j60board PRIVATE ${ENC28J60_INCLUDE_DIRS})
target_compile_options(enc28j60board PRIVATE ${ENC28J60_COMPILER_FLAGS} ${ENC28J60_COMPILER_OPTIONS})


set(LINKER_SCRIPT "${ENC28J60_DIR}/CC1352P1_LAUNCHXL_TIRTOS.cmd")
add_executable(test.out ${ENC28J60_DIR}/main_tirtos.c )
target_include_directories(test.out PRIVATE ${ENC28J60_INCLUDE_DIRS})
target_compile_options(test.out PRIVATE ${ENC28J60_COMPILER_FLAGS} ${ENC28J60_COMPILER_OPTIONS})

add_link_options(test.out ../CC1352P1_LAUNCHXL_TIRTOS.cmd) 
target_link_libraries(test.out  enc28j60board  ${CMAKE_COMPILER_PATH}/lib/libc.a ${cc1352p1_libs})
add_definitions( ${ENC28J60_COMPILE_OPTIONS}) 

# Run this command because linking will fail with this cmake file - but use the compiled binaries produced by this cmake file 

# "/Applications/ti_ccs9/ccs9/ccs/tools/compiler/ti-cgt-arm_18.12.2.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --define=DeviceFamily_CC13X2 -g --diag_warning=225 --diag_warning=255 --diag_wrap=off --display_error_number --gen_func_subsections=on -z -m"gpiointerrupt_CC1352P1_LAUNCHXL_tirtos_ccs.map" -i"/Applications/ti_ccs9/simplelink_cc13x2_26x2_sdk_3_20_00_68/source" -i"/Applications/ti_ccs9/simplelink_cc13x2_26x2_sdk_3_20_00_68/kernel/tirtos/packages" -i"/Applications/ti_ccs9/ccs9/ccs/tools/compiler/ti-cgt-arm_18.12.2.LTS/lib" --diag_wrap=off --display_error_number --warn_sections --xml_link_info="gpiointerrupt_CC1352P1_LAUNCHXL_tirtos_ccs_linkInfo.xml" --rom_model --output_file="spimaster_CC1352P1_LAUNCHXL_tirtos_ccs.out" "CMakeFiles/test.out.dir/CC1352P1_LAUNCHXL.c.obj" "CMakeFiles/test.out.dir/CC1352P1_LAUNCHXL_fxns.c.obj" "CMakeFiles/test.out.dir/ccfg.c.obj" "CMakeFiles/test.out.dir/gpiointerrupt.c.obj" "CMakeFiles/test.out.dir/enc_ethernet.c.obj" "CMakeFiles/test.out.dir/app_ethernetif.c.obj" "CMakeFiles/test.out.dir/spimaster.c.obj" "CMakeFiles/test.out.dir/main_tirtos.c.obj" "../CC1352P1_LAUNCHXL_TIRTOS.cmd" -l"/Applications/ti_ccs9/simplelink_cc13x2_26x2_sdk_3_20_00_68/source/ti/grlib/lib/ccs/m4f/grlib.a" -llibenc28j60board.a -l"ti/display/lib/display.aem4f" -l"/Applications/ti_ccs9/simplelink_cc13x2_26x2_sdk_3_20_00_68/source/third_party/spiffs/lib/ccs/m4f/spiffs_cc26xx.a" -l"/Applications/ti_ccs9/simplelink_cc13x2_26x2_sdk_3_20_00_68/source/ti/drivers/rf/lib/rf_multiMode_cc13x2.aem4f" -l"/Applications/ti_ccs9/simplelink_cc13x2_26x2_sdk_3_20_00_68/source/ti/drivers/lib/drivers_cc13x2.aem4f" -l"/Applications/ti_ccs9/simplelink_cc13x2_26x2_sdk_3_20_00_68/kernel/tirtos/packages/ti/dpl/lib/dpl_cc13x2.aem4f" -l"../linker.cmd" -l"/Applications/ti_ccs9/simplelink_cc13x2_26x2_sdk_3_20_00_68/source/ti/devices/cc13x2_cc26x2/driverlib/bin/ccs/driverlib.lib" -llibc.a  -llibenc28j60core.a 

# Another command that also works is the following, and this actually uses the library that I created instead of manually linking object files
# "/Applications/ti_ccs9/ccs9/ccs/tools/compiler/ti-cgt-arm_18.12.2.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --define=DeviceFamily_CC13X2 -g --diag_warning=225 --diag_warning=255 --diag_wrap=off --display_error_number --gen_func_subsections=on -z -m"gpiointerrupt_CC1352P1_LAUNCHXL_tirtos_ccs.map" -i"/Applications/ti_ccs9/simplelink_cc13x2_26x2_sdk_3_20_00_68/source" -i"/Applications/ti_ccs9/simplelink_cc13x2_26x2_sdk_3_20_00_68/kernel/tirtos/packages" -i"/Applications/ti_ccs9/ccs9/ccs/tools/compiler/ti-cgt-arm_18.12.2.LTS/lib" --diag_wrap=off --display_error_number --warn_sections --xml_link_info="gpiointerrupt_CC1352P1_LAUNCHXL_tirtos_ccs_linkInfo.xml" --rom_model --output_file="spimaster_CC1352P1_LAUNCHXL_tirtos_ccs.out"  "CMakeFiles/test.out.dir/main_tirtos.c.obj" "../CC1352P1_LAUNCHXL_TIRTOS.cmd" -l"/Applications/ti_ccs9/simplelink_cc13x2_26x2_sdk_3_20_00_68/source/ti/grlib/lib/ccs/m4f/grlib.a" -llibenc28j60board.a -l"ti/display/lib/display.aem4f" -l"/Applications/ti_ccs9/simplelink_cc13x2_26x2_sdk_3_20_00_68/source/third_party/spiffs/lib/ccs/m4f/spiffs_cc26xx.a" -l"/Applications/ti_ccs9/simplelink_cc13x2_26x2_sdk_3_20_00_68/source/ti/drivers/rf/lib/rf_multiMode_cc13x2.aem4f" -l"/Applications/ti_ccs9/simplelink_cc13x2_26x2_sdk_3_20_00_68/source/ti/drivers/lib/drivers_cc13x2.aem4f" -l"/Applications/ti_ccs9/simplelink_cc13x2_26x2_sdk_3_20_00_68/kernel/tirtos/packages/ti/dpl/lib/dpl_cc13x2.aem4f" -l"../linker.cmd" -l"/Applications/ti_ccs9/simplelink_cc13x2_26x2_sdk_3_20_00_68/source/ti/devices/cc13x2_cc26x2/driverlib/bin/ccs/driverlib.lib" -llibc.a  -llibenc28j60core.a

# "/Applications/ti_ccs9/ccs9/ccs/tools/compiler/ti-cgt-arm_18.12.2.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --define=DeviceFamily_CC13X2 -g --diag_warning=225 --diag_warning=255 --diag_wrap=off --display_error_number --gen_func_subsections=on -z -m"enc28j60threads_CC1352P1_LAUNCHXL_tirtos_ccs.map" -i"/Applications/ti_ccs9/simplelink_cc13x2_26x2_sdk_3_20_00_68/source" -i"/Applications/ti_ccs9/simplelink_cc13x2_26x2_sdk_3_20_00_68/kernel/tirtos/packages" -i"/Applications/ti_ccs9/ccs9/ccs/tools/compiler/ti-cgt-arm_18.12.2.LTS/lib" --diag_wrap=off --display_error_number --warn_sections --xml_link_info="enc28j60threads_CC1352P1_LAUNCHXL_tirtos_ccs_linkInfo.xml" --rom_model --output_file="enc28j60threads_CC1352P1_LAUNCHXL_tirtos_ccs.out"  "CMakeFiles/test.out.dir/main_tirtos.c.obj" "../CC1352P1_LAUNCHXL_TIRTOS.cmd" -l"/Applications/ti_ccs9/simplelink_cc13x2_26x2_sdk_3_20_00_68/source/ti/grlib/lib/ccs/m4f/grlib.a"  -llibenc28j60board.a  -l"ti/display/lib/display.aem4f" -l"/Applications/ti_ccs9/simplelink_cc13x2_26x2_sdk_3_20_00_68/source/third_party/spiffs/lib/ccs/m4f/spiffs_cc26xx.a" -l"/Applications/ti_ccs9/simplelink_cc13x2_26x2_sdk_3_20_00_68/source/ti/drivers/rf/lib/rf_multiMode_cc13x2.aem4f" -l"/Applications/ti_ccs9/simplelink_cc13x2_26x2_sdk_3_20_00_68/source/ti/drivers/lib/drivers_cc13x2.aem4f" -l"/Applications/ti_ccs9/simplelink_cc13x2_26x2_sdk_3_20_00_68/kernel/tirtos/packages/ti/dpl/lib/dpl_cc13x2.aem4f" -l"../linker.cmd" -l"/Applications/ti_ccs9/simplelink_cc13x2_26x2_sdk_3_20_00_68/source/ti/devices/cc13x2_cc26x2/driverlib/bin/ccs/driverlib.lib" -llibc.a 


# That will produce the spimaster_CC1352P1_LAUNCHXL_tirtos_ccs.out which actually works. 
