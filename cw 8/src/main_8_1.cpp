#define FOURCC_DXT1 0x31545844
#define FOURCC_DXT3 0x33545844
#define FOURCC_DXT5 0x35545844

#include "glew.h"
#include "freeglut.h"
#include "glm.hpp"
#include "ext.hpp"
#include <iostream>
#include <cmath>
#include <vector>
#include <algorithm>

#include "Shader_Loader.h"
#include "Render_Utils.h"
#include "Camera.h"
#include "Texture.h"
#include "Physics.h"
#include "Constants.h"


Core::Shader_Loader shaderLoader;
GLuint programColor;
GLuint programTexture;

// plane
obj::Model planeModel;
GLuint groundTexture;
Core::RenderContext planeContext;

// particles
GLuint billboard_vertex_buffer;
GLuint particles_position_buffer;
GLuint particles_color_buffer;
static GLfloat* g_particule_position_size_data;
static GLubyte* g_particule_color_data;
GLuint particleTexture;
int ParticlesCount = 0;
double timeSinceLastTick;

struct Particle {
    glm::vec3 pos, speed;
    unsigned char r, g, b, a;
    float size, angle, weight;
    float life;
    float cameradistance;

    bool operator<(const Particle& that) const {
        return this->cameradistance > that.cameradistance;
    }
};

const int MaxParticles = 100000;
Particle ParticlesContainer[MaxParticles];
int LastUsedParticle = 0;

const int PARTICLE_GEN_NUMBER = 4;

float particleAmount[PARTICLE_GEN_NUMBER] = { submarineBubblesAmount, 400.f, 200.f, 80.f };
glm::vec3 particlePosition[PARTICLE_GEN_NUMBER] = { glm::vec3(0, 0, 0), glm::vec3(-46.0f, 30.0f, -18.f), glm::vec3(-46.0f, 30.0f, -18.f), glm::vec3(0, 0, 0) };
float particleSpread[PARTICLE_GEN_NUMBER] = { submarineBubblesSpread, 0.6f, 0.9f, 3.f };

// functions

int FindUnusedParticle();

void SortParticles();

GLuint loadDDS(const char* imagepath);

// mouse
int lastX;
int lastY;

// camera
glm::vec3 cameraPos = glm::vec3(0, 0, 0);
glm::vec3 cameraDir;
glm::vec3 cameraSide;
float old_x, old_y = -1;
float delta_x, delta_y = 0;
float dy = 0;
float dx = 0;
glm::quat rotationCamera = glm::quat(1, 0, 0, 0);
glm::quat rotation_y = glm::normalize(glm::angleAxis(209 * 0.03f, glm::vec3(1, 0, 0)));
glm::quat rotation_x = glm::normalize(glm::angleAxis(307 * 0.03f, glm::vec3(0, 1, 0)));
glm::mat4 cameraMatrix, perspectiveMatrix;

// submarine
glm::vec3 submarinePos = glm::vec3(0, 30, 0);
glm::vec3 submarineDir;
float submarineAngle = 0;

obj::Model submarineModel;
obj::Model submarinePropellerModel;
GLuint submarineTexture;
GLuint submarinePropellerTexture;
Core::RenderContext submarineContext;
Core::RenderContext submarinePropellerContext;
glm::mat4 submarineModelMatrix;
glm::mat4 submarinePropellerModelMatrix;

