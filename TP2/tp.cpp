// -------------------------------------------
// gMini : a minimal OpenGL/GLUT application
// for 3D graphics.
// Copyright (C) 2006-2008 Tamy Boubekeur
// All rights reserved.
// -------------------------------------------

// -------------------------------------------
// Disclaimer: this code is dirty in the
// meaning that there is no attention paid to
// proper class attribute access, memory
// management or optimisation of any kind. It
// is designed for quick-and-dirty testing
// purpose.
// -------------------------------------------

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

struct Point {
    float x;
    float y;
    float z;
};

Point P0;
Point P1;
Vec3 V0;
Vec3 V1;
Point* tabPoints;
long nbu = 100;
long nbControlPoints = 5;
Point* controlPoints = new Point[nbControlPoints];
std::random_device rd;
std::mt19937 gen(rd());
std::uniform_int_distribution<> distrib(0, nbControlPoints);
int randomNumber;

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

void init () {
    camera.resize (SCREENWIDTH, SCREENHEIGHT);
    glCullFace (GL_BACK);
    glEnable (GL_CULL_FACE);
    glDepthFunc (GL_LESS);
    glEnable (GL_DEPTH_TEST);
    glClearColor (0.2f, 0.2f, 0.3f, 1.0f);
    glEnable(GL_COLOR_MATERIAL);
}

void drawLine(Point p1, Point p2) {
    glBegin(GL_LINES);
    glColor3f(1,1,0);
    glVertex3f(p1.x,p1.y,p1.z);
    glVertex3f(p2.x,p2.y,p2.z);
    glEnd();
}

void drawPoint(Point p) {
    glBegin(GL_POINTS);
    glColor3f(1,1,0);
    glPointSize(10);
    glVertex3f(p.x,p.y,p.z);
    glEnd();
}

long fact(long n) {
    long res = 1;
    for(int i = 1; i <= n; i++) {
        res *= i;
    }
    return res;
}

Point* HermiteCubicCurve(Point P0, Point P1, Vec3 V0, Vec3 V1, long nbu) {

    Point* res = new Point[nbu];

    for(int i = 0; i < nbu; i++) {
        double u = (double)i / (double)(nbu-1);
        double u2 = u*u;
        double u3 = u2*u;
        res[i].x = (2*u3 - 3*u2 + 1)*P0.x + (-2*u3 + 3*u2)*P1.x + (u3 - 2*u2 + u)*V0[0] + (u3 - u2)*V1[0];
        res[i].y = (2*u3 - 3*u2 + 1)*P0.y + (-2*u3 + 3*u2)*P1.y + (u3 - 2*u2 + u)*V0[1] + (u3 - u2)*V1[1];
        res[i].z = (2*u3 - 3*u2 + 1)*P0.z + (-2*u3 + 3*u2)*P1.z + (u3 - 2*u2 + u)*V0[2] + (u3 - u2)*V1[2];
    }
    
    return res;

}

Point* BezierCurveBernstein(Point* controlPoints, long nbControlPoints, long nbu) {

    Point* res = new Point[nbu];

    long degre = nbControlPoints - 1;

    for(int i = 0; i < nbu; i++) {
        double u = (double)i / (double)(nbu-1);
        for(int j = 0; j <= degre; j++) {
            double polynome = ((double)fact(degre)/(float)(fact(j)*(fact(degre-j)))) * pow(u,j) * pow((1-u),degre-j);
            res[i].x += polynome * controlPoints[j].x;
            res[i].y += polynome * controlPoints[j].y;
            res[i].z += polynome * controlPoints[j].z;
        }
    }

    return res;

}

