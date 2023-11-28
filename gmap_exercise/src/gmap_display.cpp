// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <iostream>
#include <float.h>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
using namespace glm;

#include <common/shader.hpp>
#include <common/objloader.hpp>
#include <common/vboindexer.hpp>

#include <algorithm>

#include "gmap.hpp"

/*******************************************************************************/

void compute_triangle_normals(std::vector< glm::vec3 > & triangle_normals,
    std::vector< glm::vec3 > & vertices,
    std::vector< std::vector< unsigned short > > & triangles) {

    for (size_t i = 0; i < triangles.size(); ++i) {
        glm::vec3 XY = vertices[triangles[i][1]] - vertices[triangles[i][0]];
        glm::vec3 XZ = vertices[triangles[i][2]] - vertices[triangles[i][0]];
        triangle_normals[i] = normalize(cross(XY, XZ));
    }
}

void collect_one_ring(std::vector< std::vector< unsigned short > > & one_ring,
    std::vector< glm::vec3 > & vertices,
    std::vector< std::vector< unsigned short > > & triangles) {
    for (size_t i = 0; i < triangles.size(); ++i)
        for (size_t k = 0; k < triangles[i].size(); ++k)
            one_ring[triangles[i][k]].push_back(static_cast<unsigned short>(i));
}

void compute_smooth_vertex_normals(unsigned int weight_type, std::vector< glm::vec3 > & vertex_normals,
    std::vector< glm::vec3 > & vertices,
    std::vector< std::vector< unsigned short > > & triangles) {
    float w = 1.0; // default: uniform weights

    // Get triangle face normals
    std::vector< glm::vec3 > triangle_normals;
    triangle_normals.resize(triangles.size());
    compute_triangle_normals(triangle_normals, vertices, triangles);

    // Get one-ring for each vertex
    std::vector< std::vector< unsigned short > > one_ring;
    one_ring.resize(vertices.size());
    collect_one_ring(one_ring, vertices, triangles);

    // Compute vertex normals
    for (size_t i = 0; i < vertices.size(); ++i) {
        for (size_t k = 0; k < one_ring[i].size(); ++k) {
            vertex_normals[i] += triangle_normals[one_ring[i][k]];
        }
    }
    for (size_t i = 0; i < vertices.size(); ++i) {
        vertex_normals[i] = normalize(vertex_normals[i]);
    }

}

