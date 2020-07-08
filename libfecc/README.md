# libfecc - lib forward error correction codes

## This library's primary purpose is to implement reed solomon error detection and correction codes in C programming language. 

## Introduction: 
The library is written in C99 with an intent of using it as backend to provide ecc support for storage media. It is in no way complete or ready for production use, and is still a work in progress. Our main focus is to provide error correction, thus we entirely omit the erasure correction part of the RS codes. Wherever possible we try to use the modern features of the processors such as SIMD. We also make certain assumptions in regards of the requirements of the code that will help with optimizations. 

## SIMD use
Although we try to keep the code easy to read and understand, but we are not going to compromise speed to achieve this. SIMD will be used extensively wherever it will lead to a performance gain. By default we are targeting SSE3 or higher by Intel (R). 

## References
Although not complete, here are all the references that we used:<BR>
Wiki at https://en.wikiversity.org/wiki/Reed%E2%80%93Solomon_codes_for_coders, and https://en.wikipedia.org/wiki/Reed%E2%80%93Solomon_error_correction<BR>
Article at BBC UK http://downloads.bbc.co.uk/rd/pubs/whp/whp-pdf-files/WHP031.pdf<BR>
Implementing SIMD in Galois Fields at http://web.eecs.utk.edu/~jplank/plank/papers/FAST-2013-GF.pdf<BR>
Horner's Method for polynomial evaluation at https://en.wikipedia.org/wiki/Horner%27s_method<BR>
Estrin's Scheme for parallel polynomial evaluation at https://en.wikipedia.org/wiki/Estrin%27s_scheme<BR>

