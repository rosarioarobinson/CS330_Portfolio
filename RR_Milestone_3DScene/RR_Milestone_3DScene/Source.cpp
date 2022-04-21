/*
* Rosario Robinson
* CS 330: Comp Graphic and Visualization
* Project Submission
* April 16th, 2022
*/

// Code adapted from CS 330 GitHub Modules 02 and 03

#include <iostream>         // cout, cerr
#include <cstdlib>          // EXIT_FAILURE
#include <GL/glew.h>        // GLEW library
#include <GLFW/glfw3.h>     // GLFW library
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h> // Image loading Utility functions

// GLM Math Header inclusions
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Created a camera header class to implement camera code 
#include <Camera.h> // Camera class

using namespace std; // Standard namespace

/*Shader program Macro*/
#ifndef GLSL
#define GLSL(Version, Source) "#version " #Version " core \n" #Source
#endif

// Unnamed namespace
namespace
{
    const char* const WINDOW_TITLE = "Milestone: 3D Scene"; // Macro for window title

    // Variables for window width and height
    const int WINDOW_WIDTH = 800;
    const int WINDOW_HEIGHT = 600;

    // Stores the GL data relative to a given mesh
    struct GLMesh
    {
        GLuint vao;         // Handle for the vertex array object
        //GLuint vao2;        // Handle for the vertex array object (cube/rectangle)
        GLuint vbo;     // Handles for the vertex buffer object
        //GLuint vbos2[2];    // Handles for the vertex buffer object (sphere)
        GLuint nVertices;    // Number of vertices of the mesh
       // GLuint rectangleIndices;
        //GLuint sphereIndices; 
    };

    // Main GLFW window
    GLFWwindow* gWindow = nullptr;
    // Triangle mesh data
    GLMesh gMesh;
    // Texture
    GLuint gTextureId, gTexture1Id;
    glm::vec2 gUVScale(5.0f, 5.0f);
    GLint gTexWrapMode = GL_REPEAT;

    // Shader programs
    GLuint gProgramId;
    GLuint gSphereId;
    GLuint gCubeProgramId;
    GLuint gLampProgramId;


    // added camera and timing to fulfill requirements for assignment
    // code provided from tutorial 4-3 in CS-330 GitHub
    // camera
    glm::vec3 gCameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
    glm::vec3 gCameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 gCameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

    Camera gCamera(glm::vec3(0.0f, 0.0f, 3.0f));
    float gLastX = WINDOW_WIDTH / 2.0f;
    float gLastY = WINDOW_HEIGHT / 2.0f;
    bool gFirstMouse = true;

    // timing
    float gDeltaTime = 0.0f; // time between current frame and last frame
    float gLastFrame = 0.0f;

    // Subject position and scale
    glm::vec3 gCubePosition(0.0f, 0.0f, 0.0f);
    glm::vec3 gCubeScale(2.0f);

    // Cube and light color
    //m::vec3 gObjectColor(0.6f, 0.5f, 0.75f);
    //glm::vec3 gObjectColor(1.f, 0.2f, 0.0f);
    glm::vec3 gLightColor(1.0f, 1.0f, 1.0f);

    // Light position and scale
    glm::vec3 gLightPosition(1.5f, 0.5f, 3.0f);
    glm::vec3 gLightScale(0.5f);

}

// Source code: https://www.codeproject.com/Articles/67357/OpenGL-Oval
// Was not able to implement this code properly 

#define PI		3.141592654
#define PI_2	PI * 2

typedef enum Shape
{
    DRAW_OVAL,
    FILL_OVAL,
    DRAW_ARC,
    FILL_ARC
} Shape;

Shape currentShape = FILL_OVAL;

float color[sizeof(Shape)];


/* User-defined Function prototypes to:
 * initialize the program, set the window size,
 * redraw graphics on the window when resized,
 * and render graphics on the screen
 */
bool UInitialize(int, char* [], GLFWwindow** window);
void UResizeWindow(GLFWwindow* window, int width, int height);
void UProcessInput(GLFWwindow* window);
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos);
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset);
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void UCreateMesh(GLMesh& mesh);
void UDestroyMesh(GLMesh& mesh);
bool UCreateTexture(const char* filename, GLuint& textureId);
void UDestroyTexture(GLuint textureId);
void URender();
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId);
void UDestroyShaderProgram(GLuint programId);

void fillOval(float x_center, float y_center, float w, float h, int n);

