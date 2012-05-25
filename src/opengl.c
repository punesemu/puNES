/*
 * opengl.c
 *
 *  Created on: 10/dic/2010
 *      Author: fhorse
 */

#include "opengl.h"
#include "sdlgfx.h"
#include "openGL/no_effect.h"
#include "openGL/cube3d.h"
#define _SHADERS_CODE_
#include "openGL/shaders.h"

char *file2string(const char *path);
void printLog(GLuint obj);





void sdlInitGL(void) {
	opengl.rotation = 0;
	opengl.surfaceGL = NULL;

	opengl.flagsOpengl = SDL_HWSURFACE | SDL_OPENGL;
	opengl.factorDistance = 0;
	opengl.xRotate = 0;
	opengl.yRotate = 0;
	opengl.xDiff = 0;
	opengl.yDiff = 0;

	opengl.glew = 0;

	memset(&opengl.texture, 0, sizeof(_texture));
	memset(&shader, 0, sizeof(shader));
}
void sdlQuitGL(void) {
	if (opengl.surfaceGL) {
		SDL_FreeSurface(opengl.surfaceGL);
	}

	if (opengl.texture.data) {
		glDeleteTextures(1, &opengl.texture.data);
	}

	if (shader.text.data) {
		glDeleteTextures(1, &shader.text.data);
	}
	delete_shader();
}
void sdlCreateSurfaceGL(SDL_Surface *src, WORD width, WORD height, BYTE flags) {
	if (opengl.rotation) {
		opengl_set = opengl_set_cube3d;
		opengl_draw_scene = opengl_draw_scene_cube3d;
	} else {
		opengl_set = opengl_set_no_effect;
		opengl_draw_scene = opengl_draw_scene_no_effect;
	}

	if (opengl.surfaceGL) {
		SDL_FreeSurface(opengl.surfaceGL);
		/*
		 * ripristino gli attributi opengl ai valori
		 * iniziali e li salvo nuovamente.
		 */
		glPopAttrib();
	}

	glPushAttrib(GL_ALL_ATTRIB_BITS);

	if (opengl.scale_force) {
		opengl.surfaceGL = gfxCreateRGBSurface(src, SCRROWS * opengl.scale,
		        SCRLINES * opengl.scale);
	} else {
		opengl.surfaceGL = gfxCreateRGBSurface(src, width, height);
	}

	opengl_create_texture(&opengl.texture, opengl.surfaceGL->w, opengl.surfaceGL->h,
			opengl.interpolation, POWER_OF_TWO);

	/* FIXME: funzionera' anche con il filtro NTSC ?!? */
	opengl.texture.x = (GLfloat) width  / (opengl.texture.w * opengl.factor);
	opengl.texture.y = (GLfloat) height / (opengl.texture.h * opengl.factor);

	glsl_shaders_init();

	opengl_set(src);

	glFinish();

	{
		/* aspect ratio */
		opengl.wTexture = src->w;
		opengl.hTexture = src->h;
		opengl.xTexture1 = 0;
		opengl.yTexture1 = 0;
		opengl.xTexture2 = src->w;
		opengl.yTexture2 = src->h;

		/* con flags intendo sia il fullscreen che il futuro resize */
		if (flags && opengl.aspectRatio) {
			float ratioSurface = (float) opengl.wTexture / opengl.hTexture;
			float ratioFrame = (float) width / height;

			//ratioFrame = (float) 4 / 3;
			//ratioFrame = (float) 16 / 9;

			//fprintf(stderr, "opengl : %f %f\n", ratioSurface, ratioFrame);

			/*
			 * se l'aspect ratio del frame e' maggiore di
			 * quello della superficie allora devo agire
			 * sull'altezza.
			 */
			if (ratioFrame > ratioSurface) {
				int centeringFactor = 0;

				opengl.hTexture = opengl.wTexture / ratioFrame;
				centeringFactor = (src->h - opengl.hTexture) / 2;

				opengl.xTexture1 = 0;
				opengl.yTexture1 = centeringFactor;
				opengl.xTexture2 = opengl.wTexture;
				opengl.yTexture2 = opengl.hTexture + centeringFactor;
				/*
				 * se l'aspect ratio del frame e' minore di
				 * quello della superficie allora devo agire
				 * sulla larghezza.
				 */
			} else if (ratioFrame < ratioSurface) {
				int centeringFactor = 0;

				opengl.wTexture = ratioFrame * opengl.hTexture;
				centeringFactor = (src->w - opengl.wTexture) / 2;

				opengl.xTexture1 = centeringFactor;
				opengl.yTexture1 = 0;
				opengl.xTexture2 = opengl.wTexture + centeringFactor;
				opengl.yTexture2 = opengl.hTexture;
			}
		}
	}
}
int opengl_flip(SDL_Surface *surface) {
	SDL_GL_SwapBuffers();

	return (0);
}

