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

#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <set>

#include <algorithm>
#include <GL/glut.h>
#include <float.h>
#include "src/Vec3.h"
#include "src/Camera.h"

enum DisplayMode{ WIRE=0, SOLID=1, LIGHTED_WIRE=2, LIGHTED=3 };

struct Triangle {
    inline Triangle () {
        v[0] = v[1] = v[2] = 0;
    }
    inline Triangle (const Triangle & t) {
        v[0] = t.v[0];   v[1] = t.v[1];   v[2] = t.v[2];
    }
    inline Triangle (unsigned int v0, unsigned int v1, unsigned int v2) {
        v[0] = v0;   v[1] = v1;   v[2] = v2;
    }
    unsigned int & operator [] (unsigned int iv) { return v[iv]; }
    unsigned int operator [] (unsigned int iv) const { return v[iv]; }
    inline virtual ~Triangle () {}
    inline Triangle & operator = (const Triangle & t) {
        v[0] = t.v[0];   v[1] = t.v[1];   v[2] = t.v[2];
        return (*this);
    }
    // membres indices des sommets du triangle:
    unsigned int v[3];
};

struct Mesh {
    std::vector< Vec3 > vertices; //array of mesh vertices positions
    std::vector< Vec3 > normals; //array of vertices normals useful for the display
    std::vector< Triangle > triangles; //array of mesh triangles
    std::vector< Vec3 > triangle_normals; //triangle normals to display face normals

    void simplify(unsigned int resolution);
    void adaptativeSimplify (unsigned int numOfPerLeafVertices);
};

struct Representant {

    Vec3 position = Vec3(0,0,0);
    Vec3 normal = Vec3(0,0,0);

};

struct GridData {

    Representant representant;
    int nbVerticesPerCell = 0;

};

struct Grid {
    std::vector<GridData> cells;
    Vec3 minPos, maxPos;
    int resolution;

    int getCellX(Vec3 pos) { return resolution * (pos[0] - minPos[0]) / (maxPos[0] - minPos[0]); }
    int getCellY(Vec3 pos) { return resolution * (pos[1] - minPos[1]) / (maxPos[1] - minPos[1]); }
    int getCellZ(Vec3 pos) { return resolution * (pos[2] - minPos[2]) / (maxPos[2] - minPos[2]); }

    int getIndex(int x, int y, int z) { return x * resolution * resolution + y * resolution + z; }
    int getIndex(Vec3 pos) { return getCellX(pos) * resolution * resolution + getCellY(pos) * resolution + getCellZ(pos); }
};

Vec3 minVectorVec3(const std::vector<Vec3> &V) {                                    // Créé un vecteur contenant les x, y, z minimaux du vecteur passé en paramètre

    size_t V_size = V.size();
    float minX = std::numeric_limits<float>::max();
    float minY = std::numeric_limits<float>::max();
    float minZ = std::numeric_limits<float>::max();

    for(size_t i = 0; i < V_size; i++) {
        if(V[i][0] < minX) {
            minX = V[i][0];
        }
        if(V[i][1] < minY) {
            minY = V[i][1];
        }
        if(V[i][2] < minZ) {
            minZ = V[i][2];
        }
    }
    return Vec3(minX,minY,minZ);
}

Vec3 maxVectorVec3(const std::vector<Vec3> &V) {                                    // Créé un vecteur contenant les x, y, z maximaux du vecteur passé en paramètre

    size_t V_size = V.size();
    float maxX = -std::numeric_limits<float>::max();
    float maxY = -std::numeric_limits<float>::max();
    float maxZ = -std::numeric_limits<float>::max();

    for(size_t i = 0; i < V_size; i++) {
        if(V[i][0] > maxX) {
            maxX = V[i][0];
        }

        if(V[i][1] > maxY) {
            maxY = V[i][1];
        }

        if(V[i][2] > maxZ) {
            maxZ = V[i][2];
        }
    }
    return Vec3(maxX,maxY,maxZ);
}

