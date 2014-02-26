################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/sdl/gfx.c \
../src/sdl/opengl.c 

OBJS += \
./src/sdl/gfx.o \
./src/sdl/opengl.o 

C_DEPS += \
./src/sdl/gfx.d \
./src/sdl/opengl.d 


# Each subdirectory must supply rules for building sources it contributes
src/sdl/%.o: ../src/sdl/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	x86_64-glibc2.12.2-linux-gnu-gcc -DGTK -DSDL -DGLEW_STATIC -I../src -I../src/sdl -I/usr/x86_64-glibc2.12.2-linux-gnu/usr/include/SDL -O3 -Wall -ffast-math -msse2 -mfpmath=sse -c -fmessage-length=0 -finline-functions -Winline `x86_64-glibc2.12.2-linux-gnu-pkg-config --cflags gtk+-2.0` -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


