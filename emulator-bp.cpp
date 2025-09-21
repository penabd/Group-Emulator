#include <iostream>
#include <unordered_map>
#include <vector>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <bits/stdc++.h>


int WORD_SIZE = 32;
int BYTE_SIZE = 8;
int A_START = 6;
int B_START = 3;
int C_START = 0;
int OP_START = 28;
int A_BYTE = 24;
int B_BYTE = 16;
int C_BYTE = 8;

inline uint32_t getOpcode(uint32_t instruction){
    uint32_t opcode = (instruction >> OP_START) & 0xF;
    return opcode;
}

inline uint32_t getRegisterA(uint32_t instruction){
    uint32_t registerLoc = (instruction >> A_START) & 7;
    return registerLoc;
}

inline uint32_t getRegisterB(uint32_t instruction){
    uint32_t registerLoc = (instruction >> B_START) & 7;
    return registerLoc;
}

inline uint32_t getRegisterC(uint32_t instruction){
    uint32_t registerLoc = (instruction >> C_START) & 7;
    return registerLoc;
}

int readInProgram(std::string& inputPath, std::vector<uint32_t>& outputWords){
    std::ifstream inputStream(inputPath, std::ios::binary);

    if (!inputStream) {
        std::cerr << "Can't open file: " << inputPath << std::endl;
        return 1;
    }

    unsigned char oneWord[4];
    while (inputStream.read(reinterpret_cast<char*>(oneWord), 4)) {
        uint32_t formattedWord = (uint32_t(oneWord[0]) << A_BYTE)
                                 | (uint32_t(oneWord[1]) << B_BYTE) 
                                 | (uint32_t(oneWord[2]) << C_BYTE) 
                                 |  uint32_t(oneWord[3]);

        outputWords.push_back(formattedWord);
    }

    if (inputStream.gcount() != 0) {
        std::cerr << "File size is not a multiple of 4 bytes." << std::endl;
        return 1;
    }

    return 0;
}


struct Machine {
    std::vector<std::vector<uint32_t>> memoryArrays;
    
    uint32_t registers[8];
    uint32_t programCounter;
    uint32_t nextId = 1;
    std::vector<uint32_t> freeAddresses;

    Machine() : programCounter(0) {
        for (int i = 0; i < 8; ++i){
            registers[i] = 0;
        }
    }
};


