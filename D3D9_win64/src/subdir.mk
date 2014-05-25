################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/apu.c \
../src/audio_quality.c \
../src/blip_buf.c \
../src/cfg_file.c \
../src/cmd_line.c \
../src/cpu.c \
../src/emu.c \
../src/external_calls.c \
../src/fds.c \
../src/fps.c \
../src/gamegenie.c \
../src/ines.c \
../src/input.c \
../src/irqA12.c \
../src/irql2f.c \
../src/main.c \
../src/mappers.c \
../src/overscan.c \
../src/ppu.c \
../src/recent_roms.c \
../src/save_slot.c \
../src/sha1.c \
../src/tas.c \
../src/text.c \
../src/timeline.c \
../src/uncompress.c \
../src/unif.c 

OBJS += \
./src/apu.o \
./src/audio_quality.o \
./src/blip_buf.o \
./src/cfg_file.o \
./src/cmd_line.o \
./src/cpu.o \
./src/emu.o \
./src/external_calls.o \
./src/fds.o \
./src/fps.o \
./src/gamegenie.o \
./src/ines.o \
./src/input.o \
./src/irqA12.o \
./src/irql2f.o \
./src/main.o \
./src/mappers.o \
./src/overscan.o \
./src/ppu.o \
./src/recent_roms.o \
./src/save_slot.o \
./src/sha1.o \
./src/tas.o \
./src/text.o \
./src/timeline.o \
./src/uncompress.o \
./src/unif.o 

C_DEPS += \
./src/apu.d \
./src/audio_quality.d \
./src/blip_buf.d \
./src/cfg_file.d \
./src/cmd_line.d \
./src/cpu.d \
./src/emu.d \
./src/external_calls.d \
./src/fds.d \
./src/fps.d \
./src/gamegenie.d \
./src/ines.d \
./src/input.d \
./src/irqA12.d \
./src/irql2f.d \
./src/main.d \
./src/mappers.d \
./src/overscan.d \
./src/ppu.d \
./src/recent_roms.d \
./src/save_slot.d \
./src/sha1.d \
./src/tas.d \
./src/text.d \
./src/timeline.d \
./src/uncompress.d \
./src/unif.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	x86_64-w64-mingw32-gcc -DMINGW64 -DD3D9 -I../src -I../src/d3d9 -I/home/fhorse/sviluppo/personale/punes/misc/DXSDK/Include -I/home/fhorse/sviluppo/personale/punes/misc/DXSDK/vc10/include -O3 -Wall -ffast-math -c -fmessage-length=0 -finline-functions -Winline -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


