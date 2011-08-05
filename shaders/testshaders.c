#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <SDL.h>
#include <GL/glew.h>


int main(int argc, char *argv[]) {
	int i;

	if (argc < 2) {
		printf("usage %s: shader [shader] ...\n", argv[0]);
		exit(0);
	}

	SDL_Init(SDL_INIT_VIDEO);
	SDL_SetVideoMode(1, 1, 32, SDL_OPENGL);
	glewInit();

	if (!GLEW_ARB_vertex_program) {
		printf("GL_ARB_vertex_program not supported!\n");
		exit(1);
	}

	if (!GLEW_ARB_fragment_program) {
		printf("GL_ARB_fragment_program not supported!\n");
		exit(1);
	}


	glEnable(GL_VERTEX_PROGRAM_ARB);
	glEnable(GL_FRAGMENT_PROGRAM_ARB);

	for (i = 1; i < argc; i++) {
		GLuint handle;
		GLenum err;
		int n;
		struct stat statbuf;
		size_t len;
		FILE *file = fopen(argv[i], "r");

		printf("Shader: %s\n", argv[i]);
		if (file == NULL) {
			printf("Load failed(%s)\n", strerror(errno));
			continue;
		}

		n = fileno(file);
		fstat(n, &statbuf);
		len = statbuf.st_size;
		char *buf = malloc(len);
		fread(buf, len, 1, file);
		GLenum progType;
		if (strncmp("!!ARBvp1.0", buf, 10) == 0)
		    progType = GL_VERTEX_PROGRAM_ARB;
		else
                    progType = GL_FRAGMENT_PROGRAM_ARB;


		glGenProgramsARB(1, &handle);
		glBindProgramARB(progType, handle);

		glProgramStringARB(progType, GL_PROGRAM_FORMAT_ASCII_ARB, len, buf);

		err = glGetError();
		if (err != 0) {
			printf("Error 0x%04x:%s\n", err, glGetString(GL_PROGRAM_ERROR_STRING_ARB));
		} else {
			printf("Success!\n");
		}

        free(buf);
		fclose(file);
	}


	return 0;
}