// fishes
const int numberOfFishes = 50;
obj::Model fishModels[numberOfFishes];
glm::vec3 fishPreviousPosition[numberOfFishes];
int fishTypes[numberOfFishes] = { 1, 2, 3, 4, 5, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5 };
GLuint fishTextures[numberOfFishes];
Core::RenderContext fishContexts[numberOfFishes];
std::vector<glm::vec3> fishRoute[numberOfFishes]{
    {
        glm::vec3(0, 30, -30)
    },
    {
        glm::vec3(15, 30, -30)
    },
    {
        glm::vec3(30, 30, -30)
    },
    {
        glm::vec3(45, 30, -30)
    },
    {
        glm::vec3(60, 30, -30)
    },
    {
        glm::vec3(0, 40, -10),
        glm::vec3(-20, 40, -40),
        glm::vec3(20, 45, -40),
        glm::vec3(0, 45, -10),
        glm::vec3(-20, 40, 40),
        glm::vec3(20, 40, 40),
        glm::vec3(0, 40, -10)
    },
    {
        glm::vec3(5, 20, -10),
        glm::vec3(5, 20, -30),
        glm::vec3(5, 20, -10),
        glm::vec3(5, 20, -30)
    },
    {
        glm::vec3(13, 20, -20),
        glm::vec3(30, 20, -20),
        glm::vec3(10, 20, -20),
    },
    {glm::vec3(0, 0, 0)}, {glm::vec3(0, 0, 0)}, {glm::vec3(0, 0, 0)}, {glm::vec3(0, 0, 0)}, {glm::vec3(0, 0, 0)},
    {glm::vec3(0, 0, 0)}, {glm::vec3(0, 0, 0)}, {glm::vec3(0, 0, 0)}, {glm::vec3(0, 0, 0)}, {glm::vec3(0, 0, 0)},
    {glm::vec3(0, 0, 0)}, {glm::vec3(0, 0, 0)}, {glm::vec3(0, 0, 0)}, {glm::vec3(0, 0, 0)}, {glm::vec3(0, 0, 0)},
    {glm::vec3(0, 0, 0)}, {glm::vec3(0, 0, 0)}, {glm::vec3(0, 0, 0)}, {glm::vec3(0, 0, 0)}, {glm::vec3(0, 0, 0)},
    {glm::vec3(0, 0, 0)}, {glm::vec3(0, 0, 0)}, {glm::vec3(0, 0, 0)}, {glm::vec3(0, 0, 0)}, {glm::vec3(0, 0, 0)},
    {glm::vec3(0, 0, 0)}, {glm::vec3(0, 0, 0)}, {glm::vec3(0, 0, 0)}, {glm::vec3(0, 0, 0)}, {glm::vec3(0, 0, 0)},
    {glm::vec3(0, 0, 0)}, {glm::vec3(0, 0, 0)}, {glm::vec3(0, 0, 0)}, {glm::vec3(0, 0, 0)}, {glm::vec3(0, 0, 0)},
    {glm::vec3(0, 0, 0)}, {glm::vec3(0, 0, 0)}, {glm::vec3(0, 0, 0)}, {glm::vec3(0, 0, 0)}, {glm::vec3(0, 0, 0)},
    {glm::vec3(0, 0, 0)}, {glm::vec3(0, 0, 0)}
};

//plants
const int numberOfPlants = 100;
obj::Model plantModels[2];
glm::vec3 plantPositions[100];
int plantTypes[2] = { 1, 2 };
GLuint plantTextures[2];
Core::RenderContext plantContexts[100];

//rocks
const int numberOfRocks = 10;
obj::Model rockModels[numberOfRocks];
glm::vec3 rockPositions[5] =
{
    glm::vec3(20.0f, -20.0f ,-12.0f),
    glm::vec3(5.5f, 2.5f ,12.0f),
    glm::vec3(3.5f, 2.5f ,12.0f),
    glm::vec3(1.5f,2.5f ,9.0f),
     glm::vec3(-10.0f, 4.5f ,10.9f),

};
glm::vec3 rockPositions_test[230];
int rockTypes[8] = { 1, 2, 3 , 4, 5, 6, 7, 8 };
GLuint rockTextures[8];
Core::RenderContext rockContexts[230];

// light
glm::vec3 lightDir = glm::normalize(glm::vec3(0.5, -1, -0.5));

// Initalization of physical scene (PhysX)
Physics pxScene(9.8 /* gravity (m/s^2) */);

// fixed timestep for stable and deterministic simulation
const double physicsStepTime = 1.f / 60.f;
double physicsTimeToProcess = 0;

// physical objects
PxRigidStatic* planeBody = nullptr;
PxMaterial* planeMaterial = nullptr;
PxRigidDynamic* boxBody = nullptr;
PxMaterial* boxMaterial = nullptr;

