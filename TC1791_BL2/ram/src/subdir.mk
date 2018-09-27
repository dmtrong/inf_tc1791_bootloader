################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
S_UPPER_SRCS += \
../src/Loader2.S 

OBJS += \
./src/Loader2.o 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.S
	@echo 'Building file: $<'
	@echo 'Invoking: TriCore Assembler'
	"$(TRICORE_TOOLS)/bin/tricore-g++" -c -I"../h" -Os -mcpu=tc1791 -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


