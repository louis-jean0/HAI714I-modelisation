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
int currentScene = 0;

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
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    camera.resize (SCREENWIDTH, SCREENHEIGHT);
    initLight ();
    // glCullFace (GL_BACK);
    // glEnable (GL_CULL_FACE);
    glDepthFunc (GL_LESS);
    glEnable (GL_DEPTH_TEST);
    glClearColor (0.2f, 0.2f, 0.3f, 1.0f);
    glEnable(GL_COLOR_MATERIAL);
}

void drawAxes() {
    glBegin(GL_LINE_STRIP);
    glColor3f(1,0,0);
    glVertex3f(0,0,0);
    glVertex3f(1,0,0);
    glColor3f(0,1,0);
    glVertex3f(0,0,0);
    glVertex3f(0,1,0);
    glColor3f(0,0,1);
    glVertex3f(0,0,0);
    glVertex3f(0,0,1);
    glEnd();
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

    //glColor3f(0.5,0.5,0.5);
    glColor3f(abs(center[0]),abs(center[1]),abs(center[2]));

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

void drawGrid(Vec3 origin, double resolution) {
    Voxel v;
    v.side = 1/resolution;
    for(int i = 0; i < resolution; i++) {
        for(int j = 0; j < resolution; j++) {
            for(int k = 0; k < resolution; k++) {
                v.center = Vec3(i * v.side, j * v.side, k * v.side);
                drawVoxel(v.center,v.side);
            }
        }
    }
}

void drawSphereVolumic(Vec3 center, double rayon, double resolution) {
    Voxel v;
    v.side = 2 * rayon / resolution;
    for(int i = 0; i < resolution; i++) {
        for(int j = 0; j < resolution; j++) {
            for(int k = 0; k < resolution; k++) {
                v.center = Vec3(center[0] - rayon + i * v.side + v.side / 2,
                center[1] - rayon + j * v.side + v.side / 2,
                center[2] - rayon + k * v.side + v.side / 2);
                if((center - v.center).length() <= rayon) {
                    drawVoxel(v.center, v.side);
                }
            }
        }
    }
}

void drawCylinderVolumic(Vec3 axisOrigin, Vec3 axisVector, double rayon, double resolution) {
    Voxel v;
    v.side = 2 * rayon / resolution;
    //std::cout<<v.side<<std::endl;
    double cylinderHeight = axisVector.length();
    //std::cout<<cylinderHeight<<std::endl;

    for(int i = 0; i < resolution; i++) {
        for(int j = 0; j < resolution; j++) {
            for(int k = 0; k < resolution; k++) {
                v.center = Vec3(axisOrigin[0] - rayon + i * v.side + v.side / 2,
                                axisOrigin[1] - rayon + j * v.side + v.side / 2,
                                axisOrigin[2] + k * v.side + v.side / 2);
                double dx = v.center[0] - axisOrigin[0];
                double dy = v.center[1] - axisOrigin[1];
                if(dx * dx + dy * dy <= rayon * rayon) {
                    if(v.center[2] >= axisOrigin[2] && v.center[2] <= axisOrigin[2] + cylinderHeight) {
                        //std::cout<<v.center[2]<<std::endl;
                        drawVoxel(v.center,v.side);
                    }
                }
            }
        }
    }
}

void drawIntersectionSphereCylinder(Vec3 centreSphere, double rayonSphere, Vec3 axisOriginCylinder, Vec3 axisVectorCylinder, double rayonCylinder, double resolution) {
    Voxel v;
    v.side = 2 * std::max(rayonSphere, rayonCylinder) / resolution;
    double cylinderHeight = axisVectorCylinder.length();

    for(int i = 0; i < resolution; i++) {
        for(int j = 0; j < resolution; j++) {
            for(int k = 0; k < resolution; k++) {
                v.center = Vec3(axisOriginCylinder[0] - rayonCylinder + i * v.side + v.side / 2,
                                axisOriginCylinder[1] - rayonCylinder + j * v.side + v.side / 2,
                                axisOriginCylinder[2] + k * v.side + v.side / 2);

                double dx = v.center[0] - axisOriginCylinder[0];
                double dy = v.center[1] - axisOriginCylinder[1];
                bool inCylinder = dx * dx + dy * dy <= rayonCylinder * rayonCylinder && v.center[2] >= axisOriginCylinder[2] && v.center[2] <= axisOriginCylinder[2] + cylinderHeight;
                bool inSphere = (centreSphere - v.center).length() <= rayonSphere;

                if (inCylinder && inSphere) {
                    drawVoxel(v.center, v.side);
                }
            }
        }
    }
}

void drawSoustractionSphereCylinder(Vec3 centreSphere, double rayonSphere, Vec3 axisOriginCylinder, Vec3 axisVectorCylinder, double rayonCylinder, double resolution) {
    Voxel v;
    v.side = 2 * std::max(rayonSphere, rayonCylinder) / resolution;
    double cylinderHeight = axisVectorCylinder.length();

    for(int i = 0; i < resolution; i++) {
        for(int j = 0; j < resolution; j++) {
            for(int k = 0; k < resolution; k++) {
                v.center = Vec3(axisOriginCylinder[0] - rayonCylinder + i * v.side + v.side / 2,
                                axisOriginCylinder[1] - rayonCylinder + j * v.side + v.side / 2,
                                axisOriginCylinder[2] + k * v.side + v.side / 2);

                double dx = v.center[0] - axisOriginCylinder[0];
                double dy = v.center[1] - axisOriginCylinder[1];
                bool inCylinder = dx * dx + dy * dy <= rayonCylinder * rayonCylinder && v.center[2] >= axisOriginCylinder[2] && v.center[2] <= axisOriginCylinder[2] + cylinderHeight;
                bool inSphere = (centreSphere - v.center).length() <= rayonSphere;

                if (inSphere && !inCylinder) {
                    drawVoxel(v.center, v.side);
                }
            }
        }
    }
}

void drawUnionSphereCylinder(Vec3 centreSphere, double rayonSphere, Vec3 axisOriginCylinder, Vec3 axisVectorCylinder, double rayonCylinder, double resolution) {
    Voxel v;
    v.side = 2 * std::max(rayonSphere, rayonCylinder) / resolution;
    double cylinderHeight = axisVectorCylinder.length();

    for(int i = 0; i < resolution; i++) {
        for(int j = 0; j < resolution; j++) {
            for(int k = 0; k < resolution; k++) {
                v.center = Vec3(axisOriginCylinder[0] - rayonCylinder + i * v.side + v.side / 2,
                                axisOriginCylinder[1] - rayonCylinder + j * v.side + v.side / 2,
                                axisOriginCylinder[2] + k * v.side + v.side / 2);

                double dx = v.center[0] - axisOriginCylinder[0];
                double dy = v.center[1] - axisOriginCylinder[1];
                bool inCylinder = dx * dx + dy * dy <= rayonCylinder * rayonCylinder && v.center[2] >= axisOriginCylinder[2] && v.center[2] <= axisOriginCylinder[2] + cylinderHeight;
                bool inSphere = (centreSphere - v.center).length() <= rayonSphere;

                if (inCylinder || inSphere) {
                    drawVoxel(v.center, v.side);
                }
            }
        }
    }
}

void draw() {
}

void display () {
    glLoadIdentity();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    camera.apply();
    drawAxes();
    switch(currentScene) {
        case 1:
            drawVoxel(Vec3(0,0,0),1);
            break;

        case 2:
            drawGrid(Vec3(0,0,0),5);
            break;

        case 3:
            drawSphereVolumic(Vec3(0,0,0),1,50);
            break;

        case 4:
            drawCylinderVolumic(Vec3(0,0,0),Vec3(0,5,3),1,100);
            //drawCylinderVolumic(Vec3(-1,-1,-1),Vec3(2,2,2),1,50);
            break;

        case 5:
            drawIntersectionSphereCylinder(Vec3(0,0,0), 2, Vec3(-0.5,0,0), Vec3(2,2,2), 1, 50);
            break;

        case 6:
            drawSoustractionSphereCylinder(Vec3(0,0,0), 1, Vec3(0.5,0,0), Vec3(0,1,0), 0.8, 50);
            break;

        case 7:
            drawUnionSphereCylinder(Vec3(1, 1, 1), 1.5, Vec3(0, 0, 0), Vec3(0, 1, 0), 1, 50);
            break;

        default:
            break;
    }
    glFlush();
    glutSwapBuffers();
}

void idle () {
    glutPostRedisplay();
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

    case '1':
        currentScene = 1;
        break;

    case '2':
        currentScene = 2;
        break;

    case '3':
        currentScene = 3;
        break;

    case '4':
        currentScene = 4;
        break;
    
    case '5':
        currentScene = 5;
        break;

    case '6':
        currentScene = 6;
        break;

    case '7':
        currentScene = 7;
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
    camera.resize(w, h);
}

int main (int argc, char ** argv) {
    if (argc > 2) {
        exit (EXIT_FAILURE);
    }
    glutInit (&argc, argv);
    glutInitDisplayMode (GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
    glutInitWindowSize (SCREENWIDTH, SCREENHEIGHT);
    window = glutCreateWindow ("TP8 HAI714I");

    init();
    glutIdleFunc(idle);

    glutDisplayFunc(display);
    glutKeyboardFunc (key);
    glutReshapeFunc (reshape);
    glutMotionFunc (motion);
    glutMouseFunc (mouse);
    key ('?', 0, 0);

    glutMainLoop ();
    return EXIT_SUCCESS;

}