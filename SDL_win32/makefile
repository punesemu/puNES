################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/sdl/openGL/subdir.mk
-include src/sdl/gui/windows/subdir.mk
-include src/sdl/glew/subdir.mk
-include src/sdl/subdir.mk
-include src/mappers/subdir.mk
-include src/l7zip/subdir.mk
-include src/filters/video/subdir.mk
-include src/filters/audio/subdir.mk
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
main-build: punes32.sdl.exe

# Tool invocations
punes32.sdl.exe: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: Cross GCC Linker'
	i686-mingw32-g++ -L"/home/fhorse/sviluppo/personale/punes/misc/lib/win32" -Wl,-subsystem,console -static-libgcc -static-libstdc++ -o "punes32.sdl.exe" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(EXECUTABLES)$(OBJS)$(C_DEPS) punes32.sdl.exe
	-@echo ' '

pre-build:
	-/home/fhorse/sviluppo/personale/bin/buildBefore -w
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY: main-build pre-build

-include ../makefile.targets
