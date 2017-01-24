#ifndef _AVRDiv_H_
#define _AVRDiv_H_


//**** A P P L I C A T I O N   N O T E   A V R 2 0 0 ************************
//*
//* Title:		Multiply and Divide Routines
//* Version:		1.1
//* Last updated:	97.07.04
//* Target:		AT90Sxxxx (All AVR Devices)
//*
//* Support E-mail:	avr@atmel.com
//* 

//* DESCRIPTION
//* This Application Note lists subroutines for the following
//* Muliply/Divide applications. Routines are straight-line implementations
//* optimized for speed:
//*
//*  8 x  8 = 16 bit unsigned
//* 16 x 16 = 32 bit unsigned
//*  8 /  8 =  8 +  8 bit unsigned
//* 16 / 16 = 16 + 16 bit unsigned
//*

// Modified by Jose Gama as inline assembly in a header file for the Arduino, May 2015

//***************************************************************************
//*
//* "mpy8u" - 8x8 Bit Unsigned Multiplication
//*
//* This subroutine multiplies the two register variables r17 and r16.
//* The result is placed in registers r18, r17
//*  
//* Number of words	:34 + return
//* Number of cycles	:34 + return
//* Low registers used	:None
//* High registers used  :3 (r16,r17/r17,r18)	
//*
//* Note: Result Low byte and the multiplier share the same register.
//* This causes the multiplier to be overwritten by the result.
//*
//***************************************************************************

//***** Subroutine Register Variables

// r16 multiplicand
// r17 multiplier
// r17 result Low byte
// r18 result High byte

//***** Code
inline uint16_t mpy8u(uint8_t multiplicand, uint8_t multiplier)
{
uint16_t result;
asm volatile ( \
"mpy8u:	clr	%B0 \n\t" /*clear result High byte*/ \
"	lsr	%A2 \n\t" /*shift multiplier*/ \
"	brcc	noad80 \n\t" /*if carry set*/ \
"	add	%B0,%A1 \n\t" /*    add multiplicand to result High byte*/ \
"noad80:	ror	%B0 \n\t" /*shift right result High byte */ \
"	ror	%A2 \n\t" /*rotate right result L byte and multiplier*/ \
"	brcc	noad81 \n\t" /*if carry set*/ \
"	add	%B0,%A1 \n\t" /*    add multiplicand to result High byte*/ \
"noad81:	ror	%B0 \n\t" /*shift right result High byte */ \
"	ror	%A2 \n\t" /*rotate right result L byte and multiplier*/ \
"	brcc	noad82 \n\t" /*if carry set*/ \
"	add	%B0,%A1 \n\t" /*    add multiplicand to result High byte*/ \
"noad82:	ror	%B0 \n\t" /*shift right result High byte */ \
"	ror	%A2 \n\t" /*rotate right result L byte and multiplier*/ \
"	brcc	noad83 \n\t" /*if carry set*/ \
"	add	%B0,%A1 \n\t" /*    add multiplicand to result High byte*/ \
"noad83:	ror	%B0 \n\t" /*shift right result High byte */ \
"	ror	%A2 \n\t" /*rotate right result L byte and multiplier*/ \
"	brcc	noad84 \n\t" /*if carry set*/ \
"	add	%B0,%A1 \n\t" /*    add multiplicand to result High byte*/ \
"noad84:	ror	%B0 \n\t" /*shift right result High byte */ \
"	ror	%A2 \n\t" /*rotate right result L byte and multiplier*/ \
"	brcc	noad85 \n\t" /*if carry set*/ \
"	add	%B0,%A1 \n\t" /*    add multiplicand to result High byte*/ \
"noad85:	ror	%B0 \n\t" /*shift right result High byte */ \
"	ror	%A2 \n\t" /*rotate right result L byte and multiplier*/ \
"	brcc	noad86 \n\t" /*if carry set*/ \
"	add	%B0,%A1 \n\t" /*    add multiplicand to result High byte*/ \
"noad86:	ror	%B0 \n\t" /*shift right result High byte */ \
"	ror	%A2 \n\t" /*rotate right result L byte and multiplier*/ \
"	brcc	noad87 \n\t" /*if carry set*/ \
"	add	%B0,%A1 \n\t" /*    add multiplicand to result High byte*/ \
"noad87:	ror	%B0 \n\t" /*shift right result High byte */ \
"	ror	%A2 \n\t" /*rotate right result L byte and multiplier*/ \
"	mov %A0,%A2 \n\t" \
: "=&a" (result) \
: "a" (multiplicand),  "a" (multiplier)\
);
return result;
}

