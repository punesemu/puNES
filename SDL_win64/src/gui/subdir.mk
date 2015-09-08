################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../src/gui/application.cpp \
../src/gui/cheatObject.cpp \
../src/gui/dlgApuChannels.cpp \
../src/gui/dlgCheats.cpp \
../src/gui/dlgInput.cpp \
../src/gui/dlgOverscanBorders.cpp \
../src/gui/dlgStdPad.cpp \
../src/gui/dlgUncomp.cpp \
../src/gui/mainWindow.cpp \
../src/gui/pStyle.cpp \
../src/gui/qt.cpp \
../src/gui/sbarWidget.cpp \
../src/gui/screenWidget.cpp \
../src/gui/settings.cpp \
../src/gui/settingsObject.cpp 

OBJS += \
./src/gui/application.o \
./src/gui/cheatObject.o \
./src/gui/dlgApuChannels.o \
./src/gui/dlgCheats.o \
./src/gui/dlgInput.o \
./src/gui/dlgOverscanBorders.o \
./src/gui/dlgStdPad.o \
./src/gui/dlgUncomp.o \
./src/gui/mainWindow.o \
./src/gui/pStyle.o \
./src/gui/qt.o \
./src/gui/sbarWidget.o \
./src/gui/screenWidget.o \
./src/gui/settings.o \
./src/gui/settingsObject.o 

CPP_DEPS += \
./src/gui/application.d \
./src/gui/cheatObject.d \
./src/gui/dlgApuChannels.d \
./src/gui/dlgCheats.d \
./src/gui/dlgInput.d \
./src/gui/dlgOverscanBorders.d \
./src/gui/dlgStdPad.d \
./src/gui/dlgUncomp.d \
./src/gui/mainWindow.d \
./src/gui/pStyle.d \
./src/gui/qt.d \
./src/gui/sbarWidget.d \
./src/gui/screenWidget.d \
./src/gui/settings.d \
./src/gui/settingsObject.d 


# Each subdirectory must supply rules for building sources it contributes
src/gui/%.o: ../src/gui/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	x86_64-w64-mingw32-g++ -std=c++11 -DSDL -DGLEW_STATIC -I/home/fhorse/sviluppo/personale/punes/src -I/home/fhorse/sviluppo/personale/punes/src/core -I/home/fhorse/sviluppo/personale/punes/src/gui -I/home/fhorse/sviluppo/personale/punes/src/gui/windows -I/home/fhorse/sviluppo/personale/punes/src/video/sdl -I/usr/x86_64-w64-mingw32/usr/include/SDL -I/usr/x86_64-w64-mingw32/usr/include/qt4 -I/home/fhorse/sviluppo/personale/punes/misc/DXSDK/Include -I/home/fhorse/sviluppo/personale/punes/misc/DXSDK/vc10/include -O3 -Wall -ffast-math -mmmx -msse -msse2 -msse3 -mfpmath=sse -c -fmessage-length=0 -finline-functions --param inline-unit-growth=200 --param large-function-growth=500 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


