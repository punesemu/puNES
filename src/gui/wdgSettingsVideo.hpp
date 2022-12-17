/*
 *  Copyright (C) 2010-2023 Fabio Cavallo (aka FHorse)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef WDGSETTINGSVIDEO_HPP_
#define WDGSETTINGSVIDEO_HPP_

#include <QtWidgets/QWidget>
#include "ui_wdgSettingsVideo.h"

class wdgSettingsVideo : public QWidget, public Ui::wdgSettingsVideo {
	Q_OBJECT

	private:
		int vsync;
		struct _shdp_brush {
			QBrush fg;
			QBrush bg;
		} shdp_brush;

	public:
		explicit wdgSettingsVideo(QWidget *parent = nullptr);
		~wdgSettingsVideo() override;

	private:
		void changeEvent(QEvent *event) override;
		void showEvent(QShowEvent *event) override;

	public:
		void retranslateUi(QWidget *wdgSettingsVideo);
		void update_widget(void);
		void change_rom(void);
		void shcut_scale(int scale);

	private:
		void scale_set(void);
		void srotation_set(void);
		void par_set(void);
		void oscan_set(void);
		void oscan_def_value_set(void);
		void oscan_brd_set(void);
		void sfilter_set(void);
		void shader_set(void);
		void shader_param_set(void);
		void palette_set(void);
#if defined (FULLSCREEN_RESFREQ)
		void resolution_set(void);
#endif
		bool call_gfx_set_screen(int mtype);

	private slots:
		void s_scale(bool checked);
		void s_par(bool checked);
		void s_par_stretch(bool checked);
		void s_oscan(bool checked);
		void s_oscan_def_value(bool checked);
		void s_oscan_brd_black_w(bool checked);
		void s_oscan_brd_black_f(bool checked);
		void s_oscan_spinbox(int i);
		void s_oscan_reset(bool checked);
		void s_sfilter(int index);
		void s_shader(int index);
		void s_shader_file(bool checked);
		void s_shader_file_clear(bool checked);
		void s_shader_param_slider(int value);
		void s_shader_param_spin(double d);
		void s_shader_param_default(bool checked);
		void s_shader_param_all_defaults(bool checked);
#if defined (WITH_OPENGL)
		void s_disable_srgb_fbo(bool checked);
#endif
		void s_palette(int index);
		void s_palette_file(bool checked);
		void s_palette_file_clear(bool checked);
		void s_disable_emphasis_swap_pal(bool checked);
		void s_vsync(bool checked);
		void s_interpolation(bool checked);
		void s_text_on_screen(bool checked);
		void s_show_fps(bool checked);
		void s_show_frames_and_lags(bool checked);
		void s_input_display(bool checked);
		void s_disable_tv_noise(bool checked);
		void s_disable_sepia(bool checked);
		void s_fullscreen_in_window(bool checked);
		void s_integer_in_fullscreen(bool checked);
		void s_stretch_in_fullscreen(bool checked);
#if defined (FULLSCREEN_RESFREQ)
		void s_adaptive_rrate(bool checked);
		void s_resolution(int index);
#endif
		void s_screen_rotation(bool checked);
		void s_horizontal_flip_screen(bool checked);
		void s_input_rotation(bool checked);
		void s_text_rotation(bool checked);
};

#endif /* WDGSETTINGSVIDEO_HPP_ */
