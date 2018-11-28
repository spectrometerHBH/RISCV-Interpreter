#ifndef _PROGRAM_H_
#define _PROGRAM_H_

#include <bitset>
#include <fstream>
using namespace std;


uint32_t num(string s){
    bitset<32> b(s);
    return b.to_ulong();
}

const uint32_t opcode_lui   = num("0110111");
const uint32_t opcode_auipc = num("0010111");
const uint32_t opcode_jal   = num("1101111");
const uint32_t opcode_jalr  = num("1100111");
const uint32_t opcode_b     = num("1100011");
const uint32_t opcode_l     = num("0000011");
const uint32_t opcode_s     = num("0100011");
const uint32_t opcode_ri    = num("0010011");
const uint32_t opcode_rr    = num("0110011");
const uint32_t funct3_beq   = num("000");
const uint32_t funct3_bne   = num("001");
const uint32_t funct3_blt   = num("100");
const uint32_t funct3_bge   = num("101");
const uint32_t funct3_bltu  = num("110");
const uint32_t funct3_bgeu  = num("111");
const uint32_t funct3_lb    = num("000");
const uint32_t funct3_lh    = num("001");
const uint32_t funct3_lw    = num("010");
const uint32_t funct3_lbu   = num("100");
const uint32_t funct3_lhu   = num("101");
const uint32_t funct3_sb    = num("000");
const uint32_t funct3_sh    = num("001");
const uint32_t funct3_sw    = num("010");
const uint32_t funct3_add   = num("000");
const uint32_t funct3_sub   = num("000");
const uint32_t funct3_sll   = num("001");
const uint32_t funct3_slt   = num("010");
const uint32_t funct3_sltu  = num("011");
const uint32_t funct3_xor   = num("100");
const uint32_t funct3_srl   = num("101");
const uint32_t funct3_sra   = num("101");
const uint32_t funct3_or    = num("110");
const uint32_t funct3_and   = num("111");

class immediate{
public:
    bitset<32> sext_imm;
    bitset<32> uext_imm;

    immediate(){
        sext_imm.reset();
        uext_imm.reset();
    }

    immediate(uint32_t x, int l, int r) : sext_imm(x), uext_imm(x){
        for (int i = 0; i < l; i++){
            sext_imm[i] = 0;
            uext_imm[i] = 0;
        }
        for (int i = r + 1; i < 32; i++){
            sext_imm[i] = sext_imm[r];
            uext_imm[i] = 0;
        }
    }

    uint32_t sval(){
        return sext_imm.to_ulong();
    }

    uint32_t uval(){
        return uext_imm.to_ulong();
    }
};


class program{
public:
    uint32_t pc, next_pc, prev_pc;
    uint32_t reg[32];
    uint8_t mem[1 << 17];
    long long counter = 0;

    program() {
        pc = 0;
        memset(reg, 0, sizeof(reg));
        memset(mem, 0, sizeof(mem));
    }

    void set_reg(int rd, uint32_t x){
        if (rd != 0) reg[rd] = x;
    }

    void prepare_mem(){
        ifstream fin("/home/spectrometer/Arch2018/riscv/test/test.data");
        //ifstream fin("/home/spectrometer/Chaos/data/test.data");
        string s;
        uint32_t addr = 0;
        while (fin >> s){
            if (s[0] == '@'){
                addr = (uint32_t)strtoll(s.substr(1, 8).c_str(), NULL, 16);
            }else{
                mem[addr] = (uint8_t)strtoll(s.c_str(), NULL, 16); 
                addr = addr + 1;
            }
        }
        fin.close();
    }

    uint32_t loadu_8(int addr){
        return mem[addr];
    }

    uint32_t loadu_16(int addr){
        return mem[addr] | (mem[addr + 1] << 8);
    }

    uint32_t load_8(int addr){
        uint8_t p = (mem[addr] & 0x80) ? 0xff : 0x0;
        return mem[addr] | (p << 8) | (p << 16) | (p << 24);
    }

    uint32_t load_16(int addr){
        uint8_t p = (mem[addr + 1] & 0x80) ? 0xff : 0x0;
        return mem[addr] | (mem[addr + 1] << 8) | (p << 16) | (p << 24);
    }

    uint32_t load_32(int addr){
        return mem[addr] | (mem[addr + 1] << 8) | (mem[addr + 2] << 16) | (mem[addr + 3] << 24);
    }

    void mem_w(int addr, uint8_t x){
        printf("write %#X to mem[%#X]\n", x, addr);
        if (addr >= (1 << 17)) return;
        mem[addr] = x;
    }

