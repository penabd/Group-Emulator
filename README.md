# Group-Emulator

Our group had a C++ version and a C version of the emulator that each had a runtime of around 15 seconds. We decided to use C++ for our final project and combined the most optimal aspects of the two. 

We mostly used C++ libraries, although we included <cstdint> to use some C programming as well. Decoding the instructions, we used the C++ emulator but simplified it to decode each instruction once per cycle such as in the C emulator. The program loader was mostly from the C++ version, using ifstream to open the file. The machine struct is inspired by the C++ emulator, but frees memory and saves empty addresses inspired by the C emulator. For the opcodes, we used the C++ structure but used simplifications from the C version, such as case 13.

Here are our timings for the new emulator:

