################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/gui/windows/cfginput.c \
../src/gui/windows/cfgstdctrl.c \
../src/gui/windows/joystick.c \
../src/gui/windows/keyboard.c \
../src/gui/windows/win.c 

OBJS += \
./src/gui/windows/cfginput.o \
./src/gui/windows/cfgstdctrl.o \
./src/gui/windows/joystick.o \
./src/gui/windows/keyboard.o \
./src/gui/windows/win.o 

C_DEPS += \
./src/gui/windows/cfginput.d \
./src/gui/windows/cfgstdctrl.d \
./src/gui/windows/joystick.d \
./src/gui/windows/keyboard.d \
./src/gui/windows/win.d 


# Each subdirectory must supply rules for building sources it contributes
src/gui/windows/%.o: ../src/gui/windows/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	i686-mingw32-gcc -DOPENGL -DMINGW32 -DGLEW_STATIC -I../src -I/usr/i686-mingw32/usr/include/SDL -O3 -Wall -ffast-math -c -fmessage-length=0 -finline-functions -Winline -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


