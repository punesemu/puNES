################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/filters/video/xBRZ/xbrz.c \
../src/filters/video/xBRZ/xbrz_wrap.c 

OBJS += \
./src/filters/video/xBRZ/xbrz.o \
./src/filters/video/xBRZ/xbrz_wrap.o 

C_DEPS += \
./src/filters/video/xBRZ/xbrz.d \
./src/filters/video/xBRZ/xbrz_wrap.d 


# Each subdirectory must supply rules for building sources it contributes
src/filters/video/xBRZ/%.o: ../src/filters/video/xBRZ/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	i686-pc-linux-gnu-g++ -std=c++11 -DGTK -DSDL -DGLEW_STATIC -DNDEBUG -I../src -I../src/sdl -I/usr/i686-pc-linux-gnu/usr/include/SDL -O3 -Wall -ffast-math -msse2 -mfpmath=sse -c -fmessage-length=0 -finline-functions --param inline-unit-growth=100 -Winline `i686-pc-linux-gnu-pkg-config --libs --cflags gtk+-2.0` -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


