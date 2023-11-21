#include <GL/gl.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <GL/glut.h>
#include <float.h>
#include "src/Vec3.h"
#include "src/Camera.h"
#include <random>
#include <unistd.h>
using namespace std;

struct Voxel {
    Vec3 center;
    double side;
};

void getVoxelPoints(Voxel v, Vec3* voxelPoints) {
    voxelPoints[0] = Vec3(v.center[0] - 0.5*v.side,v.center[1] - 0.5*v.side,v.center[2] - 0.5*v.side); // En bas à gauche au premier plan
    voxelPoints[1] = Vec3(v.center[0] + 0.5*v.side,v.center[1] - 0.5*v.side,v.center[2] - 0.5*v.side); // En bas à droite au premier plan
    voxelPoints[2] = Vec3(v.center[0] + 0.5*v.side,v.center[1] + 0.5*v.side,v.center[2] - 0.5*v.side); // En haut à droite au premier plan
    voxelPoints[3] = Vec3(v.center[0] - 0.5*v.side,v.center[1] + 0.5*v.side,v.center[2] - 0.5*v.side); // En haut à gauche au premier plan
    voxelPoints[4] = Vec3(v.center[0] - 0.5*v.side,v.center[1] - 0.5*v.side,v.center[2] + 0.5*v.side); // En bas à gauche au fond
    voxelPoints[5] = Vec3(v.center[0] + 0.5*v.side,v.center[1] - 0.5*v.side,v.center[2] + 0.5*v.side); // En bas à droite au fond
    voxelPoints[6] = Vec3(v.center[0] + 0.5*v.side,v.center[1] + 0.5*v.side,v.center[2] + 0.5*v.side); // En haut à droite au fond
    voxelPoints[7] = Vec3(v.center[0] - 0.5*v.side,v.center[1] + 0.5*v.side,v.center[2] + 0.5*v.side); // En haut à gauche au fond
}

// -------------------------------------------
// OpenGL/GLUT application code.
// -------------------------------------------

static GLint window;
static unsigned int SCREENWIDTH = 1600;
static unsigned int SCREENHEIGHT = 900;
static Camera camera;
static bool mouseRotatePressed = false;
static bool mouseMovePressed = false;
static bool mouseZoomPressed = false;
static int lastX=0, lastY=0, lastZoom=0;
static bool fullScreen = false;