// particles
GLuint programParticles;


void initRenderables()
{
    // load models
    planeModel = obj::loadModelFromFile("models/ground.obj");
    submarineModel = obj::loadModelFromFile("models/uboot_v2.obj");
    submarinePropellerModel = obj::loadModelFromFile("models/uboot_propeller.obj");

    // load textures
    groundTexture = Core::LoadTexture("textures/ground.jpg");
    submarineTexture = Core::LoadTexture("textures/uboot.png");
    submarinePropellerTexture = Core::LoadTexture("textures/uboot_propeller.png");

    planeContext.initFromOBJ(planeModel);
    submarineContext.initFromOBJ(submarineModel);
    submarinePropellerContext.initFromOBJ(submarinePropellerModel);

    int x, y, z;

    for (int i = 0; i < numberOfFishes; i++)
    {
        std::string modelFileName;
        char* textureFileName;

        if (fishTypes[i] == 1)
        {
            modelFileName = "models/shark.obj";
            textureFileName = "textures/shark.png";
        }
        if (fishTypes[i] == 2)
        {
            modelFileName = "models/fish1.obj";
            textureFileName = "textures/fish1.png";
        }
        if (fishTypes[i] == 3)
        {
            modelFileName = "models/fish2.obj";
            textureFileName = "textures/fish2.png";
        }
        if (fishTypes[i] == 4)
        {
            modelFileName = "models/fish3.obj";
            textureFileName = "textures/fish3.png";
        }
        if (fishTypes[i] == 5)
        {
            modelFileName = "models/fishies.obj";
            textureFileName = "textures/uboot_propeller.png";
        }

        fishModels[i] = obj::loadModelFromFile(modelFileName);
        fishTextures[i] = Core::LoadTexture(textureFileName);
        fishContexts[i].initFromOBJ(fishModels[i]);
    }

    //coral
    for (int i = 0; i < 2; i++)
    {
        std::string modelFileName;
        char* textureFileName;
        if (plantTypes[i] == 1)
        {
            modelFileName = "models/coral1.obj";
            textureFileName = "textures/coral1.png";
        }
        if (plantTypes[i] == 2)
        {
            modelFileName = "models/coral2.obj";
            textureFileName = "textures/coral2/png";
        }


        plantModels[i] = obj::loadModelFromFile(modelFileName);
        plantTextures[i] = Core::LoadTexture(textureFileName);
        plantContexts[i].initFromOBJ(plantModels[i]);
    }

    //rocks
    for (int i = 0; i < numberOfRocks; i++)
    {
        std::string modelFileName;
        char* textureFileName;

        if (fishTypes[i] == 1)
        {
            modelFileName = "models/rock1.obj";
            textureFileName = "textures/rocks_rock_BaseMap.png";
        }
        if (fishTypes[i] == 2)
        {
            modelFileName = "models/rock2.obj";
            textureFileName = "textures/rocks_rock_BaseMap.png";
        }
        if (fishTypes[i] == 3)
        {
            modelFileName = "models/rock3.obj";
            textureFileName = "textures/rocks_rock_BaseMap.png";
        }
        if (fishTypes[i] == 4)
        {
            modelFileName = "models/rock4.obj";
            textureFileName = "textures/rocks_rock_BaseMap.png";
        }
        if (fishTypes[i] == 5)
        {
            modelFileName = "models/rock5.obj";
            textureFileName = "textures/rocks_rock_BaseMap.png";
        }
        if (fishTypes[i] == 6)
        {
            modelFileName = "models/rock6.obj";
            textureFileName = "textures/rocks_rock_BaseMap.png";
        }
        if (fishTypes[i] == 7)
        {
            modelFileName = "models/rock7.obj";
            textureFileName = "textures/rocks_rock_BaseMap.png";
        }
        if (fishTypes[i] == 8)
        {
            modelFileName = "models/rock8.obj";
            textureFileName = "textures/rocks_rock_BaseMap.png";
        }
        rockModels[i] = obj::loadModelFromFile(modelFileName);
        rockTextures[i] = Core::LoadTexture(textureFileName);
        rockContexts[i].initFromOBJ(rockModels[i]);
    }

    for (int i = 0; i < 230; i++)
    {
        x = ((rand() % 400) - 200);
        y = (rand() % 2 + 1);
        z = ((rand() % 400) - 200);

        rockPositions_test[i] = glm::vec3(x, y, z);
    }

    for (int i = 0; i < 100; i++)
    {
        x = ((rand() % 400) - 200);
        y = (rand() % 3 + 2);
        z = ((rand() % 400) - 200);

        plantPositions[i] = glm::vec3(x, y, z);
    }
}

