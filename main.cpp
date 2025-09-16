#define GL_SILENCE_DEPRECATION // Ignore deprecation errors
#include <GLUT/glut.h>
#include <cmath>
#include <vector>
#include "levels.h" // Import levels header
#include "SOIL2/SOIL2.h"

struct Vec3 { float x, y, z; };

GLuint glassTextureID;

const float PI = 3.14159f;
const float CAMERA_SMOOTH_FACTOR = 0.05f; // How quickly the camera moves
const int TIMER_INTERVAL_MS = 16;       // 60 FPS
const float BLOCK_ANIMATION_SPEED = 0.08f;

// a vector of vectors from a 2D C-style array.
const std::vector<std::vector<int> > platformLayout = getLevelLayout(1); // CHANGE LEVEL

const int PLATFORM_ROWS = platformLayout.size();
const int PLATFORM_COLS = platformLayout[0].size();
const float TILE_SIZE = 1.0f;

// Camera State
float cameraAngleX = 30.0f;
float cameraAngleY = -45.0f;
float cameraDistance = 15.0f;

// Target values for smooth interpolation
float targetCameraAngleX = 30.0f;
float targetCameraAngleY = -45.0f;
float targetCameraDistance = 15.0f;

// Define the block's starting position in grid coordinates
const int START_ROW = 1;
const int START_COL = 1; // The second tile in the first row

// Block State
enum BlockOrientation {
    STANDING,
    LYING_X,
    LYING_Z
};

struct Block {
    // Current state
    float x, y, z;
    BlockOrientation orientation;

    // Animation state
    bool isAnimating;
    float animationProgress; // 0.0 to 1.0
    Vec3 startPos, targetPos;
    Vec3 pivotPoint;         // Pivot point for natural rolling
    float startRotZ, targetRotZ; // For rolling left/right
    float startRotX, targetRotX; // For rolling forward/backward
};

// Initial block state: Standing at grid position (1,1)
Block block = {
    (-PLATFORM_COLS / 2.0f + START_COL + 0.5f) * TILE_SIZE, // x
    1.0f,                                                  // y
    (-PLATFORM_ROWS / 2.0f + START_ROW + 0.5f) * TILE_SIZE, // z
    STANDING,                                              // orientation
    false,                                                 // isAnimating
    0.0f,                                                  // animationProgress
    {0,0,0}, {0,0,0},                                      // startPos, targetPos
    {0,0,0},                                               // pivotPoint
    0.0f, 0.0f,                                            // startRotZ, targetRotZ
    0.0f, 0.0f                                             // startRotX, targetRotX
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

// Main
int main(int argc, char** argv) {
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
        "textures/glass.jpg",
        SOIL_LOAD_AUTO,
        SOIL_CREATE_NEW_ID,
        SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y | SOIL_FLAG_TEXTURE_REPEATS
    );

    // Check for errors
    if (glassTextureID == 0) {
        printf("SOIL loading error: '%s'\n", SOIL_last_result());
    }
}

void update() {
    // Smooth camera movement
    cameraAngleX += (targetCameraAngleX - cameraAngleX) * CAMERA_SMOOTH_FACTOR;
    cameraAngleY += (targetCameraAngleY - cameraAngleY) * CAMERA_SMOOTH_FACTOR;
    cameraDistance += (targetCameraDistance - cameraDistance) * CAMERA_SMOOTH_FACTOR;

    if (block.isAnimating) {
        block.animationProgress += BLOCK_ANIMATION_SPEED;
        if (block.animationProgress >= 1.0f) {
            // Animation finished, snap to final state
            block.isAnimating = false;
            block.animationProgress = 1.0f;
            block.x = block.targetPos.x;
            block.y = block.targetPos.y;
            block.z = block.targetPos.z;
        }
    }
}

// Display
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    applyCameraTransform();

    drawPlatform(); // Draw Platform
    drawBlock(); // Draw Block
    glutSwapBuffers();
}

