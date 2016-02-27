/* Copyright (C) 2010-2015 The RetroArch team
 *
 * ---------------------------------------------------------------------------------------
 * The following license statement only applies to this file (matrix.c).
 * ---------------------------------------------------------------------------------------
 *
 * Permission is hereby granted, free of charge,
 * to any person obtaining a copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */
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
#include "matrix.h"

// Sets mat to an identity matrix
void matrix_4x4_identity(_math_matrix_4x4 *mat) {
	int i;

	memset(mat, 0, sizeof(*mat));

	for (i = 0; i < 4; i++) {
		MAT_ELEM_4X4(*mat, i, i) = 1.0f;
	}
}
// Creates an orthographic projection matrix.
void matrix_4x4_ortho(_math_matrix_4x4 *mat, float left, float right, float bottom, float top,
		float znear, float zfar) {
	float tx, ty, tz;

	matrix_4x4_identity(mat);

	tx = -(right + left) / (right - left);
	ty = -(top + bottom) / (top - bottom);
	tz = -(zfar + znear) / (zfar - znear);

	MAT_ELEM_4X4(*mat, 0, 0) = 2.0f / (right - left);
	MAT_ELEM_4X4(*mat, 1, 1) = 2.0f / (top - bottom);
	MAT_ELEM_4X4(*mat, 2, 2) = -2.0f / (zfar - znear);
	MAT_ELEM_4X4(*mat, 0, 3) = tx;
	MAT_ELEM_4X4(*mat, 1, 3) = ty;
	MAT_ELEM_4X4(*mat, 2, 3) = tz;
}


