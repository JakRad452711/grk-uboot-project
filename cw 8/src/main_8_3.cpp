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

//------------------------------------------------------------------
// TASKS
//------------------------------------------------------------------
//
// In this exercise you will learn how to get notifications
// about collisions (called contacts), along with the contact data.
// 
// For this purpose we will use the following things provided
// by PhysX:
// * PxSimulationFilterShader
// * PxSimulationEventCallback
//
// First, we implement a function compatible with the declaration
// of PxSimulationFilterShader. This function filters the
// collision pairs and tells the PhysX how to process them.
// Do not be misleaded by the term "shader". It should not be
// confused with shaders in the rendering pipeline.
//
// See simulationFilterShader for more details.
//
// Then, we implement a class derived from PxSimulationEventCallback.
// This class will process various callbacks fired by the
// physics simulation. Among the possible callbacks, there is
// onContact method, which we are currently interested in.
// Using onContact method, we can examine all the contact pairs
// and access the details about the contact.
//
// See SimulationEventCallback for more details.
//
// Now, copy the neccessary parts of the code from the exercise 8_1,
// so that we get working simulation of box falling onto the plane.
// 
// In SimulationEventCallback::onContact implement
// the following features:
// * print the number of contact pairs
// * for each contact pair print the number of contact points
//   between the given pair
// * for each point print the x,y,z coordinates of its position
//
// As the result, a number of points should be printed each time
// the box touches the ground. When it finally rests on the ground,
// the points are no longer reported.
// In order to get the points even if the box rests, add the flag
// PxPairFlag::eNOTIFY_TOUCH_PERSISTS to pairFlags
// in simulationFilterShader.
// 
// Now, copy the neccessary parts of the code from the exercise 8_2,
// so that we get working simulation of a wall of boxes and a ball
// destroying the wall.
// 
// Modify the SimulationEventCallback::onContact method,
// so that we print only the contacts for which one actor is the ball.
// (We don't want to print all the contacts between the boxes,
// and between the boxes and the ground).
//
//------------------------------------------------------------------


//------------------------------------------------------------------
// contact pairs filtering function
static PxFilterFlags simulationFilterShader(PxFilterObjectAttributes attributes0,
    PxFilterData filterData0, PxFilterObjectAttributes attributes1, PxFilterData filterData1,
    PxPairFlags& pairFlags, const void* constantBlock, PxU32 constantBlockSize)
{
    pairFlags =
        PxPairFlag::eCONTACT_DEFAULT | // default contact processing
        PxPairFlag::eNOTIFY_CONTACT_POINTS | // contact points will be available in onContact callback
        PxPairFlag::eNOTIFY_TOUCH_FOUND; // onContact callback will be called for this pair
        
    return physx::PxFilterFlag::eDEFAULT;
}

//------------------------------------------------------------------
// simulation events processor
class SimulationEventCallback : public PxSimulationEventCallback
{
public:
    void onContact(const PxContactPairHeader& pairHeader,
        const PxContactPair* pairs, PxU32 nbPairs)
    {
        // HINT: You can check which actors are in contact
        // using pairHeader.actors[0] and pairHeader.actors[1]

        for (PxU32 i = 0; i < nbPairs; i++)
        {
            const PxContactPair& cp = pairs[i];
            
            // HINT: two get the contact points, use
            // PxContactPair::extractContacts

            // You need to provide the function with a buffer
            // in which the contact points will be stored.
            // Create an array (vector) of type PxContactPairPoint
            // The number of elements in array should be at least
            // cp.contactCount (which is the number of contact points)
            // You also need to provide the function with the
            // size of the buffer (it should equal the size of the
            // array in bytes)
            // Finally, for every extracted point, you can access
            // its details, such as position
        }
    }

    // The functions below are not used in this exercise.
    // However, they need to be defined for the class to compile.
    virtual void onConstraintBreak(PxConstraintInfo* constraints, PxU32 count) {}
    virtual void onWake(PxActor** actors, PxU32 count) {}
    virtual void onSleep(PxActor** actors, PxU32 count) {}
    virtual void onTrigger(PxTriggerPair* pairs, PxU32 count) {}
    virtual void onAdvance(const PxRigidBody*const* bodyBuffer, const PxTransform* poseBuffer, const PxU32 count) {}
};

// Initalization of physical scene (PhysX)
SimulationEventCallback simulationEventCallback;
Physics pxScene(9.8 /* gravity (m/s^2) */, simulationFilterShader,
    &simulationEventCallback);

// fixed timestep for stable and deterministic simulation
const double physicsStepTime = 1.f / 60.f;
double physicsTimeToProcess = 0;

//----------------------------------------------
Core::Shader_Loader shaderLoader;
GLuint programColor;
GLuint programTexture;

glm::vec3 cameraPos = glm::vec3(0, 5, 20);
glm::vec3 cameraDir;
glm::vec3 cameraSide;
float cameraAngle = 0;
glm::mat4 cameraMatrix, perspectiveMatrix;

glm::vec3 lightDir = glm::normalize(glm::vec3(0.5, -1, -0.5));



//----------------------------------------------------------
//----------------------------------------------------------
// Here paste the necessary code for rendering and physics
//----------------------------------------------------------
//----------------------------------------------------------



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

void drawObjectColor(Core::RenderContext context, glm::mat4 modelMatrix, glm::vec3 color)
{
    GLuint program = programColor;

    glUseProgram(program);

    glUniform3f(glGetUniformLocation(program, "objectColor"), color.x, color.y, color.z);
    glUniform3f(glGetUniformLocation(program, "lightDir"), lightDir.x, lightDir.y, lightDir.z);

    glm::mat4 transformation = perspectiveMatrix * cameraMatrix * modelMatrix;
    glUniformMatrix4fv(glGetUniformLocation(program, "modelViewProjectionMatrix"), 1, GL_FALSE, (float*)&transformation);
    glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);

    Core::DrawContext(context);

    glUseProgram(0);
}

void drawObjectTexture(Core::RenderContext context, glm::mat4 modelMatrix, GLuint textureId)
{
    GLuint program = programTexture;

    glUseProgram(program);

    glUniform3f(glGetUniformLocation(program, "lightDir"), lightDir.x, lightDir.y, lightDir.z);
    Core::SetActiveTexture(textureId, "textureSampler", program, 0);

    glm::mat4 transformation = perspectiveMatrix * cameraMatrix * modelMatrix;
    glUniformMatrix4fv(glGetUniformLocation(program, "modelViewProjectionMatrix"), 1, GL_FALSE, (float*)&transformation);
    glUniformMatrix4fv(glGetUniformLocation(program, "modelMatrix"), 1, GL_FALSE, (float*)&modelMatrix);

    Core::DrawContext(context);

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

    //----------------------------------------------------------
    //----------------------------------------------------------
    // Here paste the necessary code for rendering and physics
    //----------------------------------------------------------
    //----------------------------------------------------------

    glutSwapBuffers();
}

void init()
{
    srand(time(0));
    glEnable(GL_DEPTH_TEST);
    programColor = shaderLoader.CreateProgram("shaders/shader_color.vert", "shaders/shader_color.frag");
    programTexture = shaderLoader.CreateProgram("shaders/shader_tex.vert", "shaders/shader_tex.frag");

    //----------------------------------------------------------
    //----------------------------------------------------------
    // Here paste the necessary code for rendering and physics
    //----------------------------------------------------------
    //----------------------------------------------------------
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
