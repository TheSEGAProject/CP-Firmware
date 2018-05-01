################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Each subdirectory must supply rules for building sources it contributes
MODACT.obj: ../MODACT.C $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/bin/cl430" -vmspx --abi=coffabi --code_model=large --data_model=large -O0 --opt_for_speed=1 --use_hw_mpy=F5 --include_path="c:/ti/ccsv6/ccs_base/msp430/include" --include_path="I:/WirelessLab/CP-Firmware" --include_path="I:/WirelessLab/CP-Firmware/Tasks" --include_path="I:/WirelessLab/CP-Firmware/comm_module" --include_path="I:/WirelessLab/CP-Firmware/mem_mod" --include_path="c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/include" --include_path="I:/WirelessLab/CP-Firmware/drivers" --include_path="I:/WirelessLab/CP-Firmware/hal" --advice:power="1,2,3,4,5,6,7,8,9,10,11,12,14" -g --define=__MSP430F5438A__ --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=full --preproc_with_compile --preproc_dependency="MODACT.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

MODOPT.obj: ../MODOPT.C $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/bin/cl430" -vmspx --abi=coffabi --code_model=large --data_model=large -O0 --opt_for_speed=1 --use_hw_mpy=F5 --include_path="c:/ti/ccsv6/ccs_base/msp430/include" --include_path="I:/WirelessLab/CP-Firmware" --include_path="I:/WirelessLab/CP-Firmware/Tasks" --include_path="I:/WirelessLab/CP-Firmware/comm_module" --include_path="I:/WirelessLab/CP-Firmware/mem_mod" --include_path="c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/include" --include_path="I:/WirelessLab/CP-Firmware/drivers" --include_path="I:/WirelessLab/CP-Firmware/hal" --advice:power="1,2,3,4,5,6,7,8,9,10,11,12,14" -g --define=__MSP430F5438A__ --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=full --preproc_with_compile --preproc_dependency="MODOPT.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

MODSTORE.obj: ../MODSTORE.C $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/bin/cl430" -vmspx --abi=coffabi --code_model=large --data_model=large -O0 --opt_for_speed=1 --use_hw_mpy=F5 --include_path="c:/ti/ccsv6/ccs_base/msp430/include" --include_path="I:/WirelessLab/CP-Firmware" --include_path="I:/WirelessLab/CP-Firmware/Tasks" --include_path="I:/WirelessLab/CP-Firmware/comm_module" --include_path="I:/WirelessLab/CP-Firmware/mem_mod" --include_path="c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/include" --include_path="I:/WirelessLab/CP-Firmware/drivers" --include_path="I:/WirelessLab/CP-Firmware/hal" --advice:power="1,2,3,4,5,6,7,8,9,10,11,12,14" -g --define=__MSP430F5438A__ --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=full --preproc_with_compile --preproc_dependency="MODSTORE.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

NUMCMD.obj: ../NUMCMD.C $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/bin/cl430" -vmspx --abi=coffabi --code_model=large --data_model=large -O0 --opt_for_speed=1 --use_hw_mpy=F5 --include_path="c:/ti/ccsv6/ccs_base/msp430/include" --include_path="I:/WirelessLab/CP-Firmware" --include_path="I:/WirelessLab/CP-Firmware/Tasks" --include_path="I:/WirelessLab/CP-Firmware/comm_module" --include_path="I:/WirelessLab/CP-Firmware/mem_mod" --include_path="c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/include" --include_path="I:/WirelessLab/CP-Firmware/drivers" --include_path="I:/WirelessLab/CP-Firmware/hal" --advice:power="1,2,3,4,5,6,7,8,9,10,11,12,14" -g --define=__MSP430F5438A__ --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=full --preproc_with_compile --preproc_dependency="NUMCMD.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