void Mesh::simplify(unsigned int resolution) {

    // Compute the cube C that englobes all vertices

    Vec3 min = minVectorVec3(vertices);
    Vec3 max = maxVectorVec3(vertices);
    Grid C;
    C.minPos = min;
    C.minPos[0] -= 0.15f;
    C.minPos[1] -= 0.15f;
    C.minPos[2] -= 0.15f;
    C.maxPos = max;
    C.maxPos[0] += 0.15f;
    C.maxPos[1] += 0.15f;
    C.maxPos[2] += 0.15f;
    C.resolution = resolution;

    // Create a grid of size resolution x resolution x resolution in the cube

    size_t resolution3D = resolution*resolution*resolution;
    std::vector<GridData> G(resolution3D);
    C.cells = G;

    // For each vertex, add the position and normal to its representant in the grid
    // (Count the number of vertices per cell)

    size_t nbVerticesInMesh = vertices.size();

    for(size_t i = 0; i < nbVerticesInMesh; i++) {

        Vec3 currentVertex = vertices[i];
        Vec3 currentNormal = normals[i];
        int index = C.getIndex(currentVertex);

        if(C.cells[index].representant.position == Vec3(0,0,0) && C.cells[index].representant.normal == Vec3(0,0,0)) { 
            // Cas pour gérer l'initialisation de chaque représentant

            C.cells[index].representant.position = currentVertex;
            C.cells[index].representant.normal = currentNormal;
            C.cells[index].nbVerticesPerCell++;

        }

        else {

            C.cells[index].representant.position += currentVertex;
            C.cells[index].representant.normal += currentNormal;
            C.cells[index].nbVerticesPerCell++;

        }

    }

    std::vector<Vec3> newVertices;
    std::vector<Vec3> newNormals;

    for(size_t i = 0; i < resolution3D; i++) {  

        Vec3 new_pos = C.cells[i].representant.position / C.cells[i].nbVerticesPerCell; // Divide the position of each representant by the number of vertices in the cell.
        newVertices.push_back(new_pos); 
        Vec3 new_norm = C.cells[i].representant.normal;
        new_norm.normalize(); // Normalize the normals
        newNormals.push_back(new_norm);

    }

    // For each triangle t, set the indices of its vertices to point to representants
    // If 2 vertices share the same representant, remove the triangle

    size_t nbTrianglesInMesh = triangles.size();
    std::vector<Triangle> newTriangles;

    for(size_t i = 0; i < nbTrianglesInMesh; i++) {

        int index0 = C.getIndex(vertices[triangles[i][0]]);
        int index1 = C.getIndex(vertices[triangles[i][1]]);
        int index2 = C.getIndex(vertices[triangles[i][2]]);

        if(index0 != index1 && index0 != index2 && index1 != index2) {
            
            newTriangles.push_back(Triangle(index0,index1,index2));

        }

    }

    vertices = newVertices;
    normals = newNormals;
    triangles = newTriangles;

    // BONUS EXERCICE : Instead of using the mean position of the vertices in the cell,
    // use the QEM proposed in "Out-of-core simplification of large polygonal models."
    // Peter Lindstrom. SIGGRAPH 2000 (http://wwwevasion.imag.fr/people/Franck.Hetroy/Teaching/Geo3D/Articles/lindstrom2000.pdf)
    // You might need to change the content of GridData and the way to process it
}

void Mesh::adaptativeSimplify(unsigned int numOfPerLeafVertices) {
    std::cerr << "Adaptative simplification to be done." << std::endl;

    // Create (or copy from internet) an Octree class
    // Insert vertices in the Octree. If a node contains more than "numOfPerLeafVertices", divide it into children.

    // Compute the mean position and normal for each leaf's representant

    // For each triangle t, set the indices of its vertices to point to representants
    // If 2 vertices share the same representant, remove the triangle
}


//Transformation made of a rotation and translation
struct Transformation {
    Mat3 rotation;
    Vec3 translation;
};

