################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/bsp.c \
../src/flash.c \
../src/main.c 

S_UPPER_SRCS += \
../src/crt0-tc1x.S 

OBJS += \
./src/bsp.o \
./src/crt0-tc1x.o \
./src/flash.o \
./src/main.o 

C_DEPS += \
./src/bsp.d \
./src/flash.d \
./src/main.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: TriCore C Compiler'
	"$(TRICORE_TOOLS)/bin/tricore-gcc" -c -I"../h" -fno-common -Os -fgcse-after-reload -ffast-math -funswitch-loops -fpredictive-commoning -ftree-vectorize -fipa-cp-clone -fpeel-loops -fmove-loop-invariants -frename-registers -fira-algorithm=priority -W -Wall -Wextra -Wdiv-by-zero -Warray-bounds -Wcast-align -Wignored-qualifiers -Wformat -Wformat-security -Wa,-ahlms=$(basename $(notdir $@)).lst -DTRIBOARD_TC1791 -fshort-double -mcpu=tc1791 -mversion-info -std=gnu99 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

src/%.o: ../src/%.S
	@echo 'Building file: $<'
	@echo 'Invoking: TriCore Assembler'
	"$(TRICORE_TOOLS)/bin/tricore-gcc" -c -I"../h" -mcpu=tc1791 -Wa,--insn32-only -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


