################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/sdl/gui/gtk2/cfg_apu_channels.c \
../src/sdl/gui/gtk2/cfg_input.c \
../src/sdl/gui/gtk2/cfg_std_pad.c \
../src/sdl/gui/gtk2/gtk2.c \
../src/sdl/gui/gtk2/gtk_wrap.c \
../src/sdl/gui/gtk2/joystick.c \
../src/sdl/gui/gtk2/keyboard.c \
../src/sdl/gui/gtk2/snd.c \
../src/sdl/gui/gtk2/uncompress_selection.c 

OBJS += \
./src/sdl/gui/gtk2/cfg_apu_channels.o \
./src/sdl/gui/gtk2/cfg_input.o \
./src/sdl/gui/gtk2/cfg_std_pad.o \
./src/sdl/gui/gtk2/gtk2.o \
./src/sdl/gui/gtk2/gtk_wrap.o \
./src/sdl/gui/gtk2/joystick.o \
./src/sdl/gui/gtk2/keyboard.o \
./src/sdl/gui/gtk2/snd.o \
./src/sdl/gui/gtk2/uncompress_selection.o 

C_DEPS += \
./src/sdl/gui/gtk2/cfg_apu_channels.d \
./src/sdl/gui/gtk2/cfg_input.d \
./src/sdl/gui/gtk2/cfg_std_pad.d \
./src/sdl/gui/gtk2/gtk2.d \
./src/sdl/gui/gtk2/gtk_wrap.d \
./src/sdl/gui/gtk2/joystick.d \
./src/sdl/gui/gtk2/keyboard.d \
./src/sdl/gui/gtk2/snd.d \
./src/sdl/gui/gtk2/uncompress_selection.d 


# Each subdirectory must supply rules for building sources it contributes
src/sdl/gui/gtk2/%.o: ../src/sdl/gui/gtk2/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	i686-pc-linux-gnu-gcc -DGTK -DSDL -DGLEW_STATIC -I../src -I../src/sdl -I/usr/i686-pc-linux-gnu/usr/include/SDL -O3 -Wall -ffast-math -msse2 -mfpmath=sse -c -fmessage-length=0 -finline-functions -Winline `i686-pc-linux-gnu-pkg-config --libs --cflags gtk+-2.0` -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


