################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/d3d9/cfg_file.c \
../src/d3d9/gfx.c \
../src/d3d9/joystick.c \
../src/d3d9/keyboard.c \
../src/d3d9/snd.c \
../src/d3d9/uncompress_selection.c \
../src/d3d9/win.c 

OBJS += \
./src/d3d9/cfg_file.o \
./src/d3d9/gfx.o \
./src/d3d9/joystick.o \
./src/d3d9/keyboard.o \
./src/d3d9/snd.o \
./src/d3d9/uncompress_selection.o \
./src/d3d9/win.o 

C_DEPS += \
./src/d3d9/cfg_file.d \
./src/d3d9/gfx.d \
./src/d3d9/joystick.d \
./src/d3d9/keyboard.d \
./src/d3d9/snd.d \
./src/d3d9/uncompress_selection.d \
./src/d3d9/win.d 


# Each subdirectory must supply rules for building sources it contributes
src/d3d9/%.o: ../src/d3d9/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	i686-mingw32-gcc -DMINGW32 -DD3D9 -I../src -I../src/d3d9 -I/home/fhorse/sviluppo/personale/punes/misc/DXSDK/Include -I/home/fhorse/sviluppo/personale/punes/misc/DXSDK/vc10/include -O3 -Wall -ffast-math -c -fmessage-length=0 -finline-functions -Winline -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


