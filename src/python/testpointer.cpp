#include "python/embeddedModule.hpp"
#include <stdio.h>

class B{
public:
    B(){};
    int abc;
};


class A{
private:
    B * bptr;
public:
    A(){
        this->bptr = new(B);
    };
    int a;
    B* getBptr(){
        return this->bptr;
    }
    void printAbcOfBptr(){
        printf("%d", bptr->abc);
    }
};