OTA.obj: ../OTA.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/bin/cl430" -vmspx --abi=coffabi --code_model=large --data_model=large -O0 --opt_for_speed=1 --use_hw_mpy=F5 --include_path="c:/ti/ccsv6/ccs_base/msp430/include" --include_path="I:/WirelessLab/CP-Firmware" --include_path="I:/WirelessLab/CP-Firmware/Tasks" --include_path="I:/WirelessLab/CP-Firmware/comm_module" --include_path="I:/WirelessLab/CP-Firmware/mem_mod" --include_path="c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/include" --include_path="I:/WirelessLab/CP-Firmware/drivers" --include_path="I:/WirelessLab/CP-Firmware/hal" --advice:power="1,2,3,4,5,6,7,8,9,10,11,12,14" -g --define=__MSP430F5438A__ --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=full --preproc_with_compile --preproc_dependency="OTA.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

SP_BSL.obj: ../SP_BSL.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/bin/cl430" -vmspx --abi=coffabi --code_model=large --data_model=large -O0 --opt_for_speed=1 --use_hw_mpy=F5 --include_path="c:/ti/ccsv6/ccs_base/msp430/include" --include_path="I:/WirelessLab/CP-Firmware" --include_path="I:/WirelessLab/CP-Firmware/Tasks" --include_path="I:/WirelessLab/CP-Firmware/comm_module" --include_path="I:/WirelessLab/CP-Firmware/mem_mod" --include_path="c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/include" --include_path="I:/WirelessLab/CP-Firmware/drivers" --include_path="I:/WirelessLab/CP-Firmware/hal" --advice:power="1,2,3,4,5,6,7,8,9,10,11,12,14" -g --define=__MSP430F5438A__ --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=full --preproc_with_compile --preproc_dependency="SP_BSL.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

bigsub.obj: ../bigsub.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/bin/cl430" -vmspx --abi=coffabi --code_model=large --data_model=large -O0 --opt_for_speed=1 --use_hw_mpy=F5 --include_path="c:/ti/ccsv6/ccs_base/msp430/include" --include_path="I:/WirelessLab/CP-Firmware" --include_path="I:/WirelessLab/CP-Firmware/Tasks" --include_path="I:/WirelessLab/CP-Firmware/comm_module" --include_path="I:/WirelessLab/CP-Firmware/mem_mod" --include_path="c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/include" --include_path="I:/WirelessLab/CP-Firmware/drivers" --include_path="I:/WirelessLab/CP-Firmware/hal" --advice:power="1,2,3,4,5,6,7,8,9,10,11,12,14" -g --define=__MSP430F5438A__ --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=full --preproc_with_compile --preproc_dependency="bigsub.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

button.obj: ../button.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/bin/cl430" -vmspx --abi=coffabi --code_model=large --data_model=large -O0 --opt_for_speed=1 --use_hw_mpy=F5 --include_path="c:/ti/ccsv6/ccs_base/msp430/include" --include_path="I:/WirelessLab/CP-Firmware" --include_path="I:/WirelessLab/CP-Firmware/Tasks" --include_path="I:/WirelessLab/CP-Firmware/comm_module" --include_path="I:/WirelessLab/CP-Firmware/mem_mod" --include_path="c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/include" --include_path="I:/WirelessLab/CP-Firmware/drivers" --include_path="I:/WirelessLab/CP-Firmware/hal" --advice:power="1,2,3,4,5,6,7,8,9,10,11,12,14" -g --define=__MSP430F5438A__ --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=full --preproc_with_compile --preproc_dependency="button.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

cmd.obj: ../cmd.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/bin/cl430" -vmspx --abi=coffabi --code_model=large --data_model=large -O0 --opt_for_speed=1 --use_hw_mpy=F5 --include_path="c:/ti/ccsv6/ccs_base/msp430/include" --include_path="I:/WirelessLab/CP-Firmware" --include_path="I:/WirelessLab/CP-Firmware/Tasks" --include_path="I:/WirelessLab/CP-Firmware/comm_module" --include_path="I:/WirelessLab/CP-Firmware/mem_mod" --include_path="c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/include" --include_path="I:/WirelessLab/CP-Firmware/drivers" --include_path="I:/WirelessLab/CP-Firmware/hal" --advice:power="1,2,3,4,5,6,7,8,9,10,11,12,14" -g --define=__MSP430F5438A__ --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=full --preproc_with_compile --preproc_dependency="cmd.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

