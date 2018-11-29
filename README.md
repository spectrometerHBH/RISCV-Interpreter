# RISCV-Interpreter
RISCV-Interpreter for CPU Project for debug use,  
which may help when debugging large amount of AS codes in simulation.  
By spectrometer  

## specification
It is compatible with TA's test system, which can load .data file that build_test.sh generates into mem and execute.  
You can choose to execute one by one or jump to the instruction you want to watch.  
You can modify some debug functions for specific use.  

## compile & run
g++ main.cpp -o sim  
./sim  

## how to use  
n   - next inst  
j x - jump to xth inst(not counting S & B instr)  

