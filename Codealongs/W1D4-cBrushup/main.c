#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void method1(int n){
	printf("In method1 with n=%d\n", n);
	n = 30;
}

void method2(int* p){
	printf("In method2 with *p=%d\n", *p);
	*p = 100;
}

int main(int argc, char* argv[]) {
	/*int a = 10;
	int* p = &a;

	printf("a = %d and *p = %d but p = %x and &a = %x\n", a, *p, p, &a);

	method1(*p);

	printf("a = %d and *p = %d but p = %x and &a = %x\n", a, *p, p, &a);

	method2(p);

	printf("a = %d and *p = %d but p = %x and &a = %x\n", a, *p, p, &a);
	*/

	if(argc < 2){
		printf("Usage: ./main filename\n");
		return 1;
	}

	char* filename = argv[1];

	FILE* fp = fopen(filename, "r");
	if(fp == NULL) {
		printf("File not found: %s", filename);
		return 1;
	}

	char curWord[100];
	char* longestWord = NULL;
	int largest = 0;

	while(fscanf(fp, "%s", curWord) == 1){
		int curLength = strlen(curWord);
		if(curLength > largest){
			if(longestWord != NULL){
				free(longestWord);
			}

			longestWord = (char*) malloc(curLength * sizeof(char));
			strncpy(longestWord, curWord, curLength);
			largest = curLength;
		}	
	}
	fclose(fp);
	printf("The longest word \"%s\" is of size %d\n", longestWord, largest);
	free(longestWord);
	return 0;
}