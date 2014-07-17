################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/mappers/subdir.mk
-include src/l7zip/subdir.mk
-include src/filters/video/xBRZ/subdir.mk
-include src/filters/video/subdir.mk
-include src/filters/audio/subdir.mk
-include src/d3d9/subdir.mk
-include src/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: pre-build main-build

# Main-build Target
main-build: punes64.d3d9.exe

# Tool invocations
punes64.d3d9.exe: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: Cross GCC Linker'
	x86_64-w64-mingw32-g++ -L"/home/fhorse/sviluppo/personale/punes/misc/lib/win64" -Wl,-subsystem,console -static-libgcc -static-libstdc++ -o "punes64.d3d9.exe" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(EXECUTABLES) punes64.d3d9.exe
	-@echo ' '

pre-build:
	-/home/fhorse/sviluppo/personale/bin/buildBefore -a
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY: main-build pre-build

-include ../makefile.targets
