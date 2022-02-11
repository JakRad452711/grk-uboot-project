#include "glew.h"
#include "freeglut.h"
#include "glm.hpp"
#include "ext.hpp"
#include <iostream>
#include <cmath>
#include <vector>

#include "Shader_Loader.h"
#include "Render_Utils.h"
#include "Camera.h"
#include "Texture.h"
#include "Physics.h"

Core::Shader_Loader shaderLoader;
GLuint programColor;
GLuint programTexture;

obj::Model planeModel, boxModel, sphereModel;
Core::RenderContext planeContext, boxContext, sphereContext;
GLuint boxTexture, groundTexture;

glm::vec3 cameraPos = glm::vec3(0, 5, 20);
glm::vec3 cameraDir;
glm::vec3 cameraSide;
float cameraAngle = 0;
glm::mat4 cameraMatrix, perspectiveMatrix;

glm::vec3 lightDir = glm::normalize(glm::vec3(0.5, -1, -0.5));


// Initalization of physical scene (PhysX)
Physics pxScene(9.8 /* gravity (m/s^2) */);


// fixed timestep for stable and deterministic simulation
const double physicsStepTime = 1.f / 60.f;
double physicsTimeToProcess = 0;

// physical objects
PxRigidStatic *planeBody = nullptr;
PxMaterial *planeMaterial = nullptr;
std::vector<PxRigidDynamic*> boxBodies;
PxMaterial *boxMaterial = nullptr;
PxRigidDynamic *sphereBody = nullptr;
PxMaterial *sphereMaterial = nullptr;

// renderable objects (description of a single renderable instance)
struct Renderable {
    Core::RenderContext* context;
    glm::mat4 modelMatrix;
    GLuint textureId;
};
std::vector<Renderable*> renderables;

// number of rows and columns of boxes the wall consists of
const int wallRows = 5;
const int wallCols = 5;

void initRenderables()
{
    // load models
    planeModel = obj::loadModelFromFile("models/plane.obj");
    boxModel = obj::loadModelFromFile("models/box.obj");
    sphereModel = obj::loadModelFromFile("models/sphere.obj");

    planeContext.initFromOBJ(planeModel);
    boxContext.initFromOBJ(boxModel);
    sphereContext.initFromOBJ(sphereModel);

    // load textures
    groundTexture = Core::LoadTexture("textures/sand.jpg");
    boxTexture = Core::LoadTexture("textures/a.jpg");

    // This time we organize all the renderables in a list
    // of basic properties (model, transform, texture),
    // to unify their rendering and simplify their managament
    // in connection to the physics simulation

    // create ground
    Renderable *ground = new Renderable();
    ground->context = &planeContext;
    ground->textureId = groundTexture;
    renderables.emplace_back(ground);

    for (int i = 0; i < wallCols; i++)
        for (int j = 0; j < wallRows; j++) {
            // create box
            Renderable *box = new Renderable();
            box->context = &boxContext;
            box->textureId = boxTexture;
            renderables.emplace_back(box);
        }

    //**********************************
    // UNCOMMENT FOR THE LAST TASK !!!
    /*Renderable *sphere = new Renderable();
    sphere->model = &sphereModel;
    sphere->textureId = boxTexture;
    renderables.emplace_back(sphere);*/
}