void initPhysicsScene()
{
    planeBody = pxScene.physics->createRigidStatic(PxTransformFromPlaneEquation(PxPlane(0, 1, 0, 0)));
    planeMaterial = pxScene.physics->createMaterial(0.4, 0.4, 0.4);
    PxShape* planeShape = pxScene.physics->createShape(PxPlaneGeometry(), *planeMaterial);
    planeBody->attachShape(*planeShape);
    planeShape->release();
    planeBody->userData = nullptr;
    pxScene.scene->addActor(*planeBody);
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
            // We use the userData of the objects to set up the proper model matrices.
            if (!actor->userData) continue;
            glm::mat4* modelMatrix = (glm::mat4*)actor->userData;

            // get world matrix of the object (actor)
            PxMat44 transform = actor->getGlobalPose();
            auto& c0 = transform.column0;
            auto& c1 = transform.column1;
            auto& c2 = transform.column2;
            auto& c3 = transform.column3;

            // set up the model matrix used for the rendering
            *modelMatrix = glm::mat4(
                c0.x, c0.y, c0.z, c0.w,
                c1.x, c1.y, c1.z, c1.w,
                c2.x, c2.y, c2.z, c2.w,
                c3.x, c3.y, c3.z, c3.w);
        }
    }
}

void keyboard(unsigned char key, int x, int y)
{
    float angleSpeed = submarineTurningSpeed;
    float moveSpeed = submarineMovementSpeed;
    switch (key)
    {
    case 'w': submarinePos += submarineDir * moveSpeed; break;
    case 's': submarinePos -= submarineDir * moveSpeed; break;
    case 'd': submarineAngle += angleSpeed; break;
    case 'a': submarineAngle -= angleSpeed; break;
    case 'z': submarinePos += glm::vec3(0, 1, 0) * moveSpeed; break;
    case 'x': submarinePos += glm::vec3(0, -1, 0) * moveSpeed; break;
    }
}

void mouse(int x, int y)
{
    if (old_x >= 0 && abs(x - old_x) < 50 && abs(y - old_y) < 50) {
        delta_x = (x - old_x) * mouseSensitivityH;
        delta_y = (y - old_y) * mouseSensitivityV;
    }

    old_x = x;
    old_y = y;
}

glm::mat4 createCameraMatrix()
{
    auto rot_y = glm::angleAxis(delta_y * 0.03f, glm::vec3(1, 0, 0));
    auto rot_x = glm::angleAxis(delta_x * 0.03f, glm::vec3(0, 1, 0));

    dy += delta_y;
    dx += delta_x;
    delta_x = 0;
    delta_y = 0;

    rotation_x = glm::normalize(rot_x * rotation_x);
    rotation_y = glm::normalize(rot_y * rotation_y);

    rotationCamera = glm::normalize(rotation_y * rotation_x);

    auto inverse_rot = glm::inverse(rotationCamera);

    cameraDir = inverse_rot * glm::vec3(0, 0, -1);
    glm::vec3 up = glm::vec3(0, 1, 0);
    cameraSide = inverse_rot * glm::vec3(1, 0, 0);
    cameraPos = submarinePos - (cameraDir * submarineForwardOffset + glm::vec3(0, 1, 0) * submarineVerticalOffset);

    glm::mat4 cameraTranslation;
    cameraTranslation[3] = glm::vec4(-cameraPos, 1.0f);

    return glm::mat4_cast(rotationCamera) * cameraTranslation;
}

