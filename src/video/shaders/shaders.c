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

#include <string.h>
#include "shaders.h"
#include "shdcode.h"
#include "gfx.h"
#include "conf.h"
#include "cgp.h"

#define SPALIAS(a) strncpy(sp->alias, a, sizeof(sp->alias))
#define LPPATH(a) strncpy(lp->path, LUTCODE(a), sizeof(lp->path))
#define LPNAME(a) strncpy(lp->name, a, sizeof(lp->name))
#define PRMNAME(a) strncpy(prm->name, a, sizeof(prm->name))

#define _shdpass(a) sp = &se->sp[a]; type = &sp->sc.type; scale = &sp->sc.scale; abs = &sp->sc.abs
#define shdpass() _shdpass(se->pass++)
#define lutpass() lp = &se->lp[se->luts++]
#define prmshd(a, b) prm = &se->param[se->params++]; PRMNAME(a); prm->value = b

static void sp_set_default(_shader_pass *sp);
static void lp_set_default(_lut_pass *lp);
static void se_set_default(void);
static void se_soft_stretch(void);

void shaders_set(int shader) {
	_shader_effect *se = &shader_effect;
	_shader_pass *sp = NULL;
	_xy_uint *type = NULL;
	_xy_float *scale = NULL;
	_xy_uint *abs = NULL;
	_lut_pass *lp = NULL;
	_param_shd *prm = NULL;
	int i;

	se_set_default();

	//cgp_parse();

	switch (shader) {
		case NO_FILTER:
			shdpass();
			sp->code = SHDCODE(shc_no_filter);
			se_soft_stretch();
			break;
		case sh_anti_aliasing_advanced_aa:
			shdpass();
			sp->code = SHDCODE(shc_anti_aliasing_advanced_aa);
			sp->linear = TEXTURE_LINEAR_DISAB;
			type->x = SHADER_SCALE_INPUT;
			type->y = SHADER_SCALE_INPUT;
			scale->x = 2.0f;
			scale->y = 2.0f;
			shdpass();
			sp->code = SHDCODE(shc_no_filter);
			sp->linear = TEXTURE_LINEAR_ENAB;
			break;
		case sh_anti_aliasing_fx_aa:
			shdpass();
			sp->code = SHDCODE(shc_anti_aliasing_fx_aa);
			sp->linear = TEXTURE_LINEAR_DISAB;
			break;
		case sh_anti_aliasing_fxaa_edge_detect:
			shdpass();
			sp->code = SHDCODE(shc_anti_aliasing_fxaa_edge_detect);
			sp->linear = TEXTURE_LINEAR_DISAB;
			break;
		case sh_cgp_tvout_tvout_ntsc_2phase_composite:
			shdpass();
			sp->code = SHDCODE(shc_ntsc_ntsc_pass1_composite_2phase);
			sp->linear = TEXTURE_LINEAR_DISAB;
			sp->frame_count_mod = 2;
			sp->fbo_flt = TRUE;
			type->x = SHADER_SCALE_INPUT;
			type->y = SHADER_SCALE_INPUT;
			scale->x = 4.0f;
			shdpass();
			sp->code = SHDCODE(shc_ntsc_ntsc_pass2_2phase);
			sp->linear = TEXTURE_LINEAR_DISAB;
			type->x = SHADER_SCALE_INPUT;
			type->y = SHADER_SCALE_INPUT;
			scale->x = 0.5f;
			shdpass();
			sp->code = SHDCODE(shc_crt_tvout_tweaks);
			sp->linear = TEXTURE_LINEAR_DISAB;
			type->x = SHADER_SCALE_VIEWPORT;
			type->y = SHADER_SCALE_INPUT;
			shdpass();
			sp->code = SHDCODE(shc_misc_image_adjustment);
			break;
		case sh_cgp_tvout_tvout_ntsc_256px_svideo:
			shdpass();
			sp->code = SHDCODE(shc_ntsc_ntsc_pass1_svideo_3phase);
			sp->linear = TEXTURE_LINEAR_DISAB;
			sp->frame_count_mod = 2;
			sp->fbo_flt = TRUE;
			type->x = SHADER_SCALE_ABSOLUTE;
			type->y = SHADER_SCALE_INPUT;
			abs->x = 1024.0f;
			shdpass();
			sp->code = SHDCODE(shc_ntsc_ntsc_pass2_3phase);
			sp->linear = TEXTURE_LINEAR_DISAB;
			type->x = SHADER_SCALE_INPUT;
			type->y = SHADER_SCALE_INPUT;
			scale->x = 0.5f;
			shdpass();
			sp->code = SHDCODE(shc_crt_tvout_tweaks);
			sp->linear = TEXTURE_LINEAR_DISAB;
			type->x = SHADER_SCALE_VIEWPORT;
			type->y = SHADER_SCALE_INPUT;
			shdpass();
			sp->code = SHDCODE(shc_misc_image_adjustment);
			break;
		case sh_cgp_2xbr_crt_hyllian:
			shdpass();
			sp->code = SHDCODE(shc_xbr_legacy_2xbr_v38c);
			sp->linear = TEXTURE_LINEAR_DISAB;
			type->x = SHADER_SCALE_INPUT;
			type->y = SHADER_SCALE_INPUT;
			scale->x = 2.0f;
			shdpass();
			sp->code = SHDCODE(shc_crt_crt_hyllian);
			sp->linear = TEXTURE_LINEAR_DISAB;
			break;
		case sh_cgp_2xbr_jinc2_sharper_hybrid:
			shdpass();
			sp->code = SHDCODE(shc_xbr_legacy_2xbr_v38c);
			sp->linear = TEXTURE_LINEAR_DISAB;
			type->x = SHADER_SCALE_INPUT;
			type->y = SHADER_SCALE_INPUT;
			scale->x = 2.0f;
			shdpass();
			sp->code = SHDCODE(shc_windowed_jinc2_sharper);
			sp->linear = TEXTURE_LINEAR_DISAB;
			break;
		case sh_crt_gtuv50:
			shdpass();
			sp->code = SHDCODE(shc_crt_gtu_v050_pass1);
			sp->fbo_flt = TRUE;
			type->x = SHADER_SCALE_INPUT;
			type->y = SHADER_SCALE_INPUT;
			shdpass();
			sp->code = SHDCODE(shc_crt_gtu_v050_pass2);
			sp->linear = TEXTURE_LINEAR_DISAB;
			sp->fbo_flt = TRUE;
			type->x = SHADER_SCALE_VIEWPORT;
			type->y = SHADER_SCALE_INPUT;
			shdpass();
			sp->code = SHDCODE(shc_crt_gtu_v050_pass3);
			sp->linear = TEXTURE_LINEAR_DISAB;
			type->x = SHADER_SCALE_VIEWPORT;
			type->y = SHADER_SCALE_VIEWPORT;
			break;
		case sh_crt_4xbr_hybrid_crt:
			shdpass();
			sp->code = SHDCODE(shc_crt_4xbr_hybrid_crt);
			sp->linear = TEXTURE_LINEAR_DISAB;
			type->x = SHADER_SCALE_INPUT;
			type->y = SHADER_SCALE_INPUT;
			scale->x = 4.0f;
			scale->y = 4.0f;
			shdpass();
			sp->code = SHDCODE(shc_no_filter);
			sp->linear = TEXTURE_LINEAR_ENAB;
			break;
		case sh_crt_crt_caligari:
			shdpass();
			sp->code = SHDCODE(shc_crt_crt_caligari);
			sp->linear = TEXTURE_LINEAR_DISAB;
			break;
		case sh_crt_crt_cgwg_fast:
			shdpass();
			sp->code = SHDCODE(shc_crt_crt_cgwg_fast);
			sp->linear = TEXTURE_LINEAR_DISAB;
			break;
		case sh_crt_crt_easymode:
			shdpass();
			sp->code = SHDCODE(shc_crt_crt_easymode);
			sp->linear = TEXTURE_LINEAR_DISAB;
			break;
		case sh_crt_crt_easymode_halation:
			shdpass();
			sp->code = SHDCODE(shc_crt_crt_easymode_halation_linearize);
			sp->linear = TEXTURE_LINEAR_DISAB;
			sp->fbo_srgb = TRUE;
			type->x = SHADER_SCALE_INPUT;
			type->y = SHADER_SCALE_INPUT;
			shdpass();
			sp->code = SHDCODE(shc_crt_crt_easymode_halation_blur_horiz);
			sp->linear = TEXTURE_LINEAR_DISAB;
			sp->fbo_srgb = TRUE;
			type->x = SHADER_SCALE_INPUT;
			type->y = SHADER_SCALE_INPUT;
			shdpass();
			sp->code = SHDCODE(shc_crt_crt_easymode_halation_blur_vert);
			sp->linear = TEXTURE_LINEAR_DISAB;
			sp->fbo_srgb = TRUE;
			type->x = SHADER_SCALE_INPUT;
			type->y = SHADER_SCALE_INPUT;
			shdpass();
			sp->code = SHDCODE(shc_crt_crt_easymode_halation_threshold);
			sp->linear = TEXTURE_LINEAR_DISAB;
			sp->fbo_srgb = TRUE;
			type->x = SHADER_SCALE_INPUT;
			type->y = SHADER_SCALE_INPUT;
			shdpass();
			sp->code = SHDCODE(shc_crt_crt_easymode_halation_crt_easymode_halation);
			sp->linear = TEXTURE_LINEAR_ENAB;
			break;
		case sh_crt_crt_geom:
			shdpass();
			sp->code = SHDCODE(shc_crt_crt_geom);
			sp->linear = TEXTURE_LINEAR_DISAB;
			break;
		case sh_crt_crtglow_gauss:
			shdpass();
			sp->code = SHDCODE(shc_crt_glow_linearize);
			sp->linear = TEXTURE_LINEAR_DISAB;
			sp->fbo_srgb = TRUE;
			shdpass();
			sp->code = SHDCODE(shc_crt_glow_gauss_horiz);
			sp->linear = TEXTURE_LINEAR_DISAB;
			sp->fbo_srgb = TRUE;
			type->x = SHADER_SCALE_VIEWPORT;
			type->y = SHADER_SCALE_INPUT;
			shdpass();
			sp->code = SHDCODE(shc_crt_glow_gauss_vert);
			sp->linear = TEXTURE_LINEAR_DISAB;
			sp->fbo_srgb = TRUE;
			type->x = SHADER_SCALE_VIEWPORT;
			type->y = SHADER_SCALE_VIEWPORT;
			shdpass();
			sp->code = SHDCODE(shc_crt_glow_threshold);
			sp->linear = TEXTURE_LINEAR_DISAB;
			sp->fbo_srgb = TRUE;
			shdpass();
			sp->code = SHDCODE(shc_crt_glow_blur_horiz);
			sp->mipmap_input = TRUE;
			sp->linear = TEXTURE_LINEAR_ENAB;
			sp->fbo_srgb = TRUE;
			type->x = SHADER_SCALE_INPUT;
			type->y = SHADER_SCALE_INPUT;
			scale->x = 0.25f;
			scale->y = 0.25f;
			shdpass();
			sp->code = SHDCODE(shc_crt_glow_blur_vert);
			sp->linear = TEXTURE_LINEAR_ENAB;
			sp->fbo_srgb = TRUE;
			shdpass();
			sp->code = SHDCODE(shc_crt_glow_resolve);
			sp->linear = TEXTURE_LINEAR_ENAB;
			break;
		case sh_crt_crtglow_gauss_ntsc_3phase:
			shdpass();
			sp->code = SHDCODE(shc_ntsc_ntsc_pass1_composite_3phase);
			sp->linear = TEXTURE_LINEAR_DISAB;
			sp->frame_count_mod = 2;
			sp->fbo_flt = TRUE;
			type->x = SHADER_SCALE_INPUT;
			type->y = SHADER_SCALE_INPUT;
			scale->x = 4.0f;
			scale->y = 1.0f;
			shdpass();
			sp->code = SHDCODE(shc_ntsc_ntsc_pass2_3phase_linear);
			sp->linear = TEXTURE_LINEAR_DISAB;
			sp->fbo_srgb = TRUE;
			type->x = SHADER_SCALE_INPUT;
			type->y = SHADER_SCALE_INPUT;
			scale->x = 0.5f;
			scale->y = 1.0f;
			shdpass();
			sp->code = SHDCODE(shc_crt_glow_gauss_horiz);
			sp->linear = TEXTURE_LINEAR_DISAB;
			sp->fbo_srgb = TRUE;
			type->x = SHADER_SCALE_VIEWPORT;
			type->y = SHADER_SCALE_INPUT;
			shdpass();
			sp->code = SHDCODE(shc_crt_glow_gauss_vert);
			sp->linear = TEXTURE_LINEAR_DISAB;
			sp->fbo_srgb = TRUE;
			type->x = SHADER_SCALE_VIEWPORT;
			type->y = SHADER_SCALE_VIEWPORT;
			shdpass();
			sp->code = SHDCODE(shc_crt_glow_threshold);
			sp->linear = TEXTURE_LINEAR_DISAB;
			sp->fbo_srgb = TRUE;
			shdpass();
			sp->code = SHDCODE(shc_crt_glow_blur_horiz);
			sp->mipmap_input = TRUE;
			sp->linear = TEXTURE_LINEAR_ENAB;
			sp->fbo_srgb = TRUE;
			type->x = SHADER_SCALE_INPUT;
			type->y = SHADER_SCALE_INPUT;
			scale->x = 0.25f;
			scale->y = 0.25f;
			shdpass();
			sp->code = SHDCODE(shc_crt_glow_blur_vert);
			sp->linear = TEXTURE_LINEAR_ENAB;
			sp->fbo_srgb = TRUE;
			shdpass();
			sp->code = SHDCODE(shc_crt_glow_resolve);
			sp->linear = TEXTURE_LINEAR_ENAB;
			break;
		case sh_crt_crt_hyllian:
			shdpass();
			sp->code = SHDCODE(shc_crt_crt_hyllian);
			sp->linear = TEXTURE_LINEAR_DISAB;
			break;
		case sh_crt_crt_lottes:
			shdpass();
			sp->code = SHDCODE(shc_crt_crt_lottes);
			sp->linear = TEXTURE_LINEAR_DISAB;
			break;
		case sh_crt_crt_reverse_aa:
			shdpass();
			sp->code = SHDCODE(shc_crt_crt_reverse_aa);
			sp->linear = TEXTURE_LINEAR_DISAB;
			se_soft_stretch();
			break;
		case sh_crt_dotmask:
			shdpass();
			sp->code = SHDCODE(shc_crt_dotmask);
			sp->linear = TEXTURE_LINEAR_DISAB;
			se_soft_stretch();
			break;
		case sh_eagle_super_eagle:
			shdpass();
			sp->code = SHDCODE(shc_eagle_super_eagle);
			sp->linear = TEXTURE_LINEAR_DISAB;
			type->x = SHADER_SCALE_INPUT;
			type->y = SHADER_SCALE_INPUT;
			scale->x = 2.0f;
			scale->y = 2.0f;
			shdpass();
			sp->code = SHDCODE(shc_no_filter);
			sp->linear = TEXTURE_LINEAR_ENAB;
			break;
		case sh_hunterk_borders_1080p_bigblur:
			shdpass();
			sp->code = SHDCODE(shc_no_filter);
			shdpass();
			sp->code = SHDCODE(shc_hunterk_resources_bigblur_horiz);
			shdpass();
			sp->code = SHDCODE(shc_hunterk_resources_bigblur_vert);
			shdpass();
			sp->code = SHDCODE(shc_hunterk_resources_bigblur_1080p);

			lutpass();
			LPPATH(lut_hunterk_borders_1080p_border_1080p);
			LPNAME("bg");
			break;
		case sh_hunterk_borders_1080p_color_grid:
			shdpass();
			sp->code = SHDCODE(shc_hunterk_resources_color_grid_1080p);

			lutpass();
			LPPATH(lut_hunterk_borders_1080p_border_1080p);
			LPNAME("bg");
			break;
		case sh_hunterk_borders_1080p_mudlord:
			shdpass();
			sp->code = SHDCODE(shc_hunterk_resources_mudlord_1080p);

			lutpass();
			lp->linear = TEXTURE_LINEAR_DISAB;
			LPPATH(lut_hunterk_borders_1080p_border_1080p);
			LPNAME("bg");
			break;
		case sh_hunterk_borders_1080p_shiny_iterations:
			shdpass();
			sp->code = SHDCODE(shc_hunterk_resources_shiny_iterations_1080p);

			lutpass();
			LPPATH(lut_hunterk_borders_1080p_border_1080p);
			LPNAME("bg");
			break;
		case sh_hunterk_borders_1080p_snow:
			shdpass();
			sp->code = SHDCODE(shc_hunterk_resources_snow_1080p);

			lutpass();
			LPPATH(lut_hunterk_borders_1080p_border_1080p);
			LPNAME("bg");
			break;
		case sh_hunterk_borders_1080p_voronoi:
			shdpass();
			sp->code = SHDCODE(shc_hunterk_resources_voronoi_1080p);

			lutpass();
			LPPATH(lut_hunterk_borders_1080p_border_1080p);
			LPNAME("bg");
			break;
		case sh_hunterk_borders_1080p_water:
			shdpass();
			sp->code = SHDCODE(shc_hunterk_resources_water_1080p);

			lutpass();
			LPPATH(lut_hunterk_borders_1080p_border_1080p);
			LPNAME("bg");
			break;
		case sh_hunterk_handheld_nds:
			shdpass();
			sp->code = SHDCODE(shc_hunterk_misc_color_mangler);
			sp->linear = TEXTURE_LINEAR_DISAB;
			type->x = SHADER_SCALE_INPUT;
			type->y = SHADER_SCALE_INPUT;
			shdpass();
			sp->code = SHDCODE(shc_hunterk_handheld_lcd_cgwg_lcd_grid);
			sp->linear = TEXTURE_LINEAR_DISAB;
			type->x = SHADER_SCALE_VIEWPORT;
			type->y = SHADER_SCALE_VIEWPORT;
			shdpass();
			sp->code = SHDCODE(shc_hunterk_misc_image_adjustment);

			prmshd("display_gamma", 1.9f);
			prmshd("target_gamma", 2.2f);
			prmshd("sat", 1.04f);
			prmshd("lum", 1.0f);
			prmshd("cntrst", 1.0f);
			prmshd("blr", 0.0f);
			prmshd("blg", 0.0f);
			prmshd("blb", 0.0f);
			prmshd("r", 0.77f);
			prmshd("g", 0.69f);
			prmshd("b", 0.8f);
			prmshd("rg", 0.06f);
			prmshd("rb", 0.06f);
			prmshd("gr", 0.22f);
			prmshd("gb", 0.1f);
			prmshd("br", 0.0f);
			prmshd("bg", 0.24f);
			prmshd("GRID_STRENGTH", 0.15f);
			prmshd("target_gamma", 2.2f);
			prmshd("monitor_gamma", 2.2f);
			prmshd("overscan_percent_x", 0.0f);
			prmshd("overscan_percent_y", 0.0f);
			prmshd("saturation", 1.0f);
			prmshd("contrast", 1.0f);
			prmshd("luminance", 1.2f);
			prmshd("bright_boost", 0.0f);
			prmshd("R", 1.0f);
			prmshd("G", 1.0f);
			prmshd("B", 1.0f);
			break;
		case sh_hunterk_hqx_hq3x:
			shdpass();
			sp->code = SHDCODE(shc_hunterk_hqx_pass1);
			sp->linear = TEXTURE_LINEAR_DISAB;
			type->x = SHADER_SCALE_INPUT;
			type->y = SHADER_SCALE_INPUT;
			shdpass();
			sp->code = SHDCODE(shc_hunterk_hqx_hq3x);
			sp->linear = TEXTURE_LINEAR_DISAB;
			type->x = SHADER_SCALE_INPUT;
			type->y = SHADER_SCALE_INPUT;
			scale->x = 3.0f;
			scale->y = 3.0f;

			lutpass();
			lp->linear = TEXTURE_LINEAR_DISAB;
			LPPATH(lut_hunterk_hqx_resources_hq3x);
			LPNAME("LUT");
			break;
		case sh_hunterk_motionblur_motionblur_simple:
			shdpass();
			sp->code = SHDCODE(shc_hunterk_motionblur_motionblur_simple);
			sp->linear = TEXTURE_LINEAR_DISAB;
			break;
		case sh_motionblur_feedback:
			shdpass();
			se->feedback_pass = 0;
			sp->code = SHDCODE(shc_motionblur_feedback);
			sp->linear = TEXTURE_LINEAR_DISAB;
			type->x = SHADER_SCALE_INPUT;
			type->y = SHADER_SCALE_INPUT;
			break;
		case sh_mudlord_emboss:
			shdpass();
			sp->code = SHDCODE(shc_mudlord_emboss);
			sp->linear = TEXTURE_LINEAR_DISAB;
			se_soft_stretch();
			break;
		case sh_mudlord_mud_mudlord:
			shdpass();
			sp->code = SHDCODE(shc_mudlord_mud_mudlord);
			sp->linear = TEXTURE_LINEAR_DISAB;
			se_soft_stretch();
			break;
		case sh_mudlord_noise_mudlord:
			shdpass();
			sp->code = SHDCODE(shc_mudlord_noise_mudlord);
			sp->linear = TEXTURE_LINEAR_DISAB;
			se_soft_stretch();
			break;
		case sh_mudlord_oldtv:
			shdpass();
			sp->code = SHDCODE(shc_mudlord_oldtv);
			sp->linear = TEXTURE_LINEAR_DISAB;
			se_soft_stretch();
			break;
		case sh_waterpaint_water:
			shdpass();
			sp->code = SHDCODE(shc_waterpaint_water);
			sp->linear = TEXTURE_LINEAR_DISAB;
			break;
		case sh_xbr_xbr_lv2_multipass:
			shdpass();
			sp->code = SHDCODE(shc_xbr_xbr_lv2_multipass_xbr_lv2_c_pass0);
			sp->linear = TEXTURE_LINEAR_DISAB;
			sp->fbo_flt = FALSE;
			type->x = SHADER_SCALE_INPUT;
			type->y = SHADER_SCALE_INPUT;
			shdpass();
			sp->code = SHDCODE(shc_xbr_xbr_lv2_multipass_xbr_lv2_pass1);
			sp->linear = TEXTURE_LINEAR_DISAB;
			sp->fbo_flt = FALSE;
			break;

		case sh_test:
			/**/
			shdpass();
			sp->code = SHDCODE(shc_hunterk_motionblur_braid_rewind);
			sp->linear = TEXTURE_LINEAR_DISAB;
			/**/
			//se_soft_stretch();
			break;
	}

	{
		int index = se->pass - 1;

		_shdpass(index);
		if 	((type->x != SHADER_SCALE_DEFAULT) || (type->y != SHADER_SCALE_DEFAULT)) {
			se->pass++;
			_shdpass(index + 1);
			sp_set_default(sp);
			sp->code = SHDCODE(shc_no_filter);
			type->x = type->y = SHADER_SCALE_VIEWPORT;
		}
	}

	se->last_pass = se->pass - 1;

	// pragma parameters
	for (i = 0; i < se->pass; i++) {
		_shdpass(i);
		cgp_pragma_param(sp->code);
	}
}
const char *shader_code_blend(void) {
	return (SHDCODE(shc_blend));
}