void collect_one_ring (std::vector<Vec3> const & i_vertices,
                       std::vector< Triangle > const & i_triangles,
                       std::vector<std::vector<unsigned int> > & o_one_ring) {//one-ring of each vertex, i.e. a list of vertices with which it shares an edge
    //Initialiser le vecetur de o_one_ring de la taille du vecteur vertices
    o_one_ring = std::vector<std::vector<unsigned int>>(i_vertices.size());
    //Parcourir les triangles et ajouter les voisins dans le 1-voisinage
    for (auto& t : i_triangles) {
        unsigned int i0 = t[0];
        unsigned int i1 = t[1];
        unsigned int i2 = t[2];
        if (std::find(o_one_ring[i0].begin(), o_one_ring[i0].end(), i1) == o_one_ring[i0].end()) o_one_ring[i0].push_back(i1);
        if (std::find(o_one_ring[i0].begin(), o_one_ring[i0].end(), i2) == o_one_ring[i0].end()) o_one_ring[i0].push_back(i2);
        
        if (std::find(o_one_ring[i1].begin(), o_one_ring[i1].end(), i0) == o_one_ring[i1].end()) o_one_ring[i1].push_back(i0);
        if (std::find(o_one_ring[i1].begin(), o_one_ring[i1].end(), i2) == o_one_ring[i1].end()) o_one_ring[i1].push_back(i2);
        
        if (std::find(o_one_ring[i2].begin(), o_one_ring[i2].end(), i0) == o_one_ring[i2].end()) o_one_ring[i2].push_back(i0);
        if (std::find(o_one_ring[i2].begin(), o_one_ring[i2].end(), i1) == o_one_ring[i2].end()) o_one_ring[i2].push_back(i1);
    }

}

//Input mesh loaded at the launch of the application
Mesh originalMesh;
Mesh mesh;

bool display_normals;
bool display_mesh;
DisplayMode displayMode;
int simplificationResolution = 20;  // GRID RESOLUTION
int nodesPerLeafInOctree = 2;       // OCTREE "RESOLUTION"

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

// ------------------------------------
// File I/O
// ------------------------------------
bool saveOFF( const std::string & filename ,
              std::vector< Vec3 > const & i_vertices ,
              std::vector< Vec3 > const & i_normals ,
              std::vector< Triangle > const & i_triangles,
              std::vector< Vec3 > const & i_triangle_normals ,
              bool save_normals = true ) {
    std::ofstream myfile;
    myfile.open(filename.c_str());
    if (!myfile.is_open()) {
        std::cout << filename << " cannot be opened" << std::endl;
        return false;
    }

    myfile << "OFF" << std::endl ;

    unsigned int n_vertices = i_vertices.size() , n_triangles = i_triangles.size();
    myfile << n_vertices << " " << n_triangles << " 0" << std::endl;

    for( unsigned int v = 0 ; v < n_vertices ; ++v ) {
        myfile << i_vertices[v][0] << " " << i_vertices[v][1] << " " << i_vertices[v][2] << " ";
        if (save_normals) myfile << i_normals[v][0] << " " << i_normals[v][1] << " " << i_normals[v][2] << std::endl;
        else myfile << std::endl;
    }
    for( unsigned int f = 0 ; f < n_triangles ; ++f ) {
        myfile << 3 << " " << i_triangles[f][0] << " " << i_triangles[f][1] << " " << i_triangles[f][2]<< " ";
        if (save_normals) myfile << i_triangle_normals[f][0] << " " << i_triangle_normals[f][1] << " " << i_triangle_normals[f][2];
        myfile << std::endl;
    }
    myfile.close();
    return true;
}

