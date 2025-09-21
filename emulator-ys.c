#include<stdio.h>
#include<stdlib.h>
#include<limits.h>
#include<stdint.h>

// Set an arbitrary "max number" of arrays for better memory allocation
#define NUM_ARRAYS 50000
#define NUM_ELEMS 15000

// The "unlimited" array of words
unsigned int** array;

// The registers
unsigned int registers[8] = {0, 0, 0, 0, 0, 0, 0, 0};

//The instruction number
unsigned int numIntr = 0;

// The total number of bits read
unsigned int totalBits = 0;

// The total number of bits in each array
unsigned int arrayBits[NUM_ARRAYS];

// The pointer to the next available array
unsigned int arrayPtr = 1;

int main(int argc, char* argv[]) {
  unsigned int word;

  // Must give the executable and then the name of the .um file
  if (argc < 2) {
    printf("Not enough arguments. Format: executable .um\n");
    return 0;
  }

  // allocate all arrays with 0
  array = malloc(NUM_ARRAYS * sizeof(unsigned int*));
  array[0] = malloc(NUM_ELEMS * sizeof(unsigned int));

  // set all array lengths to 0
  for (int i = 0; i < NUM_ARRAYS; i++) {
    arrayBits[i] = 0;
  }

  FILE *file = fopen(argv[1], "r");

  // read word by word, switching from big endian to little endian
  while (fread(&word, 4, 1, file)) {
    word = ((word & 0xFF) << 24) | ((word & 0xFF00) << 8) | ((word & 0xFF0000) >> 8) | ((word & 0xFF000000) >> 24);
    array[0][totalBits] = word;
    totalBits++;
  }

  arrayBits[0] = totalBits;

  fclose(file);

  // parse each word in the zero array one-by-one
  numIntr = 0;
  while (numIntr < totalBits) {
    unsigned int currentWord = array[0][numIntr];
    //printf("word: %x, ", currentWord);
    numIntr++;

    // Get the values for the instruction, a, b, and c
    unsigned int instr = currentWord >> 28;
    //printf("instruction: %x\n", instr);
    unsigned int a = (currentWord >> 6) & 0x7;
    unsigned int b = (currentWord >> 3) & 0x7;
    unsigned int c = currentWord & 0x7;

    // The op codes
    switch(instr) {
      case 0:
        if (registers[c] != 0) {
          registers[a] = registers[b];
        }
        break;

      case 1:
        if (registers[b] < NUM_ARRAYS && array[registers[b]] && registers[c] < arrayBits[registers[b]]) {
          registers[a] = array[registers[b]][registers[c]];
        }
        break;

      case 2:
        if (registers[a] < NUM_ARRAYS && array[registers[a]] && registers[b] < arrayBits[registers[a]]) {
          array[registers[a]][registers[b]] = registers[c];
        }
        break;

      case 3:
        registers[a] = (registers[b] + registers[c]);
        break;

      case 4:
        registers[a] = (registers[b] * registers[c]);
        break;

      case 5:
        if (registers[c] != 0) {
          registers[a] = registers[b] / registers[c];
        }
        break;

      case 6:
        registers[a] = ~(registers[b] & registers[c]);
        break;

      case 7:
        break;

      case 8: {
        unsigned int size = registers[c];
        for (int i = arrayPtr; i < NUM_ARRAYS; i++) {
          if (array[i] == NULL) {
            array[i] = calloc(size, sizeof(unsigned int));
            arrayBits[i] = size;
            registers[b] = i;
            arrayPtr = i + 1;
            break;
          }
        }
        break;
      }
      
      case 9:
        if (registers[c] > 0 && registers[c] < NUM_ARRAYS && array[registers[c]]) {
          free(array[registers[c]]);
          array[registers[c]] = NULL;
          arrayBits[registers[c]] = 0;
        }
        if (registers[c] < arrayPtr) {
          arrayPtr = registers[c];
        }
        
        break;

      case 10:
        putchar(registers[c]);
        break;

      case 11: {
        int input = getchar();
        if (input >= 0 && input <= 255) {
          registers[c] = input;
        }
        else {
          registers[c] = 0xFFFFFFFF;
        }
        break;

      }
      
      case 12: {
        if (registers[b] > 0 && registers[b] < NUM_ARRAYS && array[registers[b]]) {
          unsigned int size = arrayBits[registers[b]];
          free(array[0]);
          array[0] = malloc(size * sizeof(unsigned int));
          for (unsigned int i = 0; i < size; i++) {
            array[0][i] = array[registers[b]][i];
          }
          arrayBits[0] = size;
          totalBits = size;
        }
        numIntr = registers[c];
        break;
      }

      case 13:
        a = (currentWord >> 25) & 7;
        registers[a] = currentWord & 0x1FFFFFF;
        break;

      default:
        break;
    }
  }

  for (int i = 0; i < NUM_ARRAYS; i++) {
    free(array[i]);
  }
  free(array);

  return 0;
}
