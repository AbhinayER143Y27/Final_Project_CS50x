#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

// fined grained bitwise memory allocator
typedef struct {
    uint64_t* data;
    size_t total_items;
    int bits_per_item;
    size_t pool_size_bytes;
} BitArray;

BitArray* create_bit_array(size_t total_items, int max_value_range) {
    BitArray* array = (BitArray*)malloc(sizeof(BitArray));
    if (!array) return NULL;

    array->total_items = total_items;

    if (max_value_range == 0) array->bits_per_item = 1;
    else array->bits_per_item = 32 - __builtin_clz(max_value_range);

    size_t total_bits = total_items * array->bits_per_item;
    
    size_t total_words = (total_bits + 63) / 64;
    
    array->pool_size_bytes = total_words * sizeof(uint64_t);
    
    array->data = (uint64_t*)calloc(total_words, sizeof(uint64_t));
    if (!array->data) {
        free(array);
        return NULL;
    }

    return array;
}

// Patent Claim 2: The Encoder (Bit Injection)
void set_value(BitArray* array, size_t index, uint32_t value) {
    if (index >= array->total_items) return;

    uint64_t val_64 = value & ((1ULL << array->bits_per_item) - 1);

    size_t start_bit = index * array->bits_per_item;
    size_t word_index = start_bit / 64;
    size_t bit_offset = start_bit % 64;

    // CASE 1: Value fits entirely into the current 64-bit word
    if (bit_offset + array->bits_per_item <= 64) {
        uint64_t mask = ((1ULL << array->bits_per_item) - 1) << bit_offset;
        array->data[word_index] &= ~mask; // Clear
        array->data[word_index] |= (val_64 << bit_offset); // Set
    } 
    // CASE 2: Value splits across two words (The Cross-Boundary Bridge) -- god knows
    else {
        int bits_in_first = 64 - bit_offset;
        int bits_in_second = array->bits_per_item - bits_in_first;

        // Part 1: Fill end of first word
        uint64_t mask1 = (1ULL << bits_in_first) - 1;
        array->data[word_index] &= ~(mask1 << bit_offset);
        array->data[word_index] |= ((val_64 & mask1) << bit_offset);

        // Part 2: Fill start of second word
        uint64_t mask2 = (1ULL << bits_in_second) - 1;
        array->data[word_index + 1] &= ~mask2;
        array->data[word_index + 1] |= ((val_64 >> bits_in_first) & mask2);
    }
}

// Patent Claim 2: The Decoder (Bit Extraction)
uint32_t get_value(BitArray* array, size_t index) {
    if (index >= array->total_items) return 0;

    size_t start_bit = index * array->bits_per_item;
    size_t word_index = start_bit / 64;
    size_t bit_offset = start_bit % 64;

    // CASE 1: Read from single word
    if (bit_offset + array->bits_per_item <= 64) {
        uint64_t mask = (1ULL << array->bits_per_item) - 1;
        return (uint32_t)((array->data[word_index] >> bit_offset) & mask);
    } 
    // CASE 2: Stitch together from two words
    else {
        int bits_in_first = 64 - bit_offset;
        
        uint64_t mask1 = (1ULL << bits_in_first) - 1;
        uint64_t part1 = (array->data[word_index] >> bit_offset) & mask1;
        
        uint64_t part2 = array->data[word_index + 1]; 
        
        // Combine the parts -- sticthing the parting
        return (uint32_t)(part1 | ((part2 & ((1ULL << (array->bits_per_item - bits_in_first)) - 1)) << bits_in_first));
    }
}

void free_bit_array(BitArray* array) {
    if (array) {
        free(array->data);
        free(array);
    }
}

// ---- BENCHMARKING ---- we can see now where the paths has led us to

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <max_value>\n", argv[0]);
        fprintf(stderr, "Try: %s 200\n", argv[0]);
        return 1;
    }

    int MAX_VALUE = atoi(argv[1]);
    const size_t DATASET_SIZE = 1000000;

    printf("=== FINE-GRAINED MEMORY ALLOCATOR BENCHMARK ===\n");
    printf("Dataset Size: %zu Items\n", DATASET_SIZE);
    printf("Value Range:  0 to %d\n", MAX_VALUE);

    int bits_needed = (MAX_VALUE == 0) ? 1 : 32 - __builtin_clz(MAX_VALUE);
    printf("Bit Depth:    %d bits per item (Standard int uses 32)\n", bits_needed);
    printf("-----------------------------------------------\n");

    size_t standard_size = DATASET_SIZE * sizeof(int);
    int* standard_array = (int*)malloc(standard_size);
    
    for(size_t i=0; i<DATASET_SIZE; i++) {
        standard_array[i] = i % MAX_VALUE;
    }

    BitArray* packed_array = create_bit_array(DATASET_SIZE, MAX_VALUE);
    
    for(size_t i=0; i<DATASET_SIZE; i++) {
        set_value(packed_array, i, i % MAX_VALUE);
    }

    int mismatch = 0;
    for(size_t i=0; i<DATASET_SIZE; i++) {
        if (standard_array[i] != get_value(packed_array, i)) {
            mismatch = 1;
            printf("CRITICAL FAILURE at index %zu. Expected %d, Got %u\n", 
                   i, standard_array[i], get_value(packed_array, i));
            break;
        }
    }
    if (!mismatch) printf("Status: Data Integrity Verified (100%% Match)\n");

    printf("-----------------------------------------------\n");
    printf("Normal Implementation Size:   %10zu Bytes (%.2f MB)\n", 
           standard_size, standard_size / (1024.0 * 1024.0));
    
    printf("Patent Implementation Size:   %10zu Bytes (%.2f MB)\n", 
           packed_array->pool_size_bytes, packed_array->pool_size_bytes / (1024.0 * 1024.0));
    
    double saving = 100.0 * (double)(standard_size - packed_array->pool_size_bytes) / standard_size;
    printf("-----------------------------------------------\n");
    printf("MEMORY SAVING: %.2f%%\n", saving);
    printf("-----------------------------------------------\n");

    free(standard_array);
    free_bit_array(packed_array);

    return 0;
}