// Tried to draw oval, would not display when program runs
void drawOval(float x_center, float y_center, float w, float h, int n)
{
    float theta, angle_increment;
    float x, y;
    if (n <= 0)
        n = 1;
    angle_increment = PI_2 / n;
    glPushMatrix();

    //  center the oval at x_center, y_center
    glTranslatef(x_center, y_center, 0);
    //  draw the oval using line segments
    glBegin(GL_LINE_LOOP);

    for (theta = 0.0f; theta < PI_2; theta += angle_increment)
    {
        x = w / 2 * cos(theta);
        y = h / 2 * sin(theta);

        //  Color the vertices!
        if (color[currentShape])
            glColor3f(x, y, x * y);
        glVertex2f(x, y);
    }
    glEnd();
    glPopMatrix();
}

/* Cube Vertex Shader Source Code*/
const GLchar* cubeVertexShaderSource = GLSL(440,

    layout(location = 0) in vec3 position; // VAP position 0 for vertex position data
    layout(location = 1) in vec3 normal; // VAP position 1 for normals
    layout(location = 2) in vec2 textureCoordinate;

    out vec3 vertexNormal; // For outgoing normals to fragment shader
    out vec3 vertexFragmentPos; // For outgoing color / pixels to fragment shader
    out vec2 vertexTextureCoordinate;

    //Uniform / Global variables for the  transform matrices
    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;

void main()
    {
        gl_Position = projection * view * model * vec4(position, 1.0f); // Transforms vertices into clip coordinates

        vertexFragmentPos = vec3(model * vec4(position, 1.0f)); // Gets fragment / pixel position in world space only (exclude view and projection)

        vertexNormal = mat3(transpose(inverse(model))) * normal; // get normal vectors in world space only and exclude normal translation properties
        vertexTextureCoordinate = textureCoordinate;
    }
);


/* Cube Fragment Shader Source Code*/
const GLchar* cubeFragmentShaderSource = GLSL(440,

    in vec3 vertexNormal; // For incoming normals
    in vec3 vertexFragmentPos; // For incoming fragment position
    in vec2 vertexTextureCoordinate;

    out vec4 fragmentColor; // For outgoing cube color to the GPU

    // Uniform / Global variables for object color, light color, light position, and camera/view position
    uniform vec3 objectColor;
    uniform vec3 lightColor;
    uniform vec3 lightPos;
    uniform vec3 viewPosition;
    uniform sampler2D uTexture; // Useful when working with multiple textures
    uniform vec2 uvScale;

void main()
    {
        /*Phong lighting model calculations to generate ambient, diffuse, and specular components*/

        //Calculate Ambient lighting*/
        float ambientStrength = 0.1f; // Set ambient or global lighting strength
        vec3 ambient = ambientStrength * lightColor; // Generate ambient light color

        //Calculate Diffuse lighting*/
        vec3 norm = normalize(vertexNormal); // Normalize vectors to 1 unit
        vec3 lightDirection = normalize(lightPos - vertexFragmentPos); // Calculate distance (light direction) between light source and fragments/pixels on cube
        float impact = max(dot(norm, lightDirection), 0.0);// Calculate diffuse impact by generating dot product of normal and light
        vec3 diffuse = impact * lightColor; // Generate diffuse light color

        //Calculate Specular lighting*/
        float specularIntensity = 0.8f; // Set specular light strength
        float highlightSize = 16.0f; // Set specular highlight size
        vec3 viewDir = normalize(viewPosition - vertexFragmentPos); // Calculate view direction
        vec3 reflectDir = reflect(-lightDirection, norm);// Calculate reflection vector
        //Calculate specular component
        float specularComponent = pow(max(dot(viewDir, reflectDir), 0.0), highlightSize);
        vec3 specular = specularIntensity * specularComponent * lightColor;

        // Texture holds the color to be used for all three components
        vec4 textureColor = texture(uTexture, vertexTextureCoordinate * uvScale);

        // Calculate phong result
        vec3 phong = (ambient + diffuse + specular) * textureColor.xyz;

        fragmentColor = vec4(phong, 1.0); // Send lighting results to GPU
    }
);


/* Lamp Shader Source Code*/
const GLchar* lampVertexShaderSource = GLSL(440,

    layout(location = 0) in vec3 position; // VAP position 0 for vertex position data

    //Uniform / Global variables for the  transform matrices
    uniform mat4 model;
    uniform mat4 view;
    uniform mat4 projection;

    void main()
    {
        gl_Position = projection * view * model * vec4(position, 1.0f); // Transforms vertices into clip coordinates
    }
);


