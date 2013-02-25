################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/openGL/cube3d.c \
../src/openGL/no_effect.c 

OBJS += \
./src/openGL/cube3d.o \
./src/openGL/no_effect.o 

C_DEPS += \
./src/openGL/cube3d.d \
./src/openGL/no_effect.d 


# Each subdirectory must supply rules for building sources it contributes
src/openGL/%.o: ../src/openGL/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	x86_64-glibc2.12.2-linux-gnu-gcc -DGTK -DGLEW_STATIC -I../src -I/usr/x86_64-glibc2.12.2-linux-gnu/usr/include/SDL -O3 -Wall -ffast-math -msse2 -mfpmath=sse -c -fmessage-length=0 -finline-functions -Winline `x86_64-glibc2.12.2-linux-gnu-pkg-config --cflags gtk+-2.0` -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