void glew_init(void) {
	if (!opengl.glew){
		GLenum err;

		if ((err = glewInit()) != GLEW_OK) {
			fprintf(stderr, "INFO: %s\n", glewGetErrorString(err));
			opengl.glew = NO_GLEW;
		} else {
			opengl.glew = TRUE;

			if (GLEW_VERSION_2_0) {
				fprintf(stderr, "INFO: OpenGL 2.0 supported. Glsl enabled.\n");
				opengl.glsl = TRUE;
			}
			if (!GLEW_VERSION_3_1) {
				fprintf(stderr, "INFO: OpenGL 3.1 not supported.\n");
			} else {
				fprintf(stderr, "INFO: OpenGL 3.1 supported.\n");
			}
		}
	}
}
void glsl_shaders_init(void) {
	delete_shader();

	if (opengl.glsl && (opengl.shader != SHADER_NONE)) {

		opengl_create_texture(&shader.text, opengl.texture.w * opengl.factor,
				opengl.texture.w * opengl.factor, FALSE, NO_POWER_OF_TWO);

		shader.routine = &shader_routine[opengl.shader];

		//vsSource = file2string("/home/fhorse/Dropbox/gpuPeteOGL2.slv");
		//fsSource = file2string("/home/fhorse/Dropbox/gpuPeteOGL2.slf");

		/* program */
		shader.program = glCreateProgram();
#ifndef RELEASE
		printLog(shader.program);
#endif

		/* vertex */
		if (shader.routine->vert != NULL) {
			shader.vert = glCreateShader(GL_VERTEX_SHADER);
			printLog(shader.vert);
			glShaderSource(shader.vert, 1, &shader.routine->vert, NULL);
			printLog(shader.vert);
			glCompileShader(shader.vert);
#ifndef RELEASE
			printLog(shader.vert);
#endif

			glAttachShader(shader.program, shader.vert);
		}

		/* fragment */
		if (shader.routine->frag != NULL) {
			shader.frag = glCreateShader(GL_FRAGMENT_SHADER);
			glShaderSource(shader.frag, 1, &shader.routine->frag, NULL);
			glCompileShader(shader.frag);
#ifndef RELEASE
			printLog(shader.frag);
#endif

			glAttachShader(shader.program, shader.frag);
		}

		//free((char *) vsSource);
		//free((char *) fsSource);

		glLinkProgram(shader.program);
#ifndef RELEASE
		printLog(shader.program);
#endif

		shader.size_texture[0] = opengl.texture.w;
		shader.size_texture[1] = opengl.texture.h;

		shader.size_output[0] = opengl.xTexture2 - opengl.xTexture1;
		shader.size_output[1] = opengl.yTexture2 - opengl.yTexture1;

		glUseProgram(shader.program);

		shader.loc.size_texture = glGetUniformLocation(shader.program, "size_texture");
		shader.loc.size_output = glGetUniformLocation(shader.program, "size_output");
		shader.loc.texture_scr = glGetUniformLocation(shader.program, "texture_scr");
		shader.loc.texture_txt = glGetUniformLocation(shader.program, "texture_txt");

		glUniform2fv(shader.loc.size_texture, 1, shader.size_texture);
		glUniform2fv(shader.loc.size_output, 1, shader.size_output);

		glEnable(GL_TEXTURE_2D);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, opengl.texture.data);
		glUniform1i(shader.loc.texture_scr, 0);

		glEnable(GL_TEXTURE_2D);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, shader.text.data);
		glUniform1i(shader.loc.texture_txt, 1);

		glUseProgram(0);
	}
}