/* Fragment Shader Source Code*/
const GLchar* lampFragmentShaderSource = GLSL(440,

    out vec4 fragmentColor; // For outgoing lamp color (smaller cube) to the GPU

void main()
    {
        fragmentColor = vec4(1.0f); // Set color to white (1.0f,1.0f,1.0f) with alpha 1.0
    }
);

// Code adapted from SNHU-CS 330 GitHub 
// Images are loaded with Y axis going down, but OpenGL's Y axis goes up, so let's flip it
void flipImageVertically(unsigned char* image, int width, int height, int channels)
{
    for (int j = 0; j < height / 2; ++j)
    {
        int index1 = j * width * channels;
        int index2 = (height - 1 - j) * width * channels;

        for (int i = width * channels; i > 0; --i)
        {
            unsigned char tmp = image[index1];
            image[index1] = image[index2];
            image[index2] = tmp;
            ++index1;
            ++index2;
        }
    }
}

int main(int argc, char* argv[])
{
    if (!UInitialize(argc, argv, &gWindow))
        return EXIT_FAILURE;

    // Create the mesh
    UCreateMesh(gMesh); // Calls the function to create the Vertex Buffer Object

    // Create the shader programs
    if (!UCreateShaderProgram(cubeVertexShaderSource, cubeFragmentShaderSource, gCubeProgramId))
        return EXIT_FAILURE;

    if (!UCreateShaderProgram(lampVertexShaderSource, lampFragmentShaderSource, gLampProgramId))
        return EXIT_FAILURE;

    // Sets the background color of the window to black (it will be implicitely used by glClear)
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // Load texture
    const char* texWoodFilename = "resources/woodgrain.jpg";
    // Tried to add additional texture to source
    const char* texCorkFilename = "resources/corkboard.jpg";

    if (!UCreateTexture(texWoodFilename, gTextureId))
    {
        cout << "Failed to load texture " << texWoodFilename << endl;
        return EXIT_FAILURE;
    }

    if (!UCreateTexture(texCorkFilename, gTexture1Id))
    {
        cout << "Failed to load texture " << texCorkFilename << endl;
        return EXIT_FAILURE;
    }
    // tell opengl for each sampler to which texture unit it belongs to (only has to be done once)
    glUseProgram(gProgramId);
    // We set the texture as texture unit 0
    glUniform1i(glGetUniformLocation(gProgramId, "uTexture"), 0);

    // Sets the background color of the window to black (it will be implicitely used by glClear)
    //glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    // Added to assign to one 3D shape
    // Was not sure how to implement this code to focus on one 3D shape at a time

    /*unsigned int texture1, texture2;
    // texture 1

    glGenTextures(1, &texture1);
    glBindTexture(GL_TEXTURE_2D, texture1);
    // set the texture wrapping parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    // set the texture filtering parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //load image, create texture and generate mipmaps
    int width, height, nrChannels;
    stbi_set_flip_vertically_on_load(true); // tell stb_image.h to flip loaded texture's on the y-axis.
    unsigned char* data = stbi_load("resources/woodgrain.jpg", &width, &height, &nrChannels, 0);
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    }
    else {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);

    // tell opengl for each sampler to which texture unit it belongs to (only has to be done once)

    ourShader.use();
    ourShader.setInt("texture1", 0); */

    // render loop
    // -----------
    while (!glfwWindowShouldClose(gWindow))
    {
        // per-frame timing
        // --------------------
        float currentFrame = glfwGetTime();
        gDeltaTime = currentFrame - gLastFrame;
        gLastFrame = currentFrame;

        // input
        // -----
        UProcessInput(gWindow);

        // Render this frame
        URender();

        glfwPollEvents();
    }

    // Release mesh data
    UDestroyMesh(gMesh);

    // Release textures
    UDestroyTexture(gTextureId);
    UDestroyTexture(gTexture1Id);

    // Release shader programs
    UDestroyShaderProgram(gProgramId);
    UDestroyShaderProgram(gSphereId);
    UDestroyShaderProgram(gCubeProgramId);
    UDestroyShaderProgram(gLampProgramId);

    exit(EXIT_SUCCESS); // Terminates the program successfully
}