// Reshape
void reshape(int w, int h) {
    if (h == 0) h = 1;
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
        case 'w': case 'W': targetCameraDistance -= 0.5f; break;
        case 's': case 'S': targetCameraDistance += 0.5f; break;
        // Orbit
        case 'a': case 'A': targetCameraAngleY -= 5.0f; break;
        case 'd': case 'D': targetCameraAngleY += 5.0f; break;
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
        case 27: exit(0); break;
    }
}

void specialKeys(int key, int x, int y) {
    // Ignore new input if an animation is already playing
    if (block.isAnimating) {
        return;
    }

    switch (key) {
        case GLUT_KEY_LEFT:  moveBlock(-1, 0); break;
        case GLUT_KEY_RIGHT: moveBlock(1, 0); break;
        // Todo
        // case GLUT_KEY_UP:    moveBlock(0, -1); break;
        // case GLUT_KEY_DOWN:  moveBlock(0, 1); break;
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
    if (targetCameraAngleX > 89.0f) targetCameraAngleX = 89.0f;
    if (targetCameraAngleX < 5.0f) targetCameraAngleX = 5.0f;
    if (targetCameraDistance < 5.0f) targetCameraDistance = 5.0f;
    if (targetCameraDistance > 50.0f) targetCameraDistance = 50.0f;

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
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5f, -0.5f,  0.5f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f( 0.5f, -0.5f,  0.5f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f( 0.5f,  0.5f,  0.5f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.5f,  0.5f,  0.5f);

    // Back Face
    glNormal3f(0.0, 0.0, -1.0);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-0.5f, -0.5f, -0.5f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-0.5f,  0.5f, -0.5f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f( 0.5f,  0.5f, -0.5f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f( 0.5f, -0.5f, -0.5f);

    // Top Face
    glNormal3f(0.0, 1.0, 0.0);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.5f,  0.5f, -0.5f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5f,  0.5f,  0.5f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f( 0.5f,  0.5f,  0.5f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f( 0.5f,  0.5f, -0.5f);

    // Bottom Face
    glNormal3f(0.0, -1.0, 0.0);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-0.5f, -0.5f, -0.5f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f( 0.5f, -0.5f, -0.5f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f( 0.5f, -0.5f,  0.5f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-0.5f, -0.5f,  0.5f);

    // Right Face
    glNormal3f(1.0, 0.0, 0.0);
    glTexCoord2f(1.0f, 0.0f); glVertex3f( 0.5f, -0.5f, -0.5f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f( 0.5f,  0.5f, -0.5f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f( 0.5f,  0.5f,  0.5f);
    glTexCoord2f(0.0f, 0.0f); glVertex3f( 0.5f, -0.5f,  0.5f);

    // Left Face
    glNormal3f(-1.0, 0.0, 0.0);
    glTexCoord2f(0.0f, 0.0f); glVertex3f(-0.5f, -0.5f, -0.5f);
    glTexCoord2f(1.0f, 0.0f); glVertex3f(-0.5f, -0.5f,  0.5f);
    glTexCoord2f(1.0f, 1.0f); glVertex3f(-0.5f,  0.5f,  0.5f);
    glTexCoord2f(0.0f, 1.0f); glVertex3f(-0.5f,  0.5f, -0.5f);

    glEnd();
}

// Draw border
void drawCubeBorders() {
    glutWireCube(TILE_SIZE + 0.001f);
}

// Draw platform
void drawPlatform() {
    GLfloat plat_specular[] = {0.0f, 0.0f, 0.0f, 1.0f}; // No highlight
    GLfloat plat_shininess[] = {0.0f};                     // No shininess
    
    glMaterialfv(GL_FRONT, GL_SPECULAR, plat_specular);
    glMaterialfv(GL_FRONT, GL_SHININESS, plat_shininess);
    
    float offsetX = -PLATFORM_COLS * TILE_SIZE / 2.0f;
    float offsetZ = -PLATFORM_ROWS * TILE_SIZE / 2.0f;
    
    for (int i = 0; i < PLATFORM_ROWS; ++i) {
        for (int j = 0; j < PLATFORM_COLS; ++j) {
            // Only draw if the tile is not an empty space (0)
            if (platformLayout[i][j] != 0) {
                glPushMatrix();
                glTranslatef(offsetX + (j + 0.5f) * TILE_SIZE, -TILE_SIZE / 2.0f, offsetZ + (i + 0.5f) * TILE_SIZE);

                // Set material properties for the tile
                if (platformLayout[i][j] == 1) { 
                    // Regular tile
                    GLfloat mat_diffuse[] = {0.2f, 0.2f, 0.25f, 0.7f};
                    glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
                } else { 
                    // Target tile
                    GLfloat mat_diffuse[] = {0.0f, 0.8f, 0.8f, 0.6f};
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
    float currentX, currentY, currentZ;
    float currentRotZ = 0.0f;

    if (block.isAnimating) {
        float t = block.animationProgress;
        t = t * t * (3.0f - 2.0f * t); // Smooth step interpolation

        currentRotZ = block.startRotZ + (block.targetRotZ - block.startRotZ) * t;
        
        // Calculate the rotated position relative to pivot point
        float dx = block.startPos.x - block.pivotPoint.x;
        float dy = block.startPos.y - block.pivotPoint.y;
        
        // Apply Z-axis rotation around pivot (for left/right rolling)
        float cosZ = cos(currentRotZ * PI / 180.0f);
        float sinZ = sin(currentRotZ * PI / 180.0f);
        
        float rotatedX = dx * cosZ - dy * sinZ;
        float rotatedY = dx * sinZ + dy * cosZ;
        
        // Final position = pivot + rotated offset
        currentX = block.pivotPoint.x + rotatedX;
        currentY = block.pivotPoint.y + rotatedY;
        currentZ = block.startPos.z; // Z stays the same for X-axis movement
        
    } else {
        currentX = block.x;
        currentY = block.y;
        currentZ = block.z;
    }

    glPushMatrix();
    
    // Translate to current position
    glTranslatef(currentX, currentY, currentZ);
    
    // Apply Z-axis rotation for visual effect
    glRotatef(currentRotZ, 0.0f, 0.0f, 1.0f);

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
    switch (block.orientation) {
        case STANDING: glScalef(1.0f, 2.0f, 1.0f); break;
        case LYING_X:  glScalef(2.0f, 1.0f, 1.0f); break;
        case LYING_Z:  glScalef(1.0f, 1.0f, 2.0f); break;
    }
    
    drawCube();

    glPopMatrix();
  
    glDisable(GL_TEXTURE_2D);
    
    glPopMatrix();
}

void moveBlock(int dx, int dz) {
    if (dx == 0) return;

    // Store starting position and rotation
    block.startPos = {block.x, block.y, block.z};
    block.startRotZ = 0;

    // Calculate Target State with proper pivot points
    if (block.orientation == STANDING) {
        // Rolling from standing to lying on its side
        // Pivot around the bottom edge in direction of movement
        float pivotOffset = dx > 0 ? 0.5f * TILE_SIZE : -0.5f * TILE_SIZE;
        
        block.targetPos = {
            block.x + dx * 1.5f * TILE_SIZE,
            0.5f, // Center of a lying block is lower
            block.z
        };
        
        // Set pivot point at the bottom edge we're rolling over
        block.pivotPoint = {
            block.x + pivotOffset,  // Bottom edge in movement direction
            0.0f,                   // Ground level
            block.z
        };
        
        block.targetRotZ = -dx * 90.0f; // Roll 90 degrees on Z axis
        block.orientation = LYING_X; // This will be its state *after* the animation
        
    } else if (block.orientation == LYING_X) {
        // Rolling from lying on its side to standing up
        // Pivot around the bottom edge in direction of movement
        float pivotOffset = dx > 0 ? TILE_SIZE : -TILE_SIZE;
        
        block.targetPos = {
            block.x + dx * 1.5f * TILE_SIZE,
            1.0f, // Center of a standing block is higher
            block.z
        };
        
        // Set pivot point at the bottom edge we're rolling over
        block.pivotPoint = {
            block.x + pivotOffset,  // Bottom edge in movement direction
            0.0f,                   // Ground level
            block.z
        };
        
        block.targetRotZ = -dx * 90.0f;
        block.orientation = STANDING;
    }

    block.isAnimating = true;
    block.animationProgress = 0.0f;
}