void initLight () {
    GLfloat light_position1[4] = {22.0f, 16.0f, 50.0f, 0.0f};
    GLfloat direction1[3] = {-52.0f,-16.0f,-50.0f};
    GLfloat color1[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    GLfloat ambient[4] = {0.3f, 0.3f, 0.3f, 0.5f};

    glLightfv (GL_LIGHT1, GL_POSITION, light_position1);
    glLightfv (GL_LIGHT1, GL_SPOT_DIRECTION, direction1);
    glLightfv (GL_LIGHT1, GL_DIFFUSE, color1);
    glLightfv (GL_LIGHT1, GL_SPECULAR, color1);
    glLightModelfv (GL_LIGHT_MODEL_AMBIENT, ambient);
    glEnable (GL_LIGHT1);
    glEnable (GL_LIGHTING);
}

void init () {
    camera.resize (SCREENWIDTH, SCREENHEIGHT);
    initLight ();
    // glCullFace (GL_BACK);
    // glEnable (GL_CULL_FACE);
    glDepthFunc (GL_LESS);
    glEnable (GL_DEPTH_TEST);
    glClearColor (0.2f, 0.2f, 0.3f, 1.0f);
    glEnable(GL_COLOR_MATERIAL);
}

void drawVoxel(Vec3 center, double side) {
    Voxel v;
    v.center = center;
    v.side = side;
    glPointSize(10);
    //glBegin(GL_POINTS);
    //glColor3f(1,0,0);
    //glVertex3f(v.center[0],v.center[1],v.center[2]);
    Vec3 voxelPoints[8];
    getVoxelPoints(v,voxelPoints);
    // for(int i = 0; i < 8; i++) {
    //     glVertex3f(voxelPoints[i][0],voxelPoints[i][1],voxelPoints[i][2]);
    // }
    //glEnd();

    glBegin(GL_TRIANGLES);
    glColor3f(0.5,0.5,0.5);

    // Face avant
    glNormal3f(0.0f, 0.0f, -1.0f); // Normale de la face avant
    glVertex3f(voxelPoints[0][0], voxelPoints[0][1], voxelPoints[0][2]);
    glVertex3f(voxelPoints[2][0], voxelPoints[2][1], voxelPoints[2][2]);
    glVertex3f(voxelPoints[1][0], voxelPoints[1][1], voxelPoints[1][2]);

    glVertex3f(voxelPoints[0][0], voxelPoints[0][1], voxelPoints[0][2]);
    glVertex3f(voxelPoints[3][0], voxelPoints[3][1], voxelPoints[3][2]);
    glVertex3f(voxelPoints[2][0], voxelPoints[2][1], voxelPoints[2][2]);

    // Face arrière
    glNormal3f(0.0f, 0.0f, 1.0f); // Normale de la face arrière
    glVertex3f(voxelPoints[4][0], voxelPoints[4][1], voxelPoints[4][2]);
    glVertex3f(voxelPoints[6][0], voxelPoints[6][1], voxelPoints[6][2]);
    glVertex3f(voxelPoints[5][0], voxelPoints[5][1], voxelPoints[5][2]);

    glVertex3f(voxelPoints[4][0], voxelPoints[4][1], voxelPoints[4][2]);
    glVertex3f(voxelPoints[7][0], voxelPoints[7][1], voxelPoints[7][2]);
    glVertex3f(voxelPoints[6][0], voxelPoints[6][1], voxelPoints[6][2]);

    // Face droite
    glNormal3f(1.0f, 0.0f, 0.0f); // Normale de la face droite
    glVertex3f(voxelPoints[1][0], voxelPoints[1][1], voxelPoints[1][2]);
    glVertex3f(voxelPoints[6][0], voxelPoints[6][1], voxelPoints[6][2]);
    glVertex3f(voxelPoints[5][0], voxelPoints[5][1], voxelPoints[5][2]);

    glVertex3f(voxelPoints[1][0], voxelPoints[1][1], voxelPoints[1][2]);
    glVertex3f(voxelPoints[2][0], voxelPoints[2][1], voxelPoints[2][2]);
    glVertex3f(voxelPoints[6][0], voxelPoints[6][1], voxelPoints[6][2]);

    // Face gauche
    glNormal3f(-1.0f, 0.0f, 0.0f); // Normale de la face gauche
    glVertex3f(voxelPoints[0][0], voxelPoints[0][1], voxelPoints[0][2]);
    glVertex3f(voxelPoints[7][0], voxelPoints[7][1], voxelPoints[7][2]);
    glVertex3f(voxelPoints[3][0], voxelPoints[3][1], voxelPoints[3][2]);

    glVertex3f(voxelPoints[0][0], voxelPoints[0][1], voxelPoints[0][2]);
    glVertex3f(voxelPoints[4][0], voxelPoints[4][1], voxelPoints[4][2]);
    glVertex3f(voxelPoints[7][0], voxelPoints[7][1], voxelPoints[7][2]);

    // Face supérieure
    glNormal3f(0.0f, 1.0f, 0.0f); // Normale de la face supérieure
    glVertex3f(voxelPoints[3][0], voxelPoints[3][1], voxelPoints[3][2]);
    glVertex3f(voxelPoints[6][0], voxelPoints[6][1], voxelPoints[6][2]);
    glVertex3f(voxelPoints[2][0], voxelPoints[2][1], voxelPoints[2][2]);

    glVertex3f(voxelPoints[3][0], voxelPoints[3][1], voxelPoints[3][2]);
    glVertex3f(voxelPoints[7][0], voxelPoints[7][1], voxelPoints[7][2]);
    glVertex3f(voxelPoints[6][0], voxelPoints[6][1], voxelPoints[6][2]);

    // Face inférieure
    glNormal3f(0.0f, -1.0f, 0.0f); // Normale de la face inférieure
    glVertex3f(voxelPoints[0][0], voxelPoints[0][1], voxelPoints[0][2]);
    glVertex3f(voxelPoints[5][0], voxelPoints[5][1], voxelPoints[5][2]);
    glVertex3f(voxelPoints[1][0], voxelPoints[1][1], voxelPoints[1][2]);

    glVertex3f(voxelPoints[0][0], voxelPoints[0][1], voxelPoints[0][2]);
    glVertex3f(voxelPoints[4][0], voxelPoints[4][1], voxelPoints[4][2]);
    glVertex3f(voxelPoints[5][0], voxelPoints[5][1], voxelPoints[5][2]);

    glEnd();
}

void drawSphereVolumic(Vec3 center, double rayon, double resolution) {
    Voxel v;
    v.side = 1/resolution;
    for(int i = 0; i < resolution; i++) {
        for(int j = 0; j < resolution; j++) {
            for(int k = 0; k < resolution; k++) {
                v.center = Vec3(i * v.side, j * v.side, k * v.side);
                if((center - v.center).length() < rayon) {
                    drawVoxel(v.center,v.side);
                }
                //drawVoxel(v.center,v.side);
            }
        }
    }
    
}

void draw() {
    drawSphereVolumic(Vec3(0,0,0),1,100);
}

void display () {
    glLoadIdentity ();
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    camera.apply ();
    draw();
    glFlush ();
    glutSwapBuffers();
}

void idle () {
    glutPostRedisplay ();
}

void key (unsigned char keyPressed, int x, int y) {
    switch (keyPressed) {
    case 'f':
        if (fullScreen == true) {
            glutReshapeWindow (SCREENWIDTH, SCREENHEIGHT);
            fullScreen = false;
        } else {
            glutFullScreen ();
            fullScreen = true;
        }
        break;


    case 'w':
        GLint polygonMode[2];
        glGetIntegerv(GL_POLYGON_MODE, polygonMode);
        if(polygonMode[0] != GL_FILL)
            glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
        else
            glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
        break;

    default:
        break;

    }
    idle();
}

void mouse (int button, int state, int x, int y) {
    if (state == GLUT_UP) {
        mouseMovePressed = false;
        mouseRotatePressed = false;
        mouseZoomPressed = false;
    } else {
        if (button == GLUT_LEFT_BUTTON) {
            camera.beginRotate (x, y);
            mouseMovePressed = false;
            mouseRotatePressed = true;
            mouseZoomPressed = false;
        } else if (button == GLUT_RIGHT_BUTTON) {
            lastX = x;
            lastY = y;
            mouseMovePressed = true;
            mouseRotatePressed = false;
            mouseZoomPressed = false;
        } else if (button == GLUT_MIDDLE_BUTTON) {
            if (mouseZoomPressed == false) {
                lastZoom = y;
                mouseMovePressed = false;
                mouseRotatePressed = false;
                mouseZoomPressed = true;
            }
        }
    }
    idle ();
}

void motion (int x, int y) {
    if (mouseRotatePressed == true) {
        camera.rotate (x, y);
    }
    else if (mouseMovePressed == true) {
        camera.move ((x-lastX)*2/static_cast<float>(SCREENWIDTH), (lastY-y)*2/static_cast<float>(SCREENHEIGHT), 0.0);
        lastX = x;
        lastY = y;
    }
    else if (mouseZoomPressed == true) {
        camera.zoom (float (y-lastZoom)*3/SCREENHEIGHT);
        lastZoom = y;
    }
}


void reshape(int w, int h) {
    camera.resize (w, h);
}

int main (int argc, char ** argv) {
    if (argc > 2) {
        exit (EXIT_FAILURE);
    }
    glutInit (&argc, argv);
    glutInitDisplayMode (GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
    glutInitWindowSize (SCREENWIDTH, SCREENHEIGHT);
    window = glutCreateWindow ("TP8 HAI714I");

    init ();
    glutIdleFunc (idle);

    glutDisplayFunc(display);
    glutKeyboardFunc (key);
    glutReshapeFunc (reshape);
    glutMotionFunc (motion);
    glutMouseFunc (mouse);
    key ('?', 0, 0);

    glutMainLoop ();
    return EXIT_SUCCESS;

}