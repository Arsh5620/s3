	.file	"polynomials.c"
# GNU C17 (Arch Linux 9.3.0-1) version 9.3.0 (x86_64-pc-linux-gnu)
#	compiled by GNU C version 9.3.0, GMP version 6.2.0, MPFR version 4.0.2, MPC version 1.1.0, isl version isl-0.21-GMP

# GGC heuristics: --param ggc-min-expand=100 --param ggc-min-heapsize=131072
# options passed:  polynomials.c -mtune=generic -march=x86-64
# -auxbase-strip poly.s -O3 -fverbose-asm
# options enabled:  -fPIC -fPIE -faggressive-loop-optimizations
# -falign-functions -falign-jumps -falign-labels -falign-loops
# -fassume-phsa -fasynchronous-unwind-tables -fauto-inc-dec
# -fbranch-count-reg -fcaller-saves -fcode-hoisting
# -fcombine-stack-adjustments -fcommon -fcompare-elim -fcprop-registers
# -fcrossjumping -fcse-follow-jumps -fdefer-pop
# -fdelete-null-pointer-checks -fdevirtualize -fdevirtualize-speculatively
# -fdwarf2-cfi-asm -fearly-inlining -feliminate-unused-debug-types
# -fexpensive-optimizations -fforward-propagate -ffp-int-builtin-inexact
# -ffunction-cse -fgcse -fgcse-after-reload -fgcse-lm -fgnu-runtime
# -fgnu-unique -fguess-branch-probability -fhoist-adjacent-loads -fident
# -fif-conversion -fif-conversion2 -findirect-inlining -finline
# -finline-atomics -finline-functions -finline-functions-called-once
# -finline-small-functions -fipa-bit-cp -fipa-cp -fipa-cp-clone -fipa-icf
# -fipa-icf-functions -fipa-icf-variables -fipa-profile -fipa-pure-const
# -fipa-ra -fipa-reference -fipa-reference-addressable -fipa-sra
# -fipa-stack-alignment -fipa-vrp -fira-hoist-pressure
# -fira-share-save-slots -fira-share-spill-slots
# -fisolate-erroneous-paths-dereference -fivopts -fkeep-static-consts
# -fleading-underscore -flifetime-dse -floop-interchange
# -floop-unroll-and-jam -flra-remat -flto-odr-type-merging -fmath-errno
# -fmerge-constants -fmerge-debug-strings -fmove-loop-invariants
# -fomit-frame-pointer -foptimize-sibling-calls -foptimize-strlen
# -fpartial-inlining -fpeel-loops -fpeephole -fpeephole2 -fplt
# -fpredictive-commoning -fprefetch-loop-arrays -free -freg-struct-return
# -freorder-blocks -freorder-blocks-and-partition -freorder-functions
# -frerun-cse-after-loop -fsched-critical-path-heuristic
# -fsched-dep-count-heuristic -fsched-group-heuristic -fsched-interblock
# -fsched-last-insn-heuristic -fsched-rank-heuristic -fsched-spec
# -fsched-spec-insn-heuristic -fsched-stalled-insns-dep -fschedule-fusion
# -fschedule-insns2 -fsemantic-interposition -fshow-column -fshrink-wrap
# -fshrink-wrap-separate -fsigned-zeros -fsplit-ivs-in-unroller
# -fsplit-loops -fsplit-paths -fsplit-wide-types -fssa-backprop
# -fssa-phiopt -fstack-protector-strong -fstdarg-opt -fstore-merging
# -fstrict-aliasing -fstrict-volatile-bitfields -fsync-libcalls
# -fthread-jumps -ftoplevel-reorder -ftrapping-math -ftree-bit-ccp
# -ftree-builtin-call-dce -ftree-ccp -ftree-ch -ftree-coalesce-vars
# -ftree-copy-prop -ftree-cselim -ftree-dce -ftree-dominator-opts
# -ftree-dse -ftree-forwprop -ftree-fre -ftree-loop-distribute-patterns
# -ftree-loop-distribution -ftree-loop-if-convert -ftree-loop-im
# -ftree-loop-ivcanon -ftree-loop-optimize -ftree-loop-vectorize
# -ftree-parallelize-loops= -ftree-partial-pre -ftree-phiprop -ftree-pre
# -ftree-pta -ftree-reassoc -ftree-scev-cprop -ftree-sink
# -ftree-slp-vectorize -ftree-slsr -ftree-sra -ftree-switch-conversion
# -ftree-tail-merge -ftree-ter -ftree-vrp -funit-at-a-time -funswitch-loops
# -funwind-tables -fverbose-asm -fversion-loops-for-strides
# -fzero-initialized-in-bss -m128bit-long-double -m64 -m80387
# -malign-stringops -mavx256-split-unaligned-load
# -mavx256-split-unaligned-store -mfancy-math-387 -mfp-ret-in-387 -mfxsr
# -mglibc -mieee-fp -mlong-double-80 -mmmx -mno-sse4 -mpush-args -mred-zone
# -msse -msse2 -mstv -mtls-direct-seg-refs -mvzeroupper

	.text
	.p2align 4
	.globl	poly_new
	.type	poly_new, @function
