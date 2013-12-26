################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/sdl/gui/windows/cfg_apu_channels.c \
../src/sdl/gui/windows/cfg_input.c \
../src/sdl/gui/windows/cfg_std_pad.c \
../src/sdl/gui/windows/joystick.c \
../src/sdl/gui/windows/keyboard.c \
../src/sdl/gui/windows/snd.c \
../src/sdl/gui/windows/uncompress_selection.c \
../src/sdl/gui/windows/win.c 

OBJS += \
./src/sdl/gui/windows/cfg_apu_channels.o \
./src/sdl/gui/windows/cfg_input.o \
./src/sdl/gui/windows/cfg_std_pad.o \
./src/sdl/gui/windows/joystick.o \
./src/sdl/gui/windows/keyboard.o \
./src/sdl/gui/windows/snd.o \
./src/sdl/gui/windows/uncompress_selection.o \
./src/sdl/gui/windows/win.o 

C_DEPS += \
./src/sdl/gui/windows/cfg_apu_channels.d \
./src/sdl/gui/windows/cfg_input.d \
./src/sdl/gui/windows/cfg_std_pad.d \
./src/sdl/gui/windows/joystick.d \
./src/sdl/gui/windows/keyboard.d \
./src/sdl/gui/windows/snd.d \
./src/sdl/gui/windows/uncompress_selection.d \
./src/sdl/gui/windows/win.d 


# Each subdirectory must supply rules for building sources it contributes
src/sdl/gui/windows/%.o: ../src/sdl/gui/windows/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	i686-mingw32-gcc -DMINGW32 -DSDL -DGLEW_STATIC -I../src -I../src/sdl -I/usr/i686-mingw32/usr/include/SDL -I/home/fhorse/sviluppo/personale/punes/misc/DXSDK/Include -I/home/fhorse/sviluppo/personale/punes/misc/DXSDK/vc10/include -O3 -Wall -ffast-math -c -fmessage-length=0 -finline-functions -Winline -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


