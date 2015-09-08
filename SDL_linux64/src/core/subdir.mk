################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/core/apu.c \
../src/core/cheat.c \
../src/core/cmd_line.c \
../src/core/cpu.c \
../src/core/emu.c \
../src/core/external_calls.c \
../src/core/fds.c \
../src/core/fps.c \
../src/core/ines.c \
../src/core/input.c \
../src/core/irqA12.c \
../src/core/irql2f.c \
../src/core/main.c \
../src/core/mappers.c \
../src/core/overscan.c \
../src/core/palette.c \
../src/core/ppu.c \
../src/core/recent_roms.c \
../src/core/save_slot.c \
../src/core/sha1.c \
../src/core/tas.c \
../src/core/text.c \
../src/core/timeline.c \
../src/core/uncompress.c \
../src/core/unif.c 

OBJS += \
./src/core/apu.o \
./src/core/cheat.o \
./src/core/cmd_line.o \
./src/core/cpu.o \
./src/core/emu.o \
./src/core/external_calls.o \
./src/core/fds.o \
./src/core/fps.o \
./src/core/ines.o \
./src/core/input.o \
./src/core/irqA12.o \
./src/core/irql2f.o \
./src/core/main.o \
./src/core/mappers.o \
./src/core/overscan.o \
./src/core/palette.o \
./src/core/ppu.o \
./src/core/recent_roms.o \
./src/core/save_slot.o \
./src/core/sha1.o \
./src/core/tas.o \
./src/core/text.o \
./src/core/timeline.o \
./src/core/uncompress.o \
./src/core/unif.o 

C_DEPS += \
./src/core/apu.d \
./src/core/cheat.d \
./src/core/cmd_line.d \
./src/core/cpu.d \
./src/core/emu.d \
./src/core/external_calls.d \
./src/core/fds.d \
./src/core/fps.d \
./src/core/ines.d \
./src/core/input.d \
./src/core/irqA12.d \
./src/core/irql2f.d \
./src/core/main.d \
./src/core/mappers.d \
./src/core/overscan.d \
./src/core/palette.d \
./src/core/ppu.d \
./src/core/recent_roms.d \
./src/core/save_slot.d \
./src/core/sha1.d \
./src/core/tas.d \
./src/core/text.d \
./src/core/timeline.d \
./src/core/uncompress.d \
./src/core/unif.d 


# Each subdirectory must supply rules for building sources it contributes
src/core/%.o: ../src/core/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	x86_64-glibc2.12.2-linux-gnu-gcc -DSDL -DGLEW_STATIC -I../src -I../src/core -I../src/gui -I../src/gui/linux -I../src/video/sdl -I/usr/x86_64-glibc2.12.2-linux-gnu/usr/include/SDL -I/usr/x86_64-glibc2.12.2-linux-gnu/usr/include/qt4 -O3 -Wall -ffast-math -mmmx -msse -msse2 -msse3 -mfpmath=sse -c -fmessage-length=0 -finline-functions -Winline -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