static void sp_set_default(_shader_pass *sp) {
	_xy_uint *type = &sp->sc.type;
	_xy_float *scale = &sp->sc.scale;
	_xy_uint *abs = &sp->sc.abs;

	sp->code = NULL;
	memset(sp->path, 0x00, sizeof(sp->path));
	memset(sp->alias, 0x00, sizeof(sp->alias));
	sp->linear = TEXTURE_LINEAR_DEFAULT;
	sp->mipmap_input = FALSE;
	sp->fbo_flt = sp->fbo_srgb = FALSE;
	sp->wrap = TEXTURE_WRAP_BORDER;
	sp->frame_count_mod = 0;
	type->x = type->y = SHADER_SCALE_DEFAULT;
	scale->x = scale->y = 1.0f;
	abs->x = abs->y = 0;
}
static void lp_set_default(_lut_pass *lp) {
	memset(lp->path, 0x00, sizeof(lp->path));
	memset(lp->name, 0x00, sizeof(lp->name));
	lp->linear = TEXTURE_LINEAR_ENAB;
	lp->mipmap = FALSE;
	lp->wrap = TEXTURE_WRAP_BORDER;
}
static void ps_set_default(_param_shd *ps) {
	memset(ps, 0x00, sizeof(_param_shd));
}
static void se_set_default(void) {
	_shader_effect *se = &shader_effect;
	int i;

	se->pass = se->last_pass = se->running_pass = 0;
	for (i = 0; i < LENGTH(se->sp); i++) {
		sp_set_default(&se->sp[i]);
	}

	se->luts = 0;
	for (i = 0; i < LENGTH(se->lp); i++) {
		lp_set_default(&se->lp[i]);
	}

	se->params = 0;
	for (i = 0; i < LENGTH(se->param); i++) {
		ps_set_default(&se->param[i]);
	}

	se->feedback_pass = -1;
}
static void se_soft_stretch(void) {
	if ((shader_effect.sp[shader_effect.pass - 1].linear == TEXTURE_LINEAR_ENAB) ||
			!(cfg->interpolation || gfx.PSS)) {
		return;
	}

	if (!cfg->interpolation && gfx.PSS) {
		shader_effect.pass++;

		shader_effect.sp[shader_effect.pass - 1].code = SHDCODE(shc_no_filter);
		shader_effect.sp[shader_effect.pass - 1].linear = TEXTURE_LINEAR_DISAB;
		shader_effect.sp[shader_effect.pass - 1].sc.scale.x = (float) cfg->scale;
		shader_effect.sp[shader_effect.pass - 1].sc.scale.y = (float) cfg->scale;
		shader_effect.sp[shader_effect.pass - 1].sc.type.x = SHADER_SCALE_INPUT;
		shader_effect.sp[shader_effect.pass - 1].sc.type.y = SHADER_SCALE_INPUT;
	}

	shader_effect.pass++;

	shader_effect.sp[shader_effect.pass - 1].code = SHDCODE(shc_no_filter);
	shader_effect.sp[shader_effect.pass - 1].linear = TEXTURE_LINEAR_DEFAULT;
	shader_effect.sp[shader_effect.pass - 1].sc.type.x = SHADER_SCALE_DEFAULT;
	shader_effect.sp[shader_effect.pass - 1].sc.type.y = SHADER_SCALE_DEFAULT;
}
