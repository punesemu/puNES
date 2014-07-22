################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/l7zip/l7z.c 

OBJS += \
./src/l7zip/l7z.o 

C_DEPS += \
./src/l7zip/l7z.d 


# Each subdirectory must supply rules for building sources it contributes
src/l7zip/%.o: ../src/l7zip/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	i686-mingw32-g++ -DMINGW32 -DD3D9 -I../src -I../src/d3d9 -I/home/fhorse/sviluppo/personale/punes/misc/DXSDK/Include -I/home/fhorse/sviluppo/personale/punes/misc/DXSDK/vc10/include -O3 -Wall -ffast-math -c -fmessage-length=0 -finline-functions -Winline -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


