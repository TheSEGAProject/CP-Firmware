################################################################################
# Automatically-generated file. Do not edit!
################################################################################

SHELL = cmd.exe

CG_TOOL_ROOT := c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1

ORDERED_OBJS += \
$(GEN_CMDS__FLAG) \
"./MODACT.obj" \
"./MODOPT.obj" \
"./MODSTORE.obj" \
"./NUMCMD.obj" \
"./OTA.obj" \
"./SP_BSL.obj" \
"./bigsub.obj" \
"./button.obj" \
"./cmd.obj" \
"./daytime.obj" \
"./delay.obj" \
"./diag.obj" \
"./dradio.obj" \
"./fulldiag.obj" \
"./gid.obj" \
"./key.obj" \
"./main.obj" \
"./misc.obj" \
"./pick.obj" \
"./rand.obj" \
"./report.obj" \
"./rom.obj" \
"./sensor.obj" \
"./serial.obj" \
"./sysact.obj" \
"./time.obj" \
"./mem_mod/L2Sram.obj" \
"./mem_mod/L2fram.obj" \
"./mem_mod/l2flash.obj" \
"./hal/MPY32.obj" \
"./hal/PMM.obj" \
"./hal/ad.obj" \
"./hal/config.obj" \
"./hal/flash_mcu.obj" \
"./hal/irupt.obj" \
"./hal/rtc.obj" \
"./hal/spi.obj" \
"./hal/sys.obj" \
"./hal/ucs.obj" \
"./hal/usci_A0_uart.obj" \
"./drivers/SD_Card.obj" \
"./drivers/SD_Card_Testing.obj" \
"./drivers/SP_I2C.obj" \
"./drivers/SP_UART.obj" \
"./drivers/adf7020.obj" \
"./drivers/adf7020_freq.obj" \
"./drivers/buz.obj" \
"./drivers/flash.obj" \
"./drivers/fram.obj" \
"./drivers/led.obj" \
"./drivers/scc.obj" \
"./drivers/sram.obj" \
"./comm_module/LNKBLK.obj" \
"./comm_module/comm_discovery.obj" \
"./comm_module/comm_opmode.obj" \
"./comm_module/comm_routing.obj" \
"./comm_module/comm_utilities.obj" \
"./comm_module/crc.obj" \
"./comm_module/gs.obj" \
"./Tasks/rts.obj" \
"./Tasks/task_dispatch.obj" \
"./Tasks/task_manager.obj" \
"../lnk_msp430f5438a.cmd" \
-l"libc.a" \
-l"libmath.a" \

-include ../makefile.init

RM := DEL /F
RMDIR := RMDIR /S/Q

