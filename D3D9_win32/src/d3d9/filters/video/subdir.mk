################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/d3d9/filters/video/hq2x.c \
../src/d3d9/filters/video/hq3x.c \
../src/d3d9/filters/video/hq4x.c \
../src/d3d9/filters/video/hqx.c \
../src/d3d9/filters/video/nes_ntsc.c \
../src/d3d9/filters/video/ntsc.c \
../src/d3d9/filters/video/scale.c 

OBJS += \
./src/d3d9/filters/video/hq2x.o \
./src/d3d9/filters/video/hq3x.o \
./src/d3d9/filters/video/hq4x.o \
./src/d3d9/filters/video/hqx.o \
./src/d3d9/filters/video/nes_ntsc.o \
./src/d3d9/filters/video/ntsc.o \
./src/d3d9/filters/video/scale.o 

C_DEPS += \
./src/d3d9/filters/video/hq2x.d \
./src/d3d9/filters/video/hq3x.d \
./src/d3d9/filters/video/hq4x.d \
./src/d3d9/filters/video/hqx.d \
./src/d3d9/filters/video/nes_ntsc.d \
./src/d3d9/filters/video/ntsc.d \
./src/d3d9/filters/video/scale.d 


# Each subdirectory must supply rules for building sources it contributes
src/d3d9/filters/video/%.o: ../src/d3d9/filters/video/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	i686-mingw32-gcc -DMINGW32 -DD3D9 -I../src -I../src/d3d9 -I../misc/DXSDK/Include -I/usr/i686-mingw32/mingw/include -O3 -Wall -ffast-math -c -fmessage-length=0 -finline-functions -Winline -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


