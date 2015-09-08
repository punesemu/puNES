################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/video/filters/hq2x.c \
../src/video/filters/hq3x.c \
../src/video/filters/hq4x.c \
../src/video/filters/hqx.c \
../src/video/filters/nes_ntsc.c \
../src/video/filters/ntsc.c \
../src/video/filters/scale.c \
../src/video/filters/scale2x.c \
../src/video/filters/xBRZ.c 

OBJS += \
./src/video/filters/hq2x.o \
./src/video/filters/hq3x.o \
./src/video/filters/hq4x.o \
./src/video/filters/hqx.o \
./src/video/filters/nes_ntsc.o \
./src/video/filters/ntsc.o \
./src/video/filters/scale.o \
./src/video/filters/scale2x.o \
./src/video/filters/xBRZ.o 

C_DEPS += \
./src/video/filters/hq2x.d \
./src/video/filters/hq3x.d \
./src/video/filters/hq4x.d \
./src/video/filters/hqx.d \
./src/video/filters/nes_ntsc.d \
./src/video/filters/ntsc.d \
./src/video/filters/scale.d \
./src/video/filters/scale2x.d \
./src/video/filters/xBRZ.d 


# Each subdirectory must supply rules for building sources it contributes
src/video/filters/%.o: ../src/video/filters/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	x86_64-w64-mingw32-gcc -DSDL -DGLEW_STATIC -I/home/fhorse/sviluppo/personale/punes/src -I/home/fhorse/sviluppo/personale/punes/src/core -I/home/fhorse/sviluppo/personale/punes/src/gui -I/home/fhorse/sviluppo/personale/punes/src/gui/windows -I/home/fhorse/sviluppo/personale/punes/src/video/sdl -I/usr/x86_64-w64-mingw32/usr/include/SDL -I/usr/x86_64-w64-mingw32/usr/include/qt4 -I/home/fhorse/sviluppo/personale/punes/misc/DXSDK/Include -I/home/fhorse/sviluppo/personale/punes/misc/DXSDK/vc10/include -O3 -Wall -ffast-math -mmmx -msse -msse2 -msse3 -mfpmath=sse -c -fmessage-length=0 -finline-functions -Winline -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


