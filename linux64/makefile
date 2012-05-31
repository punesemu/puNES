################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include src/openGL/subdir.mk
-include src/mappers/subdir.mk
-include src/gui/gtk2/menu/subdir.mk
-include src/gui/gtk2/subdir.mk
-include src/glew/subdir.mk
-include src/filters/subdir.mk
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
all: punes64

# Tool invocations
punes64: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: Cross GCC Linker'
	gcc `pkg-config --libs --cflags gtk+-2.0` -o "punes64" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(OBJS)$(C_DEPS)$(EXECUTABLES) punes64
	-@echo ' '

.PHONY: all clean dependents
.SECONDARY:

-include ../makefile.targets
