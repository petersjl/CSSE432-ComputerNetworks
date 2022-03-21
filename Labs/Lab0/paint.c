/* File: paint.c 
   Author: Joseph Peters
   
*/
#define CAN_COVERAGE 200

#include <stdio.h>
#include <math.h>

/*Optional functions, uncomment the next two lines
 * if you want to create these functions after main: */
float readDimension(const char* name){
    float val;
    printf("Enter %s > ", name);
    scanf("%f", &val);
    return val;
}
float calcArea(float width, float height, float depth){
    return 2*(width*height + width*depth + height*depth);
}

int main(int argc, char** argv){
    float width = readDimension("width");
    float height = readDimension("height");
    float depth = readDimension("depth");
    float area = calcArea(width, height, depth);
    int cans = (int) ceil(area / CAN_COVERAGE);
    printf("Total area: %f\nNumber of cans required: %d\n", area, cans);
    return 0;
}
