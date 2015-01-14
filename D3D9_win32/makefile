################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/video/d3d9/subdir.mk
-include src/gui/windows/subdir.mk
-include src/gui/subdir.mk
-include src/core/mappers/subdir.mk
-include src/core/filters/video/subdir.mk
-include src/core/filters/audio/subdir.mk
-include src/core/subdir.mk
-include src/c++/xBRZ/subdir.mk
-include src/c++/l7zip/subdir.mk
-include src/audio/xaudio/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: pre-build main-build

# Main-build Target
main-build: punes32.d3d9.exe

# Tool invocations
punes32.d3d9.exe: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: Cross G++ Linker'
	i686-w64-mingw32-g++ -L/usr/i686-w64-mingw32/usr/lib/qt4 -L"/home/fhorse/sviluppo/personale/punes/misc/lib/win32" -Wl,-subsystem,console -static-libgcc -static-libstdc++ -o "punes32.d3d9.exe" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '
	$(MAKE) --no-print-directory post-build

# Other Targets
clean:
	-$(RM) $(C++_DEPS)$(OBJS)$(C_DEPS)$(CC_DEPS)$(CPP_DEPS)$(EXECUTABLES)$(CXX_DEPS)$(C_UPPER_DEPS) punes32.d3d9.exe
	-@echo ' '

pre-build:
	-/home/fhorse/sviluppo/personale/bin/buildBefore -q
	-@echo ' '

post-build:
	-../../bin/build_increment
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY: main-build pre-build post-build

-include ../makefile.targets
