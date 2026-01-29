# Final_Project_CS50x
Here we are at the end of this project 

Title : Fined Grained Bitwise Memory Allocator

#### Video Demo:  <  https://youtu.be/nNf8yj8klkg?si=7uhmAtCULrBv1bvW  >
#### Description: 
Standard practice in C integers is of 32 - bits. If you store a number like 9(requires 4 bits), we are wasting 28 bits of Ram. In a dataset of millions, this is a massive number which slows down the cache performance especially for the IOT based devices. 

THe Solution: 
In this program where the Bit- Level Memory Allocator in C, This doesnt uses the standard arrays, it asks for the minimum bits required for a specific range and then it basically packs the data into those memory pool.
Core Features: 
1. This uses the hardware - level instructions as to determine the bit - width needed at the runtime.
2. Croos Boundary Stitching - If the value starts at the 60 of a 64 bit word then my algorithm mathematically spilsts and stores the remaining 3 bits in the next words without any data corruption.
3. Encoding/Decoding : This uses the custom bit-masks and then shifts ( << , >> , | , &) as to bypass the standard byte addressing.

Result : 
In a test 1 million items of range 0 to 100 standard int array which consumes 4.0 Mega Byte that memory is reduced to 0.88 MB. That is a 78% memory reduction with 100% data integrity.
