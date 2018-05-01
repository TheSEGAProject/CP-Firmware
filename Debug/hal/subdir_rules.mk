################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
hal/MPY32.obj: ../hal/MPY32.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/bin/cl430" -vmspx --abi=coffabi --code_model=large --data_model=large -O0 --opt_for_speed=1 --use_hw_mpy=F5 --include_path="c:/ti/ccsv6/ccs_base/msp430/include" --include_path="I:/WirelessLab/CP-Firmware" --include_path="I:/WirelessLab/CP-Firmware/Tasks" --include_path="I:/WirelessLab/CP-Firmware/comm_module" --include_path="I:/WirelessLab/CP-Firmware/mem_mod" --include_path="c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/include" --include_path="I:/WirelessLab/CP-Firmware/drivers" --include_path="I:/WirelessLab/CP-Firmware/hal" --advice:power="1,2,3,4,5,6,7,8,9,10,11,12,14" -g --define=__MSP430F5438A__ --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=full --preproc_with_compile --preproc_dependency="hal/MPY32.pp" --obj_directory="hal" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

hal/PMM.obj: ../hal/PMM.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/bin/cl430" -vmspx --abi=coffabi --code_model=large --data_model=large -O0 --opt_for_speed=1 --use_hw_mpy=F5 --include_path="c:/ti/ccsv6/ccs_base/msp430/include" --include_path="I:/WirelessLab/CP-Firmware" --include_path="I:/WirelessLab/CP-Firmware/Tasks" --include_path="I:/WirelessLab/CP-Firmware/comm_module" --include_path="I:/WirelessLab/CP-Firmware/mem_mod" --include_path="c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/include" --include_path="I:/WirelessLab/CP-Firmware/drivers" --include_path="I:/WirelessLab/CP-Firmware/hal" --advice:power="1,2,3,4,5,6,7,8,9,10,11,12,14" -g --define=__MSP430F5438A__ --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=full --preproc_with_compile --preproc_dependency="hal/PMM.pp" --obj_directory="hal" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

hal/ad.obj: ../hal/ad.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/bin/cl430" -vmspx --abi=coffabi --code_model=large --data_model=large -O0 --opt_for_speed=1 --use_hw_mpy=F5 --include_path="c:/ti/ccsv6/ccs_base/msp430/include" --include_path="I:/WirelessLab/CP-Firmware" --include_path="I:/WirelessLab/CP-Firmware/Tasks" --include_path="I:/WirelessLab/CP-Firmware/comm_module" --include_path="I:/WirelessLab/CP-Firmware/mem_mod" --include_path="c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/include" --include_path="I:/WirelessLab/CP-Firmware/drivers" --include_path="I:/WirelessLab/CP-Firmware/hal" --advice:power="1,2,3,4,5,6,7,8,9,10,11,12,14" -g --define=__MSP430F5438A__ --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=full --preproc_with_compile --preproc_dependency="hal/ad.pp" --obj_directory="hal" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

hal/config.obj: ../hal/config.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/bin/cl430" -vmspx --abi=coffabi --code_model=large --data_model=large -O0 --opt_for_speed=1 --use_hw_mpy=F5 --include_path="c:/ti/ccsv6/ccs_base/msp430/include" --include_path="I:/WirelessLab/CP-Firmware" --include_path="I:/WirelessLab/CP-Firmware/Tasks" --include_path="I:/WirelessLab/CP-Firmware/comm_module" --include_path="I:/WirelessLab/CP-Firmware/mem_mod" --include_path="c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/include" --include_path="I:/WirelessLab/CP-Firmware/drivers" --include_path="I:/WirelessLab/CP-Firmware/hal" --advice:power="1,2,3,4,5,6,7,8,9,10,11,12,14" -g --define=__MSP430F5438A__ --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=full --preproc_with_compile --preproc_dependency="hal/config.pp" --obj_directory="hal" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

hal/flash_mcu.obj: ../hal/flash_mcu.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/bin/cl430" -vmspx --abi=coffabi --code_model=large --data_model=large -O0 --opt_for_speed=1 --use_hw_mpy=F5 --include_path="c:/ti/ccsv6/ccs_base/msp430/include" --include_path="I:/WirelessLab/CP-Firmware" --include_path="I:/WirelessLab/CP-Firmware/Tasks" --include_path="I:/WirelessLab/CP-Firmware/comm_module" --include_path="I:/WirelessLab/CP-Firmware/mem_mod" --include_path="c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/include" --include_path="I:/WirelessLab/CP-Firmware/drivers" --include_path="I:/WirelessLab/CP-Firmware/hal" --advice:power="1,2,3,4,5,6,7,8,9,10,11,12,14" -g --define=__MSP430F5438A__ --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=full --preproc_with_compile --preproc_dependency="hal/flash_mcu.pp" --obj_directory="hal" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

