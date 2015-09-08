################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/audio/alsa/snd.c 

OBJS += \
./src/audio/alsa/snd.o 

C_DEPS += \
./src/audio/alsa/snd.d 


# Each subdirectory must supply rules for building sources it contributes
src/audio/alsa/%.o: ../src/audio/alsa/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	x86_64-glibc2.12.2-linux-gnu-gcc -DSDL -DGLEW_STATIC -I../src -I../src/core -I../src/gui -I../src/gui/linux -I../src/video/sdl -I/usr/x86_64-glibc2.12.2-linux-gnu/usr/include/SDL -I/usr/x86_64-glibc2.12.2-linux-gnu/usr/include/qt4 -O3 -Wall -ffast-math -mmmx -msse -msse2 -msse3 -mfpmath=sse -c -fmessage-length=0 -finline-functions -Winline -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


