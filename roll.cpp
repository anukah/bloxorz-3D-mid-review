#include <GLUT/glut.h>
#include <cmath>
#include <vector>
#include <iostream>

// Cube dimensions
const float CUBE_WIDTH = 2.0f;
const float CUBE_HEIGHT = 1.0f;
const float CUBE_DEPTH = 1.0f;

// Ground plane
const float GROUND_SIZE = 20.0f;
const float GROUND_Y = -CUBE_HEIGHT/2.0f;

// Cube state
struct CubeState {
    float x, y, z;          // Position
    float rotX, rotY, rotZ; // Rotation angles (degrees)
    float startX, startZ;   // Position at the start of a roll
    bool rolling;           // Is currently rolling
    int rollDirection;      // 1 = forward, -1 = backward, 2 = right, -2 = left
} cube;

// Animation parameters
const float ROLL_SPEED = 2.5f; // Slightly faster for better feel
const float ROLL_ANGLE_STEP = 90.0f; // Degrees per roll step
float currentRollProgress = 0.0f;

// Camera parameters
float cameraAngleX = 20.0f;
float cameraAngleY = 45.0f;
float cameraDistance = 15.0f;

void initializeCube() {
    cube.x = 0.0f;
    cube.y = GROUND_Y + CUBE_HEIGHT/2.0f;
    cube.z = 0.0f;
    cube.rotX = 0.0f;
    cube.rotY = 0.0f;
    cube.rotZ = 0.0f;
    cube.rolling = false;
    cube.rollDirection = 0;
    currentRollProgress = 0.0f;
}

void drawGround() {
    glColor3f(0.3f, 0.7f, 0.3f); // Green ground
    glBegin(GL_QUADS);
        glNormal3f(0.0f, 1.0f, 0.0f);
        glVertex3f(-GROUND_SIZE, GROUND_Y, -GROUND_SIZE);
        glVertex3f(GROUND_SIZE, GROUND_Y, -GROUND_SIZE);
        glVertex3f(GROUND_SIZE, GROUND_Y, GROUND_SIZE);
        glVertex3f(-GROUND_SIZE, GROUND_Y, GROUND_SIZE);
    glEnd();

    // Draw grid lines
    glColor3f(0.2f, 0.5f, 0.2f);
    glBegin(GL_LINES);
    for(int i = -10; i <= 10; i++) {
        glVertex3f(i*2.0f, GROUND_Y + 0.01f, -GROUND_SIZE);
        glVertex3f(i*2.0f, GROUND_Y + 0.01f, GROUND_SIZE);
        glVertex3f(-GROUND_SIZE, GROUND_Y + 0.01f, i*2.0f);
        glVertex3f(GROUND_SIZE, GROUND_Y + 0.01f, i*2.0f);
    }
    glEnd();
}