poly_new:
.LFB5279:
	.cfi_startproc
	pushq	%r12	#
	.cfi_def_cfa_offset 16
	.cfi_offset 12, -16
	movq	%rdi, %r12	# tmp106, .result_ptr
	pushq	%rbp	#
	.cfi_def_cfa_offset 24
	.cfi_offset 6, -24
	movswq	%si, %rbp	# size,
	pushq	%rbx	#
	.cfi_def_cfa_offset 32
	.cfi_offset 3, -32
# polynomials.c:7: 	size_t size_allocated	= ALLOCATE_ALIGNMENT(size);
	movq	%rbp, %rbx	# size, iftmp.0_8
	testb	$16, %sil	#, size
	je	.L3	#,
# polynomials.c:7: 	size_t size_allocated	= ALLOCATE_ALIGNMENT(size);
	movswl	%si, %ebx	# size, size
	addl	$16, %ebx	#, tmp96
	movslq	%ebx, %rbx	# tmp96, tmp97
	andq	$-17, %rbx	#, iftmp.0_8
.L3:
# polynomials.c:8: 	poly.memory	= aligned_alloc(MEMORY_ALIGNMENT, size_allocated);
	movq	%rbx, %rsi	# iftmp.0_8,
	movl	$16, %edi	#,
	call	aligned_alloc@PLT	#
# polynomials.c:9: 	memset(poly.memory, 0, size_allocated);
	movq	%rbx, %rdx	# iftmp.0_8,
	xorl	%esi, %esi	#
	movq	%rax, %rdi	# tmp98,
	call	memset@PLT	#
# polynomials.c:12: 	return (poly);
	movq	%rbp, %xmm0	# size, tmp103
	movq	%rbx, %xmm1	# iftmp.0_8, iftmp.0_8
# polynomials.c:13: }
	popq	%rbx	#
	.cfi_def_cfa_offset 24
# polynomials.c:12: 	return (poly);
	punpcklqdq	%xmm1, %xmm0	# iftmp.0_8, tmp103
	movq	%rax, (%r12)	# tmp98, MEM[(struct poly *)&<retval>]
# polynomials.c:13: }
	popq	%rbp	#
	.cfi_def_cfa_offset 16
	movq	%r12, %rax	# .result_ptr,
# polynomials.c:12: 	return (poly);
	movups	%xmm0, 8(%r12)	# tmp103, MEM[(struct poly *)&<retval> + 8B]
# polynomials.c:13: }
	popq	%r12	#
	.cfi_def_cfa_offset 8
	ret	
	.cfi_endproc
.LFE5279:
	.size	poly_new, .-poly_new
	.p2align 4
	.globl	poly_multiply
	.type	poly_multiply, @function
