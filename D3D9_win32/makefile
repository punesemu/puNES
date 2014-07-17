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
main-build: punes32.d3d9.exe

# Tool invocations
punes32.d3d9.exe: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: Cross GCC Linker'
	i686-mingw32-g++ -L"/home/fhorse/sviluppo/personale/punes/misc/lib/win32" -L/home/fhorse/sviluppo/personale/punes/misc/DXSDK/Lib/x86 -Wl,-subsystem,console -static-libgcc -static-libstdc++ -o "punes32.d3d9.exe" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '
	$(MAKE) --no-print-directory post-build

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(EXECUTABLES) punes32.d3d9.exe
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
