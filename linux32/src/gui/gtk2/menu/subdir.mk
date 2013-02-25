################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/gui/gtk2/menu/menu.c \
../src/gui/gtk2/menu/menu_audio.c \
../src/gui/gtk2/menu/menu_audio_channels.c \
../src/gui/gtk2/menu/menu_audio_quality.c \
../src/gui/gtk2/menu/menu_audio_sample_rate.c \
../src/gui/gtk2/menu/menu_file.c \
../src/gui/gtk2/menu/menu_gamegenie.c \
../src/gui/gtk2/menu/menu_help.c \
../src/gui/gtk2/menu/menu_input.c \
../src/gui/gtk2/menu/menu_mode.c \
../src/gui/gtk2/menu/menu_nes.c \
../src/gui/gtk2/menu/menu_nes_fds.c \
../src/gui/gtk2/menu/menu_settings.c \
../src/gui/gtk2/menu/menu_state.c \
../src/gui/gtk2/menu/menu_video.c \
../src/gui/gtk2/menu/menu_video_effect.c \
../src/gui/gtk2/menu/menu_video_filter.c \
../src/gui/gtk2/menu/menu_video_fps.c \
../src/gui/gtk2/menu/menu_video_frame_skip.c \
../src/gui/gtk2/menu/menu_video_fullscreen.c \
../src/gui/gtk2/menu/menu_video_overscan.c \
../src/gui/gtk2/menu/menu_video_palette.c \
../src/gui/gtk2/menu/menu_video_rendering.c \
../src/gui/gtk2/menu/menu_video_size.c \
../src/gui/gtk2/menu/menu_video_vsync.c 

OBJS += \
./src/gui/gtk2/menu/menu.o \
./src/gui/gtk2/menu/menu_audio.o \
./src/gui/gtk2/menu/menu_audio_channels.o \
./src/gui/gtk2/menu/menu_audio_quality.o \
./src/gui/gtk2/menu/menu_audio_sample_rate.o \
./src/gui/gtk2/menu/menu_file.o \
./src/gui/gtk2/menu/menu_gamegenie.o \
./src/gui/gtk2/menu/menu_help.o \
./src/gui/gtk2/menu/menu_input.o \
./src/gui/gtk2/menu/menu_mode.o \
./src/gui/gtk2/menu/menu_nes.o \
./src/gui/gtk2/menu/menu_nes_fds.o \
./src/gui/gtk2/menu/menu_settings.o \
./src/gui/gtk2/menu/menu_state.o \
./src/gui/gtk2/menu/menu_video.o \
./src/gui/gtk2/menu/menu_video_effect.o \
./src/gui/gtk2/menu/menu_video_filter.o \
./src/gui/gtk2/menu/menu_video_fps.o \
./src/gui/gtk2/menu/menu_video_frame_skip.o \
./src/gui/gtk2/menu/menu_video_fullscreen.o \
./src/gui/gtk2/menu/menu_video_overscan.o \
./src/gui/gtk2/menu/menu_video_palette.o \
./src/gui/gtk2/menu/menu_video_rendering.o \
./src/gui/gtk2/menu/menu_video_size.o \
./src/gui/gtk2/menu/menu_video_vsync.o 

C_DEPS += \
./src/gui/gtk2/menu/menu.d \
./src/gui/gtk2/menu/menu_audio.d \
./src/gui/gtk2/menu/menu_audio_channels.d \
./src/gui/gtk2/menu/menu_audio_quality.d \
./src/gui/gtk2/menu/menu_audio_sample_rate.d \
./src/gui/gtk2/menu/menu_file.d \
./src/gui/gtk2/menu/menu_gamegenie.d \
./src/gui/gtk2/menu/menu_help.d \
./src/gui/gtk2/menu/menu_input.d \
./src/gui/gtk2/menu/menu_mode.d \
./src/gui/gtk2/menu/menu_nes.d \
./src/gui/gtk2/menu/menu_nes_fds.d \
./src/gui/gtk2/menu/menu_settings.d \
./src/gui/gtk2/menu/menu_state.d \
./src/gui/gtk2/menu/menu_video.d \
./src/gui/gtk2/menu/menu_video_effect.d \
./src/gui/gtk2/menu/menu_video_filter.d \
./src/gui/gtk2/menu/menu_video_fps.d \
./src/gui/gtk2/menu/menu_video_frame_skip.d \
./src/gui/gtk2/menu/menu_video_fullscreen.d \
./src/gui/gtk2/menu/menu_video_overscan.d \
./src/gui/gtk2/menu/menu_video_palette.d \
./src/gui/gtk2/menu/menu_video_rendering.d \
./src/gui/gtk2/menu/menu_video_size.d \
./src/gui/gtk2/menu/menu_video_vsync.d 


# Each subdirectory must supply rules for building sources it contributes
src/gui/gtk2/menu/%.o: ../src/gui/gtk2/menu/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	i686-pc-linux-gnu-gcc -DGTK -DGLEW_STATIC -I../src -I/usr/i686-pc-linux-gnu/usr/include/SDL -O3 -Wall -ffast-math -msse2 -mfpmath=sse -c -fmessage-length=0 -finline-functions -Winline `i686-pc-linux-gnu-pkg-config --libs --cflags gtk+-2.0` -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


