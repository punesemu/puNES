################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/filters/audio/linear.c \
../src/filters/audio/none.c 

OBJS += \
./src/filters/audio/linear.o \
./src/filters/audio/none.o 

C_DEPS += \
./src/filters/audio/linear.d \
./src/filters/audio/none.d 


# Each subdirectory must supply rules for building sources it contributes
src/filters/audio/%.o: ../src/filters/audio/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	x86_64-w64-mingw32-gcc -DOPENGL -DMINGW64 -DGLEW_STATIC -I../src -I/usr/x86_64-w64-mingw32/usr/include/SDL -O3 -Wall -ffast-math -c -fmessage-length=0 -finline-functions -Winline -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


