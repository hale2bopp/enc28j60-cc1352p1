# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.15

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /usr/local/Cellar/cmake/3.15.5/bin/cmake

# The command to remove a file.
RM = /usr/local/Cellar/cmake/3.15.5/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/sramnath/WirelessMicPrototyping/enc28j60-driver/driver-test

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/sramnath/WirelessMicPrototyping/enc28j60-driver/driver-test/build

# Include any dependencies generated for this target.
include CMakeFiles/test.out.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/test.out.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/test.out.dir/flags.make

CMakeFiles/test.out.dir/main_tirtos.c.obj: CMakeFiles/test.out.dir/flags.make
CMakeFiles/test.out.dir/main_tirtos.c.obj: ../main_tirtos.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/sramnath/WirelessMicPrototyping/enc28j60-driver/driver-test/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/test.out.dir/main_tirtos.c.obj"
	/Applications/ti_ccs9/ccs9/ccs/tools/compiler/ti-cgt-arm_18.12.2.LTS/bin/armcl --compile_only --c_file=/Users/sramnath/WirelessMicPrototyping/enc28j60-driver/driver-test/main_tirtos.c $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) --output_file=CMakeFiles/test.out.dir/main_tirtos.c.obj

CMakeFiles/test.out.dir/main_tirtos.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/test.out.dir/main_tirtos.c.i"
	/Applications/ti_ccs9/ccs9/ccs/tools/compiler/ti-cgt-arm_18.12.2.LTS/bin/armcl --preproc_only --c_file=/Users/sramnath/WirelessMicPrototyping/enc28j60-driver/driver-test/main_tirtos.c $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) --output_file=CMakeFiles/test.out.dir/main_tirtos.c.i

CMakeFiles/test.out.dir/main_tirtos.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/test.out.dir/main_tirtos.c.s"
	/Applications/ti_ccs9/ccs9/ccs/tools/compiler/ti-cgt-arm_18.12.2.LTS/bin/armcl --compile_only --skip_assembler --c_file=/Users/sramnath/WirelessMicPrototyping/enc28j60-driver/driver-test/main_tirtos.c $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) --output_file=CMakeFiles/test.out.dir/main_tirtos.c.s

# Object files for target test.out
test_out_OBJECTS = \
"CMakeFiles/test.out.dir/main_tirtos.c.obj"

# External object files for target test.out
test_out_EXTERNAL_OBJECTS =

test.out: CMakeFiles/test.out.dir/main_tirtos.c.obj
test.out: CMakeFiles/test.out.dir/build.make
test.out: libenc28j60board.a
test.out: /Applications/ti_ccs9/ccs9/ccs/tools/compiler/ti-cgt-arm_18.12.2.LTS/lib/libc.a
test.out: /Applications/ti_ccs9/simplelink_cc13x2_26x2_sdk_3_20_00_68/source/ti/display/lib/display.aem4f
test.out: /Applications/ti_ccs9/simplelink_cc13x2_26x2_sdk_3_20_00_68/source/ti/grlib/lib/ccs/m4f/grlib.a
test.out: /Applications/ti_ccs9/simplelink_cc13x2_26x2_sdk_3_20_00_68/source/third_party/spiffs/lib/ccs/m4f/spiffs.a
test.out: /Applications/ti_ccs9/simplelink_cc13x2_26x2_sdk_3_20_00_68/source/ti/drivers/rf/lib/rf_multiMode_cc13x2.aem4f
test.out: /Applications/ti_ccs9/simplelink_cc13x2_26x2_sdk_3_20_00_68/source/ti/drivers/lib/drivers_cc13x2.aem4f
test.out: /Applications/ti_ccs9/simplelink_cc13x2_26x2_sdk_3_20_00_68/kernel/tirtos/packages/ti/dpl/lib/dpl_cc13x2.aem4f
test.out: /Applications/ti_ccs9/simplelink_cc13x2_26x2_sdk_3_20_00_68/source/ti/devices/cc13x2_cc26x2/driverlib/bin/ccs/driverlib.lib
test.out: /Applications/ti_ccs9/simplelink_cc13x2_26x2_sdk_3_20_00_68/source/third_party/fatfs/lib/ccs/m4/fatfs.a
test.out: /Applications/ti_ccs9/ccs9/ccs/tools/compiler/ti-cgt-arm_18.12.2.LTS/lib/rtsv7M4_T_le_v4SPD16_eabi.lib
test.out: ../linker.cmd
test.out: CMakeFiles/test.out.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Users/sramnath/WirelessMicPrototyping/enc28j60-driver/driver-test/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable test.out"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/test.out.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/test.out.dir/build: test.out

.PHONY : CMakeFiles/test.out.dir/build

CMakeFiles/test.out.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/test.out.dir/cmake_clean.cmake
.PHONY : CMakeFiles/test.out.dir/clean

CMakeFiles/test.out.dir/depend:
	cd /Users/sramnath/WirelessMicPrototyping/enc28j60-driver/driver-test/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/sramnath/WirelessMicPrototyping/enc28j60-driver/driver-test /Users/sramnath/WirelessMicPrototyping/enc28j60-driver/driver-test /Users/sramnath/WirelessMicPrototyping/enc28j60-driver/driver-test/build /Users/sramnath/WirelessMicPrototyping/enc28j60-driver/driver-test/build /Users/sramnath/WirelessMicPrototyping/enc28j60-driver/driver-test/build/CMakeFiles/test.out.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/test.out.dir/depend

