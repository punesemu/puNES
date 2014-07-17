################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/filters/video/bilinear.c \
../src/filters/video/hq2x.c \
../src/filters/video/hq3x.c \
../src/filters/video/hq4x.c \
../src/filters/video/hqx.c \
../src/filters/video/nes_ntsc.c \
../src/filters/video/ntsc.c \
../src/filters/video/scale.c \
../src/filters/video/scale2x.c \
../src/filters/video/xBRZ.c 

OBJS += \
./src/filters/video/bilinear.o \
./src/filters/video/hq2x.o \
./src/filters/video/hq3x.o \
./src/filters/video/hq4x.o \
./src/filters/video/hqx.o \
./src/filters/video/nes_ntsc.o \
./src/filters/video/ntsc.o \
./src/filters/video/scale.o \
./src/filters/video/scale2x.o \
./src/filters/video/xBRZ.o 

C_DEPS += \
./src/filters/video/bilinear.d \
./src/filters/video/hq2x.d \
./src/filters/video/hq3x.d \
./src/filters/video/hq4x.d \
./src/filters/video/hqx.d \
./src/filters/video/nes_ntsc.d \
./src/filters/video/ntsc.d \
./src/filters/video/scale.d \
./src/filters/video/scale2x.d \
./src/filters/video/xBRZ.d 


# Each subdirectory must supply rules for building sources it contributes
src/filters/video/%.o: ../src/filters/video/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	i686-mingw32-gcc -DMINGW32 -DSDL -DGLEW_STATIC -I../src -I../src/sdl -I/usr/i686-mingw32/usr/include/SDL -I/home/fhorse/sviluppo/personale/punes/misc/DXSDK/Include -I/home/fhorse/sviluppo/personale/punes/misc/DXSDK/vc10/include -O3 -Wall -ffast-math -c -fmessage-length=0 -finline-functions -Winline -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


