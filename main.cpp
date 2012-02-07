


#include <cstdio>

#include <GL/glew.h>
#include <GL/glut.h>

#include <iostream>
#include <fstream>
#include <string>
#include <cassert>

static void redisplay_all(void);

// viewing angle
float phi = 0, theta = 0;

// glut window identifiers
int w_main, w_scene, w_performance, w_light, w_heightmap, w_conemap, w_texture;

// shader program handlers
int sl_shader, sl_mode_var;

// last mouse position
int m_last_x, m_last_y;

std::string load_file(const char *filename)
{
    std::ifstream ifs(filename);
    std::string res;
    char ch;
    while (ifs.get(ch))
        res += ch;
    return res;
}

void print_program_infolog(int obj)
{
    int len;
    glGetProgramiv(obj, GL_INFO_LOG_LENGTH, &len);
    if (len > 1) {
        char buf[len + 1];
        int chars;
        glGetProgramInfoLog(obj, len, &chars, buf);
        printf("%d: %s.\n", len, buf);
    }
}

void print_shader_infolog(int obj)
{
    int len;
    glGetShaderiv(obj, GL_INFO_LOG_LENGTH, &len);
    if (len > 1) {
        char buf[len + 1];
        int chars;
        glGetShaderInfoLog(obj, len, &chars, buf);
        printf("%d: %s.\n", len, buf);
    }
}

void on_speckey(int key, int x, int y)
{
    float k = glutGetModifiers() == GLUT_ACTIVE_SHIFT ? 10 : 1;
    switch (key) {
        case GLUT_KEY_UP:    theta += k; break;
        case GLUT_KEY_DOWN:  theta -= k; break;
        case GLUT_KEY_LEFT:  phi   += k; break;
        case GLUT_KEY_RIGHT: phi   -= k; break;
    }
    
    redisplay_all();
}

void on_key(unsigned char key, int x, int y)
{
    switch (key) {
        case '\b': theta = phi = 0; break;
    }

    redisplay_all();
}

void on_mouseclick(int button, int state, int x, int y)
{
    m_last_x = x, m_last_y = y;
}

void on_mousemove(int x, int y)
{
    phi += m_last_x - x;
    theta += m_last_y - y;
    m_last_x = x, m_last_y = y;

    redisplay_all();
}

static int create_subwindow(void (*display_func)(void), void (*move_func)(int,int),
                            void (*click_func)(int,int,int,int))
{
    // create a new subwindow
    int window = glutCreateSubWindow(w_main, 1, 1, 2, 2);
    // set opengl features
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    // setup callbacks
    if (display_func) glutDisplayFunc(display_func);
    if (move_func) glutMotionFunc(move_func);
    if (click_func) glutMouseFunc(click_func);
    glutSpecialFunc(on_speckey);
    glutKeyboardFunc(on_key);
    
    return window;
}

static void reshape_subwindow(int wnd, int w, int h, bool perspective,
                    float rleft, float rtop, float rwidth, float rheight)
{
    int w1 = static_cast<int>(w * rwidth), h1 = static_cast<int>(h * rheight);
    int l1 = static_cast<int>(w * rleft), t1 = static_cast<int>(h * rtop);
    glutSetWindow(wnd);

    // set window geometry
    glutPositionWindow(l1, t1);
    glutReshapeWindow(w1, h1);

    // set projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glViewport(l1, t1, w1, h1);
    if (perspective) gluPerspective(45, 1.0, 0.1, 1000);
    glMatrixMode(GL_MODELVIEW);
}

static void redisplay_all(void)
{
    int wnds[] = {w_main, w_scene, w_performance, w_light, w_heightmap, w_conemap, w_texture};

    for (int i = 0; i < sizeof(wnds)/sizeof(*wnds); ++i) {
        glutSetWindow(wnds[i]);
        glutPostRedisplay();
    }
}