void initPhysicsScene()
{
    //-----------------------------------------------------------
    // TASKS
    //-----------------------------------------------------------
    // IMPORTANT:
    //   * to create objects use:     pxScene.physics
    //   * to manage the scene use:   pxScene.scene
    //
    // Create a wall of boxes (use global constants wallRows and
    // wallCols). Create wallCols columns of stacked boxes, each
    // consisting of wallRows boxes. Finally, destroy the wall
    // with a ball.
    //
    // You should provide the physics initialization code below.
    // The required global variables have already been declared.
    // Note, that most of the variables are pointers, so remember
    // to use -> instead of . to access member methods/variables

    //**************************
    // I. Create plane

    // 1. Create the plane similarly as in main_8_1.cpp
    // with one difference:
    // 
    // 1.1 This time remember to set userData of the body to the pointer
    // of the corresponding renderable: renderables[0]
    // The plane still remains static, but we want to use the unified
    // organization of renderables.

    //**************************
    // II. Create wall:

    // 1. Create box material (can be reused for all the boxes).

    // 2. Create wallRows x wallCols boxes.
    // You need two nested for loops. Create each box similarly as in main_8_1.cpp
    // Pay attention to the following differences (we assume the indices
    // of for loops are i,j):
    //
    // 2.1 Save the created box bodies in a list (vector):
    // Use the global variable boxBodies
    // 
    // 2.2. Set up the box transforms to the proper position within the wall
    // (i.e. calculate position x,y depending on i,j).
    // You can offset the boxes by several centimeters, so that the wall has
    // some space to settle down (you can observe how it falls down and stabilizes).
    //
    // 2.3 Set userData of the bodies to the pointers of corresponding renderables:
    //       renderables[i*wallRows + j + 1]

    //**************************
    // III. See the function updateTransforms()
    // Now in userData we store pointers to renderables instead of matrices.
    // Notice the differences to main_8_1.cpp

    //**************************
    // IV. See also the function renderScene()
    // Notice how we render all the renderables in a single for loop.

    //**************************
    // V. Throw a ball towards the wall

    // 1. Uncomment the code creating the renderable for the sphere in initRenderables()
    // 2. Create a rigid body, similarly to boxes.
    // Use the global variables sphereBody and sphereMaterial.
    // 3. Use PxSphereGeometry( radius ) for the geometry instead of box geometry.
    // 4. Set the initial transform of the sphere to the position of camera.
    // 5. Set the linear velocity of the body to e.g. PxVec3(0,0,-30)
    // Use the method: setLinearVelocity ( velocity )
    // 6. Set userData properly (sphere is the last element in renderables).

    // VI.* Modify controls, use them to control the sphere. Attatch camera to the sphere.
}

void updateTransforms()
{
    // Here we retrieve the current transforms of the objects from the physical simulation.
    auto actorFlags = PxActorTypeFlag::eRIGID_DYNAMIC | PxActorTypeFlag::eRIGID_STATIC;
    PxU32 nbActors = pxScene.scene->getNbActors(actorFlags);
    if (nbActors)
    {
        std::vector<PxRigidActor*> actors(nbActors);
        pxScene.scene->getActors(actorFlags, (PxActor**)&actors[0], nbActors);
        for (auto actor : actors)
        {
            // We use the userData of the objects to set up the model matrices
            // of proper renderables.
            if (!actor->userData) continue;
            Renderable *renderable = (Renderable*)actor->userData;

            // get world matrix of the object (actor)
            PxMat44 transform = actor->getGlobalPose();
            auto &c0 = transform.column0;
            auto &c1 = transform.column1;
            auto &c2 = transform.column2;
            auto &c3 = transform.column3;

            // set up the model matrix used for the rendering
            renderable->modelMatrix = glm::mat4(
                c0.x, c0.y, c0.z, c0.w,
                c1.x, c1.y, c1.z, c1.w,
                c2.x, c2.y, c2.z, c2.w,
                c3.x, c3.y, c3.z, c3.w);
        }
    }
}

void keyboard(unsigned char key, int x, int y)
{
    float angleSpeed = 0.1f;
    float moveSpeed = 0.1f;
    switch (key)
    {
    case 'z': cameraAngle -= angleSpeed; break;
    case 'x': cameraAngle += angleSpeed; break;
    case 'w': cameraPos += cameraDir * moveSpeed; break;
    case 's': cameraPos -= cameraDir * moveSpeed; break;
    case 'd': cameraPos += cameraSide * moveSpeed; break;
    case 'a': cameraPos -= cameraSide * moveSpeed; break;
    }
}