void drawCube() {
    glPushMatrix();

    if (cube.rolling) {
        // --- Animation Logic: Pivot around the correct edge ---
        float t = currentRollProgress;
        float smoothT = t * t * (3.0f - 2.0f * t); // Smooth step for easing
        float currentAngle = smoothT * (cube.rollDirection > 0 ? -ROLL_ANGLE_STEP : ROLL_ANGLE_STEP);

        // Translate to the starting position of the roll
        glTranslatef(cube.startX, cube.y, cube.startZ);

        // 2. Move pivot point to origin, rotate, then move back
        switch(cube.rollDirection) {
            case 1: // Forward
                glTranslatef(0, -CUBE_HEIGHT/2.0f, CUBE_DEPTH/2.0f);
                glRotatef(currentAngle, 1.0f, 0.0f, 0.0f);
                glTranslatef(0, CUBE_HEIGHT/2.0f, -CUBE_DEPTH/2.0f);
                break;
            case -1: // Backward
                glTranslatef(0, -CUBE_HEIGHT/2.0f, -CUBE_DEPTH/2.0f);
                glRotatef(currentAngle, 1.0f, 0.0f, 0.0f);
                glTranslatef(0, CUBE_HEIGHT/2.0f, CUBE_DEPTH/2.0f);
                break;
            case 2: // Right
                glTranslatef(CUBE_WIDTH/2.0f, -CUBE_HEIGHT/2.0f, 0);
                glRotatef(currentAngle, 0.0f, 0.0f, 1.0f);
                glTranslatef(-CUBE_WIDTH/2.0f, CUBE_HEIGHT/2.0f, 0);
                break;
            case -2: // Left
                glTranslatef(-CUBE_WIDTH/2.0f, -CUBE_HEIGHT/2.0f, 0);
                glRotatef(currentAngle, 0.0f, 0.0f, 1.0f);
                glTranslatef(CUBE_WIDTH/2.0f, CUBE_HEIGHT/2.0f, 0);
                break;
        }

    } else {
        // --- Static Drawing Logic ---
        glTranslatef(cube.x, cube.y, cube.z);
        glRotatef(cube.rotX, 1.0f, 0.0f, 0.0f);
        glRotatef(cube.rotY, 0.0f, 1.0f, 0.0f);
        glRotatef(cube.rotZ, 0.0f, 0.0f, 1.0f);
    }

    // Draw the 2x1x1 cube with different colors for each face
    float hw = CUBE_WIDTH / 2.0f;
    float hh = CUBE_HEIGHT / 2.0f;
    float hd = CUBE_DEPTH / 2.0f;

    // Front face (red)
    glColor3f(1.0f, 0.2f, 0.2f);
    glBegin(GL_QUADS);
        glNormal3f(0.0f, 0.0f, 1.0f);
        glVertex3f(-hw, -hh, hd);
        glVertex3f(hw, -hh, hd);
        glVertex3f(hw, hh, hd);
        glVertex3f(-hw, hh, hd);
    glEnd();

    // Back face (blue)
    glColor3f(0.2f, 0.2f, 1.0f);
    glBegin(GL_QUADS);
        glNormal3f(0.0f, 0.0f, -1.0f);
        glVertex3f(-hw, -hh, -hd);
        glVertex3f(-hw, hh, -hd);
        glVertex3f(hw, hh, -hd);
        glVertex3f(hw, -hh, -hd);
    glEnd();

    // Top face (yellow)
    glColor3f(1.0f, 1.0f, 0.2f);
    glBegin(GL_QUADS);
        glNormal3f(0.0f, 1.0f, 0.0f);
        glVertex3f(-hw, hh, -hd);
        glVertex3f(-hw, hh, hd);
        glVertex3f(hw, hh, hd);
        glVertex3f(hw, hh, -hd);
    glEnd();

    // Bottom face (magenta)
    glColor3f(1.0f, 0.2f, 1.0f);
    glBegin(GL_QUADS);
        glNormal3f(0.0f, -1.0f, 0.0f);
        glVertex3f(-hw, -hh, -hd);
        glVertex3f(hw, -hh, -hd);
        glVertex3f(hw, -hh, hd);
        glVertex3f(-hw, -hh, hd);
    glEnd();

    // Left face (cyan)
    glColor3f(0.2f, 1.0f, 1.0f);
    glBegin(GL_QUADS);
        glNormal3f(-1.0f, 0.0f, 0.0f);
        glVertex3f(-hw, -hh, -hd);
        glVertex3f(-hw, -hh, hd);
        glVertex3f(-hw, hh, hd);
        glVertex3f(-hw, hh, -hd);
    glEnd();

    // Right face (orange)
    glColor3f(1.0f, 0.5f, 0.2f);
    glBegin(GL_QUADS);
        glNormal3f(1.0f, 0.0f, 0.0f);
        glVertex3f(hw, -hh, -hd);
        glVertex3f(hw, hh, -hd);
        glVertex3f(hw, hh, hd);
        glVertex3f(hw, -hh, hd);
    glEnd();

    // Draw wireframe outline
    glColor3f(0.0f, 0.0f, 0.0f);
    glLineWidth(2.0f);
    glBegin(GL_LINES);
        // Bottom edges
        glVertex3f(-hw, -hh, -hd); glVertex3f(hw, -hh, -hd);
        glVertex3f(hw, -hh, -hd); glVertex3f(hw, -hh, hd);
        glVertex3f(hw, -hh, hd); glVertex3f(-hw, -hh, hd);
        glVertex3f(-hw, -hh, hd); glVertex3f(-hw, -hh, -hd);

        // Top edges
        glVertex3f(-hw, hh, -hd); glVertex3f(hw, hh, -hd);
        glVertex3f(hw, hh, -hd); glVertex3f(hw, hh, hd);
        glVertex3f(hw, hh, hd); glVertex3f(-hw, hh, hd);
        glVertex3f(-hw, hh, hd); glVertex3f(-hw, hh, -hd);

        // Vertical edges
        glVertex3f(-hw, -hh, -hd); glVertex3f(-hw, hh, -hd);
        glVertex3f(hw, -hh, -hd); glVertex3f(hw, hh, -hd);
        glVertex3f(hw, -hh, hd); glVertex3f(hw, hh, hd);
        glVertex3f(-hw, -hh, hd); glVertex3f(-hw, hh, hd);
    glEnd();

    glPopMatrix();
}

void startRolling(int direction) {
    if (cube.rolling) return; // Already rolling

    cube.rolling = true;
    cube.rollDirection = direction;
    currentRollProgress = 0.0f;

    // Store the starting position for the animation
    cube.startX = cube.x;
    cube.startZ = cube.z;
}

