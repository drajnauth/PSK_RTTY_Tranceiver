#ifndef _TIMER_H_
#define _TIMER_H_

#define TIMER22MS  340         // Counter for 22 ms, default 344
#define TIMER32MS  500         // Counter for 32 ms, default 500
#define TIMER5MS   1250        // Counter for 5 ms, default 1250
#define TIMER1MS   250         // Counter for 1 ms, default 250
#define TIMER3MS   750         // Counter for 3 ms, default 750

// Timer Control Routines
void EnableTimers (unsigned char timer, unsigned int count);
void DisableTimers (unsigned char timer);
void Pause (int dly);
void DisableTimer0 (void);
void SaveTimerRegisters (void);
void RestoreTimerRegisters (void);


#endif // _TIMER_H_