glm::mat4 animationMatrix(float time, int fishIndex) {
    float speed = 1;
    time = time * speed;
    std::vector<float> distances;
    float timeStep = 0;
    for (int i = 0; i < fishRoute[fishIndex].size() - 1; i++) {
        timeStep += (fishRoute[fishIndex][i] - fishRoute[fishIndex][i + 1]).length();
        distances.push_back((fishRoute[fishIndex][i] - fishRoute[fishIndex][i + 1]).length());
    }
    time = fmod(time, timeStep);

    int index = 0;

    while (distances[index] <= time) {
        time = time - distances[index];
        index += 1;
    }

    float t = time / distances[index];

    int size = fishRoute[fishIndex].size() - 1;
	
    glm::vec3 pos = glm::catmullRom(
        fishRoute[fishIndex][index % size], 
        fishRoute[fishIndex][(index + 1) % size], 
        fishRoute[fishIndex][(index + 2) % size], 
        fishRoute[fishIndex][(index + 3) % size], 
        t
    );

    glm::vec3 diff = pos - fishPreviousPosition[fishIndex];
    diff.y = 0;
    fishPreviousPosition[fishIndex] = pos;

    float angle;

    angle = glm::acos(glm::dot(glm::vec3(1, 0, 0), diff) / length(diff));

    if (diff.x > -0.4f && diff.z > 0)
        angle = -angle;

    glm::mat4 animationRotation = glm::rotate(angle, glm::vec3(0, 1, 0));

    glm::mat4 result = glm::translate(pos) * animationRotation;

    return result;
}

