// Dummy file routines that just output to the screen
//
#include <stdio.h>
#include <oskit/console.h>

FILE *stdin = (FILE*)1;
FILE *stdout = (FILE*)2;
FILE *stderr = (FILE*)3;

FILE *fopen(const char *path, const char *mode)
{
	return stdout;
}

int fclose(FILE *fp)
{
	return 0;
}

int fflush(FILE *stream)
{
	return 0;
}

int setvbuf(FILE *stream, char *buf, int mode , size_t size)
{
	return 0;
}

size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream)
{
	console_putbytes(ptr, size * nmemb);
	return nmemb;
}

