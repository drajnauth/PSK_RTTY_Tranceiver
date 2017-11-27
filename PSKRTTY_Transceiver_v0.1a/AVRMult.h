#ifndef _AVRMult_H_
#define _AVRMult_H_

// **** A P P L I C A T I O N   N O T E   A V R 2 0 1 ***************************
// *
// * Title		: 16bit multiply routines using hardware multiplier
// * Version		: V2.0
// * Last updated		: 10 Jun, 2002
// * Target		: Any AVR with HW multiplier
// *
// * Support email	: avr@atmel.com
// *
// * DESCRIPTION
// * 	This application note shows a number of examples of how to implement
// *	16bit multiplication using hardware multiplier. Refer to each of the
// *	funtions headers for details. The functions included in this file
// *	are :
// *
// *	mul16x16_16	- Multiply of two 16bits numbers with 16bits result.
// *	mul16x16_32	- Unsigned multiply of two 16bits numbers with 32bits
// *			  result.
// *	mul16x16_24	- Unsigned multiply of two 16bits numbers with 24bits
// *			  result.
// *	muls16x16_32	- Signed multiply of two 16bits numbers with 32bits
// *			  result.
// *	muls16x16_24	- Signed multiply of two 16bits numbers with 24bits
// *			  result.
// *	mac16x16_24	- Signed multiply accumulate of two 16bits numbers
// *			  with a 24bits result.
// *	mac16x16_32	- Signed multiply accumulate of two 16bits numbers
// *			  with a 32bits result.
// *	fmuls16x16_32	- Signed fractional multiply of two 16bits numbers
// *			  with 32bits result.
// *	fmac16x16_32	- Signed fractional multiply accumulate of two 16bits
// *			  numbers with a 32bits result.
// *
// ******************************************************************************

// modified as inline assembly in a C header file for the Arduino by Jose Gama, May 2015

