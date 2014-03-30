################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/sdl/gui/gtk2/menu/menu.c \
../src/sdl/gui/gtk2/menu/menu_audio.c \
../src/sdl/gui/gtk2/menu/menu_audio_channels.c \
../src/sdl/gui/gtk2/menu/menu_audio_quality.c \
../src/sdl/gui/gtk2/menu/menu_audio_sample_rate.c \
../src/sdl/gui/gtk2/menu/menu_audio_stereo_delay.c \
../src/sdl/gui/gtk2/menu/menu_file.c \
../src/sdl/gui/gtk2/menu/menu_gamegenie.c \
../src/sdl/gui/gtk2/menu/menu_help.c \
../src/sdl/gui/gtk2/menu/menu_input.c \
../src/sdl/gui/gtk2/menu/menu_mode.c \
../src/sdl/gui/gtk2/menu/menu_nes.c \
../src/sdl/gui/gtk2/menu/menu_nes_fds.c \
../src/sdl/gui/gtk2/menu/menu_settings.c \
../src/sdl/gui/gtk2/menu/menu_state.c \
../src/sdl/gui/gtk2/menu/menu_video.c \
../src/sdl/gui/gtk2/menu/menu_video_effect.c \
../src/sdl/gui/gtk2/menu/menu_video_filter.c \
../src/sdl/gui/gtk2/menu/menu_video_fps.c \
../src/sdl/gui/gtk2/menu/menu_video_frame_skip.c \
../src/sdl/gui/gtk2/menu/menu_video_fullscreen.c \
../src/sdl/gui/gtk2/menu/menu_video_interpolation.c \
../src/sdl/gui/gtk2/menu/menu_video_overscan.c \
../src/sdl/gui/gtk2/menu/menu_video_palette.c \
../src/sdl/gui/gtk2/menu/menu_video_rendering.c \
../src/sdl/gui/gtk2/menu/menu_video_size.c \
../src/sdl/gui/gtk2/menu/menu_video_tv_aspect_ratio.c \
../src/sdl/gui/gtk2/menu/menu_video_txt_on_screen.c \
../src/sdl/gui/gtk2/menu/menu_video_vsync.c 

OBJS += \
./src/sdl/gui/gtk2/menu/menu.o \
./src/sdl/gui/gtk2/menu/menu_audio.o \
./src/sdl/gui/gtk2/menu/menu_audio_channels.o \
./src/sdl/gui/gtk2/menu/menu_audio_quality.o \
./src/sdl/gui/gtk2/menu/menu_audio_sample_rate.o \
./src/sdl/gui/gtk2/menu/menu_audio_stereo_delay.o \
./src/sdl/gui/gtk2/menu/menu_file.o \
./src/sdl/gui/gtk2/menu/menu_gamegenie.o \
./src/sdl/gui/gtk2/menu/menu_help.o \
./src/sdl/gui/gtk2/menu/menu_input.o \
./src/sdl/gui/gtk2/menu/menu_mode.o \
./src/sdl/gui/gtk2/menu/menu_nes.o \
./src/sdl/gui/gtk2/menu/menu_nes_fds.o \
./src/sdl/gui/gtk2/menu/menu_settings.o \
./src/sdl/gui/gtk2/menu/menu_state.o \
./src/sdl/gui/gtk2/menu/menu_video.o \
./src/sdl/gui/gtk2/menu/menu_video_effect.o \
./src/sdl/gui/gtk2/menu/menu_video_filter.o \
./src/sdl/gui/gtk2/menu/menu_video_fps.o \
./src/sdl/gui/gtk2/menu/menu_video_frame_skip.o \
./src/sdl/gui/gtk2/menu/menu_video_fullscreen.o \
./src/sdl/gui/gtk2/menu/menu_video_interpolation.o \
./src/sdl/gui/gtk2/menu/menu_video_overscan.o \
./src/sdl/gui/gtk2/menu/menu_video_palette.o \
./src/sdl/gui/gtk2/menu/menu_video_rendering.o \
./src/sdl/gui/gtk2/menu/menu_video_size.o \
./src/sdl/gui/gtk2/menu/menu_video_tv_aspect_ratio.o \
./src/sdl/gui/gtk2/menu/menu_video_txt_on_screen.o \
./src/sdl/gui/gtk2/menu/menu_video_vsync.o 

C_DEPS += \
./src/sdl/gui/gtk2/menu/menu.d \
./src/sdl/gui/gtk2/menu/menu_audio.d \
./src/sdl/gui/gtk2/menu/menu_audio_channels.d \
./src/sdl/gui/gtk2/menu/menu_audio_quality.d \
./src/sdl/gui/gtk2/menu/menu_audio_sample_rate.d \
./src/sdl/gui/gtk2/menu/menu_audio_stereo_delay.d \
./src/sdl/gui/gtk2/menu/menu_file.d \
./src/sdl/gui/gtk2/menu/menu_gamegenie.d \
./src/sdl/gui/gtk2/menu/menu_help.d \
./src/sdl/gui/gtk2/menu/menu_input.d \
./src/sdl/gui/gtk2/menu/menu_mode.d \
./src/sdl/gui/gtk2/menu/menu_nes.d \
./src/sdl/gui/gtk2/menu/menu_nes_fds.d \
./src/sdl/gui/gtk2/menu/menu_settings.d \
./src/sdl/gui/gtk2/menu/menu_state.d \
./src/sdl/gui/gtk2/menu/menu_video.d \
./src/sdl/gui/gtk2/menu/menu_video_effect.d \
./src/sdl/gui/gtk2/menu/menu_video_filter.d \
./src/sdl/gui/gtk2/menu/menu_video_fps.d \
./src/sdl/gui/gtk2/menu/menu_video_frame_skip.d \
./src/sdl/gui/gtk2/menu/menu_video_fullscreen.d \
./src/sdl/gui/gtk2/menu/menu_video_interpolation.d \
./src/sdl/gui/gtk2/menu/menu_video_overscan.d \
./src/sdl/gui/gtk2/menu/menu_video_palette.d \
./src/sdl/gui/gtk2/menu/menu_video_rendering.d \
./src/sdl/gui/gtk2/menu/menu_video_size.d \
./src/sdl/gui/gtk2/menu/menu_video_tv_aspect_ratio.d \
./src/sdl/gui/gtk2/menu/menu_video_txt_on_screen.d \
./src/sdl/gui/gtk2/menu/menu_video_vsync.d 


# Each subdirectory must supply rules for building sources it contributes
src/sdl/gui/gtk2/menu/%.o: ../src/sdl/gui/gtk2/menu/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	i686-pc-linux-gnu-gcc -DGTK -DSDL -DGLEW_STATIC -I../src -I../src/sdl -I/usr/i686-pc-linux-gnu/usr/include/SDL -O3 -Wall -ffast-math -msse2 -mfpmath=sse -c -fmessage-length=0 -finline-functions -Winline `i686-pc-linux-gnu-pkg-config --libs --cflags gtk+-2.0` -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


