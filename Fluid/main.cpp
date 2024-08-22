
#include <iostream>
#include <JGL/JGL_Window.hpp>
#include <JGL/JGL.hpp>
#include <JGL/JGL_Button.hpp>
#include <JGL/JGL_Slider.hpp>
#include <JGL/JGL__Valued.hpp>
#include <JGL/JGL_Aligner.hpp>
#include "AnimView.hpp"
#include "SPHSystem.hpp"
#include "MeshGrid.hpp"
#include "SPH.hpp"
#include "SPH_GPU.hpp"
#include "MeshGrid_GPU.hpp"
#include <algorithm>


AnimView* view;
JGL::Window* window;
SPHSystem* sphSystem;
SPHSettings* sphSettings;
MeshGrid* marchingCube;

const int GRID_W = 100;
const int GRID_H = 100;

glm::ivec2 mousePt;
glm::ivec2 lastPt;
bool pressed = false;
bool isSlime = false;

float detltaTime = 0.05;
float restDensity = 1000;
float gasConst = 300;
float viscosity = 100;
float h = 1;
float surfaceTension = 2;
float rewindConstant = 0.4;
int boundary = 3;
glm::vec3 target = glm::vec3(5, 1, 5);

void initial() {
    sphSettings = new SPHSettings(restDensity, gasConst, viscosity, surfaceTension, h, -9.8f, boundary, rewindConstant, target);
    sphSystem = new SPHSystem(5, *sphSettings);
    marchingCube = new MeshGrid;
}

void frame(float dt) {
    //UpdateParticles(*sphSystem, *marchingCube, *sphSettings, detltaTime, isSlime);
    run_simulation(*sphSystem);
    std::chrono::system_clock::time_point start = std::chrono::system_clock::now();
    CreateMesh();

    std::chrono::duration<double> sec = std::chrono::system_clock::now() - start;
    std::cout << "Simulation\t" << sec.count() * 1000.f << " ms" << std::endl;
}

void push(float x, float y) {
    mousePt = glm::ivec2(x * GRID_W, y * GRID_H);
    pressed = true;
}

void release() {
    pressed = false;
}

void move(float x, float y) {
    lastPt = mousePt;
    mousePt = glm::ivec2(x * GRID_W, y * GRID_H);
}