// Initialize GLFW, GLEW, and create a window
bool UInitialize(int argc, char* argv[], GLFWwindow** window)
{
    // GLFW: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // GLFW: window creation
    // ---------------------
    * window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, WINDOW_TITLE, NULL, NULL);
    if (*window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(*window);
    glfwSetFramebufferSizeCallback(*window, UResizeWindow);
    glfwSetCursorPosCallback(*window, UMousePositionCallback);
    glfwSetScrollCallback(*window, UMouseScrollCallback);
    glfwSetMouseButtonCallback(*window, UMouseButtonCallback);

    // tell GLFW to capture our mouse
    glfwSetInputMode(*window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    // GLEW: initialize
    // ----------------
    // Note: if using GLEW version 1.13 or earlier
    glewExperimental = GL_TRUE;
    GLenum GlewInitResult = glewInit();

    if (GLEW_OK != GlewInitResult)
    {
        std::cerr << glewGetErrorString(GlewInitResult) << std::endl;
        return false;
    }

    // Displays GPU OpenGL version
    cout << "INFO: OpenGL Version: " << glGetString(GL_VERSION) << endl;

    return true;
}


// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void UProcessInput(GLFWwindow* window)
{
    static const float cameraSpeed = 2.5f;

    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    float cameraOffset = cameraSpeed * gDeltaTime;

    // added WSAD keys to control forward, backward, left, and right movements
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        gCamera.ProcessKeyboard(Camera_Movement::FORWARD, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        gCamera.ProcessKeyboard(Camera_Movement::BACKWARD, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        gCamera.ProcessKeyboard(Camera_Movement::LEFT, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        gCamera.ProcessKeyboard(Camera_Movement::RIGHT, gDeltaTime);

    // added QE keys to control upward and downward movements
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
        gCamera.ProcessKeyboard(Camera_Movement::DOWN, gDeltaTime);
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
        gCamera.ProcessKeyboard(Camera_Movement::UP, gDeltaTime);

    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS && gTexWrapMode != GL_REPEAT)
    {
        glBindTexture(GL_TEXTURE_2D, gTextureId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glBindTexture(GL_TEXTURE_2D, 0);

        gTexWrapMode = GL_REPEAT;

        cout << "Current Texture Wrapping Mode: REPEAT" << endl;
    }
    else if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS && gTexWrapMode != GL_MIRRORED_REPEAT)
    {
        glBindTexture(GL_TEXTURE_2D, gTextureId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
        glBindTexture(GL_TEXTURE_2D, 0);

        gTexWrapMode = GL_MIRRORED_REPEAT;

        cout << "Current Texture Wrapping Mode: MIRRORED REPEAT" << endl;
    }
    else if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS && gTexWrapMode != GL_CLAMP_TO_EDGE)
    {
        glBindTexture(GL_TEXTURE_2D, gTextureId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D, 0);

        gTexWrapMode = GL_CLAMP_TO_EDGE;

        cout << "Current Texture Wrapping Mode: CLAMP TO EDGE" << endl;
    }
    else if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS && gTexWrapMode != GL_CLAMP_TO_BORDER)
    {
        float color[] = { 1.0f, 0.0f, 1.0f, 1.0f };
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, color);

        glBindTexture(GL_TEXTURE_2D, gTextureId);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glBindTexture(GL_TEXTURE_2D, 0);

        gTexWrapMode = GL_CLAMP_TO_BORDER;

        cout << "Current Texture Wrapping Mode: CLAMP TO BORDER" << endl;
    }

    if (glfwGetKey(window, GLFW_KEY_RIGHT_BRACKET) == GLFW_PRESS)
    {
        gUVScale += 0.1f;
        cout << "Current scale (" << gUVScale[0] << ", " << gUVScale[1] << ")" << endl;
    }
    else if (glfwGetKey(window, GLFW_KEY_LEFT_BRACKET) == GLFW_PRESS)
    {
        gUVScale -= 0.1f;
        cout << "Current scale (" << gUVScale[0] << ", " << gUVScale[1] << ")" << endl;
    }
}

// tried adding UpdateProjection to attempt toggling between ortho and perspective 
// Source code from: https://www.cprogramming.com/tutorial/opengl_projections.html
/*void UpdateProjection(GLboolean toggle)
{
    static GLboolean s_usePerspective = GL_TRUE;

    // toggle the control variable if appropriate
    if (toggle == GLFW_KEY_P)
        s_usePerspective = !s_usePerspective;

    // select the projection matrix and clear it out
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // choose the appropriate projection based on the currently toggled mode
    if (s_usePerspective)
    {
        // set the perspective with the appropriate aspect ratio
        glFrustum(-1.0, 1.0, -1.0, 1.0, 5, 100);
    }
    else
    {
        // set up an orthographic projection with the same near clip plane
        glOrtho(-1.0, 1.0, -1.0, 1.0, 5, 100);
    }

    // select modelview matrix and clear it out
    glMatrixMode(GL_MODELVIEW);
}*/

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void UResizeWindow(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);

}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void UMousePositionCallback(GLFWwindow* window, double xpos, double ypos)
{
    if (gFirstMouse)
    {
        gLastX = xpos;
        gLastY = ypos;
        gFirstMouse = false;
    }

    float xoffset = xpos - gLastX;
    float yoffset = gLastY - ypos; // reversed since y-coordinates go from bottom to top

    gLastX = xpos;
    gLastY = ypos;

    gCamera.ProcessMouseMovement(xoffset, yoffset);
}


// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void UMouseScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    gCamera.ProcessMouseScroll(yoffset);
}

// glfw: handle mouse button events
// --------------------------------
void UMouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    switch (button)
    {
    case GLFW_MOUSE_BUTTON_LEFT:
    {
        if (action == GLFW_PRESS)
            cout << "Left mouse button pressed" << endl;
        else
            cout << "Left mouse button released" << endl;
    }
    break;

    case GLFW_MOUSE_BUTTON_MIDDLE:
    {
        if (action == GLFW_PRESS)
            cout << "Middle mouse button pressed" << endl;
        else
            cout << "Middle mouse button released" << endl;
    }
    break;

    case GLFW_MOUSE_BUTTON_RIGHT:
    {
        if (action == GLFW_PRESS)
            cout << "Right mouse button pressed" << endl;
        else
            cout << "Right mouse button released" << endl;
    }
    break;

    default:
        cout << "Unhandled mouse button event" << endl;
        break;
    }
}

// Functioned called to render a frame
void URender()
{
    // Enable z-depth
    glEnable(GL_DEPTH_TEST);

    // Clear the frame and z buffers
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Activate the cube VAO (used by cube and lamp)
    glBindVertexArray(gMesh.vao);

    // CUBE: draw cube
    //----------------
    // Set the shader to be used
    glUseProgram(gCubeProgramId);

    // Model matrix: transformations are applied right-to-left order
    glm::mat4 model = glm::translate(gCubePosition) * glm::scale(gCubeScale);

    // camera/view transformation
    glm::mat4 view = gCamera.GetViewMatrix();

    // Creates a perspective projection
    glm::mat4 projection = glm::perspective(glm::radians(gCamera.Zoom), (GLfloat)WINDOW_WIDTH / (GLfloat)WINDOW_HEIGHT, 0.1f, 100.0f);

    // Retrieves and passes transform matrices to the Shader program
    GLint modelLoc = glGetUniformLocation(gCubeProgramId, "model");
    GLint viewLoc = glGetUniformLocation(gCubeProgramId, "view");
    GLint projLoc = glGetUniformLocation(gCubeProgramId, "projection");

    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // Reference matrix uniforms from the Cube Shader program for the cub color, light color, light position, and camera position
    GLint objectColorLoc = glGetUniformLocation(gCubeProgramId, "objectColor");
    GLint lightColorLoc = glGetUniformLocation(gCubeProgramId, "lightColor");
    GLint lightPositionLoc = glGetUniformLocation(gCubeProgramId, "lightPos");
    GLint viewPositionLoc = glGetUniformLocation(gCubeProgramId, "viewPosition");

    // Pass color, light, and camera data to the Cube Shader program's corresponding uniforms
    //glUniform3f(objectColorLoc, gObjectColor.r, gObjectColor.g, gObjectColor.b);
    glUniform3f(lightColorLoc, gLightColor.r, gLightColor.g, gLightColor.b);
    glUniform3f(lightPositionLoc, gLightPosition.x, gLightPosition.y, gLightPosition.z);
    const glm::vec3 cameraPosition = gCamera.Position;
    glUniform3f(viewPositionLoc, cameraPosition.x, cameraPosition.y, cameraPosition.z);

    GLint UVScaleLoc = glGetUniformLocation(gCubeProgramId, "uvScale");
    glUniform2fv(UVScaleLoc, 1, glm::value_ptr(gUVScale));

    // bind textures on corresponding texture units
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, gTextureId);

    // Draws the triangles
    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices);

    // LAMP: draw lamp
    //----------------
    glUseProgram(gLampProgramId);

    //Transform the smaller cube used as a visual que for the light source
    model = glm::translate(gLightPosition) * glm::scale(gLightScale);

    // Reference matrix uniforms from the Lamp Shader program
    modelLoc = glGetUniformLocation(gLampProgramId, "model");
    viewLoc = glGetUniformLocation(gLampProgramId, "view");
    projLoc = glGetUniformLocation(gLampProgramId, "projection");

    // Pass matrix data to the Lamp Shader program's matrix uniforms
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glDrawArrays(GL_TRIANGLES, 0, gMesh.nVertices);

    // Deactivate the Vertex Array Object and shader program
    glBindVertexArray(0);
    glUseProgram(0);

    // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
    glfwSwapBuffers(gWindow);    // Flips the the back buffer with the front buffer every frame.
}


