/*
Implementation of the Controller_PID struct as defined by pid.h and its functions.

This is used as data in pid.c
*/

#include "pid.h"
#include "settings.h"

void PID_Reset (Controller_PID* PID) { 
// initializes the PID controller
    PID->I = 0;
    PID->Const_P = 0;
    PID->Const_I = 0;
    PID->Const_D = 0;
    PID->Alpha = 0;
    PID->num_values = 0;
}

void PID_Update (Controller_PID* PID, double value) {
// updates the controller with a single reading
    PID->P = value;
    PID->I = (1-PID->Alpha)*PID->I * (1/TIMER_RATE_IN_HZ) + value; 
    //(1-PID->Alpha)*PID->I should be multiplied by dt, time difference between 2 calls of PID_Update
    
    int i; 
    for (i = PID_NUM_OLD_VALUES-1; i > 0; i--) // do a shift on values, knock them back and kill the oldest one
        PID->old_values[i] = PID->old_values[i-1];
    
    PID->old_values[0] = value;
    
    double temp = 0;
    if (PID->num_values >= PID_NUM_OLD_VALUES-1) { // if enough values accumulated
        // take the average difference between old valuse 
        // for (i = 1; i < PID_NUM_OLD_VALUES; i++) temp += PID->old_values[i-1] - PID->old_values[i];
        temp = PID -> old_values[0] - PID-> old_values[PID_NUM_OLD_VALUES-1];
        // change from Victor version:
        // made it average rise / dt, where dt is 1/TIMER_RATE_HZ
        PID->D = (float)temp / PID_NUM_OLD_VALUES * TIMER_RATE_IN_HZ;
    } else {
        PID->D = 0;
        PID->num_values++; // only increment here to avoid overflow
    }
}

double PID_Output (Controller_PID* PID) {
// calculate the output of the controller
    return PID->Const_P*PID->P + PID->Const_I*PID->I + PID->Const_D*PID->D;    
}

