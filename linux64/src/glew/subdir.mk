################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/glew/glew.c 

OBJS += \
./src/glew/glew.o 

C_DEPS += \
./src/glew/glew.d 


# Each subdirectory must supply rules for building sources it contributes
src/glew/%.o: ../src/glew/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	x86_64-glibc2.12.2-linux-gnu-gcc -DGTK -DGLEW_STATIC -I../src -I/usr/x86_64-glibc2.12.2-linux-gnu/usr/include/SDL -O3 -Wall -ffast-math -msse2 -mfpmath=sse -c -fmessage-length=0 -finline-functions -Winline `x86_64-glibc2.12.2-linux-gnu-pkg-config --cflags gtk+-2.0` -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


