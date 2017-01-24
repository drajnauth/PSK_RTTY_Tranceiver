#ifndef _Si5351_H_
#define _Si5351_H_



// Flags

#define SI5351_ADDRESS (0x60) 
#define I2C_READBIT (0x01)
#define FAREY_N  1048575

typedef struct {
  char PLL;
  unsigned long Fxtal, Fxtalcorr, PLL_Fvco, MS_Fout;
  unsigned long PLL_a, PLL_b, PLL_c;
  unsigned long MSN_P1, MSN_P2, MSN_P3;
  unsigned long MS_a, MS_b, MS_c;
  unsigned long MS_P1, MS_P2, MS_P3;
  unsigned int R_DIV;
  unsigned int SimpleDivider;
  unsigned char MS_DIVBY4;
  unsigned char ClkEnable;
  long int correction;   // can be + or -
} Si5351_def;

typedef struct {
  char PLL;
  unsigned char mAdrive;
        unsigned char phase;
        unsigned int maxangle;
        unsigned char reg;
        unsigned long freq;
        unsigned long PLLFreq;
} Si5351_CLK_def;

void SetFrequency (unsigned long freq);

void WriteSi5351PLL (void);
void UpdateClkControlRegister (void);
void UpdatePhaseControlRegister (void);
void CalculateDividers (unsigned long freq);
void RationalNumberApproximation(unsigned long given_numerator, unsigned long given_denominator,
        unsigned long max_numerator, unsigned long max_denominator,
        unsigned long *best_numerator, unsigned long *best_denominator);
void DisableSi5351Clocks (void);
void ResetSi5351 (unsigned int loadcap);
void UpdatePhase (unsigned int phase);
void InvertClk (unsigned char invert);

void ProgramSi5351 (void);
void Si5351WriteRegister (unsigned char reg, unsigned char value);
void Si5351RepeatedWriteRegister(unsigned char  addr, unsigned char  bytes, unsigned char *data);
unsigned char Si5351ReadRegister (unsigned char reg);




#define SI_CLK0  0
#define SI_CLK1  1
#define SI_CLK2  2

#define SIREG_0_DEVICE_STAT        0
#define SIREG_1_INT_STAT_STICKY    1
#define SIREG_2_INT_STAT_MASK      2
#define SIREG_3_OUTPUT_ENABLE_CTL  3
#define SIREG_9_OEB_PIN_ENABLE_CTL 9
#define SIREG_15_PLL_INPUT_SRC     15
#define SIREG_16_CLK0_CTL          16
#define SIREG_17_CLK1_CTL          17
#define SIREG_18_CLK2_CTL          18

#define SI_MSREGS                  8
#define SIREG_26_MSNA_1            26
#define SIREG_27_MSNA_2            27
#define SIREG_28_MSNA_3            28
#define SIREG_29_MSNA_4            29
#define SIREG_30_MSNA_5            30
#define SIREG_31_MSNA_6            31
#define SIREG_32_MSNA_7            32
#define SIREG_33_MSNA_8            33

#define SIREG_34_MSNB_1            34
#define SIREG_35_MSNB_2            35
#define SIREG_36_MSNB_3            36
#define SIREG_37_MSNB_4            37
#define SIREG_38_MSNB_5            38
#define SIREG_39_MSNB_6            39
#define SIREG_40_MSNB_7            40
#define SIREG_41_MSNB_8            41

#define SIREG_42_MSYN0_1           42
#define SIREG_43_MSYN0_2           43
#define SIREG_44_MSYN0_3           44
#define SIREG_45_MSYN0_4           45
#define SIREG_46_MSYN0_5           46
#define SIREG_47_MSYN0_6           47
#define SIREG_48_MSYN0_7           48
#define SIREG_49_MSYN0_8           49

#define SIREG_50_MSYN1_1           50
#define SIREG_51_MSYN1_2           51
#define SIREG_52_MSYN1_3           52
#define SIREG_53_MSYN1_4           53
#define SIREG_54_MSYN1_5           54
#define SIREG_55_MSYN1_6           55
#define SIREG_56_MSYN1_7           56
#define SIREG_57_MSYN1_8           57