static void on_reshape(int w, int h)
{
    reshape_subwindow(w_scene,       w, h, true, 0.0, 0.0, 0.5, 2.0/3.0);
    reshape_subwindow(w_performance, w, h, true, 0.5, 0.0, 0.5, 2.0/3.0);
    reshape_subwindow(w_light,       w, h, false, 0.00, 2.0/3.0, 0.25, 1.0/3.0);
    reshape_subwindow(w_heightmap,   w, h, false, 0.25, 2.0/3.0, 0.25, 1.0/3.0);
    reshape_subwindow(w_texture,     w, h, false, 0.50, 2.0/3.0, 0.25, 1.0/3.0);
    reshape_subwindow(w_conemap,     w, h, false, 0.75, 2.0/3.0, 0.25, 1.0/3.0);

    redisplay_all();
}

void draw_quad(void)
{
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(0,0,1, 0,0,0, 0,1,0);

    glScalef(0.4, 0.4, 0.4);
    glTranslatef(0, 0, -1);
    glRotatef(theta, 1, 0, 0);
    glRotatef(phi, 0, 0, 1);

    glBegin(GL_QUADS);
        glNormal3f(0, 0, 1);
        glVertex3f(-1, -1, 0);
        glVertex3f(+1, -1, 0);
        glVertex3f(+1, +1, 0);
        glVertex3f(-1, +1, 0);
    glEnd();
}

void render_scene(void)
{
    glutSetWindow(w_scene);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //glColor3ub(0xff, 0x00, 0x00);
    glUseProgram(sl_shader);
    draw_quad();
    
    glutSwapBuffers();
}

void render_performance(void)
{
    glutSetWindow(w_performance);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //glColor3ub(0xff, 0xff, 0x00);
    //glUseProgram(sl_shader);
    draw_quad();

    glutSwapBuffers();
}

void render_main(void)
{
    glutSetWindow(w_main);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glutSwapBuffers();
}

void compile_shader(void)
{
    GLint status;
    sl_shader = glCreateProgram();

    printf("Vertex shader setup\n");
    int vs = glCreateShader(GL_VERTEX_SHADER);
    std::string vs_source = load_file("light.vs");
    const char *vs_src = vs_source.c_str();
    glShaderSource(vs, 1, &vs_src, 0);
    glCompileShader(vs);
    glAttachShader(sl_shader, vs);
    print_shader_infolog(vs);
    glGetShaderiv(vs, GL_COMPILE_STATUS, &status);
    assert(status == GL_TRUE);

    printf("Fragment shader setup\n");
    int ps = glCreateShader(GL_FRAGMENT_SHADER);
    std::string ps_source = load_file("shader.fs");
    const char *ps_src = ps_source.c_str();
    glShaderSource(ps, 1, &ps_src, 0);
    glCompileShader(ps);
    glAttachShader(sl_shader, ps);
    print_shader_infolog(ps);
    glGetShaderiv(ps, GL_COMPILE_STATUS, &status);
    assert(status == GL_TRUE);

    printf("Shader program setup\n");
    glLinkProgram(sl_shader);
    print_program_infolog(sl_shader);
    glGetProgramiv(sl_shader, GL_LINK_STATUS, &status);
    assert(status == GL_TRUE);
    
    //glUseProgram(sl_shader);
}

int main(int argc, char **argv)
{
    // initialize glut
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    //glutInitContextProfile(GLUT_CORE_PROFILE | GLUT_COMPATIBILITY_PROFILE);
    int w = 800 * 3 / 2, h = 600 * 3 / 2;

    // init main window
    glutInitWindowSize(w, h);
    w_main = glutCreateWindow("Local Raycasting Demo");
    glutReshapeFunc(on_reshape);
    glutDisplayFunc(render_main);
    glutSpecialFunc(on_speckey);
    glutKeyboardFunc(on_key);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    glewInit();
    
    const char *glslver = (const char *)glGetString(GL_SHADING_LANGUAGE_VERSION);
    printf("GLSL ver: %s\n", glslver ? glslver : "[null]");
    assert(glewIsSupported("GL_VERSION_2_0"));
    compile_shader();

    // subwindows
    w_scene       = create_subwindow(render_scene, on_mousemove, on_mouseclick);
    w_performance = create_subwindow(render_performance, on_mousemove, on_mouseclick);
    w_light       = create_subwindow(0, 0, 0);
    w_heightmap   = create_subwindow(0, 0, 0);
    w_conemap     = create_subwindow(0, 0, 0);
    w_texture     = create_subwindow(0, 0, 0);

    glutSetWindow(w_main);
    
    glutMainLoop();
}

