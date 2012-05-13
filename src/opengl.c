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

	if (shader.texture_text) {
		glDeleteTextures(1, &shader.texture_text);
	}
	delete_shader()
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

	if (opengl.scale == X1) {
		opengl.surfaceGL = gfxCreateRGBSurface(src, opengl_power_of_two(SCRROWS),
				opengl_power_of_two(SCRLINES));
	} else {
		opengl.surfaceGL = gfxCreateRGBSurface(src, opengl_power_of_two(width),
				opengl_power_of_two(height));
	}

	/* FIXME: funzionera' anche con il filtro NTSC ?!? */
	opengl.texture.x = (GLfloat) width /  (opengl.surfaceGL->w * opengl.factor);
	opengl.texture.y = (GLfloat) height / (opengl.surfaceGL->h * opengl.factor);

	opengl_create_texture(&opengl.texture.data, opengl.surfaceGL->w, opengl.surfaceGL->h,
	        POWER_OF_TWO);

	if (opengl.glew && GLEW_VERSION_2_0) {
		opengl.glsl = TRUE;

		delete_shader()

		if (opengl.shader != SHADER_NONE) {

			//opengl_create_texture(&shader.texture_text, surfaceSDL->w, surfaceSDL->h,
			//       NO_POWER_OF_TWO);
			opengl_create_texture(&shader.texture_text, surfaceSDL->w, surfaceSDL->w,
			        NO_POWER_OF_TWO);

			shader.routine = &shader_routine[opengl.shader];

			//vsSource = file2string("/home/fhorse/Dropbox/gpuPeteOGL2.slv");
			//fsSource = file2string("/home/fhorse/Dropbox/gpuPeteOGL2.slf");

			shader.vert = glCreateShader(GL_VERTEX_SHADER);
			printLog(shader.vert);
			glShaderSource(shader.vert, 1, &shader.routine->vert, NULL);
			printLog(shader.vert);
			glCompileShader(shader.vert);
			printLog(shader.vert);

			shader.frag = glCreateShader(GL_FRAGMENT_SHADER);
			glShaderSource(shader.frag, 1, &shader.routine->frag, NULL);
			glCompileShader(shader.frag);
			printLog(shader.frag);

			shader.program = glCreateProgram();
			printLog(shader.program);

			//free((char *) vsSource);
			//free((char *) fsSource);

			glAttachShader(shader.program, shader.vert);
			glAttachShader(shader.program, shader.frag);
			glLinkProgram(shader.program);
			printLog(shader.program);

			shader.size[0] = (GLfloat) opengl.surfaceGL->w;
			shader.size[1] = (GLfloat) opengl.surfaceGL->h;
			shader.size[2] = 256.0; //SCRLINES;
			shader.size[3] = 256.0; //SCRROWS;

			shader.param[0] = 1.0 / (GLfloat) opengl.surfaceGL->w;
			shader.param[1] = 1.0 / (GLfloat) opengl.surfaceGL->h;
			shader.param[2] = 0.0;
			shader.param[3] = 0.0;

			glUseProgram(shader.program);

			shader.param_loc = glGetUniformLocation(shader.program, "param");
			shader.size_loc = glGetUniformLocation(shader.program, "size");
			shader.texture_screen_loc = glGetUniformLocation(shader.program, "texture_screen");
			shader.texture_text_loc = glGetUniformLocation(shader.program, "texture_text");

			glUniform4fv(shader.param_loc, 1, shader.param);
			glUniform4fv(shader.size_loc, 1, shader.size);

			glEnable(GL_TEXTURE_2D);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, opengl.texture.data);
			glUniform1i(shader.texture_screen_loc, 0);

			glEnable(GL_TEXTURE_2D);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, shader.texture_text);
			glUniform1i(shader.texture_text_loc, 1);
		}

		glUseProgram(0);
	} else {
		shader.program = 0;
	}

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

void opengl_create_texture(GLuint *texture, uint32_t width, uint32_t height, uint8_t pow) {
	SDL_Surface *blank;

	if (pow) {
		width = opengl_power_of_two(width);
		height = opengl_power_of_two(height);
	}

	blank = gfxCreateRGBSurface(opengl.surfaceGL, width, height);
	memset(blank->pixels, 0, width * height * blank->format->BytesPerPixel);

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

	if ((*texture)) {
		glDeleteTextures(1, texture);
	}

	glGenTextures(1, texture);
	glBindTexture(GL_TEXTURE_2D, (*texture));

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	if (0 == 1) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	} else {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}

	if (opengl.glew && !GLEW_VERSION_3_1) {
		glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
	}

	glTexImage2D(GL_TEXTURE_2D, 0, opengl.texture.format_internal, width, height, 0,
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
		glBindTexture(GL_TEXTURE_2D, shader.texture_text);
	}

	/* disabilito l'uso delle texture */
	//glDisable(GL_TEXTURE_2D);
}
void text_blit_opengl(_txt_element *txt, SDL_Rect *dst_rect) {
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, shader.texture_text);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, dst_rect->w);

	glTexSubImage2D(GL_TEXTURE_2D, 0, dst_rect->x, dst_rect->y, dst_rect->w, dst_rect->h,
	        opengl.texture.format, opengl.texture.type, txt->surface->pixels);

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