    void save_8(int addr, uint32_t x){
        //mem[addr] = x & 0x000000ff;
        mem_w(addr, x & 0x000000ff);
    }

    void save_16(int addr, uint32_t x){
        //mem[addr]     = x & 0x000000ff;
        //mem[addr + 1] = (x & 0x0000ff00) >> 8;
        mem_w(addr, x & 0x000000ff);
        mem_w(addr + 1, (x & 0x0000ff00) >> 8);
    }

    void save_32(int addr, uint32_t x){
        //mem[addr]     = x & 0x000000ff;
        //mem[addr + 1] = (x & 0x0000ff00) >> 8;
        //mem[addr + 2] = (x & 0x00ff0000) >> 16;
        //mem[addr + 3] = (x & 0xff000000) >> 24;
        mem_w(addr, x & 0x000000ff);
        mem_w(addr + 1, (x & 0x0000ff00) >> 8);
        mem_w(addr + 2, (x & 0x00ff0000) >> 16);
        mem_w(addr + 3, (x & 0xff000000) >> 24);
    }

    void show_status(){
        cout << "counter : " << counter << endl;
        printf("PC = %#X\n", prev_pc);
        for (int i = 31; i >= 0; i--){
            printf("x[%d] = %#X\n", i, reg[i]);
        }
    }
};

class statement{
public:
    bitset<32> inst;
    uint32_t opcode, funct3, funct7, shamt;
    uint32_t rs1, rs2, rd;
    immediate IImm, SImm, BImm, UImm, JImm;

    uint32_t cut(int l, int r){
        uint32_t x = 0;
        for (int i = r; i >= l; i--)
            x = (x << 1) | inst[i];
        return x;
    }

    void show(){
        printf("%#X\n", inst.to_ulong());
        cout << inst << endl;
        printf("%#X\n", opcode);
        cout << "rs1, rs2, rd : " << rs1 << " " << rs2 << " " << rd << " " << endl;
    }

    statement(uint32_t x) : inst(x){
        opcode = cut(0, 6);
        funct3 = cut(12, 14);
        funct7 = cut(25, 31);
        shamt  = cut(20, 24);

        rs1 = cut(15, 19);
        rs2 = cut(20, 24);
        rd  = cut(7, 11);

        IImm = immediate(cut(20, 31), 0, 11);
        SImm = immediate(cut(7, 11) | (cut(25, 31) << 5), 0, 11);
        BImm = immediate((cut(8, 11) << 1) | (cut(25, 30) << 5) | (cut(7, 7) << 11) | (cut(31, 31) << 12), 1, 12);
        UImm = immediate(cut(12, 31) << 12, 12, 31);
        JImm = immediate((cut(21, 30) << 1) | (cut(20, 20) << 11) | (cut(12, 19) << 12) | (cut(31, 31) << 20), 1, 20);
    }

