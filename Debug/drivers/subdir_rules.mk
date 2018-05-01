################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
drivers/SD_Card.obj: ../drivers/SD_Card.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/bin/cl430" -vmspx --abi=coffabi --code_model=large --data_model=large -O0 --opt_for_speed=1 --use_hw_mpy=F5 --include_path="c:/ti/ccsv6/ccs_base/msp430/include" --include_path="I:/WirelessLab/CP-Firmware" --include_path="I:/WirelessLab/CP-Firmware/Tasks" --include_path="I:/WirelessLab/CP-Firmware/comm_module" --include_path="I:/WirelessLab/CP-Firmware/mem_mod" --include_path="c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/include" --include_path="I:/WirelessLab/CP-Firmware/drivers" --include_path="I:/WirelessLab/CP-Firmware/hal" --advice:power="1,2,3,4,5,6,7,8,9,10,11,12,14" -g --define=__MSP430F5438A__ --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=full --preproc_with_compile --preproc_dependency="drivers/SD_Card.pp" --obj_directory="drivers" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

drivers/SD_Card_Testing.obj: ../drivers/SD_Card_Testing.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/bin/cl430" -vmspx --abi=coffabi --code_model=large --data_model=large -O0 --opt_for_speed=1 --use_hw_mpy=F5 --include_path="c:/ti/ccsv6/ccs_base/msp430/include" --include_path="I:/WirelessLab/CP-Firmware" --include_path="I:/WirelessLab/CP-Firmware/Tasks" --include_path="I:/WirelessLab/CP-Firmware/comm_module" --include_path="I:/WirelessLab/CP-Firmware/mem_mod" --include_path="c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/include" --include_path="I:/WirelessLab/CP-Firmware/drivers" --include_path="I:/WirelessLab/CP-Firmware/hal" --advice:power="1,2,3,4,5,6,7,8,9,10,11,12,14" -g --define=__MSP430F5438A__ --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=full --preproc_with_compile --preproc_dependency="drivers/SD_Card_Testing.pp" --obj_directory="drivers" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

drivers/SP_I2C.obj: ../drivers/SP_I2C.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/bin/cl430" -vmspx --abi=coffabi --code_model=large --data_model=large -O0 --opt_for_speed=1 --use_hw_mpy=F5 --include_path="c:/ti/ccsv6/ccs_base/msp430/include" --include_path="I:/WirelessLab/CP-Firmware" --include_path="I:/WirelessLab/CP-Firmware/Tasks" --include_path="I:/WirelessLab/CP-Firmware/comm_module" --include_path="I:/WirelessLab/CP-Firmware/mem_mod" --include_path="c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/include" --include_path="I:/WirelessLab/CP-Firmware/drivers" --include_path="I:/WirelessLab/CP-Firmware/hal" --advice:power="1,2,3,4,5,6,7,8,9,10,11,12,14" -g --define=__MSP430F5438A__ --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=full --preproc_with_compile --preproc_dependency="drivers/SP_I2C.pp" --obj_directory="drivers" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

drivers/SP_UART.obj: ../drivers/SP_UART.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/bin/cl430" -vmspx --abi=coffabi --code_model=large --data_model=large -O0 --opt_for_speed=1 --use_hw_mpy=F5 --include_path="c:/ti/ccsv6/ccs_base/msp430/include" --include_path="I:/WirelessLab/CP-Firmware" --include_path="I:/WirelessLab/CP-Firmware/Tasks" --include_path="I:/WirelessLab/CP-Firmware/comm_module" --include_path="I:/WirelessLab/CP-Firmware/mem_mod" --include_path="c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/include" --include_path="I:/WirelessLab/CP-Firmware/drivers" --include_path="I:/WirelessLab/CP-Firmware/hal" --advice:power="1,2,3,4,5,6,7,8,9,10,11,12,14" -g --define=__MSP430F5438A__ --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=full --preproc_with_compile --preproc_dependency="drivers/SP_UART.pp" --obj_directory="drivers" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

drivers/adf7020.obj: ../drivers/adf7020.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/bin/cl430" -vmspx --abi=coffabi --code_model=large --data_model=large -O0 --opt_for_speed=1 --use_hw_mpy=F5 --include_path="c:/ti/ccsv6/ccs_base/msp430/include" --include_path="I:/WirelessLab/CP-Firmware" --include_path="I:/WirelessLab/CP-Firmware/Tasks" --include_path="I:/WirelessLab/CP-Firmware/comm_module" --include_path="I:/WirelessLab/CP-Firmware/mem_mod" --include_path="c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/include" --include_path="I:/WirelessLab/CP-Firmware/drivers" --include_path="I:/WirelessLab/CP-Firmware/hal" --advice:power="1,2,3,4,5,6,7,8,9,10,11,12,14" -g --define=__MSP430F5438A__ --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=full --preproc_with_compile --preproc_dependency="drivers/adf7020.pp" --obj_directory="drivers" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

drivers/adf7020_freq.obj: ../drivers/adf7020_freq.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/bin/cl430" -vmspx --abi=coffabi --code_model=large --data_model=large -O0 --opt_for_speed=1 --use_hw_mpy=F5 --include_path="c:/ti/ccsv6/ccs_base/msp430/include" --include_path="I:/WirelessLab/CP-Firmware" --include_path="I:/WirelessLab/CP-Firmware/Tasks" --include_path="I:/WirelessLab/CP-Firmware/comm_module" --include_path="I:/WirelessLab/CP-Firmware/mem_mod" --include_path="c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/include" --include_path="I:/WirelessLab/CP-Firmware/drivers" --include_path="I:/WirelessLab/CP-Firmware/hal" --advice:power="1,2,3,4,5,6,7,8,9,10,11,12,14" -g --define=__MSP430F5438A__ --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=full --preproc_with_compile --preproc_dependency="drivers/adf7020_freq.pp" --obj_directory="drivers" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

