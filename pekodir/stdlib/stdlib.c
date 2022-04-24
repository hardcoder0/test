#include <stdlib.h>
#include <string.h>
#include <stdio.h>

double floor(double x) {
    long double xcopy=x<0?x*-1:x;
    unsigned int zeros=0;
    long double n=1;
    for(n=1;xcopy>n*10;n*=10,++zeros);
    for(xcopy-=n;zeros!=-1;xcopy-=n)
        if(xcopy<0)
        {
            xcopy+=n;
            n/=10;
            --zeros;
        }
    xcopy+=n;
    return x<0?(xcopy==0?x:x-(1-xcopy)):(x-xcopy);
}

double ceil(double num) {
    int inum = (int)num;
    if (num < 0 || num == (float)inum) {
        return inum;
    }
    return inum + 1;
}

char *addstr(char *strg1, char *strg2) {
    int size = strlen(strg1) + strlen(strg2); 
    char *newStr = (char *)malloc(size);
    strcat(newStr,strg1);
    strcat(newStr,strg2);
    return newStr;
}

char *mulstr(char *str, double mult) {
    int size = strlen(str)*mult;
    char *newStr = (char *)malloc(size);

    
    for(int i = 0; i < mult; i++) {
        strcat(newStr, str);
    }

    return newStr;
}

double modnum(double num1, double num2) {
    if(floor(num1) == ceil(num1) && floor(num2) == ceil(num2)) {
        return (int)num1 % (int)num2;
    } else {
        return 1.0;
    }
}

double printnum(double input) {
    printf("%g\n", input);
    return 1.0;
}

double printint(int input) {
    printf("%i\n", input);
    return 1.0;
}

double cmpstr(char *str1, char* str2) {
    return !strcmp(str1, str2);
}

double printstr(char *input) {
    printf("%s\n", input);
    return 1.0;
}

char* input(char* q) {
    char *string;
    printf("%s", q);
    scanf(" %m[^\n]", &string);
    return string;
}

double inputnum(char* q) {
    char *string;
    printf("%s", q);
    scanf(" %m[^\n]", &string);
    return strtod(string, 0);
}