poly_multiply:
.LFB5280:
	.cfi_startproc
	pushq	%r15	#
	.cfi_def_cfa_offset 16
	.cfi_offset 15, -16
	pushq	%r14	#
	.cfi_def_cfa_offset 24
	.cfi_offset 14, -24
	pushq	%r13	#
	.cfi_def_cfa_offset 32
	.cfi_offset 13, -32
	pushq	%r12	#
	.cfi_def_cfa_offset 40
	.cfi_offset 12, -40
	pushq	%rbp	#
	.cfi_def_cfa_offset 48
	.cfi_offset 6, -48
	pushq	%rbx	#
	.cfi_def_cfa_offset 56
	.cfi_offset 3, -56
	subq	$24, %rsp	#,
	.cfi_def_cfa_offset 80
# polynomials.c:20: 	for (size_t i = 0; i < poly_a.size; i++)
	movq	112(%rsp), %r15	# MEM[(struct poly *)&poly_a + 8B], _35
	movq	136(%rsp), %rbp	# MEM[(struct poly *)&poly_b + 8B], poly_b$size
# polynomials.c:20: 	for (size_t i = 0; i < poly_a.size; i++)
	testq	%r15, %r15	# _35
	je	.L6	#,
	testq	%rbp, %rbp	# ivtmp.37
	je	.L6	#,
	negq	%r15	# _35
	movq	%rdi, %r13	# tmp112, result
# polynomials.c:22: 		for (size_t j = 0; j < poly_b.size; j++)
	xorl	%r14d, %r14d	# ivtmp.36
	movq	%r15, 8(%rsp)	# _35, %sfp
	.p2align 4,,10
	.p2align 3
.L9:
	movq	104(%rsp), %rbx	# poly_a.memory, _48
	movq	128(%rsp), %r12	# poly_b.memory, tmp111
	movq	%r14, %r15	# ivtmp.36, ivtmp.31
	negq	%r15	# ivtmp.31
	subq	%r14, %rbx	# ivtmp.36, _48
	addq	%r14, %r12	# ivtmp.36, tmp111
	.p2align 4,,10
	.p2align 3
.L10:
# polynomials.c:24: 			FF_ADDITION_INPLACE(result->memory[i + j], 
	subq	$8, %rsp	#,
	.cfi_def_cfa_offset 88
	movzbl	(%r12,%r15), %esi	# MEM[base: _34, index: ivtmp.31_43, offset: 0B], MEM[base: _34, index: ivtmp.31_43, offset: 0B]
	movzbl	(%rbx), %edi	# *_48, *_48
	xorl	%eax, %eax	#
	pushq	104(%rsp)	#
	.cfi_def_cfa_offset 96
	pushq	104(%rsp)	#
	.cfi_def_cfa_offset 104
	pushq	104(%rsp)	# table
	.cfi_def_cfa_offset 112
	call	ff_multiply_lut@PLT	#
# polynomials.c:22: 		for (size_t j = 0; j < poly_b.size; j++)
	addq	$32, %rsp	#,
	.cfi_def_cfa_offset 80
# polynomials.c:24: 			FF_ADDITION_INPLACE(result->memory[i + j], 
	movl	%eax, %r8d	#, tmp113
	movq	0(%r13), %rax	# result_29(D)->memory, _11
	addq	%r15, %rax	# ivtmp.31, _11
	addq	$1, %r15	#, ivtmp.31
	xorb	%r8b, (%rax)	# tmp113, *_11
# polynomials.c:22: 		for (size_t j = 0; j < poly_b.size; j++)
	cmpq	%r15, %rbp	# ivtmp.31, ivtmp.37
	jne	.L10	#,
	subq	$1, %r14	#, ivtmp.36
	addq	$1, %rbp	#, ivtmp.37
# polynomials.c:20: 	for (size_t i = 0; i < poly_a.size; i++)
	cmpq	8(%rsp), %r14	# %sfp, ivtmp.36
	jne	.L9	#,
