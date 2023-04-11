#include <iostream>

struct t {
    int data;
    char * ptr;
};

enum f {
    EXAMPLE
};

union o {
    int data;
    char mixed;
};

int main() { // This is the entry point
    long v = -123;
    printf("Hello World!\n");
    return 0;
}