daytime.obj: ../daytime.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/bin/cl430" -vmspx --abi=coffabi --code_model=large --data_model=large -O0 --opt_for_speed=1 --use_hw_mpy=F5 --include_path="c:/ti/ccsv6/ccs_base/msp430/include" --include_path="I:/WirelessLab/CP-Firmware" --include_path="I:/WirelessLab/CP-Firmware/Tasks" --include_path="I:/WirelessLab/CP-Firmware/comm_module" --include_path="I:/WirelessLab/CP-Firmware/mem_mod" --include_path="c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/include" --include_path="I:/WirelessLab/CP-Firmware/drivers" --include_path="I:/WirelessLab/CP-Firmware/hal" --advice:power="1,2,3,4,5,6,7,8,9,10,11,12,14" -g --define=__MSP430F5438A__ --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=full --preproc_with_compile --preproc_dependency="daytime.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

delay.obj: ../delay.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/bin/cl430" -vmspx --abi=coffabi --code_model=large --data_model=large -O0 --opt_for_speed=1 --use_hw_mpy=F5 --include_path="c:/ti/ccsv6/ccs_base/msp430/include" --include_path="I:/WirelessLab/CP-Firmware" --include_path="I:/WirelessLab/CP-Firmware/Tasks" --include_path="I:/WirelessLab/CP-Firmware/comm_module" --include_path="I:/WirelessLab/CP-Firmware/mem_mod" --include_path="c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/include" --include_path="I:/WirelessLab/CP-Firmware/drivers" --include_path="I:/WirelessLab/CP-Firmware/hal" --advice:power="1,2,3,4,5,6,7,8,9,10,11,12,14" -g --define=__MSP430F5438A__ --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=full --preproc_with_compile --preproc_dependency="delay.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

diag.obj: ../diag.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/bin/cl430" -vmspx --abi=coffabi --code_model=large --data_model=large -O0 --opt_for_speed=1 --use_hw_mpy=F5 --include_path="c:/ti/ccsv6/ccs_base/msp430/include" --include_path="I:/WirelessLab/CP-Firmware" --include_path="I:/WirelessLab/CP-Firmware/Tasks" --include_path="I:/WirelessLab/CP-Firmware/comm_module" --include_path="I:/WirelessLab/CP-Firmware/mem_mod" --include_path="c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/include" --include_path="I:/WirelessLab/CP-Firmware/drivers" --include_path="I:/WirelessLab/CP-Firmware/hal" --advice:power="1,2,3,4,5,6,7,8,9,10,11,12,14" -g --define=__MSP430F5438A__ --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=full --preproc_with_compile --preproc_dependency="diag.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

dradio.obj: ../dradio.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/bin/cl430" -vmspx --abi=coffabi --code_model=large --data_model=large -O0 --opt_for_speed=1 --use_hw_mpy=F5 --include_path="c:/ti/ccsv6/ccs_base/msp430/include" --include_path="I:/WirelessLab/CP-Firmware" --include_path="I:/WirelessLab/CP-Firmware/Tasks" --include_path="I:/WirelessLab/CP-Firmware/comm_module" --include_path="I:/WirelessLab/CP-Firmware/mem_mod" --include_path="c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/include" --include_path="I:/WirelessLab/CP-Firmware/drivers" --include_path="I:/WirelessLab/CP-Firmware/hal" --advice:power="1,2,3,4,5,6,7,8,9,10,11,12,14" -g --define=__MSP430F5438A__ --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=full --preproc_with_compile --preproc_dependency="dradio.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

