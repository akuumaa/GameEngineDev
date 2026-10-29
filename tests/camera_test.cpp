// Headless camera/input regression checks. GLFW hardware polling is stubbed below.
#include "engine/src/sim/camera.h"
#include "engine/src/core/inputhandler.h"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <json.hpp>
#include <array>
#include <cassert>
#include <cmath>
#include <iostream>

using Engine::Core::InputHandler;
using Engine::Sim::Camera;

extern "C" int glfwGetGamepadState(int, GLFWgamepadstate *) { return GLFW_FALSE; }
extern "C" int glfwUpdateGamepadMappings(const char *) { return GLFW_TRUE; }

static std::array<double, 16> values(const Matrix &m)
{
    return {m.m11,m.m12,m.m13,m.m14,m.m21,m.m22,m.m23,m.m24,
            m.m31,m.m32,m.m33,m.m34,m.m41,m.m42,m.m43,m.m44};
}
static void near(const Matrix &a, const Matrix &b, double tolerance=1e-9)
{
    auto av=values(a), bv=values(b);
    for (size_t i=0;i<av.size();++i)
    {
        assert(std::isfinite(av[i]));
        assert(std::abs(av[i]-bv[i])<tolerance);
    }
}
static void settle(Camera &camera)
{
    InputHandler::resetMouse();
    InputHandler::update();
    camera.update(10.0);
}
static Matrix smoothedAt(int fps)
{
    InputHandler::resetMouse();
    auto camera=Camera::create(Json{{"type","orbit"},{"distance",10.0}});
    InputHandler::cursorPosCallback(0,0);
    InputHandler::cursorPosCallback(128,64);
    for (int frame=0;frame<fps;++frame)
    {
        InputHandler::update();
        camera.update(1.0/fps);
    }
    return camera.getViewMatrix({},{});
}
int main()
{
    // First sample must not jump; multiple callbacks accumulate; next frame clears them.
    InputHandler::resetMouse();
    InputHandler::cursorPosCallback(1000,2000);
    InputHandler::cursorPosCallback(1100,2020);
    InputHandler::cursorPosCallback(1200,2040);
    InputHandler::scrollCallback(0,2);
    InputHandler::update();
    assert(InputHandler::getMouseDeltaX()==200);
    assert(InputHandler::getMouseDeltaY()==40);
    assert(InputHandler::getMouseDeltaZ()==2);
    InputHandler::update();
    assert(InputHandler::getMouseDeltaX()==0 && InputHandler::getMouseDeltaZ()==0);
    InputHandler::cursorPosCallback(1300,2040);
    InputHandler::resetMouse();
    InputHandler::cursorPosCallback(9000,9000);
    InputHandler::update();
    assert(InputHandler::getMouseDeltaX()==0);

    auto fixed=Camera::create(Json{{"name","Chase"},{"offset",{-14,0,2}}});
    assert(fixed.getName()=="Chase");
    near(fixed.getViewMatrix({},{}),Matrix::lookAt({-14,0,2},{-13,0,2}));
    auto orbit=Camera::create(Json{{"type","orbit"},{"distance",10.0}});
    near(orbit.getViewMatrix({},{}),Matrix::lookAt({-10,0,0},{}));
    Vector3 position={20,-30,5};
    Quaternion rotation(0.6,-0.2,0.4);
    near(orbit.getViewMatrix(position,rotation)*Matrix::translation(position)*rotation.toMatrix(),
         orbit.getViewMatrix({},{}));

    // Reversed limits and invalid initial distances are clamped at creation.
    auto limited=Camera::create(Json{{"rotation",{100,90}},{"yawLimit",{45,-45}},
                                    {"pitchLimit",{30,-18}}});
    auto expected=Camera::create(Json{{"rotation",{45,30}}});
    near(limited.getViewMatrix({},{}),expected.getViewMatrix({},{}));
    auto closest=Camera::create(Json{{"type","orbit"},{"distance",-10}});
    near(closest.getViewMatrix({},{}),Matrix::lookAt({-1.5,0,0},{}));
    auto extreme=Camera::create(Json{{"pitchLimit",{120,180}},{"rotation",{0,150}}});
    for (double v:values(extreme.getViewMatrix({},{}))) assert(std::isfinite(v));

    // A mouse impulse produces the same smoothing after one second at either rate.
    near(smoothedAt(30),smoothedAt(144));
    InputHandler::resetMouse();
    InputHandler::scrollCallback(0,1);
    InputHandler::update();
    orbit.update(1.0/60.0);
    settle(orbit);
    near(orbit.getViewMatrix({},{}),Matrix::lookAt({-9,0,0},{}));

    // Stopping on camera switch preserves the visible pose with no delayed movement.
    InputHandler::cursorPosCallback(0,0);
    InputHandler::cursorPosCallback(200,100);
    InputHandler::update();
    orbit.update(1.0/60.0);
    Matrix before=orbit.getViewMatrix({},{});
    orbit.stopSmoothing();
    settle(orbit);
    near(before,orbit.getViewMatrix({},{}));

    // Unlimited yaw crosses the wrap boundary by the short route.
    auto wrap=Camera::create(Json{{"rotation",{179,0}}});
    InputHandler::resetMouse();
    InputHandler::cursorPosCallback(0,0);
    InputHandler::cursorPosCallback(-8.93608577,0); // approximately +2 degrees
    InputHandler::update();
    wrap.update(1.0/60.0);
    auto initial=Camera::create(Json{{"rotation",{179,0}}});
    auto a=values(wrap.getViewMatrix({},{})),b=values(initial.getViewMatrix({},{}));
    for(size_t i=0;i<a.size();++i) assert(std::abs(a[i]-b[i])<0.02);
    std::cout << "Camera and mouse checks passed\n";
}
