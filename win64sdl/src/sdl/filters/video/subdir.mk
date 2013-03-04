################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/sdl/filters/video/bilinear.c \
../src/sdl/filters/video/hq2x.c \
../src/sdl/filters/video/hq3x.c \
../src/sdl/filters/video/hq4x.c \
../src/sdl/filters/video/hqx.c \
../src/sdl/filters/video/nes_ntsc.c \
../src/sdl/filters/video/ntsc.c \
../src/sdl/filters/video/scale.c \
../src/sdl/filters/video/scale2x.c 

OBJS += \
./src/sdl/filters/video/bilinear.o \
./src/sdl/filters/video/hq2x.o \
./src/sdl/filters/video/hq3x.o \
./src/sdl/filters/video/hq4x.o \
./src/sdl/filters/video/hqx.o \
./src/sdl/filters/video/nes_ntsc.o \
./src/sdl/filters/video/ntsc.o \
./src/sdl/filters/video/scale.o \
./src/sdl/filters/video/scale2x.o 

C_DEPS += \
./src/sdl/filters/video/bilinear.d \
./src/sdl/filters/video/hq2x.d \
./src/sdl/filters/video/hq3x.d \
./src/sdl/filters/video/hq4x.d \
./src/sdl/filters/video/hqx.d \
./src/sdl/filters/video/nes_ntsc.d \
./src/sdl/filters/video/ntsc.d \
./src/sdl/filters/video/scale.d \
./src/sdl/filters/video/scale2x.d 


# Each subdirectory must supply rules for building sources it contributes
src/sdl/filters/video/%.o: ../src/sdl/filters/video/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	x86_64-w64-mingw32-gcc -DMINGW64 -DSDL -DGLEW_STATIC -I../src -I../src/sdl -I/usr/x86_64-w64-mingw32/usr/include/SDL -O3 -Wall -ffast-math -c -fmessage-length=0 -finline-functions -Winline -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