fulldiag.obj: ../fulldiag.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/bin/cl430" -vmspx --abi=coffabi --code_model=large --data_model=large -O0 --opt_for_speed=1 --use_hw_mpy=F5 --include_path="c:/ti/ccsv6/ccs_base/msp430/include" --include_path="I:/WirelessLab/CP-Firmware" --include_path="I:/WirelessLab/CP-Firmware/Tasks" --include_path="I:/WirelessLab/CP-Firmware/comm_module" --include_path="I:/WirelessLab/CP-Firmware/mem_mod" --include_path="c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/include" --include_path="I:/WirelessLab/CP-Firmware/drivers" --include_path="I:/WirelessLab/CP-Firmware/hal" --advice:power="1,2,3,4,5,6,7,8,9,10,11,12,14" -g --define=__MSP430F5438A__ --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=full --preproc_with_compile --preproc_dependency="fulldiag.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

gid.obj: ../gid.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/bin/cl430" -vmspx --abi=coffabi --code_model=large --data_model=large -O0 --opt_for_speed=1 --use_hw_mpy=F5 --include_path="c:/ti/ccsv6/ccs_base/msp430/include" --include_path="I:/WirelessLab/CP-Firmware" --include_path="I:/WirelessLab/CP-Firmware/Tasks" --include_path="I:/WirelessLab/CP-Firmware/comm_module" --include_path="I:/WirelessLab/CP-Firmware/mem_mod" --include_path="c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/include" --include_path="I:/WirelessLab/CP-Firmware/drivers" --include_path="I:/WirelessLab/CP-Firmware/hal" --advice:power="1,2,3,4,5,6,7,8,9,10,11,12,14" -g --define=__MSP430F5438A__ --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=full --preproc_with_compile --preproc_dependency="gid.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

key.obj: ../key.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/bin/cl430" -vmspx --abi=coffabi --code_model=large --data_model=large -O0 --opt_for_speed=1 --use_hw_mpy=F5 --include_path="c:/ti/ccsv6/ccs_base/msp430/include" --include_path="I:/WirelessLab/CP-Firmware" --include_path="I:/WirelessLab/CP-Firmware/Tasks" --include_path="I:/WirelessLab/CP-Firmware/comm_module" --include_path="I:/WirelessLab/CP-Firmware/mem_mod" --include_path="c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/include" --include_path="I:/WirelessLab/CP-Firmware/drivers" --include_path="I:/WirelessLab/CP-Firmware/hal" --advice:power="1,2,3,4,5,6,7,8,9,10,11,12,14" -g --define=__MSP430F5438A__ --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=full --preproc_with_compile --preproc_dependency="key.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

main.obj: ../main.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/bin/cl430" -vmspx --abi=coffabi --code_model=large --data_model=large -O0 --opt_for_speed=1 --use_hw_mpy=F5 --include_path="c:/ti/ccsv6/ccs_base/msp430/include" --include_path="I:/WirelessLab/CP-Firmware" --include_path="I:/WirelessLab/CP-Firmware/Tasks" --include_path="I:/WirelessLab/CP-Firmware/comm_module" --include_path="I:/WirelessLab/CP-Firmware/mem_mod" --include_path="c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/include" --include_path="I:/WirelessLab/CP-Firmware/drivers" --include_path="I:/WirelessLab/CP-Firmware/hal" --advice:power="1,2,3,4,5,6,7,8,9,10,11,12,14" -g --define=__MSP430F5438A__ --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=full --preproc_with_compile --preproc_dependency="main.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