.L6:
# polynomials.c:28: }
	addq	$24, %rsp	#,
	.cfi_def_cfa_offset 56
	popq	%rbx	#
	.cfi_def_cfa_offset 48
	popq	%rbp	#
	.cfi_def_cfa_offset 40
	popq	%r12	#
	.cfi_def_cfa_offset 32
	popq	%r13	#
	.cfi_def_cfa_offset 24
	popq	%r14	#
	.cfi_def_cfa_offset 16
	popq	%r15	#
	.cfi_def_cfa_offset 8
	ret	
	.cfi_endproc
.LFE5280:
	.size	poly_multiply, .-poly_multiply
	.p2align 4
	.globl	poly_evaluate
	.type	poly_evaluate, @function
poly_evaluate:
.LFB5281:
	.cfi_startproc
	pushq	%r12	#
	.cfi_def_cfa_offset 16
	.cfi_offset 12, -16
	pushq	%rbp	#
	.cfi_def_cfa_offset 24
	.cfi_offset 6, -24
	pushq	%rbx	#
	.cfi_def_cfa_offset 32
	.cfi_offset 3, -32
	movq	56(%rsp), %r12	# MEM[(struct poly *)&poly], poly$memory
# polynomials.c:36: 	for (size_t i = 1; i < poly.size; i++)
	movq	64(%rsp), %rdx	# MEM[(struct poly *)&poly + 8B], _23
# polynomials.c:34: 	ff_t eval	= poly.memory[0];
	movzbl	(%r12), %eax	# *poly$memory_14, <retval>
# polynomials.c:36: 	for (size_t i = 1; i < poly.size; i++)
	cmpq	$1, %rdx	#, _23
	jbe	.L20	#,
	leaq	1(%r12), %rbx	#, ivtmp.46
	movswl	%di, %ebp	# evaluate_at, _22
	addq	%rdx, %r12	# _23, _29
	.p2align 4,,10
	.p2align 3
.L22:
# polynomials.c:38: 		ff_t lookup	= ff_multiply_lut(table, eval, evaluate_at);
	subq	$8, %rsp	#,
	.cfi_def_cfa_offset 40
	movzbl	%al, %edi	# <retval>, <retval>
	movl	%ebp, %esi	# _22,
	xorl	%eax, %eax	#
	pushq	56(%rsp)	#
	.cfi_def_cfa_offset 48
	addq	$1, %rbx	#, ivtmp.46
	pushq	56(%rsp)	#
	.cfi_def_cfa_offset 56
	pushq	56(%rsp)	# table
	.cfi_def_cfa_offset 64
	call	ff_multiply_lut@PLT	#
# polynomials.c:36: 	for (size_t i = 1; i < poly.size; i++)
	addq	$32, %rsp	#,
	.cfi_def_cfa_offset 32
# polynomials.c:39: 		eval	= FF_ADDITION(lookup, *(poly.memory + i));
	xorb	-1(%rbx), %al	# MEM[base: _21, offset: 0B], <retval>
# polynomials.c:36: 	for (size_t i = 1; i < poly.size; i++)
	cmpq	%r12, %rbx	# _29, ivtmp.46
	jne	.L22	#,
.L20:
# polynomials.c:42: }
	popq	%rbx	#
	.cfi_def_cfa_offset 24
	popq	%rbp	#
	.cfi_def_cfa_offset 16
	popq	%r12	#
	.cfi_def_cfa_offset 8
	ret	
	.cfi_endproc
.LFE5281:
	.size	poly_evaluate, .-poly_evaluate
	.p2align 4
	.globl	poly_add
	.type	poly_add, @function