//***************************************************************************
//*
//* "mpy16u" - 16x16 Bit Unsigned Multiplication
//*
//* This subroutine multiplies the two 16-bit register variables 
//* r19:r18 and r17:r16.
//* The result is placed in r21:r20:r19:r18.
//*  
//* Number of words	:105 + return
//* Number of cycles	:105 + return
//* Low registers used	:None
//* High registers used  :6 (r18,r19,r16/r18,r17/r19,r20,
//*			    r21)	
//*
//***************************************************************************

//***** Subroutine Register Variables

// r16 multiplicand low byte
// r17 multiplicand high byte
// r18 multiplier low byte
// r19 multiplier high byte
// r18 result byte 0 (LSB)
// r19 result byte 1
// r20 result byte 2
// r21 result byte 3 (MSB)

//***** Code
inline uint32_t mpy16u(uint16_t multiplicand, uint16_t multiplier)
{
uint32_t result;
asm volatile ( \
"mpy16u:	clr	%D0 \n\t" /*clear 2 highest bytes of result*/ \
"	clr	%C0	\n\t" \
"	lsr	%B2 \n\t" /*rotate multiplier Low*/ \
"	ror	%A2 \n\t" /*rotate multiplier High*/ \
"	brcc	noadd0 \n\t" /*if carry set*/ \
"	add	%C0,%A1 \n\t" /*    add multiplicand Low to byte 2 of res*/ \
"	adc	%D0,%B1 \n\t" /*    add multiplicand high to byte 3 of res*/ \
"noadd0:	ror	%D0 \n\t" /*shift right result byte 3*/ \
"	ror	%C0 \n\t" /*rotate right result byte 2*/ \
"	ror	%B2 \n\t" /*rotate result byte 1 and multiplier High*/ \
"	ror	%A2 \n\t" /*rotate result byte 0 and multiplier Low*/ \
"	brcc	noadd1 \n\t" /*if carry set*/ \
"	add	%C0,%A1 \n\t" /*    add multiplicand Low to byte 2 of res*/ \
"	adc	%D0,%B1 \n\t" /*    add multiplicand high to byte 3 of res*/ \
"noadd1:	ror	%D0 \n\t" /*shift right result byte 3*/ \
"	ror	%C0 \n\t" /*rotate right result byte 2*/ \
"	ror	%B2 \n\t" /*rotate result byte 1 and multiplier High*/ \
"	ror	%A2 \n\t" /*rotate result byte 0 and multiplier Low*/ \
"	brcc	noadd2 \n\t" /*if carry set*/ \
"	add	%C0,%A1 \n\t" /*    add multiplicand Low to byte 2 of res*/ \
"	adc	%D0,%B1 \n\t" /*    add multiplicand high to byte 3 of res*/ \
"noadd2:	ror	%D0 \n\t" /*shift right result byte 3*/ \
"	ror	%C0 \n\t" /*rotate right result byte 2*/ \
"	ror	%B2 \n\t" /*rotate result byte 1 and multiplier High*/ \
"	ror	%A2 \n\t" /*rotate result byte 0 and multiplier Low*/ \
"	brcc	noadd3 \n\t" /*if carry set*/ \
"	add	%C0,%A1 \n\t" /*    add multiplicand Low to byte 2 of res*/ \
"	adc	%D0,%B1 \n\t" /*    add multiplicand high to byte 3 of res*/ \
"noadd3:	ror	%D0 \n\t" /*shift right result byte 3*/ \
"	ror	%C0 \n\t" /*rotate right result byte 2*/ \
"	ror	%B2 \n\t" /*rotate result byte 1 and multiplier High*/ \
"	ror	%A2 \n\t" /*rotate result byte 0 and multiplier Low*/ \
"	brcc	noadd4 \n\t" /*if carry set*/ \
"	add	%C0,%A1 \n\t" /*    add multiplicand Low to byte 2 of res*/ \
"	adc	%D0,%B1 \n\t" /*    add multiplicand high to byte 3 of res*/ \
"noadd4:	ror	%D0 \n\t" /*shift right result byte 3*/ \
"	ror	%C0 \n\t" /*rotate right result byte 2*/ \
"	ror	%B2 \n\t" /*rotate result byte 1 and multiplier High*/ \
"	ror	%A2 \n\t" /*rotate result byte 0 and multiplier Low*/ \
"	brcc	noadd5 \n\t" /*if carry set*/ \
"	add	%C0,%A1 \n\t" /*    add multiplicand Low to byte 2 of res*/ \
"	adc	%D0,%B1 \n\t" /*    add multiplicand high to byte 3 of res*/ \
"noadd5:	ror	%D0 \n\t" /*shift right result byte 3*/ \
"	ror	%C0 \n\t" /*rotate right result byte 2*/ \
"	ror	%B2 \n\t" /*rotate result byte 1 and multiplier High*/ \
"	ror	%A2 \n\t" /*rotate result byte 0 and multiplier Low*/ \
"	brcc	noadd6 \n\t" /*if carry set*/ \
"	add	%C0,%A1 \n\t" /*    add multiplicand Low to byte 2 of res*/ \
"	adc	%D0,%B1 \n\t" /*    add multiplicand high to byte 3 of res*/ \
"noadd6:	ror	%D0 \n\t" /*shift right result byte 3*/ \
"	ror	%C0 \n\t" /*rotate right result byte 2*/ \
"	ror	%B2 \n\t" /*rotate result byte 1 and multiplier High*/ \
"	ror	%A2 \n\t" /*rotate result byte 0 and multiplier Low*/ \
"	brcc	noadd7 \n\t" /*if carry sett*/ \
"	add	%C0,%A1 \n\t" /*    add multiplicand Low to byte 2 of res*/ \
"	adc	%D0,%B1 \n\t" /*    add multiplicand high to byte 3 of res*/ \
"noadd7:	ror	%D0 \n\t" /*shift right result byte 3*/ \
"	ror	%C0 \n\t" /*rotate right result byte 2*/ \
"	ror	%B2 \n\t" /*rotate result byte 1 and multiplier High*/ \
"	ror	%A2 \n\t" /*rotate result byte 0 and multiplier Low*/ \
"	brcc	noadd8 \n\t" /*if carry set*/ \
"	add	%C0,%A1 \n\t" /*    add multiplicand Low to byte 2 of res*/ \
"	adc	%D0,%B1 \n\t" /*    add multiplicand high to byte 3 of res*/ \
"noadd8:	ror	%D0 \n\t" /*shift right result byte 3*/ \
"	ror	%C0 \n\t" /*rotate right result byte 2*/ \
"	ror	%B2 \n\t" /*rotate result byte 1 and multiplier High*/ \
"	ror	%A2 \n\t" /*rotate result byte 0 and multiplier Low*/ \
"	brcc	noadd9 \n\t" /*if carry set*/ \
"	add	%C0,%A1 \n\t" /*    add multiplicand Low to byte 2 of res*/ \
"	adc	%D0,%B1 \n\t" /*    add multiplicand high to byte 3 of res*/ \
"noadd9:	ror	%D0 \n\t" /*shift right result byte 3*/ \
"	ror	%C0 \n\t" /*rotate right result byte 2*/ \
"	ror	%B2 \n\t" /*rotate result byte 1 and multiplier High*/ \
"	ror	%A2 \n\t" /*rotate result byte 0 and multiplier Low*/ \
"	brcc	noad10 \n\t" /*if carry set*/ \
"	add	%C0,%A1 \n\t" /*    add multiplicand Low to byte 2 of res*/ \
"	adc	%D0,%B1 \n\t" /*    add multiplicand high to byte 3 of res*/ \
"noad10:	ror	%D0 \n\t" /*shift right result byte 3*/ \
"	ror	%C0 \n\t" /*rotate right result byte 2*/ \
"	ror	%B2 \n\t" /*rotate result byte 1 and multiplier High*/ \
"	ror	%A2 \n\t" /*rotate result byte 0 and multiplier Low*/ \
"	brcc	noad11 \n\t" /*if carry set*/ \
"	add	%C0,%A1 \n\t" /*    add multiplicand Low to byte 2 of res*/ \
"	adc	%D0,%B1 \n\t" /*    add multiplicand high to byte 3 of res*/ \
"noad11:	ror	%D0 \n\t" /*shift right result byte 3*/ \
"	ror	%C0 \n\t" /*rotate right result byte 2*/ \
"	ror	%B2 \n\t" /*rotate result byte 1 and multiplier High*/ \
"	ror	%A2 \n\t" /*rotate result byte 0 and multiplier Low*/ \
"	brcc	noad12 \n\t" /*if carry set*/ \
"	add	%C0,%A1 \n\t" /*    add multiplicand Low to byte 2 of res*/ \
"	adc	%D0,%B1 \n\t" /*    add multiplicand high to byte 3 of res*/ \
"noad12:	ror	%D0 \n\t" /*shift right result byte 3*/ \
"	ror	%C0 \n\t" /*rotate right result byte 2*/ \
"	ror	%B2 \n\t" /*rotate result byte 1 and multiplier High*/ \
"	ror	%A2 \n\t" /*rotate result byte 0 and multiplier Low*/ \
"	brcc	noad13 \n\t" /*if carry set*/ \
"	add	%C0,%A1 \n\t" /*    add multiplicand Low to byte 2 of res*/ \
"	adc	%D0,%B1 \n\t" /*    add multiplicand high to byte 3 of res*/ \
"noad13:	ror	%D0 \n\t" /*shift right result byte 3*/ \
"	ror	%C0 \n\t" /*rotate right result byte 2*/ \
"	ror	%B2 \n\t" /*rotate result byte 1 and multiplier High*/ \
"	ror	%A2 \n\t" /*rotate result byte 0 and multiplier Low*/ \
"	brcc	noad14 \n\t" /*if carry set*/ \
"	add	%C0,%A1 \n\t" /*    add multiplicand Low to byte 2 of res*/ \
"	adc	%D0,%B1 \n\t" /*    add multiplicand high to byte 3 of res*/ \
"noad14:	ror	%D0 \n\t" /*shift right result byte 3*/ \
"	ror	%C0 \n\t" /*rotate right result byte 2*/ \
"	ror	%B2 \n\t" /*rotate result byte 1 and multiplier High*/ \
"	ror	%A2 \n\t" /*rotate result byte 0 and multiplier Low*/ \
"	brcc	noad15 \n\t" /*if carry set*/ \
"	add	%C0,%A1 \n\t" /*    add multiplicand Low to byte 2 of res*/ \
"	adc	%D0,%B1 \n\t" /*    add multiplicand high to byte 3 of res*/ \
"noad15:	ror	%D0 \n\t" /*shift right result byte 3*/ \
"	ror	%C0 \n\t" /*rotate right result byte 2*/ \
"	ror	%B2 \n\t" /*rotate result byte 1 and multiplier High*/ \
"	ror	%A2 \n\t" /*rotate result byte 0 and multiplier Low*/ \
"	mov %A0,%A2 \n\t" \
"	mov %B0,%B2 \n\t" \
: "=&a" (result) \
: "r" (multiplicand),  "r" (multiplier)\
);
return result;
}