void drawObjectColor(Core::RenderContext context, glm::mat4 modelMatrix, glm::vec3 color)
{
    GLuint program = programColor;

    glUseProgram(program);

    glUniform1f(glGetUniformLocation(program, "fogAppearDistance"), fogAppearDistance);
    glUniform1f(glGetUniformLocation(program, "fogFullEffectDistance"), fogFullEffectDistance);
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

    glUniform1f(glGetUniformLocation(program, "fogAppearDistance"), fogAppearDistance);
    glUniform1f(glGetUniformLocation(program, "fogFullEffectDistance"), fogFullEffectDistance);
    glUniform3f(glGetUniformLocation(program, "lightDir"), lightDir.x, lightDir.y, lightDir.z);
    glUniform4f(glGetUniformLocation(program, "skyColor"), skyColor.x, skyColor.y, skyColor.z, 1);
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

    // update submarine
    submarineDir = glm::normalize(glm::vec3(cosf(submarineAngle - glm::radians(90.0f)), 0, sinf(submarineAngle - glm::radians(90.0f))));

    // Update of camera and perspective matrices
    cameraMatrix = createCameraMatrix();
    perspectiveMatrix = Core::createPerspectiveMatrix(0.1, 200);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(skyColor.x, skyColor.y, skyColor.z, 1.0f);

    // update transforms from physics simulation
    updateTransforms();

    glm::quat circularRotationQuat = glm::angleAxis((float) time * propellerRotatingSpeed, submarineDir);
    glm::mat4 circularRotationMatrix = glm::toMat4(circularRotationQuat);
    glm::mat4 groundModelMatrix = glm::rotate(glm::radians(0.f), glm::vec3(0.f, 0.f, 1.f)) * glm::scale(glm::vec3(50));

    submarineModelMatrix = glm::translate(submarinePos) * glm::rotate(-submarineAngle + glm::radians(270.0f), glm::vec3(0, 1, 0)) * glm::scale(glm::vec3(0.85f));
    submarinePropellerModelMatrix = glm::translate(submarinePos) * glm::translate(glm::vec3(0, -0.6, 0)) * circularRotationMatrix * glm::rotate(-submarineAngle + glm::radians(270.0f), glm::vec3(0, 1, 0)) * glm::scale(glm::vec3(0.85f));
    particlePosition[0] = submarinePos - submarineDir * 13 - glm::vec3(0, 0.5, 0);
    particlePosition[3] = submarinePos + glm::vec3(0, -30.f, 0.f);

    // render models
    drawObjectTexture(planeContext, groundModelMatrix, groundTexture);
    drawObjectTexture(submarineContext, submarineModelMatrix, submarineTexture); 
    drawObjectTexture(submarinePropellerContext, submarinePropellerModelMatrix, submarinePropellerTexture); 

    // fishes
    for (int i = 0; i < numberOfFishes; i++)
    {
        glm::mat4 fishMatrix;

        if (fishRoute[i].size() > 2)
            fishMatrix = animationMatrix(time, i) * glm::scale(glm::vec3(0.25f)) * glm::rotate(glm::radians(180.f), glm::vec3(0, 1, 0));

        else
            fishMatrix = glm::translate(fishRoute[i][0]) * glm::scale(glm::vec3(0.25f)) * glm::rotate(glm::radians(180.f), glm::vec3(0, 1, 0));

        drawObjectTexture(fishContexts[i], fishMatrix, fishTextures[i]);
    }

    //rocks
    glm::mat4 rockMatrix = glm::translate(rockPositions[0]) * glm::scale(glm::vec3(20.0f));
    drawObjectTexture(rockContexts[0], rockMatrix, rockTextures[0]);
    for (int i = 1; i < numberOfRocks; i++)
    {
        glm::mat4 rockMatrix = glm::translate(rockPositions[i]) * glm::scale(glm::vec3(1.4f));
        drawObjectTexture(rockContexts[i], rockMatrix, rockTextures[1]);
    }

    // rocks 
    for (int i = 0; i < 230; i++)
    {
        glm::mat4 rockMatrix2 = glm::translate(rockPositions_test[i]) * glm::scale(glm::vec3(2.5f));
        drawObjectTexture(rockContexts[i % 8], rockMatrix2, rockTextures[1]);
    }

    //plants
    for (int i = 0; i < 100; i++)
    {
        glm::mat4 plantMatrix = glm::translate(plantPositions[i]) * glm::scale(glm::vec3(0.8f));
        drawObjectTexture(plantContexts[i % 2], plantMatrix, plantTextures[i % 2]);
    }

    // particles
    double delta = glutGet(GLUT_ELAPSED_TIME) / 1000.0f - timeSinceLastTick;
    timeSinceLastTick = time;

    for (int i = 0; i < PARTICLE_GEN_NUMBER; i++) {
        int newparticles = (int)(delta * particleAmount[i]);
        if (newparticles > (int)(0.016f * particleAmount[i]))
            newparticles = (int)(0.016f * particleAmount[i]);

        for (int j = 0; j < newparticles; j++) {
            int particleIndex = FindUnusedParticle();
            ParticlesContainer[particleIndex].life = 10.0f;
            ParticlesContainer[particleIndex].pos = particlePosition[i];

            glm::vec3 maindir = glm::vec3(0.0f, 1.0f, 0.0f);

            glm::vec3 randomdir = glm::vec3(
                (rand() % 2000 - 1000.0f) / 1000.0f,
                (rand() % 2000 - 1000.0f) / 1000.0f,
                (rand() % 2000 - 1000.0f) / 1000.0f
            );

            ParticlesContainer[particleIndex].speed = maindir + randomdir * particleSpread[i];
            ParticlesContainer[particleIndex].r = 200;
            ParticlesContainer[particleIndex].g = 200;
            ParticlesContainer[particleIndex].b = 255;
            ParticlesContainer[particleIndex].a = (rand() % 256) / 3;

            ParticlesContainer[particleIndex].size = (rand() % 1000) / 5000 + 0.05f;
        }
    }

    ParticlesCount = 0;
    for (int i = 0; i < MaxParticles; i++)
    {

        Particle& p = ParticlesContainer[i];

        if (p.life > 0.0f) {
            p.life -= delta;
            if (p.life > 0.0f)
            {
                p.speed += glm::vec3(0.0f, 1.81f, 0.0f) * (float)delta * 0.5f;
                p.pos += p.speed * (float)delta;
                p.cameradistance = glm::length2(p.pos - cameraPos);

                g_particule_position_size_data[4 * ParticlesCount + 0] = p.pos.x;
                g_particule_position_size_data[4 * ParticlesCount + 1] = p.pos.y;
                g_particule_position_size_data[4 * ParticlesCount + 2] = p.pos.z;

                g_particule_position_size_data[4 * ParticlesCount + 3] = p.size;

                g_particule_color_data[4 * ParticlesCount + 0] = p.r;
                g_particule_color_data[4 * ParticlesCount + 1] = p.g;
                g_particule_color_data[4 * ParticlesCount + 2] = p.b;
                g_particule_color_data[4 * ParticlesCount + 3] = p.a;

            }
            else {
                p.cameradistance = -1.0f;
            }

            ParticlesCount++;

        }
    }

    SortParticles();

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glUseProgram(programParticles);

    GLuint TextureID = glGetUniformLocation(programParticles, "myTextureSampler");

    GLuint cameraUp = glGetUniformLocation(programParticles, "CameraUp_worldspace");
    GLuint viewProjectionMatrix = glGetUniformLocation(programParticles, "VP");
    GLuint cameraRight = glGetUniformLocation(programParticles, "CameraRight_worldspace");

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, particleTexture);

    glUniform1i(TextureID, 0);

    glUniform3f(cameraRight, cameraSide.x, cameraSide.y, cameraSide.z);
    glUniform3f(cameraUp, 0, 1, 0);

    glm::mat4 transformation = perspectiveMatrix * cameraMatrix;
    glUniformMatrix4fv(viewProjectionMatrix, 1, GL_FALSE, (float*)&transformation);


    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, billboard_vertex_buffer);
    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        0,
        (void*)0
    );

    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
    glVertexAttribPointer(
        1,
        4,
        GL_FLOAT,
        GL_FALSE,
        0,
        (void*)0
    );

    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
    glVertexAttribPointer(
        2,
        4,
        GL_UNSIGNED_BYTE,
        GL_TRUE,
        0,
        (void*)0
    );

    glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
    glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, ParticlesCount * sizeof(GLfloat) * 4, g_particule_position_size_data);

    glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);
    glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW);
    glBufferSubData(GL_ARRAY_BUFFER, 0, ParticlesCount * sizeof(GLubyte) * 4, g_particule_color_data);

    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, ParticlesCount);

    glVertexAttribDivisor(0, 0);
    glVertexAttribDivisor(1, 1);
    glVertexAttribDivisor(2, 1);

    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, ParticlesCount);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);

    glutSwapBuffers();
}

