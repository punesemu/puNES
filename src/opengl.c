/*
 * opengl.c
 *
 *  Created on: 10/dic/2010
 *      Author: fhorse
 */

#include "opengl.h"
#include "sdlgfx.h"

char *file2string(const char *path);
void printLog(GLuint obj);
GLfloat OGL2Size[4];
GLfloat OGL2Param[4];
/*struct {
	GLfloat x;
	GLfloat y;
	GLfloat z;
	GLfloat w;
};*/
const GLchar *vsSource;
const GLchar *fsSource;
GLuint vs, /* Vertex Shader */
	   fs, /* Fragment Shader */
	   sp; /* Shader Program */
GLint baseImageLoc;
GLint OGL2SizeLoc;
GLint OGL2ParamLoc;
GLuint texture;

GLint textureIntFormat;
GLenum textureFormat, textureType;
GLint xTexturePot, yTexturePot;
GLfloat xTsh, yTsh;
float xVertex, yVertex, zVertex;
float distance;
float matrixDistance[60] = {
	-2.000f, -2.020f, -2.040f, -2.060f, -2.080f,
	-2.100f, -2.120f, -2.140f, -2.160f, -2.180f,
	-2.200f, -2.220f, -2.240f, -2.260f, -2.280f,
	-2.300f, -2.320f, -2.340f, -2.360f, -2.380f,
	-2.400f, -2.420f, -2.440f, -2.460f, -2.480f,
	-2.500f, -2.520f, -2.540f, -2.560f, -2.580f,
	-2.600f, -2.620f, -2.640f, -2.660f, -2.680f,
	-2.700f, -2.720f, -2.740f, -2.760f, -2.780f,
	-2.800f, -2.820f, -2.840f, -2.860f, -2.880f,
	-2.890f, -2.900f, -2.910f, -2.920f, -2.930f,
	-2.940f, -2.950f, -2.965f, -2.970f, -2.975f,
	-2.980f, -2.985f, -2.990f, -2.995f, -3.000f
};