//***************************************************************************
//*
//* "div8u" - 8/8 Bit Unsigned Division
//*
//* This subroutine divides the two register variables "r16" (dividend) and 
//* "r17" (divisor). The result is placed in "r16" and the remainder in
//* "r15".
//*  
//* Number of words	:66 + return
//* Number of cycles	:50/58/66 (Min/Avg/Max) + return
//* Low registers used	:1 (r15)
//* High registers used  :2 (r16/r16,r17)
//*
//***************************************************************************

//***** Subroutine Register Variables

// r15 remainder
// r16 result
// r16 dividend
// r17 divisor

//***** Code
inline void div8u(uint8_t *result, uint8_t *remainder, uint8_t dividend, uint8_t divisor)
{
asm volatile ( \
"div8u:	sub	%A1,%A1 \n\t" /*clear remainder and carry*/ \
"	rol	%A2 \n\t" /*shift left dividend*/ \
"	rol	%A1 \n\t" /*shift dividend into remainder*/ \
"	sub	%A1,%A3 \n\t" /*remainder = remainder - divisor*/ \
"	brcc	d8u_1 \n\t" /*if result negative*/ \
"	add	%A1,%A3 \n\t" /*    restore remainder*/ \
"	clc \n\t" /*    clear carry to be shifted into result*/ \
"	rjmp	d8u_2 \n\t" /*else*/ \
"d8u_1:	sec \n\t" /*    set carry to be shifted into result*/ \
"d8u_2:	rol	%A2 \n\t" /*shift left dividend*/ \
"	rol	%A1 \n\t" /*shift dividend into remainder*/ \
"	sub	%A1,%A3 \n\t" /*remainder = remainder - divisor*/ \
"	brcc	d8u_3 \n\t" /*if result negative*/ \
"	add	%A1,%A3 \n\t" /*    restore remainder*/ \
"	clc \n\t" /*    clear carry to be shifted into result*/ \
"	rjmp	d8u_4 \n\t" /*else*/ \
"d8u_3:	sec \n\t" /*    set carry to be shifted into result*/ \
"d8u_4:	rol	%A2 \n\t" /*shift left dividend*/ \
"	rol	%A1 \n\t" /*shift dividend into remainder*/ \
"	sub	%A1,%A3 \n\t" /*remainder = remainder - divisor*/ \
"	brcc	d8u_5 \n\t" /*if result negative*/ \
"	add	%A1,%A3 \n\t" /*    restore remainder*/ \
"	clc \n\t" /*    clear carry to be shifted into result*/ \
"	rjmp	d8u_6 \n\t" /*else*/ \
"d8u_5:	sec \n\t" /*    set carry to be shifted into result*/ \
"d8u_6:	rol	%A2 \n\t" /*shift left dividend*/ \
"	rol	%A1 \n\t" /*shift dividend into remainder*/ \
"	sub	%A1,%A3 \n\t" /*remainder = remainder - divisor*/ \
"	brcc	d8u_7 \n\t" /*if result negative*/ \
"	add	%A1,%A3 \n\t" /*    restore remainder*/ \
"	clc \n\t" /*    clear carry to be shifted into result*/ \
"	rjmp	d8u_8 \n\t" /*else*/ \
"d8u_7:	sec \n\t" /*    set carry to be shifted into result*/ \
"d8u_8:	rol	%A2 \n\t" /*shift left dividend*/ \
"	rol	%A1 \n\t" /*shift dividend into remainder*/ \
"	sub	%A1,%A3 \n\t" /*remainder = remainder - divisor*/ \
"	brcc	d8u_9 \n\t" /*if result negative*/ \
"	add	%A1,%A3 \n\t" /*    restore remainder*/ \
"	clc \n\t" /*    clear carry to be shifted into result*/ \
"	rjmp	d8u_10 \n\t" /*else*/ \
"d8u_9:	sec \n\t" /*    set carry to be shifted into result*/ \
"d8u_10:	rol	%A2 \n\t" /*shift left dividend*/ \
"	rol	%A1 \n\t" /*shift dividend into remainder*/ \
"	sub	%A1,%A3 \n\t" /*remainder = remainder - divisor*/ \
"	brcc	d8u_11 \n\t" /*if result negative*/ \
"	add	%A1,%A3 \n\t" /*    restore remainder*/ \
"	clc \n\t" /*    clear carry to be shifted into result*/ \
"	rjmp	d8u_12 \n\t" /*else*/ \
"d8u_11:	sec \n\t" /*    set carry to be shifted into result*/ \
"d8u_12:	rol	%A2 \n\t" /*shift left dividend*/ \
"	rol	%A1 \n\t" /*shift dividend into remainder*/ \
"	sub	%A1,%A3 \n\t" /*remainder = remainder - divisor*/ \
"	brcc	d8u_13 \n\t" /*if result negative*/ \
"	add	%A1,%A3 \n\t" /*    restore remainder*/ \
"	clc \n\t" /*    clear carry to be shifted into result*/ \
"	rjmp	d8u_14 \n\t" /*else*/ \
"d8u_13:	sec \n\t" /*    set carry to be shifted into result*/ \
"d8u_14:	rol	%A2 \n\t" /*shift left dividend*/ \
"	rol	%A1 \n\t" /*shift dividend into remainder*/ \
"	sub	%A1,%A3 \n\t" /*remainder = remainder - divisor*/ \
"	brcc	d8u_15 \n\t" /*if result negative*/ \
"	add	%A1,%A3 \n\t" /*    restore remainder*/ \
"	clc \n\t" /*    clear carry to be shifted into result*/ \
"	rjmp	d8u_16 \n\t" /*else*/ \
"d8u_15:	sec \n\t" /*    set carry to be shifted into result*/ \
"d8u_16:	rol	%A2 \n\t" /*shift left dividend*/ \
"	mov %A0,%A2 \n\t" \
: "+a" (*result), "+a" (*remainder) \
: "r" (dividend),  "r" (divisor)\
);
}

