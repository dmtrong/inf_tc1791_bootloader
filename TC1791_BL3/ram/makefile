################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

#rebuild target
rebuild: clean all

# All Target
all: TC1791_BL3.elf

# Tool invocations
TC1791_BL3.elf: $(OBJS) $(USER_OBJS) $(ASM)
	@echo 'Building target: $@'
	@echo 'Invoking: TriCore C Linker'
	"$(TRICORE_TOOLS)/bin/tricore-gcc" -o  "TC1791_BL3.elf" -nocrt0 -nostartfiles -T"..\ld\ram.ld" "src\bsp.o" "src\crt0-tc1x.o" "src\flash.o" "src\main.o" -Wl,--gc-sections -mcpu=tc1791 -Wl,--mem-holes -Wl,--no-warn-flags -Wl,-Map="$(basename $(notdir $@)).map" -Wl,--cref -fshort-double -Wl,-n -Wl,--extmap="a"
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(PCP_S)$(EXECUTABLES)$(OBJS)$(C_DEPS)$(CPCP_DEPS) TC1791_BL3.elf
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
