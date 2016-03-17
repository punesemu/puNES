/*
 *  Copyright (C) 2010-2016 Fabio Cavallo (aka FHorse)
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

#ifndef SHDCODE_H_
#define SHDCODE_H_

#define SHDCODE(index) shader_code[index].code
#define LUTCODE(index) lut_resource[index].code

enum shader_code_enum {
	shc_no_filter,
	shc_blend,
	shc_anti_aliasing_advanced_aa,
	shc_anti_aliasing_fx_aa,
	shc_anti_aliasing_fxaa_edge_detect,
	shc_crt_crt_easymode_halation_blur_horiz,
	shc_crt_crt_easymode_halation_blur_vert,
	shc_crt_crt_easymode_halation_crt_easymode_halation,
	shc_crt_crt_easymode_halation_linearize,
	shc_crt_crt_easymode_halation_threshold,
	shc_crt_glow_blur_horiz,
	shc_crt_glow_blur_vert,
	shc_crt_glow_gauss_horiz,
	shc_crt_glow_gauss_vert,
	shc_crt_glow_linearize,
	shc_crt_glow_resolve,
	shc_crt_glow_threshold,
	shc_crt_gtu_v050_pass1,
	shc_crt_gtu_v050_pass2,
	shc_crt_gtu_v050_pass3,
	shc_crt_4xbr_hybrid_crt,
	shc_crt_crt_caligari,
	shc_crt_crt_cgwg_fast, // a schermo intero l'immagine e' "sporca"
	shc_crt_crt_easymode,
	shc_crt_crt_geom,
	shc_crt_crt_hyllian,
	shc_crt_crt_lottes,
	shc_crt_crt_reverse_aa,
	shc_crt_dotmask,
	shc_crt_tvout_tweaks,
	shc_eagle_super_eagle,
	shc_hunterk_handheld_lcd_cgwg_lcd_grid,
	shc_hunterk_hqx_hq3x,
	shc_hunterk_hqx_pass1,
	shc_hunterk_misc_color_mangler,
	shc_hunterk_misc_image_adjustment,
	shc_hunterk_motionblur_motionblur_simple,
	shc_hunterk_motionblur_braid_rewind,
	shc_hunterk_resources_bigblur_1080p,
	shc_hunterk_resources_bigblur_horiz,
	shc_hunterk_resources_bigblur_vert,
	shc_hunterk_resources_color_grid_1080p,
	shc_hunterk_resources_mudlord_1080p,
	shc_hunterk_resources_shiny_iterations_1080p,
	shc_hunterk_resources_snow_1080p,
	shc_hunterk_resources_voronoi_1080p,
	shc_hunterk_resources_water_1080p,
	shc_misc_image_adjustment,
	shc_motionblur_feedback,
	shc_mudlord_emboss,
	shc_mudlord_mud_mudlord,
	shc_mudlord_noise_mudlord,
	shc_mudlord_oldtv,
	shc_ntsc_ntsc_pass1_composite_2phase,
	shc_ntsc_ntsc_pass1_composite_3phase,
	shc_ntsc_ntsc_pass1_svideo_3phase,
	shc_ntsc_ntsc_pass2_2phase,
	shc_ntsc_ntsc_pass2_3phase,
	shc_ntsc_ntsc_pass2_3phase_linear,
	shc_waterpaint_water,
	shc_windowed_jinc2_sharper,
	shc_xbr_legacy_2xbr_v38c,
	shc_xbr_xbr_lv2_multipass_xbr_lv2_c_pass0,
	shc_xbr_xbr_lv2_multipass_xbr_lv2_pass1,
};
enum lut_code_enum {
	lut_none,
	lut_hunterk_borders_1080p_border_1080p,
	lut_hunterk_hqx_resources_hq3x,
};

typedef struct _shader_code {
	const char *code;
} _shader_code;

static const _shader_code shader_code[] = {
#include "shaders/no_filter.h"
#include "shaders/no_filter.h" // per le d3d9 il blend non viene fatto dalla shader
#include "shaders/anti-aliasing/advanced-aa.h"
#include "shaders/anti-aliasing/fx-aa.h"
#include "shaders/anti-aliasing/fxaa-edge-detect.h"
#include "shaders/crt/crt-easymode-halation/blur_horiz.h"
#include "shaders/crt/crt-easymode-halation/blur_vert.h"
#include "shaders/crt/crt-easymode-halation/crt-easymode-halation.h"
#include "shaders/crt/crt-easymode-halation/linearize.h"
#include "shaders/crt/crt-easymode-halation/threshold.h"
#include "shaders/crt/glow/blur_horiz.h"
#include "shaders/crt/glow/blur_vert.h"
#include "shaders/crt/glow/gauss_horiz.h"
#include "shaders/crt/glow/gauss_vert.h"
#include "shaders/crt/glow/linearize.h"
#include "shaders/crt/glow/resolve.h"
#include "shaders/crt/glow/threshold.h"
#include "shaders/crt/gtu-v050/pass1.h"
#include "shaders/crt/gtu-v050/pass2.h"
#include "shaders/crt/gtu-v050/pass3.h"
#include "shaders/crt/4xbr-hybrid-crt.h"
#include "shaders/crt/crt-caligari.h"
#include "shaders/crt/crt-cgwg-fast.h"
#include "shaders/crt/crt-easymode.h"
#include "shaders/crt/crt-geom.h"
#include "shaders/crt/crt-hyllian.h"
#include "shaders/crt/crt-lottes.h"
#include "shaders/crt/crt-reverse-aa.h"
#include "shaders/crt/dotmask.h"
#include "shaders/crt/tvout-tweaks.h"
#include "shaders/eagle/super-eagle.h"
#include "shaders/handheld/lcd_cgwg/lcd-grid.h"
#include "shaders/hqx/hq3x.h"
#include "shaders/hqx/pass1.h"
#include "shaders/misc/color-mangler.h"
#include "shaders/misc/image-adjustment.h"
#include "shaders/motionblur/motionblur-simple.h"
#include "shaders/motionblur/braid-rewind.h"
#include "shaders/borders/resources/bigblur-1080p.h"
#include "shaders/borders/resources/bigblur-horiz.h"
#include "shaders/borders/resources/bigblur-vert.h"
#include "shaders/borders/resources/color-grid-1080p.h"
#include "shaders/borders/resources/mudlord-1080p.h"
#include "shaders/borders/resources/shiny-iterations-1080p.h"
#include "shaders/borders/resources/snow-1080p.h"
#include "shaders/borders/resources/voronoi-1080p.h"
#include "shaders/borders/resources/water-1080p.h"
#include "shaders/misc/image-adjustment.h"
#include "shaders/motionblur/feedback.h"
#include "shaders/mudlord/emboss.h"
#include "shaders/mudlord/mud-mudlord.h"
#include "shaders/mudlord/noise-mudlord.h"
#include "shaders/mudlord/oldtv.h"
#include "shaders/ntsc/ntsc-pass1-composite-2phase.h"
#include "shaders/ntsc/ntsc-pass1-composite-3phase.h"
#include "shaders/ntsc/ntsc-pass1-svideo-3phase.h"
#include "shaders/ntsc/ntsc-pass2-2phase.h"
#include "shaders/ntsc/ntsc-pass2-3phase.h"
#include "shaders/ntsc/ntsc-pass2-3phase-linear.h"
#include "shaders/waterpaint/water.h"
#include "shaders/windowed/jinc2-sharper.h"
#include "shaders/xbr/legacy/2xbr-v3.8c.h"
#include "shaders/xbr/xbr-lv2-multipass/xbr-lv2-c-pass0.h"
#include "shaders/xbr/xbr-lv2-multipass/xbr-lv2-pass1.h"
};

static const _shader_code lut_resource[] = {
	{ "" },
	{ ":/shaders/shaders/lut/hunterk/borders/1080p/border-1080p.png" },
	{ ":/shaders/shaders/lut/hunterk/hqx/hq3x.png" },
};
#endif /* SHDCODE_H_ */
