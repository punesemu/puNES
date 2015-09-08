################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/c++/xBRZ/xbrz.cpp \
../src/c++/xBRZ/xbrz_wrap.cpp 

OBJS += \
./src/c++/xBRZ/xbrz.o \
./src/c++/xBRZ/xbrz_wrap.o 

CPP_DEPS += \
./src/c++/xBRZ/xbrz.d \
./src/c++/xBRZ/xbrz_wrap.d 


# Each subdirectory must supply rules for building sources it contributes
src/c++/xBRZ/%.o: ../src/c++/xBRZ/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	x86_64-glibc2.12.2-linux-gnu-g++ -std=c++11 -DSDL -DGLEW_STATIC -I../src -I../src/core -I../src/gui -I../src/gui/linux -I../src/video/sdl -I/usr/x86_64-glibc2.12.2-linux-gnu/usr/include/SDL -I/usr/x86_64-glibc2.12.2-linux-gnu/usr/include/qt4 -O3 -Wall -ffast-math -mmmx -msse -msse2 -msse3 -mfpmath=sse -c -fmessage-length=0 -finline-functions --param inline-unit-growth=200 --param large-function-growth=500 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


