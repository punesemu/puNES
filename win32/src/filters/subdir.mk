################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/filters/bilinear.c \
../src/filters/hq2x.c \
../src/filters/hq3x.c \
../src/filters/hq4x.c \
../src/filters/hqx.c \
../src/filters/nes_ntsc.c \
../src/filters/ntsc.c \
../src/filters/scale.c \
../src/filters/scale2x.c 

OBJS += \
./src/filters/bilinear.o \
./src/filters/hq2x.o \
./src/filters/hq3x.o \
./src/filters/hq4x.o \
./src/filters/hqx.o \
./src/filters/nes_ntsc.o \
./src/filters/ntsc.o \
./src/filters/scale.o \
./src/filters/scale2x.o 

C_DEPS += \
./src/filters/bilinear.d \
./src/filters/hq2x.d \
./src/filters/hq3x.d \
./src/filters/hq4x.d \
./src/filters/hqx.d \
./src/filters/nes_ntsc.d \
./src/filters/ntsc.d \
./src/filters/scale.d \
./src/filters/scale2x.d 


# Each subdirectory must supply rules for building sources it contributes
src/filters/%.o: ../src/filters/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	i686-mingw32-gcc -DOPENGL -DMINGW32 -DGLEW_STATIC -I../src -I/usr/i686-mingw32/usr/include/SDL -O3 -Wall -ffast-math -c -fmessage-length=0 -finline-functions -Winline -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