// ******************************************************************************
// *
// * FUNCTION
// *	mul16x16_16
// * DECRIPTION
// *	Multiply of two 16bits numbers with 16bits result.
// * USAGE
// *	r17:r16 = r23:r22 * r21:r20
// * STATISTICS
// *	Cycles :	9 + ret
// *	Words :		6 + ret
// *	Register usage: r0, r1 and r16 to r23 (8 registers)
// * NOTE
// *	Full orthogonality i.e. any register pair can be used as long as
// *	the result and the two operands does not share register pairs.
// *	The routine is non-destructive to the operands.
// *
// ******************************************************************************
inline uint16_t mul16x16_16(uint16_t multiplicand, uint16_t multiplier)
{
uint16_t result;
__asm__ __volatile__ (
"	mul	%A2, %A1 \n\t" /* al * bl*/ \
"	movw	%A0, r0 \n\t" \
"	mul	%B2, %A1 \n\t" /* ah * bl*/ \
"	add	%B0, r0 \n\t" \
"	mul	%B1, %A2 \n\t" /* bh * al*/ \
"	add	%B0, r0 \n\t" \
"	clr r1 \n\t" \
: "=&a" (result) \
: "a" (multiplicand),  "a" (multiplier)\
);
return result;
}
// ******************************************************************************
// *
// * FUNCTION
// *	mul16x16_32
// * DECRIPTION
// *	Unsigned multiply of two 16bits numbers with 32bits result.
// * USAGE
// *	r19:r18:r17:r16 = r23:r22 * r21:r20
// * STATISTICS
// *	Cycles :	17 + ret
// *	Words :		13 + ret
// *	Register usage: r0 to r2 and r16 to r23 (11 registers)
// * NOTE
// *	Full orthogonality i.e. any register pair can be used as long as
// *	the 32bit result and the two operands does not share register pairs.
// *	The routine is non-destructive to the operands.
// *
// ******************************************************************************
inline uint32_t mul16x16_32(uint16_t multiplicand, uint16_t multiplier)
{
uint32_t result;
__asm__ __volatile__ ( \
"	clr	r2 \n\t" \
"	mul	%B2, %B1 \n\t" /* ah * bh*/ \
"	movw	%C0, r0 \n\t" \
"	mul	%A2, %A1 \n\t" /* al * bl*/ \
"	movw	%A0, r0 \n\t" \
"	mul	%B2, %A1 \n\t" /* ah * bl*/ \
"	add	%B0, r0 \n\t" \
"	adc	%C0, r1 \n\t" \
"	adc	%D0, r2 \n\t" \
"	mul	%B1, %A2 \n\t" /* bh * al*/ \
"	add	%B0, r0 \n\t" \
"	adc	%C0, r1 \n\t" \
"	adc	%D0, r2 \n\t" \
"       clr r1 \n\t" \
: "=&a" (result) \
: "a" (multiplicand),  "a" (multiplier) \
);
return result;
}
// ******************************************************************************
// *
// * FUNCTION
// *	mul16x16_24
// * DECRIPTION
// *	Unsigned multiply of two 16bits numbers with 24bits result.
// * USAGE
// *	r18:r17:r16 = r23:r22 * r21:r20
// * STATISTICS
// *	Cycles :	14 + ret
// *	Words :		10 + ret
// *	Register usage: r0 to r1, r16 to r18 and r20 to r23 (9 registers)
// * NOTE
// *	Full orthogonality i.e. any register pair can be used as long as
// *	the 24bit result and the two operands does not share register pairs.
// *	The routine is non-destructive to the operands.
// *
// ******************************************************************************
inline uint32_t mul16x16_24(uint16_t multiplicand, uint16_t multiplier)
{
uint32_t result;
__asm__ __volatile__ ( \
"	mul		%B2, %B1 \n\t" /* ah * bh*/ \
"	mov		%C0, r0 \n\t" \
"	mul		%A2, %A1 \n\t" /* al * bl*/ \
"	movw		%A0, r0 \n\t" \
"	mul		%B2, %A1 \n\t" /* ah * bl*/ \
"	add		%B0, r0 \n\t" \
"	adc		%C0, r1 \n\t" \
"	mul		%B1, %A2 \n\t" /* bh * al*/ \
"	add		%B0, r0 \n\t" \
"	adc		%C0, r1 \n\t" \
"       clr r1 \n\t" \
: "=&a" (result) \
: "a" (multiplicand),  "a" (multiplier) \
);
return result;
}
// ******************************************************************************
// *
// * FUNCTION
// *	muls16x16_32
// * DECRIPTION
// *	Signed multiply of two 16bits numbers with 32bits result.
// * USAGE
// *	r19:r18:r17:r16 = r23:r22 * r21:r20
// * STATISTICS
// *	Cycles :	19 + ret
// *	Words :		15 + ret
// *	Register usage: r0 to r2 and r16 to r23 (11 registers)
// * NOTE
// *	The routine is non-destructive to the operands.
// *
// ******************************************************************************
inline int32_t muls16x16_32(int16_t multiplicand, int16_t multiplier)
{
int32_t result;
__asm__ __volatile__ ( \
"	clr	r2 \n\t" \
"	muls	%B2, %B1 \n\t" /* (signed)ah * (signed)bh*/ \
"	movw	%C0, r0 \n\t" \
"	mul	%A2, %A1 \n\t" /* al * bl*/ \
"	movw	%A0, r0 \n\t" \
"	mulsu	%B2, %A1 \n\t" /* (signed)ah * bl*/ \
"	sbc	%D0, r2 \n\t" \
"	add	%B0, r0 \n\t" \
"	adc	%C0, r1 \n\t" \
"	adc	%D0, r2 \n\t" \
"	mulsu	%B1, %A2 \n\t" /* (signed)bh * al*/ \
"	sbc	%D0, r2 \n\t" \
"	add	%B0, r0 \n\t" \
"	adc	%C0, r1 \n\t" \
"	adc	%D0, r2 \n\t" \
"	clr r1 \n\t" \
: "=&a" (result) \
: "a" (multiplicand),  "a" (multiplier) \
);
return result;
}
// ******************************************************************************
// *
// * FUNCTION
// *	muls16x16_24
// * DECRIPTION
// *	Signed multiply of two 16bits numbers with 24bits result.
// * USAGE
// *	r18:r17:r16 = r23:r22 * r21:r20
// * STATISTICS
// *	Cycles :	14 + ret
// *	Words :		10 + ret
// *	Register usage: r0 to r1, r16 to r18 and r20 to r23 (9 registers)
// * NOTE
// *	The routine is non-destructive to the operands.
// *
// ******************************************************************************
inline int32_t muls16x16_24(int16_t multiplicand, int16_t multiplier)
{
int32_t result;
__asm__ __volatile__ ( \
"	muls	%B2, %B1 \n\t" /* (signed)ah * (signed)bh*/ \
"	mov		%C0, r0 \n\t" \
"	mul		%A2, %A1 \n\t" /* al * bl*/ \
"	movw	%A0, r0 \n\t" \
"	mulsu	%B2, %A1 \n\t" /* (signed)ah * bl*/ \
"	add		%B0, r0 \n\t" \
"	adc		%C0, r1 \n\t" \
"	mulsu	%B1, %A2 \n\t" /* (signed)bh * al*/ \
"	add		%B0, r0 \n\t" \
"	adc		%C0, r1 \n\t" \
"	clr r1 \n\t" \
: "=&a" (result) \
: "a" (multiplicand),  "a" (multiplier) \
);
return result;
}
// ******************************************************************************
// *
// * FUNCTION
// *	mac16x16_24
// * DECRIPTION
// *	Signed multiply accumulate of two 16bits numbers with
// *	a 24bits result.
// * USAGE
// *	r18:r17:r16 += r23:r22 * r21:r20
// * STATISTICS
// *	Cycles :	16 + ret
// *	Words :		12 + ret
// *	Register usage: r0 to r1, r16 to r18 and r20 to r23 (9 registers)
// *
// ******************************************************************************
inline void mac16x16_24(int32_t *result, int16_t multiplicand, int16_t multiplier)
{
__asm__ __volatile__ ( \
"	muls	%B2, %B1 \n\t" /* (signed)ah * (signed)bh*/ \
"	add	%C0, r0 \n\t" \
"	mul	%A2, %A1 \n\t" /* al * bl*/ \
"	add	%A0, r0 \n\t" \
"	adc	%B0, r1 \n\t" \
"	adc	%C0, r2 \n\t" \
"	mulsu	%B2, %A1 \n\t" /* (signed)ah * bl*/ \
"	add	%B0, r0 \n\t" \
"	adc	%C0, r1 \n\t" \
"	mulsu	%B1, %A2 \n\t" /* (signed)bh * al*/ \
"	add	%B0, r0 \n\t" \
"	adc	%C0, r1 \n\t" \
"	clr r1 \n\t" \
: "+a" (*result) \
: "a" (multiplicand),  "a" (multiplier) \
);
}// ******************************************************************************
// *
// * FUNCTION
// *	mac16x16_32
// * DECRIPTION
// *	Signed multiply accumulate of two 16bits numbers with
// *	a 32bits result.
// * USAGE
// *	r19:r18:r17:r16 += r23:r22 * r21:r20
// * STATISTICS
// *	Cycles :	23 + ret
// *	Words :		19 + ret
// *	Register usage: r0 to r2 and r16 to r23 (11 registers)
// *
// ******************************************************************************
inline void mac16x16_32(int32_t *result, int16_t multiplicand, int16_t multiplier)
{
__asm__ __volatile__ ( \
"	clr	r2 \n\t" \
"	muls	%B2, %B1 \n\t" /* (signed)ah * (signed)bh*/ \
"	add	%C0, r0 \n\t" \
"	adc	%D0, r1 \n\t" \
"	mul	%A2, %A1 \n\t" /* al * bl*/ \
"	add	%A0, r0 \n\t" \
"	adc	%B0, r1 \n\t" \
"	adc	%C0, r2 \n\t" \
"	adc	%D0, r2 \n\t" \
"	mulsu	%B2, %A1 \n\t" /* (signed)ah * bl*/ \
"	sbc	%D0, r2 \n\t" \
"	add	%B0, r0 \n\t" \
"	adc	%C0, r1 \n\t" \
"	adc	%D0, r2 \n\t" \
"	mulsu	%B1, %A2 \n\t" /* (signed)bh * al*/ \
"	sbc	%D0, r2 \n\t" \
"	add	%B0, r0 \n\t" \
"	adc	%C0, r1 \n\t" \
"	adc	%D0, r2 \n\t" \
"	clr r1 \n\t" \
: "+a" (*result) \
: "a" (multiplicand),  "a" (multiplier) \
);
}