void sdlInitGL(void) {
	opengl.rotation = 0;
	opengl.surfaceGL = NULL;
	opengl.texture = 0;
	opengl.flagsOpengl = SDL_HWSURFACE | SDL_OPENGL;
	opengl.factorDistance = 0;
	opengl.xRotate = 0;
	opengl.yRotate = 0;
	opengl.xDiff = 0;
	opengl.yDiff = 0;
}
void sdlCreateSurfaceGL(SDL_Surface *src, WORD width, WORD height, BYTE flags) {
	if (!opengl.glew){
		GLenum err;

		if ((err = glewInit()) != GLEW_OK) {
			fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
		} else {
			opengl.glew = TRUE;
		}
	}

	if (!opengl.surfaceGL) {
		glPushAttrib(GL_ALL_ATTRIB_BITS);
	} else {
		SDL_FreeSurface(opengl.surfaceGL);
	}

	/*
	 * ripristino gli attributi opengl ai valori
	 * iniziali e li salvo nuovamente.
	 */
	glPopAttrib();
	glPushAttrib(GL_ALL_ATTRIB_BITS);

	opengl.surfaceGL = gfxCreateRGBSurface(src, width, height);

	// contains an alpha channel
	switch (src->format->BitsPerPixel) {
		case 16:
			textureIntFormat = GL_RGB5;
			if (SDL_BYTEORDER == SDL_BIG_ENDIAN) {
				textureFormat = GL_BGR;
				textureType = GL_UNSIGNED_SHORT_5_6_5_REV;
			} else {
				textureFormat = GL_RGB;
				textureType = GL_UNSIGNED_SHORT_5_6_5;
			}
			break;
		case 24:
			textureIntFormat = GL_RGB8;
			if (SDL_BYTEORDER == SDL_BIG_ENDIAN) {
				textureFormat = GL_BGR;
			} else {
				textureFormat = GL_RGB;
			}
			textureType = GL_UNSIGNED_BYTE;
			break;
		case 32:
		default:
			/* vale sia per BIG_ENDIAN che per il LITTLE */
			//textureFormat = GL_BGRA;
			//textureType = GL_UNSIGNED_INT_8_8_8_8_REV;
			textureIntFormat = GL_RGBA8;
			textureFormat = GL_BGRA;
			textureType = GL_UNSIGNED_BYTE;
			break;
	}

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

	xVertex = 1.0f - ((1.0f / (src->w / 2)) * opengl.xTexture1);
	yVertex = 1.0f - ((1.0f / (src->h / 2)) * opengl.yTexture1);
	zVertex = xVertex;

	xTexturePot = sdlPowerOfTwoGL(width);
	yTexturePot = sdlPowerOfTwoGL(height);
	xTsh = (GLfloat) width / xTexturePot;
	yTsh = (GLfloat) height / yTexturePot;

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	/* Setup our viewport. */
	glViewport(0, 0, (GLint) src->w, (GLint) src->h);

	/*
	 * change to the projection matrix and set
	 * our viewing volume.
	 */
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	if (opengl.rotation) {
		glFrustum(-1, 1, -1, 1, 1.0f + (1.0f - zVertex), 100.0f);
	} else {
		glOrtho(0.0f, src->w, src->h, 0.0f, -1.0f, 1.0f);
	}

	/* Make sure we're chaning the model view and not the projection */
	glMatrixMode(GL_MODELVIEW);
	/* Reset The View */
	glLoadIdentity();

	if (opengl.rotation) {
		// Ensure correct display of polygons
		glEnable(GL_CULL_FACE);
		glEnable(GL_DEPTH_TEST);
		glDepthMask(GL_TRUE);
		/* Really Nice Perspective Calculations */
		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
		/* pulisco la scena */
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		/*
		 * disabilito l'utilizzo delle textures che
		 * utilizzero' solo quando disegnero' il cubo.
		 */
		glDisable(GL_TEXTURE_2D);
	} else {
		glDisable(GL_DEPTH_TEST);
		glDepthMask(GL_FALSE);
		/* pulisco la scena */
		glClear(GL_COLOR_BUFFER_BIT);
		/* abilito l'utilizzo delle textures */
		glEnable(GL_TEXTURE_2D);
	}

	/* genero una texture */
	if (opengl.texture) {
		glDeleteTextures(1, &opengl.texture);
	}
	glGenTextures(1, &opengl.texture);

	if (texture) {
		glDeleteTextures(1, &texture);
	}
	glGenTextures(1, &texture);

	/* indico la texture da utilizzare */
	glBindTexture(GL_TEXTURE_2D, opengl.texture);

	// select modulate to mix texture with color for shading
	//glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	// the texture wraps over at the edges (repeat)
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	if (opengl.glew) {
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		if (!GLEW_VERSION_3_1) {
#ifndef RELEASE
			fprintf(stderr, "INFO: OpenGL 3.1 not supported.\n");
#endif
			/* creo la minimap */
			glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
		}

		/* creo una texture vuota con i parametri corretti */
		glTexImage2D(GL_TEXTURE_2D, 0, textureIntFormat, xTexturePot, yTexturePot, 0,
				textureFormat, textureType, NULL);

		if (GLEW_VERSION_3_1) {
#ifndef RELEASE
			fprintf(stderr, "INFO: OpenGL 3.1 supported.\n");
#endif
			glGenerateMipmap(GL_TEXTURE_2D);
		}

		if (GLEW_VERSION_2_0) {
#ifndef RELEASE
			fprintf(stderr, "INFO: OpenGL 2.0 supported. Glsl enabled.\n");
#endif
			opengl.glsl_enabled = TRUE;

			/* Compile and load the program */

			/* The vertex shader */
			vsSource = file2string("/home/fhorse/Dropbox/gpuPeteOGL2.slv");
			fsSource = file2string("/home/fhorse/Dropbox/gpuPeteOGL2.slf");

			vs = glCreateShader(GL_VERTEX_SHADER);
			fs = glCreateShader(GL_FRAGMENT_SHADER);

			glShaderSource(vs, 1, &vsSource, NULL);
			glShaderSource(fs, 1, &fsSource, NULL);

			glCompileShader(vs);
			glCompileShader(fs);
			printLog(fs);

			free((char *) vsSource);
			free((char *) fsSource);

			sp = glCreateProgram();
			glAttachShader(sp, vs);
			glAttachShader(sp, fs);
			glLinkProgram(sp);

			/*glDeleteObject(vs);
			glDeleteObject(fs);
			glDeleteObject(sp);*/

			glUseProgram(sp);

			OGL2Size[0] = xTexturePot; //width;
			OGL2Size[1] = yTexturePot; //height;
			OGL2Size[2] = 256.0; //SCRLINES;
			OGL2Size[3] = 256.0;   //SCRROWS;

			OGL2Param[0] = 0.0;
			OGL2Param[1] = 0.0;
			OGL2Param[2] = 0.0;
			OGL2Param[3] = 0.0;

			OGL2ParamLoc = glGetUniformLocation(sp, "OGL2Param");
			OGL2SizeLoc = glGetUniformLocation(sp, "OGL2Size");
			baseImageLoc = glGetUniformLocation(sp, "OGL2Texture");

			printf("wave parameters location: %d %d %d\n", OGL2ParamLoc, OGL2SizeLoc, baseImageLoc);

			glUniform4fv(OGL2ParamLoc, 1, OGL2Param);
			glUniform4fv(OGL2SizeLoc, 1, OGL2Size);
			glUniform1i(baseImageLoc, 0);
			printLog(sp);
		}


	} else {
		/* setto le proprieta' di strecthing della texture */
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		/* creo una texture vuota con i parametri corretti */
		glTexImage2D(GL_TEXTURE_2D, 0, textureIntFormat, xTexturePot, yTexturePot, 0,
				textureFormat, textureType, NULL);
	}

	glFinish();
}
int sdlFlipScreenGL(SDL_Surface *surface) {
	/* ripulisco la scena opengl */
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, surface->w);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, surface->w, surface->h, textureFormat,
			textureType, surface->pixels);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, opengl.texture);
	glEnable(GL_TEXTURE_2D);

	/* disegno la texture */
	if (opengl.rotation) {
		/* resetto la matrice corrente (modelview) */
		glLoadIdentity();

		if (opengl.factorDistance < 60) {
			distance = matrixDistance[opengl.factorDistance];
			opengl.factorDistance++;
		}
		glTranslatef(0.0f, 0.0f, distance);

		if (opengl.factorDistance > 30) {
			glRotatef(opengl.xRotate, 1.0f, 0.0f, 0.0f);
			glRotatef(opengl.yRotate, 0.0f, 1.0f, 0.0f);
			/*
			 if (!mouseLeftButton) {
			 xRotate += 0.3f;
			 yRotate += 0.4f;
			 }
			 */
		}

		glColor3f(1.0f, 1.0f, 1.0f);

		/* abilito l'uso delle texture */
		glEnable(GL_TEXTURE_2D);

		/* cubo esterno */
		glBegin(GL_QUADS);
			/* avanti */
			glTexCoord2f(0.0f, yTsh); glVertex3f(-xVertex, -yVertex, +zVertex);
			glTexCoord2f(xTsh, yTsh); glVertex3f(+xVertex, -yVertex, +zVertex);
			glTexCoord2f(xTsh, 0.0f); glVertex3f(+xVertex, +yVertex, +zVertex);
			glTexCoord2f(0.0f, 0.0f); glVertex3f(-xVertex, +yVertex, +zVertex);
			/* dietro */
			glColor3f(1.0f, 0.0f, 0.0f);
			glTexCoord2f(0.0f, yTsh); glVertex3f(+xVertex, -yVertex, -zVertex);
			glTexCoord2f(xTsh, yTsh); glVertex3f(-xVertex, -yVertex, -zVertex);
			glTexCoord2f(xTsh, 0.0f); glVertex3f(-xVertex, +yVertex, -zVertex);
			glTexCoord2f(0.0f, 0.0f); glVertex3f(+xVertex, +yVertex, -zVertex);
			/* sopra */
			/*
			glTexCoord2f(0.0f, yTsh); glVertex3f(-xVertex, +yVertex, +zVertex);
			glTexCoord2f(xTsh, yTsh); glVertex3f(+xVertex, +yVertex, +zVertex);
			glTexCoord2f(xTsh, 0.0f); glVertex3f(+xVertex, +yVertex, -zVertex);
			glTexCoord2f(0.0f, 0.0f); glVertex3f(-xVertex, +yVertex, -zVertex);
			*/
			/* sotto */
			/*
			glTexCoord2f(0.0f, yTsh); glVertex3f(+xVertex, -yVertex, +zVertex);
			glTexCoord2f(xTsh, yTsh); glVertex3f(-xVertex, -yVertex, +zVertex);
			glTexCoord2f(xTsh, 0.0f); glVertex3f(-xVertex, -yVertex, -zVertex);
			glTexCoord2f(0.0f, 0.0f); glVertex3f(+xVertex, -yVertex, -zVertex);
			*/
			/* destra */
			glColor3f(0.0f, 1.0f, 0.0f);
			glTexCoord2f(0.0f, yTsh); glVertex3f(+xVertex, -yVertex, +zVertex);
			glTexCoord2f(xTsh, yTsh); glVertex3f(+xVertex, -yVertex, -zVertex);
			glTexCoord2f(xTsh, 0.0f); glVertex3f(+xVertex, +yVertex, -zVertex);
			glTexCoord2f(0.0f, 0.0f); glVertex3f(+xVertex, +yVertex, +zVertex);
			/* sinistra */
			glColor3f(0.0f, 0.0f, 1.0f);
			glTexCoord2f(0.0f, yTsh); glVertex3f(-xVertex, -yVertex, -zVertex);
			glTexCoord2f(xTsh, yTsh); glVertex3f(-xVertex, -yVertex, +zVertex);
			glTexCoord2f(xTsh, 0.0f); glVertex3f(-xVertex, +yVertex, +zVertex);
			glTexCoord2f(0.0f, 0.0f); glVertex3f(-xVertex, +yVertex, -zVertex);
		glEnd();

		/* disabilito l'uso delle texture */
		glDisable(GL_TEXTURE_2D);

		/* cubo interno */
		glBegin(GL_QUADS);
			/* avanti */
			glColor3f(1.0f, 1.0f, 1.0f);
			glVertex3f(+xVertex, -yVertex, +zVertex);
			glVertex3f(-xVertex, -yVertex, +zVertex);
			glVertex3f(-xVertex, +yVertex, +zVertex);
			glVertex3f(+xVertex, +yVertex, +zVertex);
			/* dietro */
			glColor3f(1.0f, 0.0f, 0.0f);
			glVertex3f(-xVertex, -yVertex, -zVertex);
			glVertex3f(+xVertex, -yVertex, -zVertex);
			glVertex3f(+xVertex, +yVertex, -zVertex);
			glVertex3f(-xVertex, +yVertex, -zVertex);
			/* destra */
			glColor3f(0.0f, 1.0f, 0.0f);
			glVertex3f(+xVertex, -yVertex, -zVertex);
			glVertex3f(+xVertex, -yVertex, +zVertex);
			glVertex3f(+xVertex, +yVertex, +zVertex);
			glVertex3f(+xVertex, +yVertex, -zVertex);
			/* sinistra */
			glColor3f(0.0f, 0.0f, 1.0f);
			glVertex3f(-xVertex, -yVertex, +zVertex);
			glVertex3f(-xVertex, -yVertex, -zVertex);
			glVertex3f(-xVertex, +yVertex, -zVertex);
			glVertex3f(-xVertex, +yVertex, +zVertex);
		glEnd();
	} else {
		glBegin(GL_QUADS);
			/* Bottom Left Of The Texture */
			glTexCoord2f(0.0f, 0.0f); glVertex2i(opengl.xTexture1, opengl.yTexture1);
			/* Bottom Right Of The Texture */
			glTexCoord2f(xTsh, 0.0f); glVertex2i(opengl.xTexture2, opengl.yTexture1);
			/* Top Right Of The Texture */
			glTexCoord2f(xTsh, yTsh); glVertex2i(opengl.xTexture2, opengl.yTexture2);
			/* Top Left Of The Texture */
			glTexCoord2f(0.0f, yTsh); glVertex2i(opengl.xTexture1, opengl.yTexture2);
		glEnd();
	}

	SDL_GL_SwapBuffers();

	return (0);
}
int sdlPowerOfTwoGL(int base) {
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
