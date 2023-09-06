################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
%.obj: ../%.c $(GEN_OPTS) | $(GEN_FILES)
	@echo 'Building file: "$<"'
	@echo 'Invoking: ARM Compiler'
	"/Applications/ti_ccs9/ccs9/ccs/tools/compiler/ti-cgt-arm_18.12.2.LTS/bin/armcl" -mv7M4 --code_state=16 --float_support=FPv4SPD16 -me --include_path="/Users/sramnath/ccs_workspaces/spiEthernet/spimaster_enc_eth_CC1352P1_LAUNCHXL_tirtos_ccs" --include_path="/Applications/ti_ccs9/simplelink_cc13x2_26x2_sdk_3_20_00_68/source/ti/posix/ccs" --include_path="/Applications/ti_ccs9/ccs9/ccs/tools/compiler/ti-cgt-arm_18.12.2.LTS/include" --define=DeviceFamily_CC13X2 -g --diag_warning=225 --diag_warning=255 --diag_wrap=off --display_error_number --gen_func_subsections=on --preproc_with_compile --preproc_dependency="$(basename $(<F)).d_raw" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: "$<"'
	@echo ' '


