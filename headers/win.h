#ifndef WIN_H
#define WIN_H

// Win state
extern bool hasWon;

// Win functions
void checkWinCondition(); // Check if block is standing on goal tile
void drawWinScreen();     // Draw the "You Won" overlay
void resetWinState();     // Reset win state for new game

#endif
