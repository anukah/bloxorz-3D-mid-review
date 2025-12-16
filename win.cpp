#define GL_SILENCE_DEPRECATION
#include "headers/win.h"
#include "headers/menu.h"
#include <GLUT/glut.h>

// Win state
bool hasWon = false;

// Helper function to render text
void renderWinText(float x, float y, const char *text, void *font) {
  glRasterPos2f(x, y);
  while (*text) {
    glutBitmapCharacter(font, *text);
    text++;
  }
}

// Draw the "You Won" overlay
void drawWinScreen() {
  if (!hasWon)
    return;

  // Switch to 2D orthographic projection
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  gluOrtho2D(0, windowWidth, 0, windowHeight);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();

  glDisable(GL_LIGHTING);
  glDisable(GL_DEPTH_TEST);

  // Semi-transparent dark overlay
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glColor4f(0.0f, 0.0f, 0.1f, 0.7f);
  glBegin(GL_QUADS);
  glVertex2i(0, 0);
  glVertex2i(windowWidth, 0);
  glVertex2i(windowWidth, windowHeight);
  glVertex2i(0, windowHeight);
  glEnd();

  // Draw "YOU WON!" text using stroke font (large)
  glColor3f(0.0f, 1.0f, 0.5f); // Green
  const char *title = "YOU WON!";

  float scale = 0.6f;
  int titleWidth = 0;
  for (const char *c = title; *c; c++) {
    titleWidth += glutStrokeWidth(GLUT_STROKE_ROMAN, *c);
  }
  float scaledTitleWidth = titleWidth * scale;

  glPushMatrix();
  glTranslatef((windowWidth - scaledTitleWidth) / 2.0f, windowHeight * 0.55f,
               0);
  glScalef(scale, scale, 1.0f);
  glLineWidth(4.0f);
  for (const char *c = title; *c; c++) {
    glutStrokeCharacter(GLUT_STROKE_ROMAN, *c);
  }
  glPopMatrix();

  // Draw instruction text
  const char *instruction = "Press SPACE to play again";
  int textWidth = glutBitmapLength(GLUT_BITMAP_HELVETICA_18,
                                   (const unsigned char *)instruction);
  glColor3f(1.0f, 1.0f, 1.0f);
  renderWinText((windowWidth - textWidth) / 2.0f, windowHeight * 0.35f,
                instruction, GLUT_BITMAP_HELVETICA_18);

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_LIGHTING);

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
  glPopMatrix();
}

// Reset win state
void resetWinState() { hasWon = false; }