void openOFF( std::string const & filename,
              std::vector<Vec3> & o_vertices,
              std::vector<Vec3> & o_normals,
              std::vector< Triangle > & o_triangles,
              std::vector< Vec3 > & o_triangle_normals,
              bool load_normals = true )
{
    std::ifstream myfile;
    myfile.open(filename.c_str());
    if (!myfile.is_open())
    {
        std::cout << filename << " cannot be opened" << std::endl;
        return;
    }

    std::string magic_s;
    int iLine = 0;

    myfile >> magic_s;
    iLine++;

    if( magic_s != "OFF" )
    {
        std::cout << magic_s << " != OFF :   We handle ONLY *.off files." << std::endl;
        myfile.close();
        exit(1);
    }

    int n_vertices , n_faces , dummy_int;
    myfile >> n_vertices >> n_faces >> dummy_int;
    iLine++;

    o_vertices.clear();
    o_normals.clear();

    for( int v = 0 ; v < n_vertices ; ++v )
    {
        float x , y , z ;

        myfile >> x >> y >> z ;
        o_vertices.push_back( Vec3( x , y , z ) );

        if( load_normals ) {
            myfile >> x >> y >> z;
            o_normals.push_back( Vec3( x , y , z ) );
        }
        iLine++;
    }

    o_triangles.clear();
    o_triangle_normals.clear();
    for( int f = 0 ; f < n_faces ; ++f )
    {
        int n_vertices_on_face;
        myfile >> n_vertices_on_face;

        if( n_vertices_on_face == 3 )
        {
            unsigned int _v1 , _v2 , _v3;
            myfile >> _v1 >> _v2 >> _v3;

            o_triangles.push_back(Triangle( _v1, _v2, _v3 ));

            if( load_normals ) {
                float x , y , z ;
                myfile >> x >> y >> z;
                o_triangle_normals.push_back( Vec3( x , y , z ) );
            }
        }
        else if( n_vertices_on_face == 4 )
        {
            unsigned int _v1 , _v2 , _v3 , _v4;
            myfile >> _v1 >> _v2 >> _v3 >> _v4;

            o_triangles.push_back(Triangle(_v1, _v2, _v3 ));
            o_triangles.push_back(Triangle(_v1, _v3, _v4));
            if( load_normals ) {
                float x , y , z ;
                myfile >> x >> y >> z;
                o_triangle_normals.push_back( Vec3( x , y , z ) );
            }

        }
        else
        {
            std::cout << "We handle ONLY *.off files with 3 or 4 vertices per face (here = " << n_vertices_on_face << " on line " << iLine << ")" << std::endl;
            myfile.close();
            exit(1);
        }
        iLine++;
    }

}

// ------------------------------------
// Application initialization
// ------------------------------------
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
    glCullFace (GL_BACK);
    glDisable (GL_CULL_FACE);
    glDepthFunc (GL_LESS);
    glEnable (GL_DEPTH_TEST);
    glClearColor (0.2f, 0.2f, 0.3f, 1.0f);
    glEnable(GL_COLOR_MATERIAL);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

    display_normals = false;
    display_mesh = true;
    displayMode = LIGHTED;
}


// ------------------------------------
// Rendering.
// ------------------------------------

void drawVector( Vec3 const & i_from, Vec3 const & i_to ) {

    glBegin(GL_LINES);
    glVertex3f( i_from[0] , i_from[1] , i_from[2] );
    glVertex3f( i_to[0] , i_to[1] , i_to[2] );
    glEnd();
}

void drawAxis( Vec3 const & i_origin, Vec3 const & i_direction ) {

    glLineWidth(4); // for example...
    drawVector(i_origin, i_origin + i_direction);
}

void drawReferenceFrame( Vec3 const & origin, Vec3 const & i, Vec3 const & j, Vec3 const & k ) {

    glDisable(GL_LIGHTING);
    glColor3f( 0.8, 0.2, 0.2 );
    drawAxis( origin, i );
    glColor3f( 0.2, 0.8, 0.2 );
    drawAxis( origin, j );
    glColor3f( 0.2, 0.2, 0.8 );
    drawAxis( origin, k );
    glEnable(GL_LIGHTING);

}

typedef struct {
    float r;       // ∈ [0, 1]
    float g;       // ∈ [0, 1]
    float b;       // ∈ [0, 1]
} RGB;