    void execute(program & prog){
        prog.counter++;
        prog.next_pc = prog.pc + 4;

        if (opcode == opcode_lui){
            //LUI
            prog.set_reg(rd, UImm.uval());

        }else if (opcode == opcode_auipc){
            //AUIPC
            prog.set_reg(rd, UImm.uval() + prog.pc);

        }else if (opcode == opcode_jal){
            //JAL
            prog.next_pc = prog.pc + JImm.sval();
            prog.set_reg(rd, prog.pc + 4);

        }else if (opcode == opcode_jalr){
            //JALR
            prog.next_pc = (prog.reg[rs1] + IImm.sval()) & 0xfffffffe;
            prog.set_reg(rd, prog.pc + 4);

        }else if (opcode == opcode_b){
            prog.counter--;
            uint32_t taken = prog.pc + BImm.sval();
            uint32_t untaken = prog.pc + 4;

            if (funct3 == funct3_beq){
                //BEQ
                prog.next_pc = (prog.reg[rs1] == prog.reg[rs2]) ? taken : untaken;

            }else if (funct3 == funct3_bne){
                //BNE
                prog.next_pc = (prog.reg[rs1] != prog.reg[rs2]) ? taken : untaken;

            }else if (funct3 == funct3_blt){
                //BLT
                prog.next_pc = ((int)prog.reg[rs1] < (int)prog.reg[rs2]) ? taken : untaken;

            }else if (funct3 == funct3_bltu){
                //BLTU
                prog.next_pc = (prog.reg[rs1] < prog.reg[rs2]) ? taken : untaken;

            }else if (funct3 == funct3_bge){
                //BGE
                prog.next_pc = ((int)prog.reg[rs1] >= (int)prog.reg[rs2]) ? taken : untaken;

            }else if (funct3 == funct3_bgeu){
                //BGEU
                prog.next_pc = (prog.reg[rs1] >= prog.reg[rs2]) ? taken : untaken;

            }

        }else if (opcode == opcode_l){
            int addr = prog.reg[rs1] + IImm.sval();

            if (funct3 == funct3_lb){
                //LB
                prog.reg[rd] = prog.load_8(addr);

            }else if (funct3 == funct3_lh){
                //LH
                prog.reg[rd] = prog.load_16(addr);

            }else if (funct3 == funct3_lw){
                //LW
                prog.reg[rd] = prog.load_32(addr);

            }else if (funct3 == funct3_lbu){
                //LBU
                prog.reg[rd] = prog.loadu_8(addr);

            }else if (funct3 == funct3_lhu) {
                //LHU
                prog.reg[rd] = prog.loadu_16(addr);

            }
        }else if (opcode == opcode_s){
            prog.counter--;

            int addr = prog.reg[rs1] + SImm.sval();
            uint32_t data = prog.reg[rs2];

            if (funct3 == funct3_sb){
                //SB
                prog.save_8(addr, data);

            }else if (funct3 == funct3_sh){
                //SH
                prog.save_16(addr, data);

            }else if (funct3 == funct3_sw){
                //SW
                prog.save_32(addr, data);

            }
        }else if (opcode == opcode_ri){
            if (funct3 == funct3_add){
                //ADDI
                prog.set_reg(rd, IImm.sval() + prog.reg[rs1]);

            }else if (funct3 == funct3_slt){
                //SLTI
                prog.set_reg(rd, (int)prog.reg[rs1] < (int)(IImm.sval()) ? 1 : 0);

            }else if (funct3 == funct3_sltu){
                //SLTU
                prog.set_reg(rd, prog.reg[rs1] < (IImm.sval()) ? 1 : 0);

            }else if (funct3 == funct3_xor){
                //XORI
                prog.set_reg(rd, IImm.sval() ^ prog.reg[rs1]);

            }else if (funct3 == funct3_or){
                //ORI
                prog.set_reg(rd, IImm.sval() | prog.reg[rs1]);

            }else if (funct3 == funct3_and){
                //ANDI
                prog.set_reg(rd, IImm.sval() & prog.reg[rs1]);

            }else if (funct3 == funct3_sll && funct7 == num("0000000")){
                //SLLI
                prog.set_reg(rd, prog.reg[rs1] << shamt);

            }else if (funct3 == funct3_srl && funct7 == num("0000000")){
                //SRLI
                prog.set_reg(rd, prog.reg[rs1] >> shamt);

            }else if (funct3 == funct3_sra && funct7 == num("0100000")){
                //SRAI
                prog.set_reg(rd, (uint32_t )(((int)prog.reg[rs1]) >> shamt));

            }
        }else if (opcode == opcode_rr){
            if (funct3 == funct3_add && funct7 == num("0000000")){
                //ADD
                prog.set_reg(rd, prog.reg[rs1] + prog.reg[rs2]);

            }else if (funct3 == funct3_sub && funct7 == num("0100000")) {
                //SUB
                prog.set_reg(rd, prog.reg[rs1] - prog.reg[rs2]);

            }else if (funct3 == funct3_slt){
                //SLT
                prog.set_reg(rd, (int)prog.reg[rs1] < (int)(prog.reg[rs2]) ? 1 : 0);

            }else if (funct3 == funct3_sltu){
                //SLTU
                prog.set_reg(rd, prog.reg[rs1] < (prog.reg[rs2]) ? 1 : 0);

            }else if (funct3 == funct3_xor){
                //XOR
                prog.set_reg(rd, prog.reg[rs1] ^ prog.reg[rs2]);

            }else if (funct3 == funct3_or){
                //OR
                prog.set_reg(rd, prog.reg[rs1] | prog.reg[rs2]);

            }else if (funct3 == funct3_and){
                //AND
                prog.set_reg(rd, prog.reg[rs1] & prog.reg[rs2]);

            }else if (funct3 == funct3_sll && funct7 == num("0000000")){
                //SLL
                prog.set_reg(rd, prog.reg[rs1] << (prog.reg[rs2] & (0x0000001f)));

            }else if (funct3 == funct3_srl && funct7 == num("0000000")){
                //SRL
                prog.set_reg(rd, prog.reg[rs1] >> (prog.reg[rs2] & (0x0000001f)));

            }else if (funct3 == funct3_sra && funct7 == num("0100000")){
                //SRA
                prog.set_reg(rd, (uint32_t )(((int)prog.reg[rs1]) >> (prog.reg[rs2] & 0x0000001f)));
            }
        }
        prog.prev_pc = prog.pc;
        prog.pc = prog.next_pc;
    }
};

#endif