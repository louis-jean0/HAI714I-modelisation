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

struct Point {
    float x;
    float y;
    float z;
};

Point P0;
Point P1;
Vec3 V0;
Vec3 V1;
Point* Generatrice1;
Point* Generatrice2;
Point* Directrice;
Point** SurfaceCylindrique;
Point** SurfaceReglee;
Point** SurfaceBezierBernstein;
long nbu = 1000;
long nbv = 1000;
long nbControlPoints = 5;
Point* controlPoints = new Point[nbControlPoints];
long nbCPu = 4;
long nbCPv = 4;
Point** grilleCP = new Point*[nbCPu];

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
    glPointSize(10);
    glBegin(GL_POINTS);
    glColor3f(1,1,0);
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

Point* DroiteDirectrice(Point P0, Point P1, long nbu) {

    Point* res = new Point[nbu];

    for(long i = 0; i < nbu; i++) {

        double u = (double)i / (double)(nbu-1);

        res[i].x = (1-u)*P0.x + u*P1.x;
        res[i].y = (1-u)*P0.y + u*P1.y;
        res[i].z = (1-u)*P0.z + u*P1.z;

    }

    return res;

}

Point* BezierCurveBernstein(Point* controlPoints, long nbControlPoints, long nbu) {

    Point* res = new Point[nbu];

    long degre = nbControlPoints - 1;

    for(long i = 0; i < nbu; i++) {
        double u = (double)i / (double)(nbu-1);
        for(long j = 0; j <= degre; j++) {
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

    for(long i = 0; i < nbu; i++) {

        double u = (double)i / (double)(nbu - 1);
        
        Point* Q = new Point[nbControlPoints]; // On repart du tableau de points de contrôle de base
            for(int j = 0; j < nbControlPoints; j++) {
                Q[j] = controlPoints[j];
            }

        // De Casteljau
        for(long k = 1; k < nbControlPoints; k++) {
            for(long j = 0; j < nbControlPoints - k; j++) {
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

Point** CreerSurfaceCylindrique(Point* Bezier, Point* Directrice, long nbPointsBezier, long nbPointsDirectrice) {

    Point** res = new Point*[nbPointsDirectrice];

    for(long k = 0; k < nbPointsDirectrice; k++) {

        res[k] = new Point[nbPointsBezier];

    }

    for(long i = 0; i < nbPointsDirectrice; i++) {

        for(long j = 0; j < nbPointsBezier; j++) {

            res[i][j].x = Bezier[j].x + Directrice[i].x;
            res[i][j].y = Bezier[j].y;
            res[i][j].z = Bezier[j].z;

        }

    }

    return res;

}

Point** CreerSurfaceReglee(Point* Bezier1, Point* Bezier2, long nbPointsBeziers, long nbPointsSegments) {

    Point** res = new Point*[nbPointsBeziers];

    for(long k = 0; k < nbPointsBeziers; k++) {

        res[k] = new Point[nbPointsSegments];

    }

    for(long i = 0; i < nbPointsBeziers; i++) {

        Point* segment = DroiteDirectrice(Bezier1[i],Bezier2[i],nbPointsSegments);

        for(long j = 0; j < nbPointsSegments; j++) {

            res[i][j].x = segment[j].x;
            res[i][j].y = segment[j].y;
            res[i][j].z = segment[j].z;

        }

    }

    return res;

}

Point** CreerSurfaceBezierBernstein(Point** grilleControlPoints, long nbCPu, long nbCPv, long nbu, long nbv) {
    Point** res = new Point*[nbu];
    long degreU = nbCPu - 1;
    long degreV = nbCPv - 1;
    for(long k = 0; k < nbu; k++) {
        res[k] = new Point[nbv];
    }
    for(long i = 0; i < nbu; i++) {
        double u = (double)i / (double)(nbu-1);
        for(long j = 0; j < nbv; j++) {
            double v = (double)j / (double)(nbv-1);
            for(long k = 0; k <= degreU; k++) {
                double polynomeU = ((double)fact(degreU)/(float)(fact(k)*(fact(degreU-k)))) * pow(u,k) * pow((1-u),degreU-k);
                for(long l = 0; l <= degreV; l++) {
                    double polynomeV = ((((double)fact(degreV)/(float)(fact(l)*(fact(degreV-l)))) * pow(v,l) * pow((1-v),degreV-l)));
                    res[i][j].x += polynomeU * polynomeV * grilleControlPoints[k][l].x;
                    res[i][j].y += polynomeU * polynomeV * grilleControlPoints[k][l].y;
                    res[i][j].z += polynomeU * polynomeV * grilleControlPoints[k][l].z;
                }
            }
        }
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

void drawSurface(Point** surfacePoints, long nbu, long nbv) {
    long maxUV = max(nbu,nbv);
    long minUV = min(nbu,nbv);
    glColor3f(1,0,1);
    for(int i = 0; i < minUV; i++) {
        glBegin(GL_LINE_STRIP);
        for(int j = 0; j < maxUV; j++) {
            glVertex3f(surfacePoints[i][j].x,surfacePoints[i][j].y,surfacePoints[i][j].z);
        }
        glEnd();
    }
}

void drawSurfaceQuads(Point** surfacePoints, long nbu, long nbv) {
    long maxUV = max(nbu,nbv);
    long minUV = min(nbu,nbv);
    glBegin(GL_QUADS);
    glColor3f(1,0,1);
    for(int i = 0; i < minUV - 1; i++) {
        for(int j = 0; j < maxUV - 1; j++) {
            glVertex3f(surfacePoints[i][j].x, surfacePoints[i][j].y, surfacePoints[i][j].z);
            glVertex3f(surfacePoints[i][j+1].x, surfacePoints[i][j+1].y, surfacePoints[i][j+1].z);
            glVertex3f(surfacePoints[i+1][j+1].x, surfacePoints[i+1][j+1].y, surfacePoints[i+1][j+1].z);
            glVertex3f(surfacePoints[i+1][j].x, surfacePoints[i+1][j].y, surfacePoints[i+1][j].z);
        }
    }
    glEnd();
}

void drawGrilleCP(Point** grillePoints, long nbu, long nbv) {
    long maxUV = max(nbu,nbv);
    long minUV = min(nbu,nbv);
    glColor3f(1, 0, 0);
    glPointSize(5);
    glBegin(GL_POINTS);
    for(int i = 0; i < minUV; i++) {
        for(int j = 0; j < maxUV; j++) {
            glVertex3f(grillePoints[i][j].x, grillePoints[i][j].y, grillePoints[i][j].z);
        }
    }
    glEnd();
    glBegin(GL_LINES);
    for(int i = 0; i < maxUV; i++) {
        for(int j = 0; j < minUV; j++) {
            if(j < nbu-1) {
                glVertex3f(grillePoints[i][j].x, grillePoints[i][j].y, grillePoints[i][j].z);
                glVertex3f(grillePoints[i][j+1].x, grillePoints[i][j+1].y, grillePoints[i][j+1].z);
            }
            if(i < nbv-1) {
                glVertex3f(grillePoints[i][j].x, grillePoints[i][j].y, grillePoints[i][j].z);
                glVertex3f(grillePoints[i+1][j].x, grillePoints[i+1][j].y, grillePoints[i+1][j].z);
            }
        }
    }
    glEnd();
}


void draw() {
    //drawLine(P0,P1);
    //drawControlPoints(controlPointsDirectrice,2);
    //drawControlPoints(controlPoints,nbControlPoints);
    //drawCurve(Generatrice1,nbu);
    //drawCurve(Generatrice2,nbu);
    //drawCurve(Directrice,nbv);
    //drawSurface(SurfaceCylindrique,nbu,nbv);
    //drawSurfaceQuads(SurfaceReglee,nbu,nbv);
    drawGrilleCP(grilleCP,nbCPu,nbCPv);
    drawSurface(SurfaceBezierBernstein,nbu,nbv);
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
    P1 = {6,0,0};
    V0 = {1,1,0};
    V1 = {1,-1,0};

    controlPoints[0] = P0;
    controlPoints[1] = {0,2,0};
    controlPoints[2] = {-2,0,0};
    controlPoints[3] = {-2,2,0};
    controlPoints[4] = {1,4,0};

    Directrice = DroiteDirectrice(P0,P1,nbv);
    Generatrice1 = BezierCurveBernstein(controlPoints,nbControlPoints,nbu);

    controlPoints[0] = P1;
    controlPoints[1] = {6,3,3};
    controlPoints[2] = {4,4,2};
    controlPoints[3] = {4,6,1};
    controlPoints[4] = P1;
    //Generatrice2 = BezierCurveBernstein(controlPoints,nbControlPoints,nbu);

    //SurfaceCylindrique = CreerSurfaceCylindrique(Generatrice1,Directrice,nbu,nbv);
    //SurfaceReglee = CreerSurfaceReglee(Generatrice1,Generatrice2,nbu,nbv);

    for(int i = 0; i < nbCPu; i++) {
    grilleCP[i] = new Point[nbCPv];
    }

    grilleCP[0][0] = {0,0,0};
    grilleCP[0][1] = {1,0,0.2};
    grilleCP[0][2] = {2,0,0.4};
    grilleCP[0][3] = {3,0,0.6};
    grilleCP[1][0] = {0,1,1};
    grilleCP[1][1] = {1,1,1.2};
    grilleCP[1][2] = {2,1,1.4};
    grilleCP[1][3] = {3,1,1.6};
    grilleCP[2][0] = {0,2,0};
    grilleCP[2][1] = {1,2,0.2};
    grilleCP[2][2] = {2,2,0.4};
    grilleCP[2][3] = {3,2,0.6};
    grilleCP[3][0] = {0,3,1};
    grilleCP[3][1] = {1,3,1.2};
    grilleCP[3][2] = {2,3,1.4};
    grilleCP[3][3] = {3,3,1.6};

    SurfaceBezierBernstein = CreerSurfaceBezierBernstein(grilleCP,nbCPu,nbCPv,nbu,nbv);

    glutDisplayFunc(display);
    glutKeyboardFunc (key);
    glutReshapeFunc (reshape);
    glutMotionFunc (motion);
    glutMouseFunc (mouse);
    key ('?', 0, 0);

    glutMainLoop ();
    delete[] Directrice;
    delete[] Generatrice1;
    delete[] Generatrice2;
    delete[] SurfaceCylindrique;
    delete[] SurfaceReglee;
    delete[] controlPoints;
    return EXIT_SUCCESS;

}