#include <stdio.h>

int main()
{
    int i;
    printf("Symbols: [");
    for(i=33;i<127;i++)
	printf("%c ", i);
    for(i=127;i<192;i++)
	printf(". ");
    for(i=192;i<256;i++)
	printf("%c ", i);
    printf("]\n");
    return 0;
}