// Implements the UCreateMesh function
void UCreateMesh(GLMesh& mesh)
{
    // Included source code for information on how to create a rectangle and sphere

    // Source code: https://openglbook.com/chapter-2-vertices-and-shapes.html
    // Source to understand vertices for a rectangle:
    // https://collectivesolver.com/44565/how-to-draw-a-rectangle-in-opengl-with-glfw-glad-shaders-and-ebo-index-buffer-in-c
    
    // Position and Color data
    GLfloat verts[] = {
        // 3D Rectangle
        // Vertex Positions        // Colors (r,g,b,a)
        /*0.1f,  0.3f, 0.0f,  1.0f, 0.0f, 0.0f, 1.0f,
        0.1f, -0.3f, 0.0f,   0.0f, 1.0f, 0.0f, 1.0f,
        -0.1f, -0.3f, 0.0f,  0.0f, 0.0f, 1.0f, 1.0f,
        -0.1f,  0.3f, 0.0f,   1.0f, 1.0f, 1.0f, 1.0f,

        0.1f, -0.3f, -1.0f,  0.5f, 0.5f, 1.0f, 1.0f, // 4 br  right
        0.1f,  0.3f, -1.0f,  1.0f, 1.0f, 0.5f, 1.0f, //  5 tl  right
        -0.1f,  0.3f, -1.0f,  0.2f, 0.2f, 0.5f, 1.0f, //  6 tl  top
        -0.1f, -0.3f, -1.0f,  1.0f, 0.0f, 1.0f, 1.0f,  */
        //  7 bl back

        // Rectangle for wooden spoon
         //Positions          //Texture Coordinates
        -0.1f, -0.1f, -0.2f,  0.0f, 0.0f,
         0.1f, -0.1f, -0.2f,  1.0f, 0.0f,
         0.1f,  0.1f, -0.2f,  1.0f, 1.0f,
         0.1f,  0.1f, -0.2f,  1.0f, 1.0f,
        -0.1f,  0.1f, -0.2f,  0.0f, 1.0f,
        -0.1f, -0.1f, -0.2f,  0.0f, 0.0f,

        -0.1f, -0.1f,  0.2f,  0.0f, 0.0f,
         0.1f, -0.1f,  0.2f,  1.0f, 0.0f,
         0.1f,  0.1f,  0.2f,  1.0f, 1.0f,
         0.1f,  0.1f,  0.2f,  1.0f, 1.0f,
        -0.1f,  0.1f,  0.2f,  0.0f, 1.0f,
        -0.1f, -0.1f,  0.2f,  0.0f, 0.0f,

        -0.1f,  0.1f,  0.2f,  1.0f, 0.0f,
        -0.1f,  0.1f, -0.2f,  1.0f, 1.0f,
        -0.1f, -0.1f, -0.2f,  0.0f, 1.0f,
        -0.1f, -0.1f, -0.2f,  0.0f, 1.0f,
        -0.1f, -0.1f,  0.2f,  0.0f, 0.0f,
        -0.1f,  0.1f,  0.2f,  1.0f, 0.0f,

         0.1f,  0.1f,  0.2f,  1.0f, 0.0f,
         0.1f,  0.1f, -0.2f,  1.0f, 1.0f,
         0.1f, -0.1f, -0.2f,  0.0f, 1.0f,
         0.1f, -0.1f, -0.2f,  0.0f, 1.0f,
         0.1f, -0.1f,  0.2f,  0.0f, 0.0f,
         0.1f,  0.1f,  0.2f,  1.0f, 0.0f,

        -0.1f, -0.1f, -0.2f,  0.0f, 1.0f,
         0.1f, -0.1f, -0.2f,  1.0f, 1.0f,
         0.1f, -0.1f,  0.2f,  1.0f, 0.0f,
         0.1f, -0.1f,  0.2f,  1.0f, 0.0f,
        -0.1f, -0.1f,  0.2f,  0.0f, 0.0f,
        -0.1f, -0.1f, -0.2f,  0.0f, 1.0f,

        -0.1f,  0.1f, -0.2f,  0.0f, 1.0f,
         0.1f,  0.1f, -0.2f,  1.0f, 1.0f,
         0.1f,  0.1f,  0.2f,  1.0f, 0.0f,
         0.1f,  0.1f,  0.2f,  1.0f, 0.0f,
        -0.1f,  0.1f,  0.2f,  0.0f, 0.0f,
        -0.1f,  0.1f, -0.2f,  0.0f, 1.0f,


        // 3d Book 
        //Positions          //Texture Coordinates
        -0.5f, -0.3f, -0.3f, 1.0f, 0.0f, 
         0.5f, -0.3f, -0.3f, 1.0f, 0.0f, 
         0.5f,  0.3f, -0.3f, 1.0f, 0.0f,
         0.5f,  0.3f, -0.3f, 1.0f, 0.0f, 
        -0.5f,  0.3f, -0.3f, 1.0f, 0.0f,
        -0.5f, -0.3f, -0.3f, 1.0f, 0.0f, 

        -0.5f, -0.3f,  0.3f, 0.0f, 1.0f, 
         0.5f, -0.3f,  0.3f, 0.0f, 1.0f, 
         0.5f,  0.3f,  0.3f, 0.0f, 1.0f, 
         0.5f,  0.3f,  0.3f, 0.0f, 1.0f, 
        -0.5f,  0.3f,  0.3f, 0.0f, 1.0f,
        -0.5f, -0.3f,  0.3f, 0.0f, 1.0f, 

        -0.5f,  0.3f,  0.3f, 0.0f, 0.0f,
        -0.5f,  0.3f, -0.3f, 0.0f, 0.0f, 
        -0.5f, -0.3f, -0.3f, 0.0f, 0.0f,
        -0.5f, -0.3f, -0.3f, 0.0f, 0.0f, 
        -0.5f, -0.3f,  0.3f, 0.0f, 0.0f, 
        -0.5f,  0.3f,  0.3f, 0.0f, 0.0f, 

         0.5f,  0.3f,  0.3f, 1.0f, 1.0f, 
         0.5f,  0.3f, -0.3f, 1.0f, 1.0f, 
         0.5f, -0.3f, -0.3f, 1.0f, 1.0f, 
         0.5f, -0.3f, -0.3f, 1.0f, 1.0f,
         0.5f, -0.3f,  0.3f, 1.0f, 1.0f, 
         0.5f,  0.3f,  0.3f, 1.0f, 1.0f, 
       
        -0.5f, -0.3f, -0.3f, 0.0f, 1.0f, 
         0.5f, -0.3f, -0.3f, 0.0f, 1.0f, 
         0.5f, -0.3f,  0.3f, 0.0f, 1.0f, 
         0.5f, -0.3f,  0.3f, 0.0f, 1.0f, 
        -0.5f, -0.3f,  0.3f, 0.0f, 1.0f, 
        -0.5f, -0.3f, -0.3f, 0.0f, 1.0f,

        -0.5f,  0.3f, -0.3f, 1.0f, 0.0f, 
         0.5f,  0.3f, -0.3f, 1.0f, 0.0f,
         0.5f,  0.3f,  0.3f, 1.0f, 0.0f, 
         0.5f,  0.3f,  0.3f, 1.0f, 0.0f,
        -0.5f,  0.3f,  0.3f, 1.0f, 0.0f, 
        -0.5f,  0.3f, -0.3f, 1.0f, 0.0f,

        // Attempt to draw circle to represent coaster
          0.0, 0.0, 1.0, 100,

        // Vertices for Plane
        // Will not properly display, shows as two separate triangles
         -2.0f, 0.0f, 2.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f,
         -2.0f, 0.0f, -2.0f, 1.0f, 0.0f, 0.1f, 1.0f, 0.0f,
         2.0f, 0.0f, -2.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f,
         2.0f, 0.0f, 2.0f, 1.0f, 0.0f, 1.0f, 1.0f, 1.0f,
         2.0f, 0.0f, 2.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f,

    };


    const GLuint floatsPerVertex = 3;
    const GLuint floatsPerNormal = 3;
    const GLuint floatsPerUV = 2;

    mesh.nVertices = sizeof(verts) / (sizeof(verts[0]) * (floatsPerVertex + floatsPerUV));

    glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao);

    // Create VBO
    glGenBuffers(1, &mesh.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vbo); // Activates the buffer
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW); // Sends vertex or coordinate data to the GPU

    // Strides between vertex coordinates
    GLint stride = sizeof(float) * (floatsPerVertex + floatsPerUV);

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerNormal, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, floatsPerUV, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * (floatsPerVertex + floatsPerNormal)));
    glEnableVertexAttribArray(2);



    // Sphere
    // This code would not properly work, trying to render multiple shapes at once

    /*glGenVertexArrays(1, &mesh.vao); // we can also generate multiple VAOs or buffers at the same time
    glBindVertexArray(mesh.vao);

    mesh.sphereIndices = sizeof(gSphereId) / sizeof(gSphereId);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.vbos[1]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(gSphereId), gluSphere, GL_STATIC_DRAW);

    // Create Vertex Attribute Pointers
    glVertexAttribPointer(0, floatsPerVertex, GL_FLOAT, GL_FALSE, stride, 0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, floatsPerColor, GL_FLOAT, GL_FALSE, stride, (char*)(sizeof(float) * floatsPerVertex));
    glEnableVertexAttribArray(1);*/
}


