//
// Created by antho on 1/5/2026.
//

#ifndef SUTPID_OPERATIONS_H
#define SUTPID_OPERATIONS_H

// Start checking every second for video playback
void startChecking(void);

// Stop checking
void stopChecking(void);

// Play the video file (instant)
void playVideo(void);

// Set the chance denominator (2 to 100, e.g., 5 means 1/5 chance)
void setChanceDenominator(int denominator);

// Get the current chance denominator
int getChanceDenominator(void);

#endif //SUTPID_OPERATIONS_H
