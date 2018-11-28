#include <iostream>
#include "program.hpp"

using namespace std;

program riscv;

template<typename T> void show(T msg){
    cout << msg << endl;
}

int main(){
    riscv.prepare_mem();

    show("RISCV-Interpreter Simulation Start!");

    for (; ;){
        show ("type n to continue/other to exit");
        char ch;
        scanf("%c\n", ch);
        if (ch == 'n'){
            statement inst = statement(riscv.load_32(riscv.pc));
            inst.execute(riscv);
            riscv.show_status();
        }else{
            break;
        }
    }

    return 0;
}