void updateRolling(float deltaTime) {
    if (!cube.rolling) return;

    currentRollProgress += ROLL_SPEED * deltaTime;

    if (currentRollProgress >= 1.0f) {
        // --- Rolling complete: Snap to final position and rotation ---
        cube.rolling = false;
        currentRollProgress = 0.0f;

        // Calculate final position based on which way it rolled
        float distanceMoved;
        switch (abs(cube.rollDirection)) {
             case 1: // Forward/Backward roll
                distanceMoved = (CUBE_DEPTH + CUBE_HEIGHT) / 2.0f;
                break;
            case 2: // Left/Right roll
                distanceMoved = (CUBE_WIDTH + CUBE_HEIGHT) / 2.0f;
                break;
        }

        switch (cube.rollDirection) {
            case 1:  // Forward
                cube.z += distanceMoved;
                cube.rotX -= ROLL_ANGLE_STEP;
                break;
            case -1: // Backward
                cube.z -= distanceMoved;
                cube.rotX += ROLL_ANGLE_STEP;
                break;
            case 2:  // Right
                cube.x += distanceMoved;
                cube.rotZ -= ROLL_ANGLE_STEP;
                break;
            case -2: // Left
                cube.x -= distanceMoved;
                cube.rotZ += ROLL_ANGLE_STEP;
                break;
        }
    }
    // NOTE: No "else" block. The interpolation is now handled entirely
    // by the matrix transformations in drawCube().
}


void display() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLoadIdentity();

    // Set up camera
    float camX = cube.x + cameraDistance * cos(cameraAngleY * M_PI / 180.0f) * sin(cameraAngleX * M_PI / 180.0f);
    float camY = cube.y + cameraDistance * cos(cameraAngleX * M_PI / 180.0f);
    float camZ = cube.z + cameraDistance * sin(cameraAngleY * M_PI / 180.0f) * sin(cameraAngleX * M_PI / 180.0f);

    gluLookAt(camX, camY, camZ, cube.x, cube.y, cube.z, 0.0f, 1.0f, 0.0f);

    // Draw scene
    drawGround();
    drawCube();

    glutSwapBuffers();
}

void reshape(int width, int height) {
    glViewport(0, 0, width, height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(45.0, (double)width / height, 0.1, 100.0);
    glMatrixMode(GL_MODELVIEW);
}

void keyboard(unsigned char key, int x, int y) {
    switch (key) {
        case 'w': case 'W':
            startRolling(1);  // Forward
            break;
        case 's': case 'S':
            startRolling(-1); // Backward
            break;
        case 'd': case 'D':
            startRolling(2);  // Right
            break;
        case 'a': case 'A':
            startRolling(-2); // Left
            break;
        case 'r': case 'R':
            initializeCube(); // Reset cube position
            break;
        case 27: // Escape
            exit(0);
            break;
    }
}

void specialKeys(int key, int x, int y) {
    switch (key) {
        case GLUT_KEY_UP:
            cameraAngleX += 5.0f;
            if (cameraAngleX > 175.0f) cameraAngleX = 175.0f;
            break;
        case GLUT_KEY_DOWN:
            cameraAngleX -= 5.0f;
            if (cameraAngleX < 5.0f) cameraAngleX = 5.0f;
            break;
        case GLUT_KEY_LEFT:
            cameraAngleY -= 5.0f;
            break;
        case GLUT_KEY_RIGHT:
            cameraAngleY += 5.0f;
            break;
        case GLUT_KEY_PAGE_UP:
            cameraDistance -= 1.0f;
            if (cameraDistance < 5.0f) cameraDistance = 5.0f;
            break;
        case GLUT_KEY_PAGE_DOWN:
            cameraDistance += 1.0f;
            if (cameraDistance > 50.0f) cameraDistance = 50.0f;
            break;
    }
}

void timer(int value) {
    static int lastTime = 0;
    int currentTime = glutGet(GLUT_ELAPSED_TIME);
    if (lastTime == 0) { // First frame initialization
        lastTime = currentTime;
    }
    float deltaTime = (currentTime - lastTime) / 1000.0f;
    lastTime = currentTime;

    updateRolling(deltaTime);

    glutPostRedisplay();
    glutTimerFunc(16, timer, 0); // ~60 FPS
}

void init() {
    glClearColor(0.5f, 0.8f, 1.0f, 1.0f); // Sky blue background
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    // Set up lighting
    GLfloat light_position[] = {10.0f, 10.0f, 10.0f, 1.0f};
    GLfloat light_ambient[] = {0.3f, 0.3f, 0.3f, 1.0f};
    GLfloat light_diffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};

    glLightfv(GL_LIGHT0, GL_POSITION, light_position);
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);

    initializeCube();
}

void printControls() {
    std::cout << "=== 2x1x1 Cube Rolling Test Controls ===" << std::endl;
    std::cout << "Movement:" << std::endl;
    std::cout << "  W - Roll forward" << std::endl;
    std::cout << "  S - Roll backward" << std::endl;
    std::cout << "  A - Roll left" << std::endl;
    std::cout << "  D - Roll right" << std::endl;
    std::cout << "  R - Reset cube position" << std::endl;
    std::cout << std::endl;
    std::cout << "Camera:" << std::endl;
    std::cout << "  Arrow Keys - Rotate camera" << std::endl;
    std::cout << "  Page Up/Down - Zoom in/out" << std::endl;
    std::cout << std::endl;
    std::cout << "  ESC - Exit" << std::endl;
    std::cout << "================================" << std::endl;
}

int main(int argc, char** argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutInitWindowPosition(100, 100);
    glutCreateWindow("2x1x1 Cube Rolling Test (Fixed)");

    init();
    printControls();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);
    glutTimerFunc(0, timer, 0);

    glutMainLoop();
    return 0;
}