misc.obj: ../misc.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/bin/cl430" -vmspx --abi=coffabi --code_model=large --data_model=large -O0 --opt_for_speed=1 --use_hw_mpy=F5 --include_path="c:/ti/ccsv6/ccs_base/msp430/include" --include_path="I:/WirelessLab/CP-Firmware" --include_path="I:/WirelessLab/CP-Firmware/Tasks" --include_path="I:/WirelessLab/CP-Firmware/comm_module" --include_path="I:/WirelessLab/CP-Firmware/mem_mod" --include_path="c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/include" --include_path="I:/WirelessLab/CP-Firmware/drivers" --include_path="I:/WirelessLab/CP-Firmware/hal" --advice:power="1,2,3,4,5,6,7,8,9,10,11,12,14" -g --define=__MSP430F5438A__ --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=full --preproc_with_compile --preproc_dependency="misc.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

pick.obj: ../pick.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/bin/cl430" -vmspx --abi=coffabi --code_model=large --data_model=large -O0 --opt_for_speed=1 --use_hw_mpy=F5 --include_path="c:/ti/ccsv6/ccs_base/msp430/include" --include_path="I:/WirelessLab/CP-Firmware" --include_path="I:/WirelessLab/CP-Firmware/Tasks" --include_path="I:/WirelessLab/CP-Firmware/comm_module" --include_path="I:/WirelessLab/CP-Firmware/mem_mod" --include_path="c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/include" --include_path="I:/WirelessLab/CP-Firmware/drivers" --include_path="I:/WirelessLab/CP-Firmware/hal" --advice:power="1,2,3,4,5,6,7,8,9,10,11,12,14" -g --define=__MSP430F5438A__ --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=full --preproc_with_compile --preproc_dependency="pick.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

rand.obj: ../rand.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/bin/cl430" -vmspx --abi=coffabi --code_model=large --data_model=large -O0 --opt_for_speed=1 --use_hw_mpy=F5 --include_path="c:/ti/ccsv6/ccs_base/msp430/include" --include_path="I:/WirelessLab/CP-Firmware" --include_path="I:/WirelessLab/CP-Firmware/Tasks" --include_path="I:/WirelessLab/CP-Firmware/comm_module" --include_path="I:/WirelessLab/CP-Firmware/mem_mod" --include_path="c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/include" --include_path="I:/WirelessLab/CP-Firmware/drivers" --include_path="I:/WirelessLab/CP-Firmware/hal" --advice:power="1,2,3,4,5,6,7,8,9,10,11,12,14" -g --define=__MSP430F5438A__ --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=full --preproc_with_compile --preproc_dependency="rand.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

report.obj: ../report.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/bin/cl430" -vmspx --abi=coffabi --code_model=large --data_model=large -O0 --opt_for_speed=1 --use_hw_mpy=F5 --include_path="c:/ti/ccsv6/ccs_base/msp430/include" --include_path="I:/WirelessLab/CP-Firmware" --include_path="I:/WirelessLab/CP-Firmware/Tasks" --include_path="I:/WirelessLab/CP-Firmware/comm_module" --include_path="I:/WirelessLab/CP-Firmware/mem_mod" --include_path="c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/include" --include_path="I:/WirelessLab/CP-Firmware/drivers" --include_path="I:/WirelessLab/CP-Firmware/hal" --advice:power="1,2,3,4,5,6,7,8,9,10,11,12,14" -g --define=__MSP430F5438A__ --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=full --preproc_with_compile --preproc_dependency="report.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

rom.obj: ../rom.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/bin/cl430" -vmspx --abi=coffabi --code_model=large --data_model=large -O0 --opt_for_speed=1 --use_hw_mpy=F5 --include_path="c:/ti/ccsv6/ccs_base/msp430/include" --include_path="I:/WirelessLab/CP-Firmware" --include_path="I:/WirelessLab/CP-Firmware/Tasks" --include_path="I:/WirelessLab/CP-Firmware/comm_module" --include_path="I:/WirelessLab/CP-Firmware/mem_mod" --include_path="c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/include" --include_path="I:/WirelessLab/CP-Firmware/drivers" --include_path="I:/WirelessLab/CP-Firmware/hal" --advice:power="1,2,3,4,5,6,7,8,9,10,11,12,14" -g --define=__MSP430F5438A__ --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=full --preproc_with_compile --preproc_dependency="rom.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

