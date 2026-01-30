# Final_Project_CS50x
Here we are at the end of this project 

Title : Fined Grained Bitwise Memory Allocator

#### Video Demo:  <  https://youtu.be/nNf8yj8klkg?si=7uhmAtCULrBv1bvW  >
#### Description: 
We have been negligent with memory management in contemporary program development. Programmers frequently use a 32-bit int or a 64-bit long to store numbers that only range from 0 to 10 because RAM is measured in gigabytes. For instance, if you store the number '9' in a normal integer, you are utilizing 4 bits for meaningful data and wasting 28 bits on needless "leading zeros." This results in a hefty "Memory Tax."

This wastes about 35 gigabytes of RAM on a single array when applied to a dataset of 10 million items, which is typical in high-frequency trading or IoT sensor networks. This waste might be the difference between a functional product and a system in constrained areas such as embedded systems or edge computing devices crash. 


My project, the Fine-Grained Bitwise Memory Allocator, offers a solution to reclaim this wasted space by breaking through the constraints of "byte-alignment."

Project Overview

The Fine-Grained Bitwise Memory Allocator is a custom memory management library written in C. It bypasses the usual limits of the C type system, which forces data into 8, 16, 32, or 64-bit boundaries. Instead, my allocator calculates the minimum bit-depth needed for a given numerical range and packs data into a continuous bit stream.

If a dataset requires only 7 bits per item, my allocator uses exactly 7 bits. It sees memory as a raw pool of bits, not as an array of fixed-width boxes.

Key Features

1. Dynamic Bit-Depth Analysis

The system does not rely on hardcoded sizes. It uses the hardware-accelerated __builtin_clz (Count Leading Zeros) instruction. When the user initializes the array with a maximum value range, the allocator calculates the log2 of that range to find the smallest bit-window required. This ensures that the compression is always mathematically optimal.

2. The Cross-Boundary "Bridge" Logic

One significant challenge in bit-packing is handling values that don’t align with word boundaries. In a 64-bit system, a 7-bit value might begin at bit 60. This means 4 bits stay at the end of "Word A," and 3 bits must move into the beginning of "Word B."

I developed a complex "Stitching" algorithm (The Cross-Boundary Bridge) that uses bitwise masks and shifts to split the data during the set_value process and smoothly recombine it during the get_value process. This logic guarantees 100% data integrity, no matter where the bit-offset occurs.

3. High-Performance Bit Manipulation

To ensure the allocator is fast enough for real-world use, I avoided loops for bit-calculation. Every operation uses low-level bitwise operators:

- Bitwise AND (&): Used for creating masks to isolate specific bit ranges.
- Bitwise OR (|): Used for "injecting" data into the bit stream.
- Bitwise NOT (~): Used for clearing specific segments of memory before writing.
- Shifts (<<, >>): Used for aligning data to the correct sub-byte offset.

File Structure and Functionality

bitwise_allocator.c (or main.c): This is the core of the project. It contains the BitArray struct, which tracks the raw memory pool, the total item count, and the dynamic bit-depth.

create_bit_array(): This function acts as the constructor. It allocates a pool of uint64_t blocks. I chose 64-bit words because they match the native register size for modern CPUs, giving the best performance.

set_value(): This is the "Encoder." It calculates the exact bit-index and word-index for an item and performs the complex bit-injection.

get_value(): This is the "Decoder." It extracts the bits, shifts them to "right-align" the data, and returns it to the user as a standard 32-bit unsigned integer.

Benchmark Suite: The program includes a built-in benchmarking tool that compares the custom allocator against a standard C array. It reports the total bytes saved and confirms that every single bit was stored and retrieved correctly.

Design Choices: Why C?

I chose to implement this project in C because it is the only language that offers the transparency required for such low-level work. Languages like Python or Java abstract away memory addresses, making true bit-level control impossible. By working directly with pointers and raw heap memory in C, I demonstrated how software can interact directly with hardware limits.

Results and Impact

In my benchmark tests with a dataset of 1,000,000 items with a range of 0-100 (requiring 7 bits):

Standard Implementation: 4,000,000 Bytes (4.0 MB)

My Implementation: 875,000 Bytes (0.88 MB)

Total Savings: 78.12%

This outcome is transformative for applications like:

- IoT/Edge Computing: Allowing sensors to store 4x more data locally before needing a battery-draining Wi-Fi sync.
- Bioinformatics: Packing DNA sequences (which only have 4 possible values) into 2-bit windows.

Global Reach: My knowledge of Spanish and Russian allows me to see the potential for this in localized, low-power hardware distributed in developing regions where memory-efficient code can extend the life of older devices.
