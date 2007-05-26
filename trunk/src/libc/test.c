#include <stdio.h>

int main(void) {
	long long test = 0x12345678;

	char buffer[256];
	char ctest[16] = "test";
	int i=123, j=456;

	test *= 16;
	printf("error: line %d op(%s) sect(%d) mand(%d): brick %llx does not exist\n", __LINE__, ctest, i, j, test);
	return 0;
}
int     console_putchar(int c)
{
	fputc(c, stdout);
	
        return 1;
}


int     console_putbytes(const char *s, int len)
{
	int i;
	for (i = 0; i < len; i++)
		console_putchar(s[i]);
	return 0;
}
