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
	x86_64-w64-mingw32-gcc -DMINGW64 -DGLEW_STATIC -I../src -I/usr/x86_64-w64-mingw32/usr/include/SDL -O3 -Wall -ffast-math -c -fmessage-length=0 -finline-functions -Winline -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


