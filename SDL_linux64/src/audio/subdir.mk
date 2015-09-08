################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/audio/blip_buf.c \
../src/audio/blipbuf.c \
../src/audio/channels.c \
../src/audio/delay.c \
../src/audio/handler.c \
../src/audio/mono.c \
../src/audio/original.c \
../src/audio/panning.c \
../src/audio/quality.c 

OBJS += \
./src/audio/blip_buf.o \
./src/audio/blipbuf.o \
./src/audio/channels.o \
./src/audio/delay.o \
./src/audio/handler.o \
./src/audio/mono.o \
./src/audio/original.o \
./src/audio/panning.o \
./src/audio/quality.o 

C_DEPS += \
./src/audio/blip_buf.d \
./src/audio/blipbuf.d \
./src/audio/channels.d \
./src/audio/delay.d \
./src/audio/handler.d \
./src/audio/mono.d \
./src/audio/original.d \
./src/audio/panning.d \
./src/audio/quality.d 


# Each subdirectory must supply rules for building sources it contributes
src/audio/%.o: ../src/audio/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	x86_64-glibc2.12.2-linux-gnu-gcc -DSDL -DGLEW_STATIC -I../src -I../src/core -I../src/gui -I../src/gui/linux -I../src/video/sdl -I/usr/x86_64-glibc2.12.2-linux-gnu/usr/include/SDL -I/usr/x86_64-glibc2.12.2-linux-gnu/usr/include/qt4 -O3 -Wall -ffast-math -mmmx -msse -msse2 -msse3 -mfpmath=sse -c -fmessage-length=0 -finline-functions -Winline -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


