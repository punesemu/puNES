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
	i686-mingw32-g++ -std=c++11 -DMINGW32 -DD3D9 -DNDEBUG -I../src -I../src/d3d9 -I/home/fhorse/sviluppo/personale/punes/misc/DXSDK/Include -I/home/fhorse/sviluppo/personale/punes/misc/DXSDK/vc10/include -O3 -Wall -ffast-math -c -fmessage-length=0 -finline-functions --param inline-unit-growth=100 -Winline -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