RGB scalarToRGB( float scalar_value ) //Scalar_value ∈ [0, 1]
{
    RGB rgb;
    float H = scalar_value*360., S = 1., V = 0.85,
            P, Q, T,
            fract;

    (H == 360.)?(H = 0.):(H /= 60.);
    fract = H - floor(H);

    P = V*(1. - S);
    Q = V*(1. - S*fract);
    T = V*(1. - S*(1. - fract));

    if      (0. <= H && H < 1.)
        rgb = (RGB){.r = V, .g = T, .b = P};
    else if (1. <= H && H < 2.)
        rgb = (RGB){.r = Q, .g = V, .b = P};
    else if (2. <= H && H < 3.)
        rgb = (RGB){.r = P, .g = V, .b = T};
    else if (3. <= H && H < 4.)
        rgb = (RGB){.r = P, .g = Q, .b = V};
    else if (4. <= H && H < 5.)
        rgb = (RGB){.r = T, .g = P, .b = V};
    else if (5. <= H && H < 6.)
        rgb = (RGB){.r = V, .g = P, .b = Q};
    else
        rgb = (RGB){.r = 0., .g = 0., .b = 0.};

    return rgb;
}

void drawSmoothTriangleMesh( Mesh const & i_mesh , bool draw_field = false ) {
    glBegin(GL_TRIANGLES);
    for(unsigned int tIt = 0 ; tIt < i_mesh.triangles.size(); ++tIt) {

        for(unsigned int i = 0 ; i < 3 ; i++) {
            const Vec3 & p = i_mesh.vertices[i_mesh.triangles[tIt][i]]; //Vertex position
            const Vec3 & n = i_mesh.normals[i_mesh.triangles[tIt][i]]; //Vertex normal

            glNormal3f( n[0] , n[1] , n[2] );
            glVertex3f( p[0] , p[1] , p[2] );
        }
    }
    glEnd();

}

void drawTriangleMesh( Mesh const & i_mesh , bool draw_field = false  ) {
    glBegin(GL_TRIANGLES);
    for(unsigned int tIt = 0 ; tIt < i_mesh.triangles.size(); ++tIt) {
        const Vec3 & n = i_mesh.triangle_normals[ tIt ]; //Triangle normal
        for(unsigned int i = 0 ; i < 3 ; i++) {
            const Vec3 & p = i_mesh.vertices[i_mesh.triangles[tIt][i]]; //Vertex position
            glNormal3f( n[0] , n[1] , n[2] );
            glVertex3f( p[0] , p[1] , p[2] );
        }
    }
    glEnd();
}

void drawMesh( Mesh const & i_mesh , bool draw_field = false ){
    drawSmoothTriangleMesh(i_mesh, draw_field) ; //Smooth display with vertices normals
}

void drawVectorField( std::vector<Vec3> const & i_positions, std::vector<Vec3> const & i_directions ) {
    glLineWidth(1.);
    for(unsigned int pIt = 0 ; pIt < i_directions.size() ; ++pIt) {
        Vec3 to = i_positions[pIt] + 0.02*i_directions[pIt];
        drawVector(i_positions[pIt], to);
    }
}

void drawNormals(Mesh const& i_mesh){
    drawVectorField( i_mesh.vertices, i_mesh.normals );
}

//Draw fonction
void draw () {

    if(displayMode == LIGHTED || displayMode == LIGHTED_WIRE){

        glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);
        glEnable(GL_LIGHTING);

    }  else if(displayMode == WIRE){

        glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
        glDisable (GL_LIGHTING);

    }  else if(displayMode == SOLID ){
        glDisable (GL_LIGHTING);
        glPolygonMode (GL_FRONT_AND_BACK, GL_FILL);

    }

    glColor3f(0.8,1,0.8);
    drawMesh(mesh, true);

    if(displayMode == SOLID || displayMode == LIGHTED_WIRE){
        glEnable (GL_POLYGON_OFFSET_LINE);
        glPolygonMode (GL_FRONT_AND_BACK, GL_LINE);
        glLineWidth (1.0f);
        glPolygonOffset (-2.0, 1.0);

        glColor3f(0.,0.,0.);
        drawMesh(mesh, false);

        glDisable (GL_POLYGON_OFFSET_LINE);
        glEnable (GL_LIGHTING);
    }



    glDisable(GL_LIGHTING);
    if(display_normals){
        glColor3f(1.,0.,0.);
        drawNormals(mesh);
    }
    glEnable(GL_LIGHTING);

    glutSetWindowTitle(("TP HAI714I | verts: " + std::to_string(mesh.vertices.size()) + " res: " + std::to_string(simplificationResolution) + " nodes: " + std::to_string(nodesPerLeafInOctree)).c_str());

}

