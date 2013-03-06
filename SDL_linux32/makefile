################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/sdl/openGL/subdir.mk
-include src/sdl/gui/gtk2/menu/subdir.mk
-include src/sdl/gui/gtk2/subdir.mk
-include src/sdl/glew/subdir.mk
-include src/sdl/filters/video/subdir.mk
-include src/sdl/filters/audio/subdir.mk
-include src/sdl/subdir.mk
-include src/mappers/subdir.mk
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
main-build: punes32

# Tool invocations
punes32: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: Cross GCC Linker'
	i686-pc-linux-gnu-gcc -L/usr/i686-pc-linux-gnu/usr/lib -export-dynamic -o "punes32" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(EXECUTABLES) punes32
	-@echo ' '

pre-build:
	-../src/sdl/gui/gtk2/glade/glade_to_h ../src/sdl/gui/gtk2/glade
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY: main-build pre-build

-include ../makefile.targets