// Source code: https://www.codeproject.com/Articles/67357/OpenGL-Oval
// Was not able to implement this code properly 

void fillOval(float x_center, float y_center, float w, float h, int n)
{
    float theta, angle_increment;
    float x, y;

    if (n <= 0)
        n = 1;

    angle_increment = PI_2 / n;

    glPushMatrix();

    //  center the oval at x_center, y_center
    glTranslatef(x_center, y_center, 0);

    //  draw the oval using triangle fan
    glBegin(GL_TRIANGLE_FAN);

    //  specify the center
    glVertex2f(0.0f, 0.0f);

    for (theta = 0.0f; theta - PI_2 < 0.001; theta += angle_increment)
    {
        x = w / 2 * cos(theta);
        y = h / 2 * sin(theta);

        //  Color the vertices!
        if (color[currentShape])
            glColor3f(x, y, x * y);

        glVertex2f(x, y);
    }

    glEnd();

    glPopMatrix();
}


void UDestroyMesh(GLMesh& mesh)
{
    glDeleteVertexArrays(1, &mesh.vao);
    glDeleteBuffers(1, &mesh.vbo);

    /*glDeleteVertexArrays(1, &mesh.vao2);
    glDeleteBuffers(2, mesh.vbos2);*/
}

/*Generate and load the texture*/
bool UCreateTexture(const char* filename, GLuint& textureId)
{
    int width, height, channels;
    unsigned char* image = stbi_load(filename, &width, &height, &channels, 0);
    if (image)
    {
        flipImageVertically(image, width, height, channels);

        glGenTextures(1, &textureId);
        glBindTexture(GL_TEXTURE_2D, textureId);

        // set the texture wrapping parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // set texture filtering parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        if (channels == 3)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image);
        else if (channels == 4)
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
        else
        {
            cout << "Not implemented to handle image with " << channels << " channels" << endl;
            return false;
        }

        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(image);
        glBindTexture(GL_TEXTURE_2D, 0); // Unbind the texture

        return true;
    }

    // Error loading the image
    return false;
}


