################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
Tasks/rts.obj: ../Tasks/rts.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/bin/cl430" -vmspx --abi=coffabi --code_model=large --data_model=large -O0 --opt_for_speed=1 --use_hw_mpy=F5 --include_path="c:/ti/ccsv6/ccs_base/msp430/include" --include_path="I:/WirelessLab/CP-Firmware" --include_path="I:/WirelessLab/CP-Firmware/Tasks" --include_path="I:/WirelessLab/CP-Firmware/comm_module" --include_path="I:/WirelessLab/CP-Firmware/mem_mod" --include_path="c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/include" --include_path="I:/WirelessLab/CP-Firmware/drivers" --include_path="I:/WirelessLab/CP-Firmware/hal" --advice:power="1,2,3,4,5,6,7,8,9,10,11,12,14" -g --define=__MSP430F5438A__ --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=full --preproc_with_compile --preproc_dependency="Tasks/rts.pp" --obj_directory="Tasks" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

Tasks/task_dispatch.obj: ../Tasks/task_dispatch.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/bin/cl430" -vmspx --abi=coffabi --code_model=large --data_model=large -O0 --opt_for_speed=1 --use_hw_mpy=F5 --include_path="c:/ti/ccsv6/ccs_base/msp430/include" --include_path="I:/WirelessLab/CP-Firmware" --include_path="I:/WirelessLab/CP-Firmware/Tasks" --include_path="I:/WirelessLab/CP-Firmware/comm_module" --include_path="I:/WirelessLab/CP-Firmware/mem_mod" --include_path="c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/include" --include_path="I:/WirelessLab/CP-Firmware/drivers" --include_path="I:/WirelessLab/CP-Firmware/hal" --advice:power="1,2,3,4,5,6,7,8,9,10,11,12,14" -g --define=__MSP430F5438A__ --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=full --preproc_with_compile --preproc_dependency="Tasks/task_dispatch.pp" --obj_directory="Tasks" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

Tasks/task_manager.obj: ../Tasks/task_manager.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/bin/cl430" -vmspx --abi=coffabi --code_model=large --data_model=large -O0 --opt_for_speed=1 --use_hw_mpy=F5 --include_path="c:/ti/ccsv6/ccs_base/msp430/include" --include_path="I:/WirelessLab/CP-Firmware" --include_path="I:/WirelessLab/CP-Firmware/Tasks" --include_path="I:/WirelessLab/CP-Firmware/comm_module" --include_path="I:/WirelessLab/CP-Firmware/mem_mod" --include_path="c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/include" --include_path="I:/WirelessLab/CP-Firmware/drivers" --include_path="I:/WirelessLab/CP-Firmware/hal" --advice:power="1,2,3,4,5,6,7,8,9,10,11,12,14" -g --define=__MSP430F5438A__ --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=full --preproc_with_compile --preproc_dependency="Tasks/task_manager.pp" --obj_directory="Tasks" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