sensor.obj: ../sensor.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/bin/cl430" -vmspx --abi=coffabi --code_model=large --data_model=large -O0 --opt_for_speed=1 --use_hw_mpy=F5 --include_path="c:/ti/ccsv6/ccs_base/msp430/include" --include_path="I:/WirelessLab/CP-Firmware" --include_path="I:/WirelessLab/CP-Firmware/Tasks" --include_path="I:/WirelessLab/CP-Firmware/comm_module" --include_path="I:/WirelessLab/CP-Firmware/mem_mod" --include_path="c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/include" --include_path="I:/WirelessLab/CP-Firmware/drivers" --include_path="I:/WirelessLab/CP-Firmware/hal" --advice:power="1,2,3,4,5,6,7,8,9,10,11,12,14" -g --define=__MSP430F5438A__ --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=full --preproc_with_compile --preproc_dependency="sensor.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

serial.obj: ../serial.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/bin/cl430" -vmspx --abi=coffabi --code_model=large --data_model=large -O0 --opt_for_speed=1 --use_hw_mpy=F5 --include_path="c:/ti/ccsv6/ccs_base/msp430/include" --include_path="I:/WirelessLab/CP-Firmware" --include_path="I:/WirelessLab/CP-Firmware/Tasks" --include_path="I:/WirelessLab/CP-Firmware/comm_module" --include_path="I:/WirelessLab/CP-Firmware/mem_mod" --include_path="c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/include" --include_path="I:/WirelessLab/CP-Firmware/drivers" --include_path="I:/WirelessLab/CP-Firmware/hal" --advice:power="1,2,3,4,5,6,7,8,9,10,11,12,14" -g --define=__MSP430F5438A__ --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=full --preproc_with_compile --preproc_dependency="serial.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

sysact.obj: ../sysact.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/bin/cl430" -vmspx --abi=coffabi --code_model=large --data_model=large -O0 --opt_for_speed=1 --use_hw_mpy=F5 --include_path="c:/ti/ccsv6/ccs_base/msp430/include" --include_path="I:/WirelessLab/CP-Firmware" --include_path="I:/WirelessLab/CP-Firmware/Tasks" --include_path="I:/WirelessLab/CP-Firmware/comm_module" --include_path="I:/WirelessLab/CP-Firmware/mem_mod" --include_path="c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/include" --include_path="I:/WirelessLab/CP-Firmware/drivers" --include_path="I:/WirelessLab/CP-Firmware/hal" --advice:power="1,2,3,4,5,6,7,8,9,10,11,12,14" -g --define=__MSP430F5438A__ --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=full --preproc_with_compile --preproc_dependency="sysact.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '

time.obj: ../time.c $(GEN_OPTS) $(GEN_HDRS)
	@echo 'Building file: $<'
	@echo 'Invoking: MSP430 Compiler'
	"c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/bin/cl430" -vmspx --abi=coffabi --code_model=large --data_model=large -O0 --opt_for_speed=1 --use_hw_mpy=F5 --include_path="c:/ti/ccsv6/ccs_base/msp430/include" --include_path="I:/WirelessLab/CP-Firmware" --include_path="I:/WirelessLab/CP-Firmware/Tasks" --include_path="I:/WirelessLab/CP-Firmware/comm_module" --include_path="I:/WirelessLab/CP-Firmware/mem_mod" --include_path="c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/include" --include_path="I:/WirelessLab/CP-Firmware/drivers" --include_path="I:/WirelessLab/CP-Firmware/hal" --advice:power="1,2,3,4,5,6,7,8,9,10,11,12,14" -g --define=__MSP430F5438A__ --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=full --preproc_with_compile --preproc_dependency="time.pp" $(GEN_OPTS__FLAG) "$<"
	@echo 'Finished building: $<'
	@echo ' '


