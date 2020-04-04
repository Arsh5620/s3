# Memory Debug information

### Problem Statement:
What we are trying to do is to track the memory allocations for the program. Such as when we are doing malloc to allocate the memory, we want to keep track of how much memory was allocated, and if this memory was rellocated, or freed later. 

### How we solve it:
We are going to override the default malloc, realloc, calloc and free with own functions that are prepended with a "m_". If the variable "*DEBUG_MEMORY*" is defined during compilation, the compiler will add the code for keeping track of memory, otherwise it will inline the functions to the defaults provided by the operating system. 

### What needs to be done (TODO): 
1. Setup logging output for the memory allocations. 
2. Add asserts.
3. Removing the allocation from memory tracking once it has been freed.

### Technical Details:
The way we are going to be tracking memory allocations is making use of a combination of array list and hash table. 
 
**Why not use hash table only?** (For now!!)
The reason we are not using hash table only is because of a system limitation. If you were to allocate say 4kb of memory, and you were given address 0xabcd by the system, and then you free this memory after use. Once you will call the next malloc if the size is less than 4kb, the OS will give you the same address again 0xabcd. 

### More Information: 

Not applicable, will update :)
