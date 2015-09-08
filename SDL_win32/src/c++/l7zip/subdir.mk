################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/c++/l7zip/l7z.cpp 

OBJS += \
./src/c++/l7zip/l7z.o 

CPP_DEPS += \
./src/c++/l7zip/l7z.d 


# Each subdirectory must supply rules for building sources it contributes
src/c++/l7zip/%.o: ../src/c++/l7zip/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	i686-w64-mingw32-g++ -std=c++11 -DSDL -DGLEW_STATIC -I../src -I../src/core -I../src/gui -I../src/gui/windows -I../src/video/sdl -I/usr/i686-w64-mingw32/usr/include/SDL -I/usr/i686-w64-mingw32/usr/include/qt4 -I/home/fhorse/sviluppo/personale/punes/misc/DXSDK/Include -I/home/fhorse/sviluppo/personale/punes/misc/DXSDK/vc10/include -O3 -Wall -ffast-math -mmmx -msse -msse2 -msse3 -mfpmath=sse -c -fmessage-length=0 -finline-functions --param inline-unit-growth=200 --param large-function-growth=500 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


