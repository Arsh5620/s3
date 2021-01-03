# libfecc - lib forward error correction codes

## This library implements Reed Solomon error detection and correction codes in C programming language. 

## Introduction: 
This is only written for fun and is not recommended to be used without prior testing

## SIMD
By default we are targeting SSE3 or higher on Intel, although it probably will work on AMD too. 

## References
All of the following references are good reads: <BR>
Wiki at https://en.wikiversity.org/wiki/Reed%E2%80%93Solomon_codes_for_coders, and https://en.wikipedia.org/wiki/Reed%E2%80%93Solomon_error_correction<BR>
Article at BBC UK http://downloads.bbc.co.uk/rd/pubs/whp/whp-pdf-files/WHP031.pdf<BR>
Implementing SIMD in Galois Fields at http://web.eecs.utk.edu/~jplank/plank/papers/FAST-2013-GF.pdf<BR>
Horner's Method for polynomial evaluation at https://en.wikipedia.org/wiki/Horner%27s_method<BR>
Estrin's Scheme for parallel polynomial evaluation at https://en.wikipedia.org/wiki/Estrin%27s_scheme<BR>