hal/irupt.obj: ../hal/irupt.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/bin/cl430" -vmspx --abi=coffabi --code_model=large --data_model=large -O0 --opt_for_speed=1 --use_hw_mpy=F5 --include_path="c:/ti/ccsv6/ccs_base/msp430/include" --include_path="I:/WirelessLab/CP-Firmware" --include_path="I:/WirelessLab/CP-Firmware/Tasks" --include_path="I:/WirelessLab/CP-Firmware/comm_module" --include_path="I:/WirelessLab/CP-Firmware/mem_mod" --include_path="c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/include" --include_path="I:/WirelessLab/CP-Firmware/drivers" --include_path="I:/WirelessLab/CP-Firmware/hal" --advice:power="1,2,3,4,5,6,7,8,9,10,11,12,14" -g --define=__MSP430F5438A__ --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=full --preproc_with_compile --preproc_dependency="hal/irupt.pp" --obj_directory="hal" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

hal/rtc.obj: ../hal/rtc.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/bin/cl430" -vmspx --abi=coffabi --code_model=large --data_model=large -O0 --opt_for_speed=1 --use_hw_mpy=F5 --include_path="c:/ti/ccsv6/ccs_base/msp430/include" --include_path="I:/WirelessLab/CP-Firmware" --include_path="I:/WirelessLab/CP-Firmware/Tasks" --include_path="I:/WirelessLab/CP-Firmware/comm_module" --include_path="I:/WirelessLab/CP-Firmware/mem_mod" --include_path="c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/include" --include_path="I:/WirelessLab/CP-Firmware/drivers" --include_path="I:/WirelessLab/CP-Firmware/hal" --advice:power="1,2,3,4,5,6,7,8,9,10,11,12,14" -g --define=__MSP430F5438A__ --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=full --preproc_with_compile --preproc_dependency="hal/rtc.pp" --obj_directory="hal" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

hal/spi.obj: ../hal/spi.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/bin/cl430" -vmspx --abi=coffabi --code_model=large --data_model=large -O0 --opt_for_speed=1 --use_hw_mpy=F5 --include_path="c:/ti/ccsv6/ccs_base/msp430/include" --include_path="I:/WirelessLab/CP-Firmware" --include_path="I:/WirelessLab/CP-Firmware/Tasks" --include_path="I:/WirelessLab/CP-Firmware/comm_module" --include_path="I:/WirelessLab/CP-Firmware/mem_mod" --include_path="c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/include" --include_path="I:/WirelessLab/CP-Firmware/drivers" --include_path="I:/WirelessLab/CP-Firmware/hal" --advice:power="1,2,3,4,5,6,7,8,9,10,11,12,14" -g --define=__MSP430F5438A__ --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=full --preproc_with_compile --preproc_dependency="hal/spi.pp" --obj_directory="hal" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

hal/sys.obj: ../hal/sys.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/bin/cl430" -vmspx --abi=coffabi --code_model=large --data_model=large -O0 --opt_for_speed=1 --use_hw_mpy=F5 --include_path="c:/ti/ccsv6/ccs_base/msp430/include" --include_path="I:/WirelessLab/CP-Firmware" --include_path="I:/WirelessLab/CP-Firmware/Tasks" --include_path="I:/WirelessLab/CP-Firmware/comm_module" --include_path="I:/WirelessLab/CP-Firmware/mem_mod" --include_path="c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/include" --include_path="I:/WirelessLab/CP-Firmware/drivers" --include_path="I:/WirelessLab/CP-Firmware/hal" --advice:power="1,2,3,4,5,6,7,8,9,10,11,12,14" -g --define=__MSP430F5438A__ --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=full --preproc_with_compile --preproc_dependency="hal/sys.pp" --obj_directory="hal" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

hal/ucs.obj: ../hal/ucs.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/bin/cl430" -vmspx --abi=coffabi --code_model=large --data_model=large -O0 --opt_for_speed=1 --use_hw_mpy=F5 --include_path="c:/ti/ccsv6/ccs_base/msp430/include" --include_path="I:/WirelessLab/CP-Firmware" --include_path="I:/WirelessLab/CP-Firmware/Tasks" --include_path="I:/WirelessLab/CP-Firmware/comm_module" --include_path="I:/WirelessLab/CP-Firmware/mem_mod" --include_path="c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/include" --include_path="I:/WirelessLab/CP-Firmware/drivers" --include_path="I:/WirelessLab/CP-Firmware/hal" --advice:power="1,2,3,4,5,6,7,8,9,10,11,12,14" -g --define=__MSP430F5438A__ --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=full --preproc_with_compile --preproc_dependency="hal/ucs.pp" --obj_directory="hal" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

hal/usci_A0_uart.obj: ../hal/usci_A0_uart.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/bin/cl430" -vmspx --abi=coffabi --code_model=large --data_model=large -O0 --opt_for_speed=1 --use_hw_mpy=F5 --include_path="c:/ti/ccsv6/ccs_base/msp430/include" --include_path="I:/WirelessLab/CP-Firmware" --include_path="I:/WirelessLab/CP-Firmware/Tasks" --include_path="I:/WirelessLab/CP-Firmware/comm_module" --include_path="I:/WirelessLab/CP-Firmware/mem_mod" --include_path="c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/include" --include_path="I:/WirelessLab/CP-Firmware/drivers" --include_path="I:/WirelessLab/CP-Firmware/hal" --advice:power="1,2,3,4,5,6,7,8,9,10,11,12,14" -g --define=__MSP430F5438A__ --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=full --preproc_with_compile --preproc_dependency="hal/usci_A0_uart.pp" --obj_directory="hal" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


