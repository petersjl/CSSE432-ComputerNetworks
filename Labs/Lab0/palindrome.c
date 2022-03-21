/* File: palindrome.c 
  Author: 
   
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h> //allows to use "bool" as a boolean type
#include <ctype.h>

/*Optional functions, uncomment the next two lines
  * if you want to create these functions after main: */
//bool readLine(char** line, size_t* size, size_t* length);

/* 
  * NOTE that I used char** for the line above... this is a pointer to
  * a char pointer.  I used this because of the availability of
  * a newer function getline which takes 3 arguments (you should look it
  * up) and the first argument is a char**.  You can create a char*, say
  * called var, and to make it a char** just use &var when calling this
  * function.  If this is too confusing, you can use fgets instead.  Feel
  * free to change the function prototypes as you need them.
  * Also, note the use of size_t as a type.  You can look this up, but
  * essentially, this is just a special type of int to track sizes of
  * things like strings...
*/

bool isPalindrome(const char* line, size_t len){
  int i = 0;
  int j = (int) len;

  while (true) {
    while (!isalpha(line[i]) && i < len) i++;
    while (!isalpha(line[j]) && j >= 0) --j;
    if(i >= j) return true;
    if(tolower(line[i]) != tolower(line[j])) return false;
    ++i;
    j--;
  }
}

int main(int argc, char *argv[]){
  size_t size = 64;
  char* string = (char*) malloc(64*sizeof(char));
  while(true){
    printf("word> ");
    size_t char_count = getline(&string, &size, stdin);
    string[(int) char_count - 1] = '\0';
    char_count -= 1;
    if(string[0] == '.') break;
    if(isPalindrome(string, char_count)){
      printf("\"%s\" is a palindrome\n", string);
    }else{
      printf("\"%s\" is not a palindrome\n", string);
    }
  }
  free(string);
}