// mac16x16_32_method_B: uses two temporary registers, but reduces cycles/words by 1 
inline void mac16x16_32_method_B(int32_t *result, int16_t multiplicand, int16_t multiplier)
{
__asm__ __volatile__ ( \
"	clr	r2 \n\t" \
"	muls	%B2, %B1 \n\t" /* (signed)ah * (signed)bh*/ \
"	movw	r4,r0 \n\t" \
"	mul	%A2, %A1 \n\t" /* al * bl*/ \
"	add	%A0, r0 \n\t" \
"	adc	%B0, r1 \n\t" \
"	adc	%C0, r4 \n\t" \
"	adc	%D0, r5 \n\t" \
"	mulsu	%B2, %A1 \n\t" /* (signed)ah * bl*/ \
"	sbc	%D0, r2 \n\t" \
"	add	%B0, r0 \n\t" \
"	adc	%C0, r1 \n\t" \
"	adc	%D0, r2 \n\t" \
"	mulsu	%B1, %A2 \n\t" /* (signed)bh * al*/ \
"	sbc	%D0, r2 \n\t" \
"	add	%B0, r0 \n\t" \
"	adc	%C0, r1 \n\t" \
"	adc	%D0, r2 \n\t" \
"	clr r1 \n\t" \
: "+a" (*result) \
: "a" (multiplicand),  "a" (multiplier) \
);
}

