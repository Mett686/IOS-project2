# IOS Project 2 – Process Synchronization

Second project for the **IOS (Operating Systems)** course, written in C.

## Overview
A multi-process synchronization solution based on classical concurrency problems. Uses POSIX semaphores and shared memory (`mmap` / `shm_open`) to coordinate multiple concurrent processes without deadlocks, race conditions, or busy waiting.

## Build & Usage
```bash
# Compile
make

# Run with project parameters
./proj2 <args>
```
