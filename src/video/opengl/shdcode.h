/*
 *  Copyright (C) 2010-2024 Fabio Cavallo (aka FHorse)
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

enum shader_code_enum {
	shc_no_filter,
	shc_blend,
	shc_crt_crt_caligari,
	shc_crt_crt_geom,
	shc_crt_dotmask,
	shc_crt_tvout_tweaks,
	shc_misc_image_adjustment,
	shc_mudlord_emboss,
	shc_mudlord_noise_mudlord,
	shc_mudlord_oldtv,
	shc_ntsc_ntsc_pass1_composite_2phase,
	shc_ntsc_ntsc_pass2_2phase,
};
enum lut_code_enum {
	lut_none,
};

typedef struct _shader_code {
	const char *code;
} _shader_code;

static const _shader_code shader_code[] = {
#include "shaders/no_filter.h"
#include "shaders/blend.h"
#include "shaders/crt/crt-caligari.h"
#include "shaders/crt/crt-geom.h"
#include "shaders/crt/dotmask.h"
#include "shaders/crt/tvout-tweaks.h"
#include "shaders/misc/image-adjustment.h"
#include "shaders/mudlord/emboss.h"
#include "shaders/mudlord/noise-mudlord.h"
#include "shaders/mudlord/oldtv.h"
#include "shaders/ntsc/ntsc-pass1-composite-2phase.h"
#include "shaders/ntsc/ntsc-pass2-2phase.h"
};

static const _shader_code lut_resource[] = {
	{ "" },
};
#endif /* SHDCODE_H_ */