void changeDisplayMode(){
    if(displayMode == LIGHTED)
        displayMode = LIGHTED_WIRE;
    else if(displayMode == LIGHTED_WIRE)
        displayMode = SOLID;
    else if(displayMode == SOLID)
        displayMode = WIRE;
    else
        displayMode = LIGHTED;
}

void display () {
    glLoadIdentity ();
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    camera.apply ();
    draw ();
    glFlush ();
    glutSwapBuffers ();
}

void idle () {
    glutPostRedisplay ();
}

// ------------------------------------
// User inputs
// ------------------------------------
//Keyboard event
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


    case 'w': //Change le mode d'affichage
        changeDisplayMode();
        break;


    case 'n': //Press n key to display normals
        display_normals = !display_normals;
        break;

    case '1': // Reset the mesh
        mesh = originalMesh;
        break;

    case '2': // Simplification by grid
        mesh = originalMesh;
        mesh.simplify(simplificationResolution);
        break;

    case '3': // Simplification from Octree
        mesh = originalMesh;
        mesh.adaptativeSimplify(nodesPerLeafInOctree);
        break;

    // Set the key input to increase/decrease resolutions
    case 'a':
        simplificationResolution = std::max(1, simplificationResolution - 1);
        break;
    case 'q':
        simplificationResolution = std::max(1, simplificationResolution + 1);
        break;

    case 'd':
        nodesPerLeafInOctree = std::max(1, nodesPerLeafInOctree - 1);
        break;
    case 'e':
        nodesPerLeafInOctree = std::max(1, nodesPerLeafInOctree + 1);
        break;

    default:
        break;
    }
    idle ();
}

//Mouse events
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

//Mouse motion, update camera
void motion (int x, int y) {
    if (mouseRotatePressed == true) {
        camera.rotate (x, y);
    }
    else if (mouseMovePressed == true) {
        camera.move ((x-lastX)/static_cast<float>(SCREENWIDTH), (lastY-y)/static_cast<float>(SCREENHEIGHT), 0.0);
        lastX = x;
        lastY = y;
    }
    else if (mouseZoomPressed == true) {
        camera.zoom (float (y-lastZoom)/SCREENHEIGHT);
        lastZoom = y;
    }
}


void reshape(int w, int h) {
    camera.resize (w, h);
}

// ------------------------------------
// Start of graphical application
// ------------------------------------
int main (int argc, char ** argv) {
    if (argc > 2) {
        exit (EXIT_FAILURE);
    }
    glutInit (&argc, argv);
    glutInitDisplayMode (GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
    glutInitWindowSize (SCREENWIDTH, SCREENHEIGHT);
    window = glutCreateWindow ("TP HAI714I");

    init ();
    glutIdleFunc (idle);
    glutDisplayFunc (display);
    glutKeyboardFunc (key);
    glutReshapeFunc (reshape);
    glutMotionFunc (motion);
    glutMouseFunc (mouse);
    key ('?', 0, 0);

    //Mesh loaded with precomputed normals
    //openOFF("data/elephant_n.off", originalMesh.vertices, originalMesh.normals, originalMesh.triangles, originalMesh.triangle_normals);
    //openOFF("data/unit_sphere_n.off", originalMesh.vertices, originalMesh.normals, originalMesh.triangles, originalMesh.triangle_normals);
    openOFF("data/avion_n.off", originalMesh.vertices, originalMesh.normals, originalMesh.triangles, originalMesh.triangle_normals);
    //openOFF("data/camel_n.off", originalMesh.vertices, originalMesh.normals, originalMesh.triangles, originalMesh.triangle_normals);

    mesh = originalMesh;

    glutMainLoop ();
    return EXIT_SUCCESS;
}

