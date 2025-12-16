#define GL_SILENCE_DEPRECATION // Ignore deprecation errors
#include "dependencies/include/SOIL2/SOIL2.h"
#include "headers/levels.h"
#include <GLUT/glut.h>
#include <cmath>
#include <vector>

struct Vec3 {
  float x, y, z;
};

GLuint glassTextureID;

const float PI = 3.14159f;
const float CAMERA_SMOOTH_FACTOR = 0.05f; // How quickly the camera moves
const int TIMER_INTERVAL_MS = 16;         // 60 FPS
const float BLOCK_ANIMATION_SPEED = 0.08f;
const float FALL_SPEED = 0.15f;

// a vector of vectors from a 2D C-style array.
std::vector<std::vector<int>> platformLayout =
    getLevelLayout(3); // CHANGE LEVEL

const int PLATFORM_ROWS = platformLayout.size();
const int PLATFORM_COLS = platformLayout[0].size();
const float TILE_SIZE = 1.0f;

// Toggle tile state (initially hidden)
bool toggleGroup1Visible = false; // Bridge at columns 4-5 (row 3)
bool toggleGroup2Visible = false; // Bridge at columns 10-11 (row 3)

// Toggle tile positions: {row, col} pairs for toggle tiles controlled by each
// action tile Action tile at (1,2) controls tiles at (3,4) and (3,5) Action
// tile at (1,8) controls tiles at (3,10) and (3,11)
const int TOGGLE_GROUP_1_ACTION_ROW = 1, TOGGLE_GROUP_1_ACTION_COL = 2;
const int TOGGLE_GROUP_2_ACTION_ROW = 1, TOGGLE_GROUP_2_ACTION_COL = 8;
const int TOGGLE_TILES_1[][2] = {{3, 4}, {3, 5}};
const int TOGGLE_TILES_2[][2] = {{3, 10}, {3, 11}};

// Camera State
float cameraAngleX = 30.0f;
float cameraAngleY = -45.0f;
float cameraDistance = 15.0f;

// Target values for smooth interpolation
float targetCameraAngleX = 30.0f;
float targetCameraAngleY = -45.0f;
float targetCameraDistance = 15.0f;

// Define the block's starting position in grid coordinates (will be set by
// findStartPosition)
int START_ROW = 1;
int START_COL = 1;

// Block State
enum BlockOrientation { STANDING, LYING_X, LYING_Z };

struct Block {
  // Current state
  float x, y, z;
  BlockOrientation orientation;

  // Animation state
  bool isAnimating;
  float animationProgress; // 0.0 to 1.0
  Vec3 startPos, targetPos;
  Vec3 pivotPoint;             // Pivot point for natural rolling
  float startRotZ, targetRotZ; // For rolling left/right
  float startRotX, targetRotX; // For rolling forward/backward

  // Fall state
  bool isFalling;
  float fallVelocity;
};

Block block = {
    (-PLATFORM_COLS / 2.0f + START_COL + 0.5f) * TILE_SIZE, // x
    1.0f,                                                   // y
    (-PLATFORM_ROWS / 2.0f + START_ROW + 0.5f) * TILE_SIZE, // z
    STANDING,                                               // orientation
    false,                                                  // isAnimating
    0.0f,                                                   // animationProgress
    {0, 0, 0},
    {0, 0, 0}, // startPos, targetPos
    {0, 0, 0}, // pivotPoint
    0.0f,
    0.0f, // startRotZ, targetRotZ
    0.0f,
    0.0f,  // startRotX, targetRotX
    false, // isFalling
    0.0f   // fallVelocity
};

// Functions
void init();
void update();
void display();
void reshape(int w, int h);
void keyboard(unsigned char key, int x, int y);
void timer(int value);
void applyCameraTransform();
void drawCube();
void drawCubeBorders();
void drawPlatform();
void drawBlock();
void specialKeys(int key, int x, int y);
void moveBlock(int dx, int dz);
bool checkBlockFall();    // Returns true if block should fall
void resetBlock();        // Reset block to starting position
void checkToggleTiles();  // Check and toggle tiles when block lands on action
                          // tile