poly_add:
.LFB5282:
	.cfi_startproc
	pushq	%r12	#
	.cfi_def_cfa_offset 16
	.cfi_offset 12, -16
	pushq	%rbp	#
	.cfi_def_cfa_offset 24
	.cfi_offset 6, -24
	movq	%rdi, %rbp	# tmp116, result
	pushq	%rbx	#
	.cfi_def_cfa_offset 32
	.cfi_offset 3, -32
	movq	88(%rsp), %r12	# MEM[(struct poly *)&poly_b + 8B], poly_b$size
	movq	64(%rsp), %rdx	# MEM[(struct poly *)&poly_a + 8B], poly_a$size
# polynomials.c:52: 	memcpy(result->memory + (result_length - poly_a.size), poly_a.memory, poly_a.size);
	movq	56(%rsp), %rsi	# poly_a.memory,
# polynomials.c:48: 	size_t result_length =  MAX(poly_a.size, poly_b.size);
	movq	%r12, %rbx	# poly_b$size, result_length
	cmpq	%r12, %rdx	# poly_b$size, poly_a$size
	cmovge	%rdx, %rbx	# poly_a$size,, result_length
# polynomials.c:52: 	memcpy(result->memory + (result_length - poly_a.size), poly_a.memory, poly_a.size);
	movq	%rbx, %rdi	# result_length, tmp103
	subq	%rdx, %rdi	# poly_a$size, tmp103
# polynomials.c:52: 	memcpy(result->memory + (result_length - poly_a.size), poly_a.memory, poly_a.size);
	addq	0(%rbp), %rdi	# result_21(D)->memory, tmp104
# polynomials.c:52: 	memcpy(result->memory + (result_length - poly_a.size), poly_a.memory, poly_a.size);
	call	memcpy@PLT	#
	movq	%rbx, %rax	# result_length, ivtmp.54
# polynomials.c:56: 		result->memory[i + result_length - poly_b.size]	^= poly_b.memory[i];
	movq	%r12, %rsi	# poly_b$size, tmp113
	subq	%rbx, %rsi	# result_length, tmp113
	subq	%r12, %rax	# poly_b$size, ivtmp.54
	addq	80(%rsp), %rsi	# poly_b.memory, tmp114
# polynomials.c:54: 	for (size_t i = 0; i < poly_b.size; i++)
	testq	%r12, %r12	# poly_b$size
	je	.L28	#,
	.p2align 4,,10
	.p2align 3
.L27:
# polynomials.c:56: 		result->memory[i + result_length - poly_b.size]	^= poly_b.memory[i];
	movq	0(%rbp), %rdx	# result_21(D)->memory, _10
	movzbl	(%rsi,%rax), %ecx	# MEM[base: _36, index: ivtmp.54_33, offset: 0B], MEM[base: _36, index: ivtmp.54_33, offset: 0B]
	addq	%rax, %rdx	# ivtmp.54, _10
	addq	$1, %rax	#, ivtmp.54
	xorb	%cl, (%rdx)	# MEM[base: _36, index: ivtmp.54_33, offset: 0B], *_10
# polynomials.c:54: 	for (size_t i = 0; i < poly_b.size; i++)
	cmpq	%rax, %rbx	# ivtmp.54, result_length
	jne	.L27	#,
.L28:
# polynomials.c:58: }
	popq	%rbx	#
	.cfi_def_cfa_offset 24
	xorl	%eax, %eax	#
	popq	%rbp	#
	.cfi_def_cfa_offset 16
	popq	%r12	#
	.cfi_def_cfa_offset 8
	ret	
	.cfi_endproc
.LFE5282:
	.size	poly_add, .-poly_add
	.p2align 4
	.globl	poly_add_sse
	.type	poly_add_sse, @function
poly_add_sse:
.LFB5283:
	.cfi_startproc
	pushq	%r12	#
	.cfi_def_cfa_offset 16
	.cfi_offset 12, -16
	movq	%rdi, %r12	# tmp119, result
	pushq	%rbp	#
	.cfi_def_cfa_offset 24
	.cfi_offset 6, -24
	pushq	%rbx	#
	.cfi_def_cfa_offset 32
	.cfi_offset 3, -32
	movq	64(%rsp), %rdx	# MEM[(struct poly *)&poly_a + 8B], poly_a$size
	movq	88(%rsp), %rbx	# MEM[(struct poly *)&poly_b + 8B], poly_b$size
