/*
 * cube3d.h
 *
 *  Created on: 09/mag/2012
 *      Author: fhorse
 */

#ifndef CUBE3D_H_
#define CUBE3D_H_

#include "opengl.h"

void opengl_init_cube3d(void);
void opengl_set_cube3d(SDL_Surface *src);
void opengl_unset_cube3d(void);
void opengl_draw_scene_cube3d(SDL_Surface *surface);

#endif /* CUBE3D_H_ */