void mouse(int x, int y)
{

}

glm::mat4 createCameraMatrix()
{
    cameraDir = glm::normalize(glm::vec3(cosf(cameraAngle - glm::radians(90.0f)), 0, sinf(cameraAngle - glm::radians(90.0f))));
    glm::vec3 up = glm::vec3(0, 1, 0);
    cameraSide = glm::cross(cameraDir, up);

    return Core::createViewMatrix(cameraPos, cameraDir, up);
}

void drawObjectColor(Core::RenderContext* context, glm::mat4 modelMatrix, glm::vec3 color)
{
    GLuint program = programColor;

    glUseProgram(program);

    glUniform3f(glGetUniformLocation(program, "objectColor"), color.x, color.y, color.z);
    glUniform3f(glGetUniformLocation(program, "lightDir"), lightDir.x, lightDir.y, lightDir.z);

    glm::mat4 transformation = perspectiveMatrix * cameraMatrix * modelMatrix;
    glUniformMatrix4fv(glGetUniformLocation(program, "modelViewProjectionMatrix"), 1, GL_FALSE, (float*)&transformation);
    glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);

    Core::DrawContext(*context);

    glUseProgram(0);
}

void drawObjectTexture(Core::RenderContext * context, glm::mat4 modelMatrix, GLuint textureId)
{
    GLuint program = programTexture;

    glUseProgram(program);

    glUniform3f(glGetUniformLocation(program, "lightDir"), lightDir.x, lightDir.y, lightDir.z);
    Core::SetActiveTexture(textureId, "textureSampler", program, 0);

    glm::mat4 transformation = perspectiveMatrix * cameraMatrix * modelMatrix;
    glUniformMatrix4fv(glGetUniformLocation(program, "modelViewProjectionMatrix"), 1, GL_FALSE, (float*)&transformation);
    glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);

    Core::DrawContext(*context);

    glUseProgram(0);
}

void renderScene()
{
    double time = glutGet(GLUT_ELAPSED_TIME) / 1000.0;
    static double prevTime = time;
    double dtime = time - prevTime;
    prevTime = time;

    // Update physics
    if (dtime < 1.f) {
        physicsTimeToProcess += dtime;
        while (physicsTimeToProcess > 0) {
            // here we perform the physics simulation step
            pxScene.step(physicsStepTime);
            physicsTimeToProcess -= physicsStepTime;
        }
    }

    // Update of camera and perspective matrices
    cameraMatrix = createCameraMatrix();
    perspectiveMatrix = Core::createPerspectiveMatrix();

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.0f, 0.1f, 0.3f, 1.0f);

    // update transforms from physics simulation
    updateTransforms();

    // render models
    for (Renderable* renderable : renderables) {
        drawObjectTexture(renderable->context, renderable->modelMatrix, renderable->textureId);
    }

    glutSwapBuffers();
}

void init()
{
    srand(time(0));
    glEnable(GL_DEPTH_TEST);
    programColor = shaderLoader.CreateProgram("shaders/shader_color.vert", "shaders/shader_color.frag");
    programTexture = shaderLoader.CreateProgram("shaders/shader_tex.vert", "shaders/shader_tex.frag");

    initRenderables();
    initPhysicsScene();
}

void shutdown()
{
    shaderLoader.DeleteProgram(programColor);
    shaderLoader.DeleteProgram(programTexture);
}

void idle()
{
    glutPostRedisplay();
}

int main(int argc, char ** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(200, 200);
    glutInitWindowSize(600, 600);
    glutCreateWindow("OpenGL + PhysX");
    glewInit();

    init();
    glutKeyboardFunc(keyboard);
    glutPassiveMotionFunc(mouse);
    glutDisplayFunc(renderScene);
    glutIdleFunc(idle);

    glutMainLoop();

    shutdown();

    return 0;
}