# polynomials.c:65: 	memcpy(result->memory + (result_length - poly_a.size), poly_a.memory, poly_a.size);
	movq	56(%rsp), %rsi	# poly_a.memory,
# polynomials.c:64: 	size_t result_length =  MAX(poly_a.size, poly_b.size);
	cmpq	%rbx, %rdx	# poly_b$size, poly_a$size
	movq	%rbx, %rbp	# poly_b$size, result_length
	cmovge	%rdx, %rbp	# poly_a$size,, result_length
# polynomials.c:65: 	memcpy(result->memory + (result_length - poly_a.size), poly_a.memory, poly_a.size);
	movq	%rbp, %rdi	# result_length, tmp106
	subq	%rdx, %rdi	# poly_a$size, tmp106
# polynomials.c:65: 	memcpy(result->memory + (result_length - poly_a.size), poly_a.memory, poly_a.size);
	addq	(%r12), %rdi	# result_15(D)->memory, tmp107
# polynomials.c:65: 	memcpy(result->memory + (result_length - poly_a.size), poly_a.memory, poly_a.size);
	call	memcpy@PLT	#
# polynomials.c:67: 	size_t size_diff	= result_length - poly_b.size;
	movq	%rbp, %rcx	# result_length, result_length
	subq	%rbx, %rcx	# poly_b$size, result_length
# polynomials.c:68: 	for (size_t i = 0; i < poly_b.size; i+=MEMORY_ALIGNMENT)
	testq	%rbx, %rbx	# poly_b$size
	je	.L34	#,
# polynomials.c:71: 		__m128i v1	= _mm_load_si128((__m128i*)(poly_b.memory));
	movq	80(%rsp), %rsi	# poly_b.memory, _10
# polynomials.c:68: 	for (size_t i = 0; i < poly_b.size; i+=MEMORY_ALIGNMENT)
	xorl	%edx, %edx	# i
	.p2align 4,,10
	.p2align 3
.L35:
# polynomials.c:70: 		__m128i *mem_ptr	= result->memory + i + size_diff;
	leaq	(%rcx,%rdx), %rax	#, tmp115
# polynomials.c:70: 		__m128i *mem_ptr	= result->memory + i + size_diff;
	addq	(%r12), %rax	# result_15(D)->memory, mem_ptr
# polynomials.c:68: 	for (size_t i = 0; i < poly_b.size; i+=MEMORY_ALIGNMENT)
	addq	$16, %rdx	#, i
# /usr/lib/gcc/x86_64-pc-linux-gnu/9.3.0/include/emmintrin.h:1305:   return (__m128i) ((__v2du)__A ^ (__v2du)__B);
	movdqa	(%rax), %xmm0	# MEM[(const __m128i * {ref-all})mem_ptr_19], tmp117
	pxor	(%rsi), %xmm0	# MEM[(const __m128i * {ref-all})_10], tmp117
# polynomials.c:73: 		*mem_ptr	= _mm_xor_si128(v1, v2);
	movaps	%xmm0, (%rax)	# tmp117, *mem_ptr_19
# polynomials.c:68: 	for (size_t i = 0; i < poly_b.size; i+=MEMORY_ALIGNMENT)
	cmpq	%rdx, %rbx	# i, poly_b$size
	ja	.L35	#,
.L34:
# polynomials.c:75: }
	popq	%rbx	#
	.cfi_def_cfa_offset 24
	popq	%rbp	#
	.cfi_def_cfa_offset 16
	popq	%r12	#
	.cfi_def_cfa_offset 8
	ret	
	.cfi_endproc
.LFE5283:
	.size	poly_add_sse, .-poly_add_sse
	.ident	"GCC: (Arch Linux 9.3.0-1) 9.3.0"
	.section	.note.GNU-stack,"",@progbits
