################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/gui/gtk2/cfginput.c \
../src/gui/gtk2/cfgstdctrl.c \
../src/gui/gtk2/gtk2.c \
../src/gui/gtk2/joystick.c \
../src/gui/gtk2/keyboard.c 

OBJS += \
./src/gui/gtk2/cfginput.o \
./src/gui/gtk2/cfgstdctrl.o \
./src/gui/gtk2/gtk2.o \
./src/gui/gtk2/joystick.o \
./src/gui/gtk2/keyboard.o 

C_DEPS += \
./src/gui/gtk2/cfginput.d \
./src/gui/gtk2/cfgstdctrl.d \
./src/gui/gtk2/gtk2.d \
./src/gui/gtk2/joystick.d \
./src/gui/gtk2/keyboard.d 


# Each subdirectory must supply rules for building sources it contributes
src/gui/gtk2/%.o: ../src/gui/gtk2/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	gcc -DOPENGL -DGTK -DGLEW_STATIC -I../src -I/usr/include/SDL -O3 -Wall -ffast-math -msse2 -mfpmath=sse -c -fmessage-length=0 -finline-functions -Winline `pkg-config --libs --cflags gtk+-2.0` -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


