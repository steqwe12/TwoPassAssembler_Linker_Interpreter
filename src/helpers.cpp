#include "../inc/helpers.hpp"
#include <iostream>  //#include <stdlib.h>

class instr* global_instr = nullptr;

class arg* mk_argument(char* name, int tip_arg, class arg* next){
     arg* a = new  arg();
    a->next=nullptr;
    a->name=name;
    a->tip_arg=tip_arg;
    a->next=next;
    return a;
}

class instr* mk_instruction(char* label,char* name, class arg* arg, class instr* next){
     instr* a = new  instr();
    a->args=nullptr;
    a->next=nullptr;
    a->label=label;
    a->name=name;
    a->args=arg;
    a->next=next;

    if (global_instr==nullptr) global_instr=a;
    else {
       class instr* b= global_instr;
       while (b->next!=nullptr) b=b->next;
       b->next=a;
    }

    return a;
}

void free_instrs(class instr* global_instr){
    class instr *i1,*i2;
    class arg *a1,*a2;

    i1=global_instr;
    while(i1!=nullptr){

        a1=i1->args;
        while(a1!=nullptr){
            a2=a1->next;
            delete [] a1->name;
            delete a1;
            a1=a2;
        }

        i2=i1->next;
        
        delete [] i1->label;
        delete [] i1->name;
        delete i1;
        i1=i2;
    }

    
}

void print_instrs(class instr *global_instr){
    class instr *i1;
    class arg *a1;
    i1=global_instr;

    while (i1!=nullptr){
        if(i1->label!=nullptr) std::cout << "labela: "<<i1->label;
        std::cout << " naziv: " << i1->name << "   args:";

        a1=i1->args;
        while(a1!=nullptr){
            std::cout << a1->name << " ,  ";
            a1=a1->next;
        }

        std::cout << std::endl;
        i1=i1->next;
    }
}