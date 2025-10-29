    #include <stdio.h>
	#include <stdbool.h>
    #include <string.h>
    #include <stdlib.h>
    #include <stdint.h>

	//Configuration variables
	int NUM_SETS;
	int ASSOC;
    int REPLACEMENT; // 0 = LRU & 1 = FIFO
    int WB; //0 = write through & 1 = write back
    int CACHE_SIZE;
	#define BLOCK_SIZE 64 // 64 byte block size

	// dynamic arrays
	long long int **tag_array;
	long long int **lru_position;
	bool **dirty;
    long long int **fifo_index;
    bool **valid;
    long long int counter = 0;

	// Variables to maintain the simulation statistics
	int Hit = 0;
	int Miss = 0;
    int writes = 0;
    int reads = 0;

	// Forward declarations
	void Update_lru(uint64_t add);
	void Update_fifo(uint64_t add);

	// Function to simulate cache access
	void Simulate_access(char op, uint64_t add) {
        int set = (add / BLOCK_SIZE) % NUM_SETS;
        long long int tag = add / BLOCK_SIZE;
        bool hit = false;
        
        // Normalize lowercase ops
        if (op == 'r') op = 'R';
        if (op == 'w') op = 'W';

        // Cache hit scenario
        for(int i = 0; i < ASSOC; i++) {
            if(tag == tag_array[set][i] && valid[set][i]) {
                Hit++;
                hit = true;

                // Choose policy based on the configuration
                if(REPLACEMENT == 0) {
                    Update_lru(add);
                }
                
                //write hit
                if(op == 'W') {
                    if(WB == 1){
                        dirty[set][i] = true; // Mark dirty on write back
                    } else {
                        writes++; // write thru
                    }
                } 
                break;
                
            }
        }

        // Cache miss scenario
        if(!hit){
            Miss++;
            reads++; // always reads 
            int evict = -1;
            bool empty = false;

            //check for empty block
            for(int i = 0; i < ASSOC; i++){
                if(!valid[set][i]){
                    evict = i;
                    empty = true;
                    break;
                }
            }
            

            if(!empty){
                //LRU Policy
                if(REPLACEMENT == 0){
                    evict = 0;
                    for(int i = 1; i < ASSOC; i++){
                        if(lru_position[set][i] < lru_position[set][evict]){
                            evict = i;
                        }
                    }
                }
                    
                //FIFO Policy
                else{
                    evict = 0;
                    long long int min = fifo_index[set][0];
                    for(int i = 1; i < ASSOC; i++){
                        if(fifo_index[set][i] < min){
                            min = fifo_index[set][i];
                            evict = i;
                        }
                    }
                }

                //checks if evicted block is dirty
                if(dirty[set][evict] && WB ==1){
                    writes++;
                }
            }

            //fill new block
            tag_array[set][evict] = tag;
            valid[set][evict] = true;
            dirty[set][evict] = false;

            if (REPLACEMENT == 0) {
                Update_lru(add);
            } else {
                Update_fifo(add);
            }
            
            //handles any write misses
            if(op == 'W') {
                if(WB == 1){
                    dirty[set][evict] = true; // Mark dirty on write back
                } else {
                    writes++; // write thru
                }
            } 
        } 
	}

	//LRU policy updates
	void Update_lru (uint64_t add) {
        int set = (add / BLOCK_SIZE) % NUM_SETS;
        long long int tag = add / BLOCK_SIZE;
        int index = -1;

        for(int i = 0; i < ASSOC; i++) {
            if(tag_array[set][i] == tag && valid[set][i]) {
                index = i;
                break;
            }
        }

        if(index != -1){
            counter++;
            lru_position[set][index] = counter;
        }
	}

    //FIFO policy updates
	void Update_fifo (uint64_t add) {
        int set = (add / BLOCK_SIZE) % NUM_SETS;
        long long int tag = add / BLOCK_SIZE;
        int index = -1;

        for(int i = 0; i < ASSOC; i++) {
            if(tag_array[set][i] == tag && valid[set][i]) {
                index = i;
                break;
            }
        }

        if(index != -1){
            counter++;
            fifo_index[set][index] = counter;
        }
    }

	int main (int argc, char *argv[]) {
        //parse arguments
        if(argc != 6){
            return 0;
        }

        if (sscanf(argv[1], "%d", &CACHE_SIZE) != 1 ||
            sscanf(argv[2], "%d", &ASSOC) != 1 ||
            sscanf(argv[3], "%d", &REPLACEMENT) != 1 ||
            sscanf(argv[4], "%d", &WB) != 1) {
            printf("Error: Invalid arguments\n");
            return 1;
        }
        char *traceFile = argv[5];
        NUM_SETS = CACHE_SIZE / (BLOCK_SIZE * ASSOC);

        // allocate space for arrays
        tag_array = (long long int **)malloc(NUM_SETS * sizeof(long long int *));
        lru_position = (long long int **)malloc(NUM_SETS * sizeof(long long int *));
        dirty = (bool **)malloc(NUM_SETS * sizeof(bool *));
        fifo_index = (long long int **)malloc(NUM_SETS * sizeof(long long int *));
        valid = (bool **)malloc(NUM_SETS * sizeof(bool *));

        for (int i = 0; i < NUM_SETS; i++) {
            tag_array[i] = (long long int *)malloc(ASSOC * sizeof(long long int));
            lru_position[i] = (long long int *)malloc(ASSOC * sizeof(long long int));
            dirty[i] = (bool *)malloc(ASSOC * sizeof(bool));
            fifo_index[i] = (long long int *)malloc(ASSOC * sizeof(long long int));
            valid[i] = (bool *)malloc(ASSOC * sizeof(bool));
        }

        char op;
        uint64_t add;
        FILE * file = fopen (traceFile, "r");

        // Initialize cache
        for (int i = 0; i < NUM_SETS; i++) {
            for (int j = 0; j < ASSOC; j++) {
                tag_array[i][j] = 0;
                dirty[i][j] = false;
                valid[i][j] = false;
                lru_position[i][j] = 0;
                fifo_index[i][j] = 0;
            }
        }


        // Check if the file opened successfully
        if(!file) {
            printf("Error: Could not open the trace file.\n");
            return 1;
        }

        // Read until end of file
        while (fscanf(file, " %c %llx", &op, &add ) == 2) {
            Simulate_access(op, add);
        }

        fclose(file);

        // Print out the statistics
        double missRatio = (double)Miss / (Hit + Miss);
        //print hits and misses for debugging
        //printf("Hits: %d\n", Hit);
        //printf("Misses: %d\n", Miss);
        printf("Miss Ratio: %.6f\n", missRatio);
        printf("Writes: %d\n", writes);
        printf("Reads: %d\n", reads);

        //Free memory
        for (int i = 0; i < NUM_SETS; i++) {
            free(tag_array[i]);
            free(lru_position[i]);
            free(dirty[i]);
            free(fifo_index[i]);
            free(valid[i]);
        }
        free(tag_array);
        free(lru_position);
        free(dirty);
        free(fifo_index);
        free(valid);

        return 0;
    }
