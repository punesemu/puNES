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
-include src/sdl/filters/audio/subdir.mk
-include src/sdl/subdir.mk
-include src/mappers/subdir.mk
-include src/filters/video/subdir.mk
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
main-build: punes32.exe

# Tool invocations
punes32.exe: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: Cross GCC Linker'
	i686-mingw32-gcc -Wl,-subsystem,console -o "punes32.exe" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(EXECUTABLES) punes32.exe
	-@echo ' '

pre-build:
	-i686-mingw32-windres -DOPENGL ../src/sdl/gui/windows/resources.rc src/sdl/gui/windows/resources.o
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY: main-build pre-build

-include ../makefile.targets
