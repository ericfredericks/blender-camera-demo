#include "shaderprog.h"



static void *fileIntoBuffer(const char*);
static size_t getFileSize(const char*);



GLuint shaderprog_load(const char *vFilename,const char *gFilename,const char *fFilename)
{
	GLuint id;
	char *buffer;
	int success;
	char infoLog[512];
	GLint vertexShader,geometryShader,fragmentShader;



	vertexShader = glCreateShader(GL_VERTEX_SHADER);
	if ((buffer = fileIntoBuffer(vFilename)) == NULL)
		return 0;
	glShaderSource(vertexShader,1,(const GLchar* const*)&buffer,NULL);
	glCompileShader(vertexShader);
	glGetShaderiv(vertexShader,GL_COMPILE_STATUS,&success);
	if (!success)
	{
		glGetShaderInfoLog(vertexShader,512,NULL,infoLog);
		printf("%s:\n%s",vFilename,infoLog);
		free(buffer);
		return 0;
	}
	free(buffer);



	if (gFilename != NULL)
	{
		geometryShader = glCreateShader(GL_GEOMETRY_SHADER);
		if ((buffer = fileIntoBuffer(gFilename)) == NULL)
			return 0;
		glShaderSource(geometryShader,1,(const GLchar* const*)&buffer,NULL);
		glCompileShader(geometryShader);
		glGetShaderiv(geometryShader,GL_COMPILE_STATUS,&success);
		if (!success)
		{
			glGetShaderInfoLog(geometryShader,512,NULL,infoLog);
			printf("%s:\n%s",gFilename,infoLog);
			free(buffer);
			return 0;
		}
		free(buffer);
	}



	fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	if ((buffer = fileIntoBuffer(fFilename)) == NULL)
		return 0;
	glShaderSource(fragmentShader,1,(const GLchar* const*)&buffer,NULL);
	glCompileShader(fragmentShader);
	glGetShaderiv(fragmentShader,GL_COMPILE_STATUS,&success);
	if (!success)
	{
		glGetShaderInfoLog(fragmentShader,512,NULL,infoLog);
		printf("%s:\n%s",fFilename,infoLog);
		free(buffer);
		return 0;
	}
	free(buffer);



	id = glCreateProgram();
	glAttachShader(id,vertexShader);
	if (gFilename != NULL)
		glAttachShader(id,geometryShader);
	glAttachShader(id,fragmentShader);
	glLinkProgram(id);
	glGetProgramiv(id,GL_LINK_STATUS,&success);
	if (!success)
	{
		glGetProgramInfoLog(id,512,NULL,infoLog);
		printf("GL:\n%s",infoLog);
		return 0;
	}
	
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);
	if (gFilename != NULL)
		glDeleteShader(geometryShader);



	return id;
}



static void *fileIntoBuffer(const char *filename)
{
	FILE *file;
	unsigned char *buffer;
	if ((file = fopen(filename,"rb")) == NULL)
	{
		perror("fopen");
		return 0;
	}
	size_t len = getFileSize(filename);
	if ((buffer = calloc(len,1)) == NULL)
	{
		perror("calloc");
		return 0;
	}
	fread(buffer,1,len,file);
	fclose(file);
	return buffer;
}
static size_t getFileSize(const char *filename)
{
	size_t result = 0;
#if defined (_WIN64)
	struct _stat64 st;
	wchar_t filenameB[100];
	mbstowcs(filenameB,filename,100);
	_wstat64(filenameB,&st);
#elif defined (__linux__)
	struct stat st;
	stat(filename,&st);
#endif
	result = (size_t)st.st_size;
	return result;
}