drivers/buz.obj: ../drivers/buz.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/bin/cl430" -vmspx --abi=coffabi --code_model=large --data_model=large -O0 --opt_for_speed=1 --use_hw_mpy=F5 --include_path="c:/ti/ccsv6/ccs_base/msp430/include" --include_path="I:/WirelessLab/CP-Firmware" --include_path="I:/WirelessLab/CP-Firmware/Tasks" --include_path="I:/WirelessLab/CP-Firmware/comm_module" --include_path="I:/WirelessLab/CP-Firmware/mem_mod" --include_path="c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/include" --include_path="I:/WirelessLab/CP-Firmware/drivers" --include_path="I:/WirelessLab/CP-Firmware/hal" --advice:power="1,2,3,4,5,6,7,8,9,10,11,12,14" -g --define=__MSP430F5438A__ --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=full --preproc_with_compile --preproc_dependency="drivers/buz.pp" --obj_directory="drivers" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

drivers/flash.obj: ../drivers/flash.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/bin/cl430" -vmspx --abi=coffabi --code_model=large --data_model=large -O0 --opt_for_speed=1 --use_hw_mpy=F5 --include_path="c:/ti/ccsv6/ccs_base/msp430/include" --include_path="I:/WirelessLab/CP-Firmware" --include_path="I:/WirelessLab/CP-Firmware/Tasks" --include_path="I:/WirelessLab/CP-Firmware/comm_module" --include_path="I:/WirelessLab/CP-Firmware/mem_mod" --include_path="c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/include" --include_path="I:/WirelessLab/CP-Firmware/drivers" --include_path="I:/WirelessLab/CP-Firmware/hal" --advice:power="1,2,3,4,5,6,7,8,9,10,11,12,14" -g --define=__MSP430F5438A__ --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=full --preproc_with_compile --preproc_dependency="drivers/flash.pp" --obj_directory="drivers" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

drivers/fram.obj: ../drivers/fram.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/bin/cl430" -vmspx --abi=coffabi --code_model=large --data_model=large -O0 --opt_for_speed=1 --use_hw_mpy=F5 --include_path="c:/ti/ccsv6/ccs_base/msp430/include" --include_path="I:/WirelessLab/CP-Firmware" --include_path="I:/WirelessLab/CP-Firmware/Tasks" --include_path="I:/WirelessLab/CP-Firmware/comm_module" --include_path="I:/WirelessLab/CP-Firmware/mem_mod" --include_path="c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/include" --include_path="I:/WirelessLab/CP-Firmware/drivers" --include_path="I:/WirelessLab/CP-Firmware/hal" --advice:power="1,2,3,4,5,6,7,8,9,10,11,12,14" -g --define=__MSP430F5438A__ --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=full --preproc_with_compile --preproc_dependency="drivers/fram.pp" --obj_directory="drivers" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

drivers/led.obj: ../drivers/led.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/bin/cl430" -vmspx --abi=coffabi --code_model=large --data_model=large -O0 --opt_for_speed=1 --use_hw_mpy=F5 --include_path="c:/ti/ccsv6/ccs_base/msp430/include" --include_path="I:/WirelessLab/CP-Firmware" --include_path="I:/WirelessLab/CP-Firmware/Tasks" --include_path="I:/WirelessLab/CP-Firmware/comm_module" --include_path="I:/WirelessLab/CP-Firmware/mem_mod" --include_path="c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/include" --include_path="I:/WirelessLab/CP-Firmware/drivers" --include_path="I:/WirelessLab/CP-Firmware/hal" --advice:power="1,2,3,4,5,6,7,8,9,10,11,12,14" -g --define=__MSP430F5438A__ --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=full --preproc_with_compile --preproc_dependency="drivers/led.pp" --obj_directory="drivers" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

drivers/scc.obj: ../drivers/scc.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/bin/cl430" -vmspx --abi=coffabi --code_model=large --data_model=large -O0 --opt_for_speed=1 --use_hw_mpy=F5 --include_path="c:/ti/ccsv6/ccs_base/msp430/include" --include_path="I:/WirelessLab/CP-Firmware" --include_path="I:/WirelessLab/CP-Firmware/Tasks" --include_path="I:/WirelessLab/CP-Firmware/comm_module" --include_path="I:/WirelessLab/CP-Firmware/mem_mod" --include_path="c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/include" --include_path="I:/WirelessLab/CP-Firmware/drivers" --include_path="I:/WirelessLab/CP-Firmware/hal" --advice:power="1,2,3,4,5,6,7,8,9,10,11,12,14" -g --define=__MSP430F5438A__ --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=full --preproc_with_compile --preproc_dependency="drivers/scc.pp" --obj_directory="drivers" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

drivers/sram.obj: ../drivers/sram.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/bin/cl430" -vmspx --abi=coffabi --code_model=large --data_model=large -O0 --opt_for_speed=1 --use_hw_mpy=F5 --include_path="c:/ti/ccsv6/ccs_base/msp430/include" --include_path="I:/WirelessLab/CP-Firmware" --include_path="I:/WirelessLab/CP-Firmware/Tasks" --include_path="I:/WirelessLab/CP-Firmware/comm_module" --include_path="I:/WirelessLab/CP-Firmware/mem_mod" --include_path="c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/include" --include_path="I:/WirelessLab/CP-Firmware/drivers" --include_path="I:/WirelessLab/CP-Firmware/hal" --advice:power="1,2,3,4,5,6,7,8,9,10,11,12,14" -g --define=__MSP430F5438A__ --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=full --preproc_with_compile --preproc_dependency="drivers/sram.pp" --obj_directory="drivers" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