// ******************************************************************************
// *
// * FUNCTION
// *	fmuls16x16_32
// * DECRIPTION
// *	Signed fractional multiply of two 16bits numbers with 32bits result.
// * USAGE
// *	r19:r18:r17:r16 = ( r23:r22 * r21:r20 ) << 1
// * STATISTICS
// *	Cycles :	20 + ret
// *	Words :		16 + ret
// *	Register usage: r0 to r2 and r16 to r23 (11 registers)
// * NOTE
// *	The routine is non-destructive to the operands.
// *
// ******************************************************************************
inline int32_t fmuls16x16_32(int16_t multiplicand, int16_t multiplier)
{
int32_t result;
__asm__ __volatile__ ( \
"	clr	r2 \n\t" \
"	fmuls	%B2, %B1 \n\t" /* ( (signed)ah * (signed)bh ) << 1*/ \
"	movw	%C0, r0 \n\t" \
"	fmul	%A2, %A1 \n\t" /* ( al * bl ) << 1*/ \
"	adc	%C0, r2 \n\t" \
"	movw	%A0, r0 \n\t" \
"	fmulsu	%B2, %A1 \n\t" /* ( (signed)ah * bl ) << 1*/ \
"	sbc	%D0, r2 \n\t" \
"	add	%B0, r0 \n\t" \
"	adc	%C0, r1 \n\t" \
"	adc	%D0, r2 \n\t" \
"	fmulsu	%B1, %A2 \n\t" /* ( (signed)bh * al ) << 1*/ \
"	sbc	%D0, r2 \n\t" \
"	add	%B0, r0 \n\t" \
"	adc	%C0, r1 \n\t" \
"	adc	%D0, r2 \n\t" \
"	clr r1 \n\t" \
: "=&a" (result) \
: "a" (multiplicand),  "a" (multiplier) \
);
return result;
}// ******************************************************************************
// *
// * FUNCTION
// *	fmac16x16_32
// * DECRIPTION
// *	Signed fractional multiply accumulate of two 16bits numbers with
// *	a 32bits result.
// * USAGE
// *	r19:r18:r17:r16 += (r23:r22 * r21:r20) << 1
// * STATISTICS
// *	Cycles :	25 + ret
// *	Words :		21 + ret
// *	Register usage: r0 to r2 and r16 to r23 (11 registers)
// *
// ******************************************************************************
inline void fmac16x16_32(int32_t *result, int16_t multiplicand, int16_t multiplier)
{
__asm__ __volatile__ ( \
"	clr	r2 \n\t" \
"	fmuls	%B2, %B1 \n\t" /* ( (signed)ah * (signed)bh ) << 1*/ \
"	add	%C0, r0 \n\t" \
"	adc	%D0, r1 \n\t" \
"	fmul	%A2, %A1 \n\t" /* ( al * bl ) << 1*/ \
"	adc	%C0, r2 \n\t" \
"	adc	%D0, r2 \n\t" \
"	add	%A0, r0 \n\t" \
"	adc	%B0, r1 \n\t" \
"	adc	%C0, r2 \n\t" \
"	adc	%D0, r2 \n\t" \
"	fmulsu	%B2, %A1 \n\t" /* ( (signed)ah * bl ) << 1*/ \
"	sbc	%D0, r2 \n\t" \
"	add	%B0, r0 \n\t" \
"	adc	%C0, r1 \n\t" \
"	adc	%D0, r2 \n\t" \
"	fmulsu	%B1, %A2 \n\t" /* ( (signed)bh * al ) << 1*/ \
"	sbc	%D0, r2 \n\t" \
"	add	%B0, r0 \n\t" \
"	adc	%C0, r1 \n\t" \
"	adc	%D0, r2 \n\t" \
"	clr r1 \n\t" \
: "+a" (*result) \
: "a" (multiplicand),  "a" (multiplier) \
);
}