#define SIREG_58_MSYN2_1           58
#define SIREG_59_MSYN2_2           59
#define SIREG_60_MSYN2_3           60
#define SIREG_61_MSYN2_4           61
#define SIREG_62_MSYN2_5           62
#define SIREG_63_MSYN2_6           63
#define SIREG_64_MSYN2_7           64
#define SIREG_65_MSYN2_8           65

#define SIREG_092_CLOCK_6_7_OUTPUT_DIVIDER  92
#define SIREG_165_CLK0_PHASE_OFFSET         165
#define SIREG_166_CLK1_PHASE_OFFSET         166
#define SIREG_167_CLK2_PHASE_OFFSET         167
#define SIREG_177_PLL_RESET                 177
#define SIREG_183_CRY_LOAD_CAP        183

#define SI_ENABLE_CLK0     B00000001
#define SI_ENABLE_CLK1     B00000010
#define SI_ENABLE_CLK2     B00000100

#define SI_PLL_A        'A'
#define SI_PLL_B        'B'
#define SI_XTAL       'X'

#define SI_CRY_LOAD_6PF     0x52
#define SI_CRY_LOAD_8PF     0x92
#define SI_CRY_LOAD_10PF    0x52

#define SI_CRY_FREQ_25MHZ     25000000
#define SI_CRY_FREQ_27MHZ     27000000

#define SI_MAXIMUM_DENOMINATOR  1048575   // 20 bits of denomintor and 2^20 = 1048576, which is 0 to 1048575
#define SI5351_PLL_A_MIN          15
#define SI5351_PLL_A_MAX          90
#define SI5351_MULTISYNTH_A_MIN        6
#define SI5351_MULTISYNTH_A_MAX       1800

#define SI_MAX_PLL_FREQ     900000000
#define SI_MIN_PLL_FREQ     600000000
#define SI_AUTO_PLL_FREQ    0

#define SI_MAX_OUT_FREQ     30000000             // Fixed arbituarly to HF frequencies
#define SI_MIN_OUT_FREQ     1000000

#define SI_MAX_MS_FREQ      150000000
#define SI_MSYN_DIV_4     4
#define SI_MSYN_DIV_6     6
#define SI_MSYN_DIV_8     8

#define SI_PHASE_CONSTANT     11430      // This is 127 x 360 / 4 (127 is the max value allowed in phase register)

#define SI_R_DIV_1              0
#define SI_R_DIV_2              1
#define SI_R_DIV_4              2
#define SI_R_DIV_8              3
#define SI_R_DIV_16             4
#define SI_R_DIV_32             5
#define SI_R_DIV_64             6
#define SI_R_DIV_128            7

#define SI_CLK_OFF    B10000000
#define SI_CLK_MS_INT   B01000000
#define SI_CLK_SRC_PLLB   B00100000
#define SI_CLK_SRC_MS   B00001100 
#define SI_CLK_2MA    B00000000
#define SI_CLK_4MA    B00000001
#define SI_CLK_6MA    B00000010
#define SI_CLK_8MA    B00000011

#define SI_CLK_INVERT   B00010000


#define SI_CLK_CLR_DRIVE        B11111100

  
#define SI_PLLA_RESET   B10000000
#define SI_PLLB_RESET   B00100000


/* Macro definitions */
/*
 * Based on former asm-ppc/div64.h and asm-m68knommu/div64.h
 *
 * The semantics of do_div() are:
 *
 * uint32_t do_div(uint64_t *n, uint32_t base)
 * {
 *      uint32_t remainder = *n % base;
 *      *n = *n / base;
 *      return remainder;
 * }
 *
 * NOTE: macro parameter n is evaluated multiple times,
 *       beware of side effects!
*/

# define do_div(n,base) ({                                      \
        uint64_t __base = (base);                               \
        uint64_t __rem;                                         \
        __rem = ((uint64_t)(n)) % __base;                       \
        (n) = ((uint64_t)(n)) / __base;                         \
        __rem;                                                  \
 })

uint32_t div64(uint64_t *n, uint32_t base);

#endif // _Si5351_H_