int cycle(Machine &machine){
    uint32_t instruction = machine.memoryArrays[0][machine.programCounter];
    uint32_t opcode = getOpcode(instruction);
    machine.programCounter++;

    uint32_t regA = 0;
    uint32_t regB = 0;
    uint32_t regC = 0;
    
    // std::cout << "opcode " << opcode << std::endl;
    // std::cout << "instruction " << instruction << std::endl;
    
    switch(opcode){
        case 0:{
             regA = getRegisterA(instruction);
             regB = getRegisterB(instruction);
             regC = getRegisterC(instruction);

            if(machine.registers[regC] != 0){
                machine.registers[regA] = machine.registers[regB];
            }
            break;
        }
        case 1:{
             regA = getRegisterA(instruction);
             regB = getRegisterB(instruction);
             regC = getRegisterC(instruction);

            uint32_t offset = machine.registers[regC];
            uint32_t arrayID = machine.registers[regB];

            machine.registers[regA] = machine.memoryArrays[arrayID][offset];
            break;
        }

        case 2:{
             regA = getRegisterA(instruction);
             regB = getRegisterB(instruction);
             regC = getRegisterC(instruction);

            uint32_t offset = machine.registers[regB];
            uint32_t arrayID = machine.registers[regA];

            machine.memoryArrays[arrayID][offset] = machine.registers[regC];
            break;
        }
        case 3:{
             regA = getRegisterA(instruction);
             regB = getRegisterB(instruction);
             regC = getRegisterC(instruction);

            // TODO: double check this is right
            machine.registers[regA] = (machine.registers[regB] + machine.registers[regC]); //% (1 << 32);
            break;
        }

        case 4:{
             regA = getRegisterA(instruction);
             regB = getRegisterB(instruction);
             regC = getRegisterC(instruction);

            // TODO: double check 
            machine.registers[regA] = (machine.registers[regB] * machine.registers[regC]); //% (1 << 32);
            break;
        }

        case 5:{
             regA = getRegisterA(instruction);
             regB = getRegisterB(instruction);
             regC = getRegisterC(instruction);

            // TODO: double check 
            machine.registers[regA] = (machine.registers[regB] / machine.registers[regC]); //% (1 << 32);
            break;  
        }
      
        case 6:{
             regA = getRegisterA(instruction);
             regB = getRegisterB(instruction);
             regC = getRegisterC(instruction);
            
            uint32_t complValRegB = ~machine.registers[regB];
            uint32_t complValRegC = ~machine.registers[regC];

            machine.registers[regA] = complValRegB | complValRegC;
            break;
        }
        case 7:{
            machine.programCounter = machine.memoryArrays[0].size();
            break;
        }

        case 8:{
            regB = getRegisterB(instruction);
            regC = getRegisterC(instruction);
            
            uint32_t numWords = machine.registers[regC];
            uint32_t newId;

            if (!machine.freeAddresses.empty()) {
                newId = machine.freeAddresses.back();
                machine.freeAddresses.pop_back();
            } else {
                newId = machine.memoryArrays.size();
                machine.memoryArrays.resize(newId + 1);
            }

            machine.memoryArrays[newId] = std::vector<uint32_t>(numWords, 0);
            machine.registers[regB] = newId;
            break;
        }

        case 9:{
            regC = getRegisterC(instruction);
            uint32_t addressToFree = machine.registers[regC];

            if(addressToFree < machine.memoryArrays.size() && addressToFree != 0){
                machine.memoryArrays[addressToFree].clear();
                machine.freeAddresses.push_back(addressToFree);
            }
            break;
        }
        case 10:{
            regC = getRegisterC(instruction);
            // std::cout << machine.registers[regC] << std::endl;
            std::putchar(static_cast<unsigned char>(machine.registers[regC]));   
            break;
        }

        case 11:{
            regC = getRegisterC(instruction);

            std::cout << "Enter input: ";
            int ch = std::getchar(); // Read a character from stdin

            if (ch == EOF) {
                machine.registers[regC] = 0xFFFFFFFFu;  // all 1s
            } else {
                uint32_t input = static_cast<uint32_t>(static_cast<unsigned char>(ch));
                
                if(input > 255 || input < 0){
                    std::cerr << "Input must be < 255 and > 0." << std::endl;
                    return 1;
                }
                machine.registers[regC] = input;
            }
            break;
        }


        case 12:{
            regB = getRegisterB(instruction);
            regC = getRegisterC(instruction);

            machine.memoryArrays[0] = machine.memoryArrays[machine.registers[regB]];
            machine.programCounter = machine.registers[regC];
            break;
        }

        case 13:{
            regA = (instruction >> 25) & 0x7;
            uint32_t value = instruction & 0x1FFFFFF;

            machine.registers[regA] = value;
            break;
        }

    }


    return 0;
}

int main(int argc, char* argv[]) {

    Machine machine;
    
    std::string programPath = argv[1];
    std::vector<uint32_t> program;

    if(readInProgram(programPath, program)){
        std::cerr << "Problem reading program." << std::endl;
        return 1;
    }else{
        machine.memoryArrays.resize(1);   
        machine.memoryArrays[0] = program;
        machine.programCounter = 0;
    }

    while (true)
    {
        if(machine.programCounter >= machine.memoryArrays[0].size()){
            return 0;
        }

        int cycleFailed = cycle(machine);

        if(cycleFailed){
            std::cout << "Cycle failed" << std::endl;
            return cycleFailed;
        }
    }

    return 0;
}
