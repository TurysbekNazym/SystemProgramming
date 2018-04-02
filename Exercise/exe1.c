#include <windows.h>
#include <tchar.h>
#include <stdio.h>

int main(int argc, LPTSTR argv[])
{
	LPTSTR str;
	
    scanf("%s",&str);
    printf("%s\n","Hello" );
    printf("%s\n",str );
    //printf("%s\n",argv[1]);
    printf("\n");
    return 0;
}
