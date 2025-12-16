#define GL_SILENCE_DEPRECATION
#include <GLUT/glut.h>
#include "headers/menu.h"

// Define the external variables
GameState currentGameState = MENU;
int windowWidth = 1280;
int windowHeight = 720;

// Render text at screen position using GLUT bitmap font
void renderText(float x, float y, const char* text, void* font) {
    glRasterPos2f(x, y);
    while (*text) {
        glutBitmapCharacter(font, *text);
        text++;
    }
}

// Draw the startup menu
void drawMenu() {
    // Switch to 2D orthographic projection for menu
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, windowWidth, 0, windowHeight);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    // Disable lighting for 2D rendering
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);

    // Draw title using stroke font (scalable, much larger)
    glColor3f(0.0f, 0.9f, 0.9f);  // Cyan
    const char* title = "BLOXORZ 3D";
    
    // Calculate title width for centering (stroke characters are ~104 units wide)
    float scale = 0.5f;  // Scale factor for stroke font
    int titleWidth = 0;
    for (const char* c = title; *c; c++) {
        titleWidth += glutStrokeWidth(GLUT_STROKE_ROMAN, *c);
    }
    float scaledTitleWidth = titleWidth * scale;
    
    glPushMatrix();
    glTranslatef((windowWidth - scaledTitleWidth) / 2.0f, windowHeight * 0.65f, 0);
    glScalef(scale, scale, 1.0f);
    glLineWidth(3.0f);
    for (const char* c = title; *c; c++) {
        glutStrokeCharacter(GLUT_STROKE_ROMAN, *c);
    }
    glPopMatrix();

    // Button dimensions
    int buttonWidth = 200;
    int buttonHeight = 50;
    int buttonX = (windowWidth - buttonWidth) / 2;
    int buttonY = (windowHeight - buttonHeight) / 2 - 50;

    // Draw "New Game" button background
    glColor3f(0.3f, 0.3f, 0.4f);  // Dark gray
    glBegin(GL_QUADS);
    glVertex2i(buttonX, buttonY);
    glVertex2i(buttonX + buttonWidth, buttonY);
    glVertex2i(buttonX + buttonWidth, buttonY + buttonHeight);
    glVertex2i(buttonX, buttonY + buttonHeight);
    glEnd();

    // Draw button border
    glColor3f(0.0f, 0.9f, 0.9f);  // Cyan
    glLineWidth(2.0f);
    glBegin(GL_LINE_LOOP);
    glVertex2i(buttonX, buttonY);
    glVertex2i(buttonX + buttonWidth, buttonY);
    glVertex2i(buttonX + buttonWidth, buttonY + buttonHeight);
    glVertex2i(buttonX, buttonY + buttonHeight);
    glEnd();

    // Draw button text
    const char* buttonText = "New Game";
    int textWidth = glutBitmapLength(GLUT_BITMAP_HELVETICA_18, (const unsigned char*)buttonText);
    glColor3f(1.0f, 1.0f, 1.0f);  // White
    renderText(buttonX + (buttonWidth - textWidth) / 2.0f, buttonY + buttonHeight / 2.0f - 5, buttonText, GLUT_BITMAP_HELVETICA_18);

    // Restore state
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
}

// Handle mouse clicks
void mouseClick(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        if (currentGameState == MENU) {
            // Convert to OpenGL coordinates (flip Y)
            int mouseY = windowHeight - y;

            // Button bounds
            int buttonWidth = 200;
            int buttonHeight = 50;
            int buttonX = (windowWidth - buttonWidth) / 2;
            int buttonY = (windowHeight - buttonHeight) / 2 - 50;

            // Check if click is within "New Game" button
            if (x >= buttonX && x <= buttonX + buttonWidth &&
                mouseY >= buttonY && mouseY <= buttonY + buttonHeight) {
                // Start the game with level 1
                currentGameState = PLAYING;
            }
        }
    }
}