void init()
{
    srand(time(0));
    glEnable(GL_DEPTH_TEST);
    programParticles = shaderLoader.CreateProgram("shaders/shader_particle.vert", "shaders/shader_particle.frag");
    programColor = shaderLoader.CreateProgram("shaders/shader_color.vert", "shaders/shader_color.frag");
    programTexture = shaderLoader.CreateProgram("shaders/shader_tex.vert", "shaders/shader_tex.frag");

    initRenderables();
    initPhysicsScene();

    particleTexture = loadDDS("textures/bubble.dds");

    g_particule_position_size_data = new GLfloat[MaxParticles * 4];
    g_particule_color_data = new GLubyte[MaxParticles * 4];

    for (int i = 0; i < MaxParticles; i++) {
        ParticlesContainer[i].life = -1.0f;
        ParticlesContainer[i].cameradistance = -1.0f;
    }

    static const GLfloat g_vertex_buffer_data[] = {
     -0.5f, -0.5f, 0.0f,
     0.5f, -0.5f, 0.0f,
     -0.5f, 0.5f, 0.0f,
     0.5f, 0.5f, 0.0f,
    };

    glGenBuffers(1, &billboard_vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, billboard_vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

    glGenBuffers(1, &particles_position_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, particles_position_buffer);
    glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLfloat), NULL, GL_STREAM_DRAW);

    glGenBuffers(1, &particles_color_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, particles_color_buffer);

    glBufferData(GL_ARRAY_BUFFER, MaxParticles * 4 * sizeof(GLubyte), NULL, GL_STREAM_DRAW);
}