void UDestroyTexture(GLuint textureId)
{
    glGenTextures(1, &textureId);
}

// Implements the UCreateShaders function
bool UCreateShaderProgram(const char* vtxShaderSource, const char* fragShaderSource, GLuint& programId)
{
    // Compilation and linkage error reporting
    int success = 0;
    char infoLog[512];

    // Create a Shader program object.
    programId = glCreateProgram();

    // Create the vertex and fragment shader objects
    GLuint vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

    // Retrive the shader source
    glShaderSource(vertexShaderId, 1, &vtxShaderSource, NULL);
    glShaderSource(fragmentShaderId, 1, &fragShaderSource, NULL);

    // Compile the vertex shader, and print compilation errors (if any)
    glCompileShader(vertexShaderId); // compile the vertex shader
    // check for shader compile errors
    glGetShaderiv(vertexShaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShaderId, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;

        return false;
    }

    glCompileShader(fragmentShaderId); // compile the fragment shader
    // check for shader compile errors
    glGetShaderiv(fragmentShaderId, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShaderId, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;

        return false;
    }

    // Attached compiled shaders to the shader program
    glAttachShader(programId, vertexShaderId);
    glAttachShader(programId, fragmentShaderId);

    glLinkProgram(programId);   // links the shader program
    // check for linking errors
    glGetProgramiv(programId, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(programId, sizeof(infoLog), NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;

        return false;
    }

    glUseProgram(programId);    // Uses the shader program

    return true;
}


void UDestroyShaderProgram(GLuint programId)
{
    glDeleteProgram(programId);
}