// fmac16x16_32_method_B: uses two temporary registers (r4,r5), but reduces cycles/words by 2
inline void fmac16x16_32_method_B(int32_t *result, int16_t multiplicand, int16_t multiplier)
{
__asm__ __volatile__ ( \
"	clr	r2 \n\t" \
"	clr	r2 \n\t" \
"	fmuls	%B2, %B1 \n\t" /* ( (signed)ah * (signed)bh ) << 1*/ \
"	movw	r4,r0 \n\t" \
"	fmul	%A2, %A1 \n\t" /* ( al * bl ) << 1*/ \
"	adc	r4, r2 \n\t" \
"	add	%A0, r0 \n\t" \
"	adc	%B0, r1 \n\t" \
"	adc	%C0, r4 \n\t" \
"	adc	%D0, r5 \n\t" \
"	fmulsu	%B2, %A1 \n\t" /* ( (signed)ah * bl ) << 1*/ \
"	sbc	%D0, r2 \n\t" \
"	add	%B0, r0 \n\t" \
"	adc	%C0, r1 \n\t" \
"	adc	%D0, r2 \n\t" \
"	fmulsu	%B1, %A2 \n\t" /* ( (signed)bh * al ) << 1*/ \
"	sbc	%D0, r2 \n\t" \
"	add	%B0, r0 \n\t" \
"	adc	%C0, r1 \n\t" \
"	adc	%D0, r2 \n\t" \
"	clr r1 \n\t" \
: "+a" (*result) \
: "a" (multiplicand),  "a" (multiplier) \
);
}


#endif // _AVRMult_
