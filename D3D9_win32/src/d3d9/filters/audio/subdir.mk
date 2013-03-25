################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/d3d9/filters/audio/original.c 

OBJS += \
./src/d3d9/filters/audio/original.o 

C_DEPS += \
./src/d3d9/filters/audio/original.d 


# Each subdirectory must supply rules for building sources it contributes
src/d3d9/filters/audio/%.o: ../src/d3d9/filters/audio/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	i686-mingw32-gcc -DMINGW32 -DD3D9 -I"/home/fhorse/sviluppo/personale/punes/src" -I"/home/fhorse/sviluppo/personale/punes/src/d3d9" -I/home/fhorse/sviluppo/personale/punes/misc/DXSDK/Include -I/home/fhorse/sviluppo/personale/punes/misc/DXSDK/vc10/include -O3 -Wall -ffast-math -c -fmessage-length=0 -finline-functions -Winline -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