int display(const  GMap3D& gmap)
{
    // Initialise GLFW
    if( !glfwInit() )
    {
        fprintf( stderr, "Failed to initialize GLFW\n" );
        getchar();
        return -1;
    }

    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Open a window and create its OpenGL context
    window = glfwCreateWindow( 1024, 768, "TP3 - GMap", NULL, NULL);
    if( window == NULL ){
        fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
        getchar();
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        getchar();
        glfwTerminate();
        return -1;
    }

    // Ensure we can capture the escape key being pressed below
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    // Hide the mouse and enable unlimited mouvement
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Set the mouse at the center of the screen
    glfwPollEvents();
    glfwSetCursorPos(window, 1024/2, 768/2);

    // Dark blue background
    glClearColor(0.4f, 0.4f, 0.4f, 0.0f);

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);

    // Cull triangles which normal is not towards the camera
    glEnable(GL_CULL_FACE);

    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    glBindVertexArray(VertexArrayID);

    // Create and compile our GLSL program from the shaders
    GLuint programID = LoadShaders( "vertex_shader.glsl", "fragment_shader.glsl" );

    // Get a handle for our "MVP" uniform
    GLuint MatrixID = glGetUniformLocation(programID, "MVP");
    GLuint ViewMatrixID = glGetUniformLocation(programID, "V");
    GLuint ModelMatrixID = glGetUniformLocation(programID, "M");


    // transform the gmap into set of triangles
    // TOCOMPLETE
    // for the property, you can first use a random value for each vertices.

    std::vector<unsigned short> indices; //Triangles concaténés dans une liste
    std::vector<glm::vec3> indexed_vertices; // Liste des indices dans l'ordre
    std::vector<glm::vec3> indexed_normals;  // Normale des triangles
    std::vector<unsigned int> property;

    // transform the gmap into set of triangles
    GMap::idlist_t vhandler = gmap.elements(0);

    std::unordered_map<id_t,id_t> vertex_index;

    //Pour chaque brin lié à un sommet
    for (size_t i = 0; i < vhandler.size(); i++) {
        vhandler[i] = gmap.get_embedding_dart( vhandler[i] );
        vertex_index[vhandler[i]] = indexed_vertices.size();
        indexed_vertices.push_back( gmap.get_position( vhandler[i] ) );
    }

    GMap::idlist_t fdarts = gmap.elements(2);

    // Pour chaque brin par face
    for (size_t i = 0; i < fdarts.size(); i++){
        GMap::idlist_t orbit_i_darts = gmap.orderedorbit( {0,1}, fdarts[i]);

        std::vector<unsigned short> findices;
        for (size_t j = 0; j < orbit_i_darts.size(); j+=2){
            GMap::id_t fvert = gmap.get_embedding_dart( orbit_i_darts[j] );
            // unsigned short ifvert = std::distance(vhandler.begin(), std::find(vhandler.begin(), vhandler.end(), fvert));
            findices.push_back(vertex_index[fvert]);
        }
        if (findices.size() >= 3){
            for(std::vector<unsigned short>::const_iterator itI = findices.begin()+1 ; itI != findices.end()-1 ; ++itI ){
                indices.push_back(findices[0]);
                indices.push_back(*itI);
                indices.push_back(*(itI+1));
            }

        }
    }

    // Calcul des normales
    indexed_normals.resize(indexed_vertices.size(), glm::vec3(0.));
    std::vector<std::vector<unsigned short> > triangles;
    for (size_t i = 0; i < indices.size(); i+=3){
        std::vector<unsigned short> Tp;
        Tp.push_back( indices[i] );
        Tp.push_back( indices[i+1] );
        Tp.push_back( indices[i+2] );
        triangles.push_back(Tp);
    }
    compute_smooth_vertex_normals(0, indexed_normals, indexed_vertices, triangles);

    // for the property, you can first use a random value for each vertices.
    property.resize(indexed_vertices.size(), static_cast <float> (rand()) / static_cast <float> (RAND_MAX));

    for( unsigned int i = 0 ; i < indexed_vertices.size() ; i++ ){
        printf("%i:%f %f %f \n",i, indexed_vertices[i][0],indexed_vertices[i][1],indexed_vertices[i][2]);
    }

    for( unsigned int i = 0 ; i < indices.size()/3 ; i++ ){
        printf("%i:%i %i %i \n",i, indices[3*i],indices[3*i+1],indices[3*i+2]);
    }

    for( unsigned int i = 0 ; i < indexed_normals.size() ; i++ ){
        printf("%i:%f %f %f \n",i, indexed_normals[i][0],indexed_normals[i][1],indexed_normals[i][2]);
    }

    glm::vec3 bb_min( FLT_MAX, FLT_MAX, FLT_MAX );
    glm::vec3 bb_max( FLT_MIN, FLT_MIN, FLT_MIN );

    //Calcul de la boite englobante du modèle
    for( unsigned int i = 0 ; i < indexed_vertices.size() ; i++ ){
        bb_min = glm::min(bb_min, indexed_vertices[i]);
        bb_max = glm::max(bb_max, indexed_vertices[i]);
    }

    glm::vec3 size = bb_max - bb_min;
    glm::vec3 center = glm::vec3(bb_min.x + size.x/2, bb_min.y + size.y/2 , bb_min.z + size.z/2 );
    float model_scale = 2.0/std::max( std::max(size.x, size.y), size.z );


    //****************************************************************/

    // Load it into a VBO

    GLuint vertexbuffer;
    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, indexed_vertices.size() * sizeof(glm::vec3), &indexed_vertices[0], GL_STATIC_DRAW);

    GLuint normalbuffer;
    glGenBuffers(1, &normalbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
    glBufferData(GL_ARRAY_BUFFER, indexed_normals.size() * sizeof(glm::vec3), &indexed_normals[0], GL_STATIC_DRAW);

    GLuint propertybuffer;
    glGenBuffers(1, &propertybuffer);
    glBindBuffer(GL_ARRAY_BUFFER, propertybuffer);
    glBufferData(GL_ARRAY_BUFFER, property.size() * sizeof(float), &property[0], GL_STATIC_DRAW);

    // Generate a buffer for the indices as well
    GLuint elementbuffer;
    glGenBuffers(1, &elementbuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned short), &indices[0] , GL_STATIC_DRAW);

    // Get a handle for our "LightPosition" uniform
    glUseProgram(programID);
    GLuint LightID = glGetUniformLocation(programID, "LightPosition_worldspace");

    // For speed computation
    double lastTime = glfwGetTime();
    int nbFrames = 0;

    do{

        // Measure speed
        double currentTime = glfwGetTime();
        nbFrames++;
        if ( currentTime - lastTime >= 1.0 ){ // If last prinf() was more than 1sec ago
            // printf and reset
            printf("%f ms/frame\n", 1000.0/double(nbFrames));
            nbFrames = 0;
            lastTime += 1.0;
        }

        // Clear the screen
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Use our shader
        glUseProgram(programID);

        // Projection matrix : 45 Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
        glm::mat4 ProjectionMatrix = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 100.0f);
        // Camera matrix
        glm::mat4 ViewMatrix       = glm::lookAt(
                    glm::vec3(0,0,5), // Camera is at (4,3,3), in World Space
                    glm::vec3(0,0,0), // and looks at the origin
                    glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
                    );
        // Model matrix : an identity matrix (model will be at the origin)
        glm::mat4 ModelMatrix      = glm::scale(glm::mat4(1.0f), glm::vec3(model_scale))*glm::translate(glm::mat4(1.0f), glm::vec3(-center.x, -center.y, -center.z));
        glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

        // Send our transformation to the currently bound shader,
        // in the "MVP" uniform
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
        glUniformMatrix4fv(ModelMatrixID, 1, GL_FALSE, &ModelMatrix[0][0]);
        glUniformMatrix4fv(ViewMatrixID, 1, GL_FALSE, &ViewMatrix[0][0]);

        glm::vec3 lightPos = glm::vec3(4,4,4);
        glUniform3f(LightID, lightPos.x, lightPos.y, lightPos.z);

        // 1rst attribute buffer : vertices
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
        glVertexAttribPointer(
                    0,                  // attribute
                    3,                  // size
                    GL_FLOAT,           // type
                    GL_FALSE,           // normalized?
                    0,                  // stride
                    (void*)0            // array buffer offset
                    );


        // 3rd attribute buffer : normals
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
        glVertexAttribPointer(
                    1,                                // attribute
                    3,                                // size
                    GL_FLOAT,                         // type
                    GL_FALSE,                         // normalized?
                    0,                                // stride
                    (void*)0                          // array buffer offset
                    );

        // 4th attribute buffer : property
        glEnableVertexAttribArray(2);
        glBindBuffer(GL_ARRAY_BUFFER, propertybuffer);
        glVertexAttribPointer(
                    2,                                // attribute
                    1,                                // size
                    GL_FLOAT,                         // type
                    GL_FALSE,                         // normalized?
                    0,                                // stride
                    (void*)0                          // array buffer offset
                    );

        // Index buffer
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);

        // Draw the triangles !
        glDrawElements(
                    GL_TRIANGLES,      // mode
                    indices.size(),    // count
                    GL_UNSIGNED_SHORT,   // type
                    (void*)0           // element array buffer offset
                    );

        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
        glDisableVertexAttribArray(2);

        // Swap buffers
        glfwSwapBuffers(window);
        glfwPollEvents();

    } // Check if the ESC key was pressed or the window was closed
    while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
           glfwWindowShouldClose(window) == 0 );

    // Cleanup VBO and shader
    glDeleteBuffers(1, &vertexbuffer);
    glDeleteBuffers(1, &normalbuffer);
    glDeleteBuffers(1, &propertybuffer);
    glDeleteBuffers(1, &elementbuffer);
    glDeleteProgram(programID);
    glDeleteVertexArrays(1, &VertexArrayID);

    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    return 0;
}

