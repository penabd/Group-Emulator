#include <iostream>
#include <vector>
#include <cstdint>
#include <fstream>

const int A_START = 6;
const int B_START = 3;
const int C_START = 0;
const int OP_START = 28;

inline uint32_t getOpcode(uint32_t instr) { return instr >> OP_START; }
inline uint32_t getA(uint32_t instr) { return (instr >> A_START) & 7; }
inline uint32_t getB(uint32_t instr) { return (instr >> B_START) & 7; }
inline uint32_t getC(uint32_t instr) { return instr & 7; }

int readInProgram(const std::string &path, std::vector<uint32_t> &outputWords) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        std::cerr << "Can't open file: " << path << std::endl;
        return 1;
    }
    unsigned char oneWord[4];
    while (file.read(reinterpret_cast<char*>(oneWord), 4)) {
        uint32_t val = (uint32_t(oneWord[0]) << 24) |
                       (uint32_t(oneWord[1]) << 16) |
                       (uint32_t(oneWord[2]) << 8)  |
                       uint32_t(oneWord[3]);
        outputWords.push_back(val);
    }
    if (file.gcount() != 0) {
        std::cerr << "File size not multiple of 4 bytes." << std::endl;
        return 1;
    }
    return 0;
}

struct Machine {
    std::vector<std::vector<uint32_t>> memoryArrays;
    uint32_t regs[8]{};
    uint32_t pc = 0;
    std::vector<uint32_t> freeIds;
};

inline int cycle(Machine &machine) {
    uint32_t instr = machine.memoryArrays[0][machine.pc++];
    uint32_t op = instr >> OP_START;

    if (op == 13) {
        uint32_t a = (instr >> 25) & 7;
        machine.regs[a] = instr & 0x1FFFFFF;
        return 0;
    }

    uint32_t a = (instr >> A_START) & 7;
    uint32_t b = (instr >> B_START) & 7;
    uint32_t c = instr & 7;

    switch (op) {
        case 0:{ 
            if (machine.regs[c]) machine.regs[a] = machine.regs[b]; 
            break;
        }
        case 1:{
            machine.regs[a] = machine.memoryArrays[machine.regs[b]][machine.regs[c]]; 
            break;
        }
        case 2:{
            machine.memoryArrays[machine.regs[a]][machine.regs[b]] = machine.regs[c];
            break;
        }
        case 3:{
            machine.regs[a] = machine.regs[b] + machine.regs[c];
            break;
        }
        case 4:{
            machine.regs[a] = machine.regs[b] * machine.regs[c];
            break;
        }
        case 5:{
            if (machine.regs[c]) machine.regs[a] = machine.regs[b] / machine.regs[c];
            break;
        }
        case 6:{
            machine.regs[a] = ~(machine.regs[b] & machine.regs[c]);
            break;
        }
        case 7:{
            return 1;
        }
        case 8:{
            uint32_t id;
            if (!machine.freeIds.empty()) { 
                id = machine.freeIds.back();
                machine.freeIds.pop_back(); 
            }
            else {
                id = machine.memoryArrays.size();
                machine.memoryArrays.resize(id + 1);
            }
            machine.memoryArrays[id] = std::vector<uint32_t>(machine.regs[c], 0);
            machine.regs[b] = id;
            break;
        }
        case 9: {
            uint32_t id = machine.regs[c];
            if (id && id < machine.memoryArrays.size()) {
                machine.memoryArrays[id].clear();
                machine.freeIds.push_back(id);
            }
            break;
        }
        case 10:{
            std::putchar(static_cast<unsigned char>(machine.regs[c] & 0xFF));
            break;
        }
        case 11: {
            int ch = std::getchar();
            if (ch == EOF) {
                machine.regs[c] = 0xFFFFFFFFu;
            }
            else {
                uint32_t input = static_cast<uint32_t>(ch & 0xFF);
            }
            break;
        }
        case 12: {
            if (machine.regs[b]) machine.memoryArrays[0] = machine.memoryArrays[machine.regs[b]];
            machine.pc = machine.regs[c];
            break;
        }
    }
    return 0;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Not enough arguments. Format: executable .um\n ";
        return 1;
    }

    Machine machine;
    std::vector<uint32_t> program;

    if (readInProgram(argv[1], program)) {
        return 1;
    }
    machine.memoryArrays.resize(1);
    machine.memoryArrays[0] = std::move(program);

    while (true) {
        if (cycle(machine)) {
            break;
        }
    }
    return 0;
}

