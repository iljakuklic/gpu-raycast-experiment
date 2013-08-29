#include "Application.hpp"

#include <stdexcept>
#include <GL/glut.h>

class ApplicationPriv {
    public:
        // GLUT event handlers
        static void do_display() { Application::get()->do_display(); }
        static void do_resize(int w, int h) { Application::get()->do_resize(w, h); }
};

Application::Application(const char *name) :
    w_(0), h_(0)
{
    // setup singleton instance
    if (instance != 0)
        throw std::logic_error("Multiple Application instantiations! Fix your app!");
    instance = this;


    // fake program params
    static char progname[] = "foo";
    static char *fake_argv[] = { progname, 0 };
    static int fake_argc = 1;
    // initialise glut
    glutInit(&fake_argc, fake_argv);

    // initialise window
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(800, 600);
    glutCreateWindow(name);

    // register callbacks
    glutReshapeFunc(ApplicationPriv::do_resize);
    glutDisplayFunc(ApplicationPriv::do_display);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
}

Application::~Application() {}

int Application::run()
{
    glutMainLoop();
    return 0;
}

// window resize handler
void Application::do_resize(int w, int h)
{
    if (h < 1) h = 1;

    GLfloat rng = 100.0f;
    GLfloat ratio = static_cast<float>(w) / h;

    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();


    if (w < h) w_ = rng, h_ = rng / ratio;
    else       w_ = rng * ratio, h_ = rng;

    glOrtho(-w_, w_, -h_, h_, -2.0f * rng, 2.0f * rng);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    viewport_resize();
}

void Application::do_display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    display(); // polymorphic call
    glutSwapBuffers();
}

void Application::viewport_resize() {}

// application instance pointer
Application* Application::instance = 0;