void shutdown()
{
    shaderLoader.DeleteProgram(programColor);
    shaderLoader.DeleteProgram(programTexture);

    planeContext.initFromOBJ(planeModel);
    submarineContext.initFromOBJ(submarineModel);

    delete[] g_particule_position_size_data;
    glDeleteBuffers(1, &particles_color_buffer);
    glDeleteBuffers(1, &particles_position_buffer);
    glDeleteBuffers(1, &billboard_vertex_buffer);
}

void idle()
{
    glutPostRedisplay();
}

int main(int argc, char** argv)
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

    int x, y, z;

    for (int i = 8; i < numberOfFishes; i++)
    {
        x = ((rand() % 300) - 150);
        y = (rand() % 40 + 15);
        z = ((rand() % 300) - 150);
        int type = (rand() % 5) + 1;

        std::vector<glm::vec3> route;

        for (int j = 0; j < 5; j++)
        {
            int localX = (rand() % 50);
            int localZ = (rand() % 50);
            int localY = (rand() % 5);

            route.push_back(glm::vec3(x + localX, y + localY, z + localZ));
        }

        fishRoute[i] = route;
    }

    glutMainLoop();

    shutdown();

    return 0;
}

int FindUnusedParticle() {

    for (int i = LastUsedParticle; i < MaxParticles; i++) {
        if (ParticlesContainer[i].life < 0) {
            LastUsedParticle = i;
            return i;
        }
    }

    for (int i = 0; i < LastUsedParticle; i++) {
        if (ParticlesContainer[i].life < 0) {
            LastUsedParticle = i;
            return i;
        }
    }

    return 0;
}

void SortParticles() {
    std::sort(&ParticlesContainer[0], &ParticlesContainer[MaxParticles]);
}

GLuint loadDDS(const char* imagepath)
{
    unsigned char header[124];

    FILE* fp;

    fp = fopen(imagepath, "rb");
    if (fp == NULL) {
        printf("%s could not be opened. Are you in the right directory ? Don't forget to read the FAQ !\n", imagepath); getchar();
        return 0;
    }

    char filecode[4];
    fread(filecode, 1, 4, fp);
    if (strncmp(filecode, "DDS ", 4) != 0) {
        fclose(fp);
        return 0;
    }

    fread(&header, 124, 1, fp);

    unsigned int height = *(unsigned int*)&(header[8]);
    unsigned int width = *(unsigned int*)&(header[12]);
    unsigned int linearSize = *(unsigned int*)&(header[16]);
    unsigned int mipMapCount = *(unsigned int*)&(header[24]);
    unsigned int fourCC = *(unsigned int*)&(header[80]);


    unsigned char* buffer;
    unsigned int bufsize;
    bufsize = mipMapCount > 1 ? linearSize * 2 : linearSize;
    buffer = (unsigned char*)malloc(bufsize * sizeof(unsigned char));
    fread(buffer, 1, bufsize, fp);
    fclose(fp);

    unsigned int components = (fourCC == FOURCC_DXT1) ? 3 : 4;
    unsigned int format;
    switch (fourCC)
    {
    case FOURCC_DXT1:
        format = GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
        break;
    case FOURCC_DXT3:
        format = GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
        break;
    case FOURCC_DXT5:
        format = GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
        break;
    default:
        free(buffer);
        return 0;
    }

    GLuint textureID;
    glGenTextures(1, &textureID);

    glBindTexture(GL_TEXTURE_2D, textureID);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    unsigned int blockSize = (format == GL_COMPRESSED_RGBA_S3TC_DXT1_EXT) ? 8 : 16;
    unsigned int offset = 0;

    for (unsigned int level = 0; level < mipMapCount && (width || height); ++level)
    {
        unsigned int size = ((width + 3) / 4) * ((height + 3) / 4) * blockSize;
        glCompressedTexImage2D(GL_TEXTURE_2D, level, format, width, height,
            0, size, buffer + offset);

        offset += size;
        width /= 2;
        height /= 2;

        if (width < 1) width = 1;
        if (height < 1) height = 1;
    }

    free(buffer);

    return textureID;
}