void initToggleTiles();   // Initialize toggle tiles to hidden
void findStartPosition(); // Find starting position from tile 9 in level data

// Main
int main(int argc, char **argv) {
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutInitWindowSize(1280, 720);
  glutInitWindowPosition(100, 100);
  glutCreateWindow("Bloxorz-3D");

  init();

  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  glutKeyboardFunc(keyboard);
  glutSpecialFunc(specialKeys);
  glutTimerFunc(TIMER_INTERVAL_MS, timer, 0);

  glutMainLoop();
  return 0;
}

void init() {
  glClearColor(0.05f, 0.0f, 0.1f, 1.0f); // Dark purple-blue background
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // Lighting
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);

  // Light source
  GLfloat light_position[] = {5.0f, 10.0f, 5.0f, 0.0f};
  GLfloat light_ambient[] = {0.3f, 0.3f, 0.4f, 1.0f};
  GLfloat light_diffuse[] = {0.8f, 0.8f, 1.0f, 1.0f};

  glLightfv(GL_LIGHT0, GL_POSITION, light_position);
  glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);

  glassTextureID = SOIL_load_OGL_texture(
      "textures/glass.jpg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID,
      SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_TEXTURE_REPEATS);

  // Check for errors
  if (glassTextureID == 0) {
    printf("SOIL loading error: '%s'\n", SOIL_last_result());
  }

  // Find starting position from tile 9 in level data
  findStartPosition();

  // Initialize toggle tiles (hide them initially)
  initToggleTiles();

  // Reset block to found starting position
  resetBlock();
}

void update() {
  // Smooth camera movement
  cameraAngleX += (targetCameraAngleX - cameraAngleX) * CAMERA_SMOOTH_FACTOR;
  cameraAngleY += (targetCameraAngleY - cameraAngleY) * CAMERA_SMOOTH_FACTOR;
  cameraDistance +=
      (targetCameraDistance - cameraDistance) * CAMERA_SMOOTH_FACTOR;

  // Handle falling
  if (block.isFalling) {
    block.fallVelocity += 0.02f; // Gravity acceleration
    block.y -= block.fallVelocity;

    // Reset when fallen far enough
    if (block.y < -10.0f) {
      resetBlock();
    }
    return;
  }

  if (block.isAnimating) {
    block.animationProgress += BLOCK_ANIMATION_SPEED;
    if (block.animationProgress >= 1.0f) {
      // Animation finished, snap to final state
      block.isAnimating = false;
      block.animationProgress = 1.0f;
      block.x = block.targetPos.x;
      block.y = block.targetPos.y;
      block.z = block.targetPos.z;

      // Check for toggle tile activation
      checkToggleTiles();

      // Check if block should fall
      if (checkBlockFall()) {
        block.isFalling = true;
        block.fallVelocity = 0.0f;
      }
    }
  }
}

// Display
void display() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glLoadIdentity();

  applyCameraTransform();

  drawPlatform(); // Draw Platform
  drawBlock();    // Draw Block
  glutSwapBuffers();
}

