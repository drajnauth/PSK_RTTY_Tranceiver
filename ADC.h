#ifndef _ADC_H_
#define _ADC_H_

// ADC Sampling Routines
void EnableADC (void);
void StartSampling (void);
void StopSampling (void);
void ToggleSampling (unsigned char mode);
void CheckForClip (void);

#define MAX_CLIP_COUNT 2
#define MAX_CLIP_RESET_COUNT 500
#define MIN_ADC_DELTA 3
#define ADC_RESET_COUNT 3000
#define ADC_CLIPPING_THRESHOLD 200






#endif // _ADC_H_
