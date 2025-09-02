#define GL_SILENCE_DEPRECATION // Ignore deprecation errors
#include <GLUT/glut.h>
#include <cmath>
#include <vector>
#include "levels.h" // Import levels header

const float PI = 3.14159f;
const float CAMERA_SMOOTH_FACTOR = 0.05f; // How quickly the camera moves
const int TIMER_INTERVAL_MS = 16;       // 60 FPS

// a vector of vectors from a 2D C-style array.
const std::vector<std::vector<int> > platformLayout = getLevelLayout(1);

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
}

void update() {
    cameraAngleX += (targetCameraAngleX - cameraAngleX) * CAMERA_SMOOTH_FACTOR;
    cameraAngleY += (targetCameraAngleY - cameraAngleY) * CAMERA_SMOOTH_FACTOR;
    cameraDistance += (targetCameraDistance - cameraDistance) * CAMERA_SMOOTH_FACTOR;
}

// Display
void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glLoadIdentity();

    applyCameraTransform();

    drawPlatform(); // Draw Platform

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
    glutSolidCube(TILE_SIZE);
}

// Draw border of cube
void drawCubeBorders() {
    glutWireCube(TILE_SIZE + 0.001f);
}

// Draw platform
void drawPlatform() {
    
    float offsetX = -PLATFORM_COLS * TILE_SIZE / 2.0f;
    float offsetZ = -PLATFORM_ROWS * TILE_SIZE / 2.0f;
    
    for (int i = 0; i < PLATFORM_ROWS; ++i) {
        for (int j = 0; j < PLATFORM_COLS; ++j) {
            // Only draw if the tile is not an empty space (0)
            if (platformLayout[i][j] != 0) {
                glPushMatrix();
                glTranslatef(offsetX + j * TILE_SIZE, -TILE_SIZE / 2.0f, offsetZ + i * TILE_SIZE);

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