// Reshape
void reshape(int w, int h) {
  if (h == 0)
    h = 1;
  glViewport(0, 0, w, h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(45.0f, (float)w / h, 0.1f, 100.0f);
  glMatrixMode(GL_MODELVIEW);
}

// Input
void keyboard(unsigned char key, int x, int y) {
  switch (key) {
  // Zoom
  case 'w':
  case 'W':
    targetCameraDistance -= 0.5f;
    break;
  case 's':
  case 'S':
    targetCameraDistance += 0.5f;
    break;
  // Orbit
  case 'a':
  case 'A':
    targetCameraAngleY -= 5.0f;
    break;
  case 'd':
  case 'D':
    targetCameraAngleY += 5.0f;
    break;
  // Presets
  case '1':
    targetCameraAngleX = 30.0f;
    targetCameraAngleY = -45.0f;
    targetCameraDistance = 15.0f;
    break;
  case '2':
    targetCameraAngleX = 30.0f;
    targetCameraAngleY = 135.0f;
    targetCameraDistance = 15.0f;
    break;
  // Exit
  case 27:
    exit(0);
    break;
  }
}

void specialKeys(int key, int x, int y) {
  // Ignore new input if an animation is already playing
  if (block.isAnimating) {
    return;
  }

  switch (key) {
  case GLUT_KEY_LEFT:
    moveBlock(-1, 0);
    break;
  case GLUT_KEY_RIGHT:
    moveBlock(1, 0);
    break;
  case GLUT_KEY_UP:
    moveBlock(0, -1);
    break;
  case GLUT_KEY_DOWN:
    moveBlock(0, 1);
    break;
  }
}

// Timer
void timer(int value) {
  update();
  glutPostRedisplay();
  glutTimerFunc(TIMER_INTERVAL_MS, timer, 0);
}

// Camera Helper
void applyCameraTransform() {
  if (targetCameraAngleX > 89.0f)
    targetCameraAngleX = 89.0f;
  if (targetCameraAngleX < 5.0f)
    targetCameraAngleX = 5.0f;
  if (targetCameraDistance < 5.0f)
    targetCameraDistance = 5.0f;
  if (targetCameraDistance > 50.0f)
    targetCameraDistance = 50.0f;

  float radX = cameraAngleX * PI / 180.0f;
  float radY = cameraAngleY * PI / 180.0f;

  float camX = cameraDistance * sin(radY) * cos(radX);
  float camY = cameraDistance * sin(radX);
  float camZ = cameraDistance * cos(radY) * cos(radX);

  gluLookAt(camX, camY, camZ, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
}

// Draw cube
void drawCube() {
  glBegin(GL_QUADS);

  // Front Face
  glNormal3f(0.0, 0.0, 1.0);
  glTexCoord2f(0.0f, 0.0f);
  glVertex3f(-0.5f, -0.5f, 0.5f);
  glTexCoord2f(1.0f, 0.0f);
  glVertex3f(0.5f, -0.5f, 0.5f);
  glTexCoord2f(1.0f, 1.0f);
  glVertex3f(0.5f, 0.5f, 0.5f);
  glTexCoord2f(0.0f, 1.0f);
  glVertex3f(-0.5f, 0.5f, 0.5f);

  // Back Face
  glNormal3f(0.0, 0.0, -1.0);
  glTexCoord2f(1.0f, 0.0f);
  glVertex3f(-0.5f, -0.5f, -0.5f);
  glTexCoord2f(1.0f, 1.0f);
  glVertex3f(-0.5f, 0.5f, -0.5f);
  glTexCoord2f(0.0f, 1.0f);
  glVertex3f(0.5f, 0.5f, -0.5f);
  glTexCoord2f(0.0f, 0.0f);
  glVertex3f(0.5f, -0.5f, -0.5f);

  // Top Face
  glNormal3f(0.0, 1.0, 0.0);
  glTexCoord2f(0.0f, 1.0f);
  glVertex3f(-0.5f, 0.5f, -0.5f);
  glTexCoord2f(0.0f, 0.0f);
  glVertex3f(-0.5f, 0.5f, 0.5f);
  glTexCoord2f(1.0f, 0.0f);
  glVertex3f(0.5f, 0.5f, 0.5f);
  glTexCoord2f(1.0f, 1.0f);
  glVertex3f(0.5f, 0.5f, -0.5f);

  // Bottom Face
  glNormal3f(0.0, -1.0, 0.0);
  glTexCoord2f(1.0f, 1.0f);
  glVertex3f(-0.5f, -0.5f, -0.5f);
  glTexCoord2f(0.0f, 1.0f);
  glVertex3f(0.5f, -0.5f, -0.5f);
  glTexCoord2f(0.0f, 0.0f);
  glVertex3f(0.5f, -0.5f, 0.5f);
  glTexCoord2f(1.0f, 0.0f);
  glVertex3f(-0.5f, -0.5f, 0.5f);

  // Right Face
  glNormal3f(1.0, 0.0, 0.0);
  glTexCoord2f(1.0f, 0.0f);
  glVertex3f(0.5f, -0.5f, -0.5f);
  glTexCoord2f(1.0f, 1.0f);
  glVertex3f(0.5f, 0.5f, -0.5f);
  glTexCoord2f(0.0f, 1.0f);
  glVertex3f(0.5f, 0.5f, 0.5f);
  glTexCoord2f(0.0f, 0.0f);
  glVertex3f(0.5f, -0.5f, 0.5f);

  // Left Face
  glNormal3f(-1.0, 0.0, 0.0);
  glTexCoord2f(0.0f, 0.0f);
  glVertex3f(-0.5f, -0.5f, -0.5f);
  glTexCoord2f(1.0f, 0.0f);
  glVertex3f(-0.5f, -0.5f, 0.5f);
  glTexCoord2f(1.0f, 1.0f);
  glVertex3f(-0.5f, 0.5f, 0.5f);
  glTexCoord2f(0.0f, 1.0f);
  glVertex3f(-0.5f, 0.5f, -0.5f);

  glEnd();
}

// Draw border
void drawCubeBorders() { glutWireCube(TILE_SIZE + 0.001f); }

// Draw platform
void drawPlatform() {
  GLfloat plat_specular[] = {0.0f, 0.0f, 0.0f, 1.0f}; // No highlight
  GLfloat plat_shininess[] = {0.0f};                  // No shininess

  glMaterialfv(GL_FRONT, GL_SPECULAR, plat_specular);
  glMaterialfv(GL_FRONT, GL_SHININESS, plat_shininess);

  float offsetX = -PLATFORM_COLS * TILE_SIZE / 2.0f;
  float offsetZ = -PLATFORM_ROWS * TILE_SIZE / 2.0f;

  for (int i = 0; i < PLATFORM_ROWS; ++i) {
    for (int j = 0; j < PLATFORM_COLS; ++j) {
      // Only draw if the tile is not an empty space (0)
      if (platformLayout[i][j] != 0) {
        glPushMatrix();
        glTranslatef(offsetX + (j + 0.5f) * TILE_SIZE, -TILE_SIZE / 2.0f,
                     offsetZ + (i + 0.5f) * TILE_SIZE);

        // Set material properties for the tile
        int tileType = platformLayout[i][j];
        if (tileType == 1) {
          // Regular tile - dark gray
          GLfloat mat_diffuse[] = {0.2f, 0.2f, 0.25f, 0.7f};
          glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
        } else if (tileType == 2) {
          // Target tile - cyan
          GLfloat mat_diffuse[] = {0.0f, 0.8f, 0.8f, 0.6f};
          glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
        } else if (tileType == 4) {
          // Toggle tile (bridge) - orange
          GLfloat mat_diffuse[] = {1.0f, 0.6f, 0.2f, 0.7f};
          glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
        } else if (tileType == 5) {
          // Toggle action tile - purple
          GLfloat mat_diffuse[] = {0.7f, 0.3f, 0.9f, 0.7f};
          glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
        }
        drawCube();

        glDisable(GL_LIGHTING);
        glColor3f(0.0f, 0.9f, 0.9f); // Cyan
        drawCubeBorders();
        glEnable(GL_LIGHTING);

        glPopMatrix();
      }
    }
  }
}

void drawBlock() {
  glPushMatrix();

  if (block.isAnimating) {
    float t = block.animationProgress;
    t = t * t * (3.0f - 2.0f * t); // Smooth step interpolation

    float currentRotZ =
        block.startRotZ + (block.targetRotZ - block.startRotZ) * t;
    float currentRotX =
        block.startRotX + (block.targetRotX - block.startRotX) * t;

    // Translate to pivot point first
    glTranslatef(block.pivotPoint.x, block.pivotPoint.y, block.pivotPoint.z);

    // Apply rotation around pivot
    if (block.targetRotZ != 0) {
      glRotatef(currentRotZ, 0.0f, 0.0f, 1.0f);
    }
    if (block.targetRotX != 0) {
      glRotatef(currentRotX, 1.0f, 0.0f, 0.0f);
    }

    // Translate back from pivot to block's original position relative to pivot
    glTranslatef(block.startPos.x - block.pivotPoint.x,
                 block.startPos.y - block.pivotPoint.y,
                 block.startPos.z - block.pivotPoint.z);

  } else {
    glTranslatef(block.x, block.y, block.z);
  }

  GLfloat mat_ambient[] = {0.8f, 0.8f, 0.8f, 1.0f};
  GLfloat mat_diffuse[] = {1.0f, 1.0f, 1.0f, 0.8f}; // White base color, 80%
  GLfloat mat_specular[] = {1.0f, 1.0f, 1.0f, 1.0f};
  GLfloat mat_shininess[] = {90.0f};

  glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);
  glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
  glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
  glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

  glEnable(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_2D, glassTextureID);

  glPushMatrix();

  // Use the START orientation for scaling during animation
  BlockOrientation drawOrientation;
  if (block.isAnimating) {
    // Determine what orientation the block was BEFORE the move
    // by checking what it will become and reversing
    if (block.targetRotZ != 0) {
      // Moving in X direction
      if (block.orientation == STANDING) {
        drawOrientation = LYING_X; // Was lying, becoming standing
      } else if (block.orientation == LYING_X) {
        drawOrientation = STANDING; // Was standing, becoming lying
      } else {
        drawOrientation = LYING_Z; // Was lying Z, stays lying Z
      }
    } else if (block.targetRotX != 0) {
      // Moving in Z direction
      if (block.orientation == STANDING) {
        drawOrientation = LYING_Z;
      } else if (block.orientation == LYING_Z) {
        drawOrientation = STANDING;
      } else {
        drawOrientation = LYING_X;
      }
    } else {
      drawOrientation = block.orientation;
    }
  } else {
    drawOrientation = block.orientation;
  }

  switch (drawOrientation) {
  case STANDING:
    glScalef(1.0f, 2.0f, 1.0f);
    break;
  case LYING_X:
    glScalef(2.0f, 1.0f, 1.0f);
    break;
  case LYING_Z:
    glScalef(1.0f, 1.0f, 2.0f);
    break;
  }

  drawCube();

  glPopMatrix();

  glDisable(GL_TEXTURE_2D);

  glPopMatrix();
}

void moveBlock(int dx, int dz) {
  if (dx == 0 && dz == 0)
    return;

  // Store starting position and rotation
  block.startPos = {block.x, block.y, block.z};
  block.startRotZ = 0;
  block.startRotX = 0;
  block.targetRotZ = 0;
  block.targetRotX = 0;

  if (dx != 0) {
    // Horizontal movement (left/right) - rotates around Z axis
    if (block.orientation == STANDING) {
      // Standing (1x2x1) rolling to lying on X axis (2x1x1)
      // Block center is at height 1.0, bottom edge is at y=0
      // Pivot is at the bottom edge in the direction of movement
      block.pivotPoint = {block.x +
                              dx * 0.5f * TILE_SIZE, // Edge of the 1x1 base
                          0.0f,                      // Ground level
                          block.z};

      block.targetPos = {block.x + dx * 1.5f *
                                       TILE_SIZE, // Move 1.5 tiles (0.5 + 1.0)
                         0.5f,                    // Lying block center height
                         block.z};

      block.targetRotZ = -dx * 90.0f;
      block.orientation = LYING_X;

    } else if (block.orientation == LYING_X) {
      // Lying on X axis (2x1x1) - rolling to stand up
      // Block extends 1 tile in each X direction from center
      // Pivot is at the far edge in movement direction
      block.pivotPoint = {block.x +
                              dx * TILE_SIZE, // Far edge of the 2x1 footprint
                          0.0f,               // Ground level
                          block.z};

      block.targetPos = {block.x + dx * 1.5f * TILE_SIZE, // Move 1.5 tiles
                         1.0f, // Standing block center height
                         block.z};

      block.targetRotZ = -dx * 90.0f;
      block.orientation = STANDING;

    } else if (block.orientation == LYING_Z) {
      // Lying on Z axis (1x1x2) rolling sideways - stays lying on Z
      block.pivotPoint = {block.x + dx * 0.5f * TILE_SIZE, 0.0f, block.z};

      block.targetPos = {block.x + dx * TILE_SIZE, 0.5f, block.z};

      block.targetRotZ = -dx * 90.0f;
      // Orientation stays LYING_Z
    }
  }

  if (dz != 0) {
    // Forward/backward movement - rotates around X axis
    if (block.orientation == STANDING) {
      // Standing rolling forward/backward
      block.pivotPoint = {block.x, 0.0f, block.z + dz * 0.5f * TILE_SIZE};

      block.targetPos = {block.x, 0.5f, block.z + dz * 1.5f * TILE_SIZE};

      block.targetRotX = dz * 90.0f;
      block.orientation = LYING_Z;

    } else if (block.orientation == LYING_Z) {
      // Lying on Z axis rolling forward/backward to standing
      block.pivotPoint = {block.x, 0.0f, block.z + dz * TILE_SIZE};

      block.targetPos = {block.x, 1.0f, block.z + dz * 1.5f * TILE_SIZE};

      block.targetRotX = dz * 90.0f;
      block.orientation = STANDING;

    } else if (block.orientation == LYING_X) {
      // Lying on X axis rolling forward/backward - stays lying on X
      block.pivotPoint = {block.x, 0.0f, block.z + dz * 0.5f * TILE_SIZE};

      block.targetPos = {block.x, 0.5f, block.z + dz * TILE_SIZE};

      block.targetRotX = dz * 90.0f;
      // Orientation stays LYING_X
    }
  }

  block.isAnimating = true;
  block.animationProgress = 0.0f;
}

// Convert world X coordinate to grid column
int worldToGridCol(float worldX) {
  float offsetX = -PLATFORM_COLS * TILE_SIZE / 2.0f;
  return (int)floor((worldX - offsetX) / TILE_SIZE);
}

// Convert world Z coordinate to grid row
int worldToGridRow(float worldZ) {
  float offsetZ = -PLATFORM_ROWS * TILE_SIZE / 2.0f;
  return (int)floor((worldZ - offsetZ) / TILE_SIZE);
}

// Get tile value at grid position (returns 0 if out of bounds)
int getTileAt(int row, int col) {
  if (row < 0 || row >= PLATFORM_ROWS || col < 0 || col >= PLATFORM_COLS) {
    return 0; // Out of bounds = empty
  }
  return platformLayout[row][col];
}

// Check if block should fall
bool checkBlockFall() {
  int centerCol = worldToGridCol(block.x);
  int centerRow = worldToGridRow(block.z);

  switch (block.orientation) {
  case STANDING:
    // Standing block occupies only one tile
    if (getTileAt(centerRow, centerCol) == 0) {
      return true;
    }
    break;

  case LYING_X:
    // Block extends 1 tile in X direction (left and right from center)
    // Check both tiles the block occupies
    {
      int leftCol = worldToGridCol(block.x - 0.5f * TILE_SIZE);
      int rightCol = worldToGridCol(block.x + 0.5f * TILE_SIZE);
      if (getTileAt(centerRow, leftCol) == 0 ||
          getTileAt(centerRow, rightCol) == 0) {
        return true;
      }
    }
    break;

  case LYING_Z:
    // Block extends 1 tile in Z direction (forward and backward from center)
    {
      int frontRow = worldToGridRow(block.z - 0.5f * TILE_SIZE);
      int backRow = worldToGridRow(block.z + 0.5f * TILE_SIZE);
      if (getTileAt(frontRow, centerCol) == 0 ||
          getTileAt(backRow, centerCol) == 0) {
        return true;
      }
    }
    break;
  }

  return false;
}

// Reset block to starting position
void resetBlock() {
  block.x = (-PLATFORM_COLS / 2.0f + START_COL + 0.5f) * TILE_SIZE;
  block.y = 1.0f;
  block.z = (-PLATFORM_ROWS / 2.0f + START_ROW + 0.5f) * TILE_SIZE;
  block.orientation = STANDING;
  block.isAnimating = false;
  block.animationProgress = 0.0f;
  block.isFalling = false;
  block.fallVelocity = 0.0f;
}

// Initialize toggle tiles to be hidden at start
void initToggleTiles() {
  // Hide toggle tiles for group 1 (columns 4-5)
  for (int i = 0; i < 2; i++) {
    platformLayout[TOGGLE_TILES_1[i][0]][TOGGLE_TILES_1[i][1]] = 0;
  }
  // Hide toggle tiles for group 2 (columns 10-11)
  for (int i = 0; i < 2; i++) {
    platformLayout[TOGGLE_TILES_2[i][0]][TOGGLE_TILES_2[i][1]] = 0;
  }
  toggleGroup1Visible = false;
  toggleGroup2Visible = false;
}

// Check if block is on a toggle action tile and toggle the corresponding tiles
void checkToggleTiles() {
  int centerCol = worldToGridCol(block.x);
  int centerRow = worldToGridRow(block.z);

  // Get all tiles the block occupies
  int occupiedRow1 = centerRow, occupiedCol1 = centerCol;
  int occupiedRow2 = -1,
      occupiedCol2 = -1; // -1 means not occupied (standing block)

  if (block.orientation == LYING_X) {
    occupiedCol1 = worldToGridCol(block.x - 0.5f * TILE_SIZE);
    occupiedCol2 = worldToGridCol(block.x + 0.5f * TILE_SIZE);
    occupiedRow2 = centerRow;
  } else if (block.orientation == LYING_Z) {
    occupiedRow1 = worldToGridRow(block.z - 0.5f * TILE_SIZE);
    occupiedRow2 = worldToGridRow(block.z + 0.5f * TILE_SIZE);
    occupiedCol2 = centerCol;
  }

  // Check if any occupied tile is a toggle action tile (5)
  // Check group 1 action tile
  bool onGroup1Action =
      (occupiedRow1 == TOGGLE_GROUP_1_ACTION_ROW &&
       occupiedCol1 == TOGGLE_GROUP_1_ACTION_COL) ||
      (occupiedRow2 >= 0 && occupiedRow2 == TOGGLE_GROUP_1_ACTION_ROW &&
       occupiedCol2 == TOGGLE_GROUP_1_ACTION_COL);

  // Check group 2 action tile
  bool onGroup2Action =
      (occupiedRow1 == TOGGLE_GROUP_2_ACTION_ROW &&
       occupiedCol1 == TOGGLE_GROUP_2_ACTION_COL) ||
      (occupiedRow2 >= 0 && occupiedRow2 == TOGGLE_GROUP_2_ACTION_ROW &&
       occupiedCol2 == TOGGLE_GROUP_2_ACTION_COL);

  // Toggle group 1
  if (onGroup1Action) {
    toggleGroup1Visible = !toggleGroup1Visible;
    for (int i = 0; i < 2; i++) {
      platformLayout[TOGGLE_TILES_1[i][0]][TOGGLE_TILES_1[i][1]] =
          toggleGroup1Visible ? 4 : 0;
    }
  }

  // Toggle group 2
  if (onGroup2Action) {
    toggleGroup2Visible = !toggleGroup2Visible;
    for (int i = 0; i < 2; i++) {
      platformLayout[TOGGLE_TILES_2[i][0]][TOGGLE_TILES_2[i][1]] =
          toggleGroup2Visible ? 4 : 0;
    }
  }
}

// Find starting position from tile 9 in level data and convert it to normal tile
void findStartPosition() {
  for (int i = 0; i < PLATFORM_ROWS; i++) {
    for (int j = 0; j < PLATFORM_COLS; j++) {
      if (platformLayout[i][j] == 9) {
        START_ROW = i;
        START_COL = j;
        // Convert starting tile to normal tile
        platformLayout[i][j] = 1;
        return;
      }
    }
  }
}