Point* BezierCurveDeCasteljau(Point* controlPoints, long nbControlPoints, long nbu) {

    Point* res = new Point[nbu];

    for(int i = 0; i < nbu; i++) {

        double u = (double)i / (double)(nbu - 1);
        
        Point* Q = new Point[nbControlPoints]; // On repart du tableau de points de contrôle de base
            for(int j = 0; j < nbControlPoints; j++) {
                Q[j] = controlPoints[j];
            }

        // De Casteljau
        for(int k = 1; k < nbControlPoints; k++) {
            for(int j = 0; j < nbControlPoints - k; j++) {
                drawLine(Q[j],Q[j+1]);
                Q[j].x = (1 - u) * Q[j].x + u * Q[j + 1].x;
                Q[j].y = (1 - u) * Q[j].y + u * Q[j + 1].y;
                Q[j].z = (1 - u) * Q[j].z + u * Q[j + 1].z;
                drawPoint(Q[j]);
            }
        }
        res[i] = Q[0];
        drawPoint(Q[0]);
        
    }

    return res;

}

void drawCurve(Point* curvePoints, long nbPoints) {
    glBegin(GL_LINE_STRIP);
    glColor3f(0,1,0);
    for(int i = 0; i < nbPoints; i++) {
        glVertex3f(curvePoints[i].x, curvePoints[i].y, curvePoints[i].z);
    }
    glEnd();
}

void drawControlPoints(Point* controlPoints, long nbControlPoints) {
    glBegin(GL_LINE_LOOP);
    glColor3f(1,0,0);
    for(int i = 0; i < nbControlPoints; i++) {
        glVertex3f(controlPoints[i].x, controlPoints[i].y, controlPoints[i].z);
    }
    glEnd();

    // On explicite les points de contrôle en les affichant
    glPointSize(5);
    glBegin(GL_POINTS);
    for(int i = 0; i < nbControlPoints; i++) {
        glVertex3f(controlPoints[i].x, controlPoints[i].y, controlPoints[i].z);
    }
    glEnd();

}

void display () {
    glLoadIdentity ();
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    camera.apply ();
    tabPoints = BezierCurveDeCasteljau(controlPoints,nbControlPoints,nbu); // Si je n'appelle pas cette fonction ici, les appels à drawLine et drawPoint ne fonctionnent pas (dans Casteljau)
    drawCurve(tabPoints,nbu);
    drawControlPoints(controlPoints,nbControlPoints);
    glFlush ();
    glutSwapBuffers();
}

void idle () {
    glutPostRedisplay ();
}

void key (unsigned char keyPressed, int x, int y) {
    switch (keyPressed) {

    case 'p':
        randomNumber = distrib(gen);
        if(randomNumber % nbControlPoints <= 3) {
            controlPoints[randomNumber].x *=2;
        }
        else {
            controlPoints[randomNumber].y -= 3;
            controlPoints[randomNumber].z += 1;
        }
        tabPoints = BezierCurveDeCasteljau(controlPoints,nbControlPoints,nbu);
        break;

    case 'f':
        if (fullScreen == true) {
            glutReshapeWindow (SCREENWIDTH, SCREENHEIGHT);
            fullScreen = false;
        } else {
            glutFullScreen ();
            fullScreen = true;
        }
        break;

    default:
        break;
    }
    idle ();
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
    window = glutCreateWindow ("TP2 HAI714I");

    init ();
    glutIdleFunc (idle);

    P0 = {0,0,0};
    P1 = {2,0,0};
    V0 = {1,1,0};
    V1 = {1,-1,0};

    controlPoints[0] = P0;
    controlPoints[1] = (Point){2,3,2};
    //controlPoints[2] = (Point){5,0,0};
    controlPoints[2] = (Point){4,5,1};
    controlPoints[3] = (Point){6,3,1};
    controlPoints[4] = (Point){9,0,6};

    //tabPoints = HermiteCubicCurve(P0,P1,V0,V1,nbu);
    //tabPoints = BezierCurveBernstein(controlPoints,nbControlPoints,nbu);
    tabPoints = BezierCurveDeCasteljau(controlPoints,nbControlPoints,nbu);
    glutDisplayFunc(display);

    glutKeyboardFunc (key);
    glutReshapeFunc (reshape);
    glutMotionFunc (motion);
    glutMouseFunc (mouse);
    key ('?', 0, 0);

    glutMainLoop ();
    delete[] tabPoints;
    delete[] controlPoints;
    return EXIT_SUCCESS;
}

