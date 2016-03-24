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

#define SHDCODE(index) (char *) shader_code[index].code
#define LUTCODE(index) lut_resource[index].code
#define SPALIAS(a) strncpy(sp->alias, a, sizeof(sp->alias))
#define LPPATH(a) strncpy(lp->path, LUTCODE(a), sizeof(lp->path))
#define LPNAME(a) strncpy(lp->name, a, sizeof(lp->name))
#define PRMNAME(a) strncpy(prm->name, a, sizeof(prm->name))

#define _shdpass(a) sp = &se->sp[a]; type = &sp->sc.type; scale = &sp->sc.scale //; abs = &sp->sc.abs
#define shdpass() _shdpass(se->pass++)
#define lutpass() lp = &se->lp[se->luts++]
#define prmshd(a, b) prm = &se->param[se->params++]; PRMNAME(a); prm->value = b

static void sp_set_default(_shader_pass *sp);
static void lp_set_default(_lut_pass *lp);
static void ps_set_default(_param_shd *ps);
static void se_soft_stretch(void);

BYTE shaders_set(int shader) {
	_shader_effect *se = &shader_effect;
	_shader_pass *sp = NULL;
	_xy_uint *type = NULL;
	_xy_float *scale = NULL;
	//_xy_uint *abs = NULL;
	//_lut_pass *lp = NULL;
	//_param_shd *prm = NULL;
	int i;

	shader_se_set_default(&shader_effect);

	switch (shader) {
		case NO_FILTER:
			shdpass();
			sp->code = SHDCODE(shc_no_filter);
			se_soft_stretch();
			break;
		case SHADER_CRTDOTMASK:
			shdpass();
			sp->code = SHDCODE(shc_crt_dotmask);
			sp->linear = TEXTURE_LINEAR_DISAB;
			se_soft_stretch();
			break;
		case SHADER_CRTHYLLIAN:
			shdpass();
			sp->code = SHDCODE(shc_crt_crt_hyllian);
			sp->linear = TEXTURE_LINEAR_DISAB;
			break;
		case SHADER_CRTSCANLINES:
			shdpass();
			sp->code = SHDCODE(shc_crt_crt_caligari);
			sp->linear = TEXTURE_LINEAR_DISAB;
			break;
		case SHADER_CRTWITHCURVE:
			shdpass();
			sp->code = SHDCODE(shc_crt_crt_geom);
			sp->linear = TEXTURE_LINEAR_DISAB;
			break;
		case SHADER_EMBOSS:
			shdpass();
			sp->code = SHDCODE(shc_mudlord_emboss);
			sp->linear = TEXTURE_LINEAR_DISAB;
			se_soft_stretch();
			break;
		case SHADER_NOISE:
			shdpass();
			sp->code = SHDCODE(shc_mudlord_noise_mudlord);
			sp->linear = TEXTURE_LINEAR_DISAB;
			se_soft_stretch();
			break;
		case SHADER_NTSC2PHASECOMPOSITE:
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
		case SHADER_OLDTV:
			shdpass();
			sp->code = SHDCODE(shc_mudlord_oldtv);
			sp->linear = TEXTURE_LINEAR_DISAB;
			se_soft_stretch();
			break;
		case SHADER_FILE:
			if (cgp_parse(cfg->shader_file) == EXIT_ERROR) {
				return (EXIT_ERROR);
			}
			break;

		case SHADER_TEST:
			/*
			shdpass();
			sp->code = SHDCODE(shc_hunterk_motionblur_braid_rewind);
			sp->linear = TEXTURE_LINEAR_DISAB;
			*/
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
		cgp_pragma_param(sp->code, sp->path);
	}

	return (EXIT_OK);
}
void shader_se_set_default(_shader_effect *se) {
	int i;

	se->type = MS_MEM;

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
char *shader_code_blend(void) {
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
