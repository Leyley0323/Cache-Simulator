# Computer Architecture Project:

## Cache Simulator

A flexible cache simulator that models different cache configurations, replacement policies, and write policies for computer architecture analysis.

## Project Overview

This cache simulator implements a configurable L1 cache with support for:
- **Replacement Policies**: LRU (Least Recently Used) and FIFO (First In First Out)
- **Write Policies**: Write-through and Write-back
- **Variable Configurations**: Adjustable cache size and associativity

The simulator processes memory access traces and reports cache performance metrics including miss ratio, memory reads, and memory writes.

## Features

- Dynamic cache configuration via command-line arguments
- Support for 64-bit memory addresses
- LRU and FIFO replacement policy implementations
- Write-back and write-through policy support
- Dirty bit tracking for write-back cache
- Comprehensive statistics reporting


### Compilation via Terminal
```bash
gcc SIM.c -o SIM
```

## Usage

```bash
./SIM <CACHE_SIZE> <ASSOC> <REPLACEMENT> <WB> <TRACE_FILE>
```

### Parameters

| Parameter | Description | Valid Values |
|-----------|-------------|--------------|
| `CACHE_SIZE` | Cache size in bytes | Positive integer (e.g., 32768 for 32KB) |
| `ASSOC` | Cache associativity | Positive integer (e.g., 1, 2, 4, 8) |
| `REPLACEMENT` | Replacement policy | `0` = LRU, `1` = FIFO |
| `WB` | Write policy | `0` = Write-through, `1` = Write-back |
| `TRACE_FILE` | Path to memory trace file | Full path to trace file |

### Example Commands

```bash
# 32KB cache, 8-way associative, LRU, write-through
./SIM 32768 8 0 0 smallTest.t

# 32KB cache, 8-way associative, FIFO, write-back
./SIM 32768 8 1 1 mediumTest.t

# 16KB cache, 4-way associative, LRU, write-back
./SIM 16384 4 0 1 /home/TRACES/MCF.t
```

## Input Format

The trace file should contain memory operations in the following format:
```
R 0x2356257
W 0x257777
R 0x1a2b3c4
```

Where:
- First column: Operation type (`R` for read, `W` for write)
- Second column: Hexadecimal memory address

## Output Format

The simulator outputs three metrics:
```
<miss_ratio>
<memory_writes>
<memory_reads>
```

Example output:
```
0.143000
392
143
```

Where:
- **Miss Ratio**: Total misses / Total accesses
- **Memory Writes**: Number of writes to main memory
- **Memory Reads**: Number of reads from main memory

## Cache Configuration Details

### Block Size
- Fixed at **64 bytes** for all configurations

### Set Calculation
```
NUM_SETS = CACHE_SIZE / (BLOCK_SIZE × ASSOC)
```

### Address Breakdown
```
Tag = Address / BLOCK_SIZE
Set Index = (Address / BLOCK_SIZE) % NUM_SETS
```

## Replacement Policies

### LRU (Least Recently Used)
- Updates on both cache hits and misses
- Evicts the block that was least recently accessed
- Uses timestamp counters to track access order

### FIFO (First In First Out)
- Updates only on cache misses (insertions)
- Evicts the block that was inserted earliest
- Uses insertion timestamp counters

## Write Policies

### Write-Through (WB = 0)
- Writes to cache and memory simultaneously
- No dirty bit tracking needed
- Higher memory traffic but simpler coherence

### Write-Back (WB = 1)
- Writes only to cache, marks block as dirty
- Writes to memory only when dirty block is evicted
- Lower memory traffic but requires dirty bit management

## Testing

### Sample Test Cases

The project includes test files for verification:

**Small Test (1,000 addresses):**
```bash
./SIM 32768 8 0 0 smallTest.t
```
Expected Output:
```
0.143000
392
143
```

**Medium Test (10,000 addresses):**
```bash
./SIM 32768 8 0 0 mediumTest.t
```
Expected Output:
```
0.112800
2537
1128
```

**Write-back Test:**
```bash
./SIM 32768 8 0 1 mediumTest.t
```
Expected Output:
```
0.112800
34
1128
```

**FIFO Test:**
```bash
./SIM 32768 8 1 0 mediumTest.t
```
Expected Output:
```
0.117800
2537
1178
```

## Implementation Details

### Data Structures
- **tag_array**: Stores tags for each cache block
- **valid**: Tracks validity of each cache block
- **dirty**: Tracks dirty status for write-back policy
- **lru_position**: Timestamp counters for LRU policy
- **fifo_index**: Timestamp counters for FIFO policy

### Key Functions
- `Simulate_access()`: Handles cache access simulation
- `Update_lru()`: Updates LRU timestamp counters
- `Update_fifo()`: Updates FIFO timestamp counters

## Performance Analysis

The simulator can be used to analyze:
- Impact of cache size on miss ratio
- Effect of associativity on performance
- Comparison between LRU and FIFO policies
- Write-back vs write-through memory traffic
