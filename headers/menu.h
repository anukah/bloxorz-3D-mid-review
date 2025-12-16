#ifndef MENU_H
#define MENU_H

// Game State
enum GameState { MENU, PLAYING };

// External variables needed by menu
extern GameState currentGameState;
extern int windowWidth;
extern int windowHeight;

// Menu functions
void drawMenu();
void mouseClick(int button, int state, int x, int y);
void renderText(float x, float y, const char *text, void *font);

#endif