void drag(float x, float y) {
    mousePt = glm::ivec2(x * GRID_W, y * GRID_H);
}
void control(char c) {
    if (c == '2') {
        isSlime = isSlime ? false : true;
    }
    if (c == ' ') {
        for (auto& p : sphSystem->particles)
            if (!p->isBoundary) p->velocity += glm::vec3(0, 5, 0);
    }
    else if (c == 87) {
        sphSettings->target += glm::vec3(0, 0, -0.5);
        for (auto& p : sphSystem->particles)
            if (!p->isBoundary) p->velocity += glm::vec3(0, 0, -.5);
    }
    else if (c == 83) {
        sphSettings->target += glm::vec3(0, 0, +0.5);
        for (auto& p : sphSystem->particles)
            if (!p->isBoundary) p->velocity += glm::vec3(0, 0, +.5);
    }
    else if (c == 65) {
        sphSettings->target += glm::vec3(-0.5, 0, 0);
        for (auto& p : sphSystem->particles)
            if (!p->isBoundary) p->velocity += glm::vec3(-.5, 0, 0);
    }
    else if (c == 68) {
        sphSettings->target += glm::vec3(+0.5, 0, 0);
        for (auto& p : sphSystem->particles)
            if (!p->isBoundary) p->velocity += glm::vec3(+.5, 0, 0);
    }
}
void render() {
    int box = 2 * boundary;
    //drawCylinder(glm::vec3(box,  box,  box), glm::vec3(box, box, 0  ), 0.01f);
    //drawCylinder(glm::vec3(box,  box,  box), glm::vec3(box, 0  , box), 0.01f);
    //drawCylinder(glm::vec3(box,  box,  box), glm::vec3(0  , box, box), 0.01f);
    //drawCylinder(glm::vec3(0  ,  0  ,  0  ), glm::vec3(0  , 0  , box), 0.01f);
    //drawCylinder(glm::vec3(0  ,  0  ,  0  ), glm::vec3(0  , box, 0  ), 0.01f);
    //drawCylinder(glm::vec3(0  ,  0  ,  0  ), glm::vec3(box, 0  , 0  ), 0.01f);
    //drawCylinder(glm::vec3(0  ,  box,  box), glm::vec3(0  , 0  , box), 0.01f);
    //drawCylinder(glm::vec3(0  ,  box,  box), glm::vec3(0  , box, 0  ), 0.01f);
    //drawCylinder(glm::vec3(box,  box,  0  ), glm::vec3(0  , box, 0  ), 0.01f);
    //drawCylinder(glm::vec3(box,  box,  0  ), glm::vec3(box, 0  , 0  ), 0.01f);
    //drawCylinder(glm::vec3(box,  0  ,  box), glm::vec3(0  , 0  , box), 0.01f);
    //drawCylinder(glm::vec3(box,  0  ,  box), glm::vec3(box, 0  , 0  ), 0.01f);
    //drawSphere(sphSettings->target, 0.5f);
    renderMesh();
    drawQuad(glm::vec3(0, -.7, 0), glm::vec3(0, 1, 0), glm::vec2(100, 100), glm::vec4(1));
    sphSystem->draw();
    //marchingCube->DrawMeshes();

}
void UIinit() {
    
    window->alignment(JGL::ALIGN_ALL);
    JGL::Aligner* imageAligner = new JGL::Aligner(0, 0, window->w(), window->h());
    imageAligner->type(JGL::Aligner::HORIZONTAL);
    
    view = new AnimView(0, 0, 1024, 720);
    imageAligner->resizable(view);
    JGL::Aligner* sliderAligner = new JGL::Aligner(0, 0, window->w() - view->w(), window->h());
    sliderAligner->type(JGL::Aligner::VERTICAL);
    sliderAligner->padding(5);
    sliderAligner->topPadding(10);
    sliderAligner->spacing(4);
    sliderAligner->alignment(JGL::ALIGN_TOP | JGL::ALIGN_SIDE);

    JGL::Slider<float>* dtSld = new JGL::Slider<float>(0, 0, sliderAligner->w(), JGL::_size_button_height(), "dt");
    dtSld->range(0.01, 0.1);
    dtSld->autoValue(detltaTime);

    JGL::Slider<float>* restDensitySld = new JGL::Slider<float>(0, 0, sliderAligner->w(), JGL::_size_button_height(), "surfaceTension");
    restDensitySld->range(0.01, 3);
    restDensitySld->autoValue(surfaceTension);

    JGL::Slider<float>* gasConstSld = new JGL::Slider<float>(0, 0, sliderAligner->w(), JGL::_size_button_height(), "gasConst");
    gasConstSld->range(1, 100);
    gasConstSld->autoValue(gasConst);

    JGL::Slider<float>* viscositySld = new JGL::Slider<float>(0, 0, sliderAligner->w(), JGL::_size_button_height(), "viscosity");
    viscositySld->range(0.1, 2);
    viscositySld->autoValue(viscosity);

    JGL::Slider<float>* rewindConstantSld = new JGL::Slider<float>(0, 0, sliderAligner->w(), JGL::_size_button_height(), "rewindConstant");
    rewindConstantSld->range(0.1, 2);
    rewindConstantSld->autoValue(rewindConstant);
}

int main(int argc, const char * argv[]) {
    initial();
    window = new JGL::Window(1024, 720,"simulation");
    view = new AnimView(0, 0, 1024, 720);
    //UIinit();

    view->initFunction = initial;
    view->frameFunction = frame;
    view->renderFunction = render;
    view->controlFunction = control;

    window->show();
    InitSPHGL(*sphSystem);
    InitMeshGL();
    JGL::_JGL::run();
    return 0;
}

