################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/sdl/filters/audio/blip.c \
../src/sdl/filters/audio/blip2.c \
../src/sdl/filters/audio/original.c 

OBJS += \
./src/sdl/filters/audio/blip.o \
./src/sdl/filters/audio/blip2.o \
./src/sdl/filters/audio/original.o 

C_DEPS += \
./src/sdl/filters/audio/blip.d \
./src/sdl/filters/audio/blip2.d \
./src/sdl/filters/audio/original.d 


# Each subdirectory must supply rules for building sources it contributes
src/sdl/filters/audio/%.o: ../src/sdl/filters/audio/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	x86_64-glibc2.12.2-linux-gnu-gcc -DGTK -DSDL -DGLEW_STATIC -I../src -I../src/sdl -I/usr/x86_64-glibc2.12.2-linux-gnu/usr/include/SDL -O3 -Wall -ffast-math -msse2 -mfpmath=sse -c -fmessage-length=0 -finline-functions -Winline `x86_64-glibc2.12.2-linux-gnu-pkg-config --cflags gtk+-2.0` -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