//***************************************************************************
//*
//* "div16u" - 16/16 Bit Unsigned Division
//*
//* This subroutine divides the two 16-bit numbers 
//* "r16H:r16L" (dividend) and "r19:r18" (divisor). 
//* The result is placed in "r17:r16" and the remainder in
//* "r15:r14".
//*  
//* Number of words	:196 + return
//* Number of cycles	:148/173/196 (Min/Avg/Max)
//* Low registers used	:2 (r14,r15)
//* High registers used  :4 (r16/r16,r17/r17,r18,r19)
//*
//***************************************************************************

//***** Code
inline void div16u(uint16_t *result, uint16_t *remainder, uint16_t dividend, uint16_t divisor)
{
asm volatile ( \
"div16u:	clr	%A1 \n\t" /*clear remainder Low byte*/ \
"	sub	%B1,%B1 \n\t" /*clear remainder High byte and carry*/ \
"	rol	%A2 \n\t" /*shift left dividend*/ \
"	rol	%B2 \n\t" \
"	rol	%A1 \n\t" /*shift dividend into remainder*/ \
"	rol	%B1 \n\t" \
"	sub	%A1,%A3 \n\t" /*remainder = remainder - divisor*/ \
"	sbc	%B1,%B3 \n\t" \
"	brcc	d16u_1 \n\t" /*if result negative*/ \
"	add	%A1,%A3 \n\t" /*    restore remainder*/ \
"	adc	%B1,%B3 \n\t" \
"	clc \n\t" /*    clear carry to be shifted into result*/ \
"	rjmp	d16u_2 \n\t" /*else*/ \
"d16u_1:	sec \n\t" /*    set carry to be shifted into result*/ \
"d16u_2:	rol	%A2 \n\t" /*shift left dividend*/ \
"	rol	%B2 \n\t" \
"	rol	%A1 \n\t" /*shift dividend into remainder*/ \
"	rol	%B1 \n\t" \
"	sub	%A1,%A3 \n\t" /*remainder = remainder - divisor*/ \
"	sbc	%B1,%B3 \n\t" \
"	brcc	d16u_3 \n\t" /*if result negative*/ \
"	add	%A1,%A3 \n\t" /*    restore remainder*/ \
"	adc	%B1,%B3 \n\t" \
"	clc \n\t" /*    clear carry to be shifted into result*/ \
"	rjmp	d16u_4 \n\t" /*else*/ \
"d16u_3:	sec \n\t" /*    set carry to be shifted into result*/ \
"d16u_4:	rol	%A2 \n\t" /*shift left dividend*/ \
"	rol	%B2 \n\t" \
"	rol	%A1 \n\t" /*shift dividend into remainder*/ \
"	rol	%B1 \n\t" \
"	sub	%A1,%A3 \n\t" /*remainder = remainder - divisor*/ \
"	sbc	%B1,%B3 \n\t" \
"	brcc	d16u_5 \n\t" /*if result negative*/ \
"	add	%A1,%A3 \n\t" /*    restore remainder*/ \
"	adc	%B1,%B3 \n\t" \
"	clc \n\t" /*    clear carry to be shifted into result*/ \
"	rjmp	d16u_6 \n\t" /*else*/ \
"d16u_5:	sec \n\t" /*    set carry to be shifted into result*/ \
"d16u_6:	rol	%A2 \n\t" /*shift left dividend*/ \
"	rol	%B2 \n\t" \
"	rol	%A1 \n\t" /*shift dividend into remainder*/ \
"	rol	%B1 \n\t" \
"	sub	%A1,%A3 \n\t" /*remainder = remainder - divisor*/ \
"	sbc	%B1,%B3 \n\t" \
"	brcc	d16u_7 \n\t" /*if result negative*/ \
"	add	%A1,%A3 \n\t" /*    restore remainder*/ \
"	adc	%B1,%B3 \n\t" \
"	clc \n\t" /*    clear carry to be shifted into result*/ \
"	rjmp	d16u_8 \n\t" /*else*/ \
"d16u_7:	sec \n\t" /*    set carry to be shifted into result*/ \
"d16u_8:	rol	%A2 \n\t" /*shift left dividend*/ \
"	rol	%B2 \n\t" \
"	rol	%A1 \n\t" /*shift dividend into remainder*/ \
"	rol	%B1 \n\t" \
"	sub	%A1,%A3 \n\t" /*remainder = remainder - divisor*/ \
"	sbc	%B1,%B3 \n\t" \
"	brcc	d16u_9 \n\t" /*if result negative*/ \
"	add	%A1,%A3 \n\t" /*    restore remainder*/ \
"	adc	%B1,%B3 \n\t" \
"	clc \n\t" /*    clear carry to be shifted into result*/ \
"	rjmp	d16u_10 \n\t" /*else*/ \
"d16u_9:	sec \n\t" /*    set carry to be shifted into result*/ \
"d16u_10:rol	%A2 \n\t" /*shift left dividend*/ \
"	rol	%B2 \n\t" \
"	rol	%A1 \n\t" /*shift dividend into remainder*/ \
"	rol	%B1 \n\t" \
"	sub	%A1,%A3 \n\t" /*remainder = remainder - divisor*/ \
"	sbc	%B1,%B3 \n\t" \
"	brcc	d16u_11 \n\t" /*if result negative*/ \
"	add	%A1,%A3 \n\t" /*    restore remainder*/ \
"	adc	%B1,%B3 \n\t" \
"	clc \n\t" /*    clear carry to be shifted into result*/ \
"	rjmp	d16u_12 \n\t" /*else*/ \
"d16u_11:sec \n\t" /*    set carry to be shifted into result*/ \
"d16u_12:rol	%A2 \n\t" /*shift left dividend*/ \
"	rol	%B2 \n\t" \
"	rol	%A1 \n\t" /*shift dividend into remainder*/ \
"	rol	%B1 \n\t" \
"	sub	%A1,%A3 \n\t" /*remainder = remainder - divisor*/ \
"	sbc	%B1,%B3 \n\t" \
"	brcc	d16u_13 \n\t" /*if result negative*/ \
"	add	%A1,%A3 \n\t" /*    restore remainder*/ \
"	adc	%B1,%B3 \n\t" \
"	clc \n\t" /*    clear carry to be shifted into result*/ \
"	rjmp	d16u_14 \n\t" /*else*/ \
"d16u_13:sec \n\t" /*    set carry to be shifted into result*/ \
"d16u_14:rol	%A2 \n\t" /*shift left dividend*/ \
"	rol	%B2 \n\t" \
"	rol	%A1 \n\t" /*shift dividend into remainder*/ \
"	rol	%B1 \n\t" \
"	sub	%A1,%A3 \n\t" /*remainder = remainder - divisor*/ \
"	sbc	%B1,%B3 \n\t" \
"	brcc	d16u_15 \n\t" /*if result negative*/ \
"	add	%A1,%A3 \n\t" /*    restore remainder*/ \
"	adc	%B1,%B3 \n\t" \
"	clc \n\t" /*    clear carry to be shifted into result*/ \
"	rjmp	d16u_16 \n\t" /*else*/ \
"d16u_15:sec \n\t" /*    set carry to be shifted into result*/ \
"d16u_16:rol	%A2 \n\t" /*shift left dividend*/ \
"	rol	%B2 \n\t" \
"	rol	%A1 \n\t" /*shift dividend into remainder*/ \
"	rol	%B1 \n\t" \
"	sub	%A1,%A3 \n\t" /*remainder = remainder - divisor*/ \
"	sbc	%B1,%B3 \n\t" \
"	brcc	d16u_17 \n\t" /*if result negative*/ \
"	add	%A1,%A3 \n\t" /*    restore remainder*/ \
"	adc	%B1,%B3 \n\t" \
"	clc \n\t" /*    clear carry to be shifted into result*/ \
"	rjmp	d16u_18 \n\t" /*else*/ \
"d16u_17:	sec \n\t" /*    set carry to be shifted into result*/ \
"d16u_18:rol	%A2 \n\t" /*shift left dividend*/ \
"	rol	%B2 \n\t" \
"	rol	%A1 \n\t" /*shift dividend into remainder*/ \
"	rol	%B1 \n\t" \
"	sub	%A1,%A3 \n\t" /*remainder = remainder - divisor*/ \
"	sbc	%B1,%B3 \n\t" \
"	brcc	d16u_19 \n\t" /*if result negative*/ \
"	add	%A1,%A3 \n\t" /*    restore remainder*/ \
"	adc	%B1,%B3 \n\t" \
"	clc \n\t" /*    clear carry to be shifted into result*/ \
"	rjmp	d16u_20 \n\t" /*else*/ \
"d16u_19:sec \n\t" /*    set carry to be shifted into result*/ \
"d16u_20:rol	%A2 \n\t" /*shift left dividend*/ \
"	rol	%B2 \n\t" \
"	rol	%A1 \n\t" /*shift dividend into remainder*/ \
"	rol	%B1 \n\t" \
"	sub	%A1,%A3 \n\t" /*remainder = remainder - divisor*/ \
"	sbc	%B1,%B3 \n\t" \
"	brcc	d16u_21 \n\t" /*if result negative*/ \
"	add	%A1,%A3 \n\t" /*    restore remainder*/ \
"	adc	%B1,%B3 \n\t" \
"	clc \n\t" /*    clear carry to be shifted into result*/ \
"	rjmp	d16u_22 \n\t" /*else*/ \
"d16u_21:sec \n\t" /*    set carry to be shifted into result*/ \
"d16u_22:rol	%A2 \n\t" /*shift left dividend*/ \
"	rol	%B2 \n\t" \
"	rol	%A1 \n\t" /*shift dividend into remainder*/ \
"	rol	%B1 \n\t" \
"	sub	%A1,%A3 \n\t" /*remainder = remainder - divisor*/ \
"	sbc	%B1,%B3 \n\t" \
"	brcc	d16u_23 \n\t" /*if result negative*/ \
"	add	%A1,%A3 \n\t" /*    restore remainder*/ \
"	adc	%B1,%B3 \n\t" \
"	clc \n\t" /*    clear carry to be shifted into result*/ \
"	rjmp	d16u_24 \n\t" /*else*/ \
"d16u_23:sec \n\t" /*    set carry to be shifted into result*/ \
"d16u_24:rol	%A2 \n\t" /*shift left dividend*/ \
"	rol	%B2 \n\t" \
"	rol	%A1 \n\t" /*shift dividend into remainder*/ \
"	rol	%B1 \n\t" \
"	sub	%A1,%A3 \n\t" /*remainder = remainder - divisor*/ \
"	sbc	%B1,%B3 \n\t" \
"	brcc	d16u_25 \n\t" /*if result negative*/ \
"	add	%A1,%A3 \n\t" /*    restore remainder*/ \
"	adc	%B1,%B3 \n\t" \
"	clc \n\t" /*    clear carry to be shifted into result*/ \
"	rjmp	d16u_26 \n\t" /*else*/ \
"d16u_25:sec \n\t" /*    set carry to be shifted into result*/ \
"d16u_26:rol	%A2 \n\t" /*shift left dividend*/ \
"	rol	%B2 \n\t" \
"	rol	%A1 \n\t" /*shift dividend into remainder*/ \
"	rol	%B1 \n\t" \
"	sub	%A1,%A3 \n\t" /*remainder = remainder - divisor*/ \
"	sbc	%B1,%B3 \n\t" \
"	brcc	d16u_27 \n\t" /*if result negative*/ \
"	add	%A1,%A3 \n\t" /*    restore remainder*/ \
"	adc	%B1,%B3 \n\t" \
"	clc \n\t" /*    clear carry to be shifted into result*/ \
"	rjmp	d16u_28 \n\t" /*else*/ \
"d16u_27:sec \n\t" /*    set carry to be shifted into result*/ \
"d16u_28:rol	%A2 \n\t" /*shift left dividend*/ \
"	rol	%B2 \n\t" \
"	rol	%A1 \n\t" /*shift dividend into remainder*/ \
"	rol	%B1 \n\t" \
"	sub	%A1,%A3 \n\t" /*remainder = remainder - divisor*/ \
"	sbc	%B1,%B3 \n\t" \
"	brcc	d16u_29 \n\t" /*if result negative*/ \
"	add	%A1,%A3 \n\t" /*    restore remainder*/ \
"	adc	%B1,%B3 \n\t" \
"	clc \n\t" /*    clear carry to be shifted into result*/ \
"	rjmp	d16u_30 \n\t" /*else*/ \
"d16u_29:sec \n\t" /*    set carry to be shifted into result*/ \
"d16u_30:rol	%A2 \n\t" /*shift left dividend*/ \
"	rol	%B2 \n\t" \
"	rol	%A1 \n\t" /*shift dividend into remainder*/ \
"	rol	%B1 \n\t" \
"	sub	%A1,%A3 \n\t" /*remainder = remainder - divisor*/ \
"	sbc	%B1,%B3 \n\t" \
"	brcc	d16u_31 \n\t" /*if result negative*/ \
"	add	%A1,%A3 \n\t" /*    restore remainder*/ \
"	adc	%B1,%B3 \n\t" \
"	clc \n\t" /*    clear carry to be shifted into result*/ \
"	rjmp	d16u_32 \n\t" /*else*/ \
"d16u_31:sec \n\t" /*    set carry to be shifted into result*/ \
"d16u_32:rol	%A2 \n\t" /*shift left dividend*/ \
"	rol	%B2 \n\t" \
"	mov %A0,%A2 \n\t" \
"	mov %B0,%B2 \n\t" \
: "+a" (*result), "+a" (*remainder) \
: "a" (dividend),  "a" (divisor)\
);
}

#endif // _AVRDiv_
