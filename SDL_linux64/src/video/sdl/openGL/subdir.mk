################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/video/sdl/openGL/cube3d.c \
../src/video/sdl/openGL/no_effect.c 

OBJS += \
./src/video/sdl/openGL/cube3d.o \
./src/video/sdl/openGL/no_effect.o 

C_DEPS += \
./src/video/sdl/openGL/cube3d.d \
./src/video/sdl/openGL/no_effect.d 


# Each subdirectory must supply rules for building sources it contributes
src/video/sdl/openGL/%.o: ../src/video/sdl/openGL/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	x86_64-glibc2.12.2-linux-gnu-gcc -DSDL -DGLEW_STATIC -I../src -I../src/core -I../src/gui -I../src/gui/linux -I../src/video/sdl -I/usr/x86_64-glibc2.12.2-linux-gnu/usr/include/SDL -I/usr/x86_64-glibc2.12.2-linux-gnu/usr/include/qt4 -O3 -Wall -ffast-math -mmmx -msse -msse2 -msse3 -mfpmath=sse -c -fmessage-length=0 -finline-functions -Winline -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


