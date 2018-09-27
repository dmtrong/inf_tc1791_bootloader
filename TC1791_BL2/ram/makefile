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
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

#rebuild target
rebuild: clean all

# All Target
all: TC1791_BL2

# Tool invocations
TC1791_BL2.elf: $(OBJS) $(USER_OBJS) $(ASM)
	@echo 'Building target: $@'
	@echo 'Invoking: TriCore C++ Linker'
	"$(TRICORE_TOOLS)/bin/tricore-g++" -o  "TC1791_BL2.elf" -nocrt0 -nostartfiles -T"../ld/Loader2.ld" "src\Loader2.o" -mcpu=tc1791 -Wl,-Map="$(basename $(notdir $@)).map" -fshort-double -Wl,-n
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(CC_DEPS)$(C++_DEPS)$(PCP_S)$(EXECUTABLES)$(OBJS)$(C_UPPER_DEPS)$(CXX_DEPS)$(C_DEPS)$(CPP_DEPS)$(CPCP_DEPS) TC1791_BL2.elf
	-@echo ' '

TC1791_BL2:TC1791_BL2.elf
	@echo 'Creating target: $@.bin'
	"$(TRICORE_TOOLS)/bin/tricore-objcopy" -O  binary "TC1791_BL2.elf" "TC1791_BL2.bin"
	@echo 'Finished Creating target: $@.bin'
	@echo ' '
	
.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
