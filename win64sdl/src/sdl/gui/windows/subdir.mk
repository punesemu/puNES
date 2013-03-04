################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/sdl/gui/windows/cfginput.c \
../src/sdl/gui/windows/cfgstdctrl.c \
../src/sdl/gui/windows/joystick.c \
../src/sdl/gui/windows/keyboard.c \
../src/sdl/gui/windows/win.c 

OBJS += \
./src/sdl/gui/windows/cfginput.o \
./src/sdl/gui/windows/cfgstdctrl.o \
./src/sdl/gui/windows/joystick.o \
./src/sdl/gui/windows/keyboard.o \
./src/sdl/gui/windows/win.o 

C_DEPS += \
./src/sdl/gui/windows/cfginput.d \
./src/sdl/gui/windows/cfgstdctrl.d \
./src/sdl/gui/windows/joystick.d \
./src/sdl/gui/windows/keyboard.d \
./src/sdl/gui/windows/win.d 


# Each subdirectory must supply rules for building sources it contributes
src/sdl/gui/windows/%.o: ../src/sdl/gui/windows/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	x86_64-w64-mingw32-gcc -DMINGW64 -DSDL -DGLEW_STATIC -I../src -I../src/sdl -I/usr/x86_64-w64-mingw32/usr/include/SDL -O3 -Wall -ffast-math -c -fmessage-length=0 -finline-functions -Winline -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


