/*
 * opengl.c
 *
 *  Created on: 10/dic/2010
 *      Author: fhorse
 */

#include "opengl.h"
#include "sdlgfx.h"
#include "openGL/shaders.h"
#include "openGL/no_effect.h"
#include "openGL/cube3d.h"

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
}
void sdlCreateSurfaceGL(SDL_Surface *src, WORD width, WORD height, BYTE flags) {
	GLint texture_real_width, texture_real_height;

	if (opengl.rotation) {
		opengl_set = opengl_set_cube3d;
		opengl_draw_scene = opengl_draw_scene_cube3d;
	} else {
		opengl_set = opengl_set_no_effect;
		opengl_draw_scene = opengl_draw_scene_no_effect;
	}

	if (!opengl.glew){
		GLenum err;

		if ((err = glewInit()) != GLEW_OK) {
			fprintf(stderr, "INFO: %s\n", glewGetErrorString(err));
		} else {
			opengl.glew = TRUE;
		}
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

	opengl.surfaceGL = gfxCreateRGBSurface(src, opengl_power_of_two(SCRROWS),
			opengl_power_of_two(SCRLINES));

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

		texture_real_width = opengl.surfaceGL->w * gfx.scale;
		texture_real_height = opengl.surfaceGL->h * gfx.scale;

		opengl.texture.x = (GLfloat) width / texture_real_width;
		opengl.texture.y = (GLfloat) height / texture_real_height;
	}

	opengl_create_texture(&opengl.texture.data, texture_real_width, texture_real_height);

	opengl_set(src);

	glFinish();
}

int opengl_flip(SDL_Surface *surface) {
	SDL_GL_SwapBuffers();

	return (0);
}




void opengl_create_texture(GLuint *texture, GLint texture_real_width, GLint texture_real_height) {
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

	if (texture) {
		glDeleteTextures(1, texture);
	}

	glGenTextures(1, texture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, (*texture));

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	if (opengl.glew && !GLEW_VERSION_3_1) {
#ifndef RELEASE
		fprintf(stderr, "INFO: OpenGL 3.1 not supported.\n");
#endif
		glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
	}

	glTexImage2D(GL_TEXTURE_2D, 0, opengl.texture.format_internal, opengl.surfaceGL->w,
	        opengl.surfaceGL->h, 0, opengl.texture.format, opengl.texture.type, NULL);

	if (opengl.glew && GLEW_VERSION_3_1) {
#ifndef RELEASE
		fprintf(stderr, "INFO: OpenGL 3.1 supported.\n");
#endif
		glGenerateMipmap(GL_TEXTURE_2D);
	}

	if (opengl.glew && GLEW_VERSION_2_0) {
#ifndef RELEASE
		fprintf(stderr, "INFO: OpenGL 2.0 supported. Glsl enabled.\n");
#endif
		opengl.glsl_enabled = TRUE;

		//vsSource = file2string("/home/fhorse/Dropbox/gpuPeteOGL2.slv");
		//fsSource = file2string("/home/fhorse/Dropbox/gpuPeteOGL2.slf");

		vert_shader = glCreateShader(GL_VERTEX_SHADER);
		printLog(vert_shader);
		glShaderSource(vert_shader, 1, &shader.vert_source, NULL);
		printLog(vert_shader);
		glCompileShader(vert_shader);
		printLog(vert_shader);

		frag_shader = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(frag_shader, 1, &shader.frag_source, NULL);
		glCompileShader(frag_shader);
		printLog(frag_shader);

		program = glCreateProgram();
		printLog(program);

		//free((char *) vsSource);
		//free((char *) fsSource);

		glAttachShader(program, vert_shader);
		glAttachShader(program, frag_shader);
		glLinkProgram(program);
		printLog(program);

		//glDeleteObject(vs);
		//glDeleteObject(fs);
		//glDeleteObject(sp);

		OGL2Size[0] = (GLfloat) texture_real_width; //width;
		OGL2Size[1] = (GLfloat) texture_real_height; //height;
		OGL2Size[2] = 256.0; //SCRLINES;
		OGL2Size[3] = 256.0;   //SCRROWS;

		OGL2Param[0] = 1.0 / (GLfloat) texture_real_width;
		OGL2Param[1] = 1.0 / (GLfloat) texture_real_height;
		OGL2Param[2] = 0.0;
		OGL2Param[3] = 0.0;

		glUseProgram(program);

		OGL2ParamLoc = glGetUniformLocation(program, "OGL2Param");
		OGL2SizeLoc = glGetUniformLocation(program, "OGL2Size");
		baseImageLoc = glGetUniformLocation(program, "OGL2Texture");

		printf("wave parameters location: %d %d %d\n", OGL2ParamLoc, OGL2SizeLoc, baseImageLoc);

		glUniform4fv(OGL2ParamLoc, 1, OGL2Param);
		glUniform4fv(OGL2SizeLoc, 1, OGL2Size);
		glUniform1i(baseImageLoc, 0);

		glUseProgram(0);
	}
}
void opengl_update_texture(SDL_Surface *surface) {
	glEnable(GL_TEXTURE_2D);

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

	/* disabilito l'uso delle texture */
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