void opengl_create_texture(_texture *texture, uint32_t width, uint32_t height,
        uint8_t interpolation, uint8_t pow) {
	SDL_Surface *blank;

	switch (opengl.surfaceGL->format->BitsPerPixel) {
		case 16:
			opengl.texture.format_internal = GL_RGB5;
			if (SDL_BYTEORDER == SDL_BIG_ENDIAN) {
				opengl.texture.format = GL_BGR;
				opengl.texture.type = GL_UNSIGNED_SHORT_5_6_5_REV;
			} else {
				opengl.texture.format = GL_RGB;
				opengl.texture.type = GL_UNSIGNED_SHORT_5_6_5;
			}
			break;
		case 24:
			opengl.texture.format_internal = GL_RGB8;
			if (SDL_BYTEORDER == SDL_BIG_ENDIAN) {
				opengl.texture.format = GL_BGR;
			} else {
				opengl.texture.format = GL_RGB;
			}
			opengl.texture.type = GL_UNSIGNED_BYTE;
			break;
		case 32:
		default:
			opengl.texture.format_internal = GL_RGBA8;
			opengl.texture.format = GL_BGRA;
			opengl.texture.type = GL_UNSIGNED_BYTE;
			break;
	}

	if (pow) {
		texture->w = opengl_power_of_two(width);
		texture->h = opengl_power_of_two(height);
	} else {
		texture->w = width;
		texture->h = height;
	}

	blank = gfxCreateRGBSurface(opengl.surfaceGL, texture->w, texture->h);
	memset(blank->pixels, 0, texture->w * texture->h * blank->format->BytesPerPixel);

	if (texture->data) {
		glDeleteTextures(1, &texture->data);
	}

	glGenTextures(1, &texture->data);
	glBindTexture(GL_TEXTURE_2D, texture->data);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	if (interpolation) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	} else {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}

	if (opengl.glew && !GLEW_VERSION_3_1) {
		glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
	}

	glTexImage2D(GL_TEXTURE_2D, 0, opengl.texture.format_internal, texture->w, texture->h, 0,
	        opengl.texture.format, opengl.texture.type, blank->pixels);

	if (opengl.glew && GLEW_VERSION_3_1) {
		glGenerateMipmap(GL_TEXTURE_2D);
	}

	SDL_FreeSurface(blank);
}
void opengl_update_texture(SDL_Surface *surface) {
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, opengl.texture.data);

	if (opengl.glew && !GLEW_VERSION_3_1) {
		glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
	}

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, surface->w);

	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, surface->w, surface->h, opengl.texture.format,
	        opengl.texture.type, surface->pixels);

	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

	if (opengl.glew && GLEW_VERSION_3_1) {
		glGenerateMipmap(GL_TEXTURE_2D);
	}

	if (opengl.glsl && text.on_screen && (opengl.shader != 255)) {
		glBindTexture(GL_TEXTURE_2D, shader.text.data);
	}

	/* disabilito l'uso delle texture */
	//glDisable(GL_TEXTURE_2D);
}
void text_blit_opengl(_txt_element *ele, SDL_Rect *dst_rect) {
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, shader.text.data);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, dst_rect->w);

	glTexSubImage2D(GL_TEXTURE_2D, 0, dst_rect->x, dst_rect->y, dst_rect->w, dst_rect->h,
	        opengl.texture.format, opengl.texture.type, ele->surface->pixels);

	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

	glDisable(GL_TEXTURE_2D);
}

int opengl_power_of_two(int base) {
	int pot = 1;

	while (pot < base) {
		pot <<= 1;
	}
	return (pot);
}


char *file2string(const char *path)
{
	FILE *fd;
	long len,
		 r;
	char *str;

	if (!(fd = fopen(path, "r")))
	{
		fprintf(stderr, "Can't open file '%s' for reading\n", path);
		return NULL;
	}

	fseek(fd, 0, SEEK_END);
	len = ftell(fd);

	printf("File '%s' is %ld long\n", path, len);

	fseek(fd, 0, SEEK_SET);

	if (!(str = malloc(len * sizeof(char))))
	{
		fprintf(stderr, "Can't malloc space for '%s'\n", path);
		return NULL;
	}

	r = fread(str, sizeof(char), len, fd);

	str[r - 1] = '\0'; /* Shader sources have to term with null */

	fclose(fd);

	return str;
}
void printLog(GLuint obj)
{
	int infologLength = 0;
	int maxLength;

	if(glIsShader(obj))
		glGetShaderiv(obj,GL_INFO_LOG_LENGTH,&maxLength);
	else
		glGetProgramiv(obj,GL_INFO_LOG_LENGTH,&maxLength);

	char infoLog[maxLength];

	if (glIsShader(obj))
		glGetShaderInfoLog(obj, maxLength, &infologLength, infoLog);
	else
		glGetProgramInfoLog(obj, maxLength, &infologLength, infoLog);

	if (infologLength > 0)
		printf("%s\n",infoLog);
}