# All of the sources participating in the build are defined here
-include sources.mk
-include subdir_vars.mk
-include mem_mod/subdir_vars.mk
-include hal/subdir_vars.mk
-include drivers/subdir_vars.mk
-include comm_module/subdir_vars.mk
-include Tasks/subdir_vars.mk
-include subdir_rules.mk
-include mem_mod/subdir_rules.mk
-include hal/subdir_rules.mk
-include drivers/subdir_rules.mk
-include comm_module/subdir_rules.mk
-include Tasks/subdir_rules.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(S_DEPS)),)
-include $(S_DEPS)
endif
ifneq ($(strip $(S_UPPER_DEPS)),)
-include $(S_UPPER_DEPS)
endif
ifneq ($(strip $(S62_DEPS)),)
-include $(S62_DEPS)
endif
ifneq ($(strip $(C64_DEPS)),)
-include $(C64_DEPS)
endif
ifneq ($(strip $(ASM_DEPS)),)
-include $(ASM_DEPS)
endif
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(S55_DEPS)),)
-include $(S55_DEPS)
endif
ifneq ($(strip $(C67_DEPS)),)
-include $(C67_DEPS)
endif
ifneq ($(strip $(C??_DEPS)),)
-include $(C??_DEPS)
endif
ifneq ($(strip $(CLA_DEPS)),)
-include $(CLA_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(S??_DEPS)),)
-include $(S??_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(C62_DEPS)),)
-include $(C62_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(ASM_UPPER_DEPS)),)
-include $(ASM_UPPER_DEPS)
endif
ifneq ($(strip $(K_DEPS)),)
-include $(K_DEPS)
endif
ifneq ($(strip $(C43_DEPS)),)
-include $(C43_DEPS)
endif
ifneq ($(strip $(S67_DEPS)),)
-include $(S67_DEPS)
endif
ifneq ($(strip $(SA_DEPS)),)
-include $(SA_DEPS)
endif
ifneq ($(strip $(S43_DEPS)),)
-include $(S43_DEPS)
endif
ifneq ($(strip $(OPT_DEPS)),)
-include $(OPT_DEPS)
endif
ifneq ($(strip $(S64_DEPS)),)
-include $(S64_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(C55_DEPS)),)
-include $(C55_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 
EXE_OUTPUTS += \
CP-Firmware.out \

EXE_OUTPUTS__QUOTED += \
"CP-Firmware.out" \

BIN_OUTPUTS += \
CP-Firmware.txt \

BIN_OUTPUTS__QUOTED += \
"CP-Firmware.txt" \


# All Target
all: CP-Firmware.out secondary-outputs

# Tool invocations
CP-Firmware.out: $(OBJS) $(CMD_SRCS) $(GEN_CMDS)
	@echo 'Building target: $@'
	@echo 'Invoking: MSP430 Linker'
	"c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/bin/cl430" -vmspx --abi=coffabi --code_model=large --data_model=large -O0 --opt_for_speed=1 --use_hw_mpy=F5 --advice:power="1,2,3,4,5,6,7,8,9,10,11,12,14" -g --define=__MSP430F5438A__ --diag_warning=225 --display_error_number --diag_wrap=off --silicon_errata=CPU21 --silicon_errata=CPU22 --silicon_errata=CPU23 --silicon_errata=CPU40 --printf_support=full -z -m"CP-Firmware.map" --heap_size=320 --stack_size=160 --cinit_hold_wdt=on -i"c:/ti/ccsv6/ccs_base/msp430/include" -i"c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/lib" -i"c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/include" -i"c:/ti/ccsv6/ccs_base/msp430/lib/5xx_6xx_FRxx" --reread_libs --warn_sections --display_error_number --diag_wrap=off --xml_link_info="CP-Firmware_linkInfo.xml" --rom_model -o "CP-Firmware.out" $(ORDERED_OBJS)
	@echo 'Finished building target: $@'
	@echo ' '

CP-Firmware.txt: $(EXE_OUTPUTS)
	@echo 'Invoking: MSP430 Hex Utility'
	"c:/ti/ccsv6/tools/compiler/ti-cgt-msp430_4.4.1/bin/hex430" --memwidth=8 --romwidth=8 --ti_txt -o "CP-Firmware.txt" $(EXE_OUTPUTS__QUOTED)
	@echo 'Finished building: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(EXE_OUTPUTS__QUOTED)$(BIN_OUTPUTS__QUOTED)
	-$(RM) "OTA.pp" "SP_BSL.pp" "bigsub.pp" "button.pp" "cmd.pp" "daytime.pp" "delay.pp" "diag.pp" "dradio.pp" "fulldiag.pp" "gid.pp" "key.pp" "main.pp" "misc.pp" "pick.pp" "rand.pp" "report.pp" "rom.pp" "sensor.pp" "serial.pp" "sysact.pp" "time.pp" "mem_mod\L2Sram.pp" "mem_mod\L2fram.pp" "mem_mod\l2flash.pp" "hal\MPY32.pp" "hal\PMM.pp" "hal\ad.pp" "hal\config.pp" "hal\flash_mcu.pp" "hal\irupt.pp" "hal\rtc.pp" "hal\spi.pp" "hal\sys.pp" "hal\ucs.pp" "hal\usci_A0_uart.pp" "drivers\SD_Card.pp" "drivers\SD_Card_Testing.pp" "drivers\SP_I2C.pp" "drivers\SP_UART.pp" "drivers\adf7020.pp" "drivers\adf7020_freq.pp" "drivers\buz.pp" "drivers\flash.pp" "drivers\fram.pp" "drivers\led.pp" "drivers\scc.pp" "drivers\sram.pp" "comm_module\comm_discovery.pp" "comm_module\comm_opmode.pp" "comm_module\comm_routing.pp" "comm_module\comm_utilities.pp" "comm_module\crc.pp" "comm_module\gs.pp" "Tasks\rts.pp" "Tasks\task_dispatch.pp" "Tasks\task_manager.pp" 
	-$(RM) "MODACT.pp" "MODOPT.pp" "MODSTORE.pp" "NUMCMD.pp" "comm_module\LNKBLK.pp" 
	-$(RM) "MODACT.obj" "MODOPT.obj" "MODSTORE.obj" "NUMCMD.obj" "OTA.obj" "SP_BSL.obj" "bigsub.obj" "button.obj" "cmd.obj" "daytime.obj" "delay.obj" "diag.obj" "dradio.obj" "fulldiag.obj" "gid.obj" "key.obj" "main.obj" "misc.obj" "pick.obj" "rand.obj" "report.obj" "rom.obj" "sensor.obj" "serial.obj" "sysact.obj" "time.obj" "mem_mod\L2Sram.obj" "mem_mod\L2fram.obj" "mem_mod\l2flash.obj" "hal\MPY32.obj" "hal\PMM.obj" "hal\ad.obj" "hal\config.obj" "hal\flash_mcu.obj" "hal\irupt.obj" "hal\rtc.obj" "hal\spi.obj" "hal\sys.obj" "hal\ucs.obj" "hal\usci_A0_uart.obj" "drivers\SD_Card.obj" "drivers\SD_Card_Testing.obj" "drivers\SP_I2C.obj" "drivers\SP_UART.obj" "drivers\adf7020.obj" "drivers\adf7020_freq.obj" "drivers\buz.obj" "drivers\flash.obj" "drivers\fram.obj" "drivers\led.obj" "drivers\scc.obj" "drivers\sram.obj" "comm_module\LNKBLK.obj" "comm_module\comm_discovery.obj" "comm_module\comm_opmode.obj" "comm_module\comm_routing.obj" "comm_module\comm_utilities.obj" "comm_module\crc.obj" "comm_module\gs.obj" 
	-$(RM) "Tasks\rts.obj" "Tasks\task_dispatch.obj" "Tasks\task_manager.obj" 
	-@echo 'Finished clean'
	-@echo ' '

secondary-outputs: $(BIN_OUTPUTS)

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets

