################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/gui/windows/jstick.c 

OBJS += \
./src/gui/windows/jstick.o 

C_DEPS += \
./src/gui/windows/jstick.d 


# Each subdirectory must supply rules for building sources it contributes
src/gui/windows/%.o: ../src/gui/windows/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	x86_64-w64-mingw32-gcc -DSDL -DGLEW_STATIC -I/home/fhorse/sviluppo/personale/punes/src -I/home/fhorse/sviluppo/personale/punes/src/core -I/home/fhorse/sviluppo/personale/punes/src/gui -I/home/fhorse/sviluppo/personale/punes/src/gui/windows -I/home/fhorse/sviluppo/personale/punes/src/video/sdl -I/usr/x86_64-w64-mingw32/usr/include/SDL -I/usr/x86_64-w64-mingw32/usr/include/qt4 -I/home/fhorse/sviluppo/personale/punes/misc/DXSDK/Include -I/home/fhorse/sviluppo/personale/punes/misc/DXSDK/vc10/include -O3 -Wall -ffast-math -mmmx -msse -msse2 -msse3 -mfpmath=sse -c -fmessage-length=0 -finline-functions -Winline -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


