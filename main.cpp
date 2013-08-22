


#include <cstdio>

#include <GL/glew.h>
#include <GL/glut.h>

#include <iostream>
#include <fstream>
#include <string>
#include <cassert>
#include <cmath>

#include <png.h>

void use_shader(bool draw_performance);
static void redisplay_all(void);
bool loadPngImage(char *name, int &outWidth, int &outHeight, int &outChannels, GLubyte **outData);
void load_texture(int tex, const char *filename);

enum Textures {
    T_DIFFUSE = GL_TEXTURE0,
    T_HEIGHTMAP,
    T_NORMALMAP,
    T_CONEMAP,
    T_RCONEMAP,
    T_END_,
    T_COUNT = T_END_ - GL_TEXTURE0
};

// viewing angle
float phi = 0, theta = 0;

// shader program handlers
int sl_shader, sl_mode_var, sl_lightpos_var, sl_draw_performance_var, sl_root_var;
// shader texture vars
int slt_diffuse, slt_heightmap, slt_conemap;

// last mouse position
int m_last_x, m_last_y;
// screen width, height
int width, height;

// mode
// 0 - color only, 1 - normal map, 2 - parallax map, 3 - relief map, 4 - cone map, 5 - relaxed cone map
int mode;
// root-finding method: 0 - none, 1 - binary search
int root_method;

// light position
float lt_x = 0.0, lt_y = 0.0, lt_z = 15;

// texture memory
GLubyte *textures[T_COUNT];

std::string load_file(const char *filename)
{
    std::ifstream ifs(filename);
    std::string res;
    char ch;
    while (ifs.get(ch)) res += ch;
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
    
    glutPostRedisplay();
}

void on_key(unsigned char key, int x, int y)
{
    const float k = 0.2;
    switch (tolower(key)) {
        case '\b': lt_y = lt_x = 0.0; theta = phi = 0; break;
        case 'w': lt_y += k; break;
        case 's': lt_y -= k; break;
        case 'd': lt_x += k; break;
        case 'a': lt_x -= k; break;
        case 'm': root_method++; root_method %= 2; break;
    }
    if (key >= '0' && key <= '9')
        mode = key - '0';

    glutPostRedisplay();
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

    glutPostRedisplay();
}

static void set_viewport(bool perspective, float rleft, float rtop, float rwidth, float rheight)
{
    // calclulate absolute screen space coordinates
    int w1 = static_cast<int>(width * rwidth), h1 = static_cast<int>(height * rheight);
    int l1 = static_cast<int>(width * rleft), t1 = static_cast<int>(height * rtop);

    // set projection
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glViewport(l1, t1, w1, h1);
    if (perspective) gluPerspective(45, 1.0, 0.1, 1000);
    glMatrixMode(GL_MODELVIEW);
}

static void on_reshape(int w, int h)
{
    width = w; height = h;
    glutPostRedisplay();
}

void draw_quad(void)
{
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glTranslatef(0, 0, -1);
    glRotatef(theta, 1, 0, 0);
    glRotatef(phi, 0, 0, 1);
    glScalef(0.3, 0.3, 0.3);

    glBegin(GL_QUADS);
        glTexCoord2f(0.0, 0.0);
        glVertex3f(-1, -1, 0);
        glTexCoord2f(1.0, 0.0);
        glVertex3f(+1, -1, 0);
        glTexCoord2f(1.0, 1.0);
        glVertex3f(+1, +1, 0);
        glTexCoord2f(0.0, 1.0);
        glVertex3f(-1, +1, 0);
    glEnd();
}

void render_scene(void)
{
    use_shader(false);
    set_viewport(true, 0.0, 0.0, 0.5, 1.0);
    draw_quad();
}

void render_performance(void)
{
    use_shader(true);
    set_viewport(true, 0.5, 0.0, 0.5, 1.0);
    draw_quad();
}

void render_main(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    render_scene();
    render_performance();
    glutSwapBuffers();
}

void use_shader(bool draw_performance)
{
    // set uniform vars
    glUniform3f(sl_lightpos_var, lt_x, lt_y, lt_z);
    glUniform1i(sl_mode_var, mode);
    glUniform1i(sl_draw_performance_var, draw_performance);
    glUniform1i(sl_root_var, root_method);
    
    // texture variable locations
    static int difloc = glGetUniformLocation(sl_shader, "TDiffuse");
    static int htloc  = glGetUniformLocation(sl_shader, "THeightMap");
    static int nloc   = glGetUniformLocation(sl_shader, "TNormalMap");
    static int cloc   = glGetUniformLocation(sl_shader, "TConeMap");
    static int rcloc  = glGetUniformLocation(sl_shader, "TRConeMap");
    
    // bind texture samplers
    glUniform1i(difloc, T_DIFFUSE - GL_TEXTURE0);
    glUniform1i(htloc,  T_HEIGHTMAP - GL_TEXTURE0);
    glUniform1i(nloc,   T_NORMALMAP - GL_TEXTURE0);
    glUniform1i(cloc,   T_CONEMAP - GL_TEXTURE0);
    glUniform1i(rcloc,  T_RCONEMAP - GL_TEXTURE0);
}

void compile_shader(void)
{
    GLint status;
    sl_shader = glCreateProgram();

    // vertex shader
    printf("Vertex shader setup\n");
    int vs = glCreateShader(GL_VERTEX_SHADER);
    std::string vs_source = load_file("shader.v.glsl");
    const char *vs_src = vs_source.c_str();
    glShaderSource(vs, 1, &vs_src, 0);
    glCompileShader(vs);
    glAttachShader(sl_shader, vs);
    print_shader_infolog(vs);
    glGetShaderiv(vs, GL_COMPILE_STATUS, &status);
    assert(status == GL_TRUE);

    // fragment shader
    printf("Fragment shader setup\n");
    int ps = glCreateShader(GL_FRAGMENT_SHADER);
    std::string ps_source = load_file("shader.f.glsl");
    const char *ps_src = ps_source.c_str();
    glShaderSource(ps, 1, &ps_src, 0);
    glCompileShader(ps);
    glAttachShader(sl_shader, ps);
    print_shader_infolog(ps);
    glGetShaderiv(ps, GL_COMPILE_STATUS, &status);
    assert(status == GL_TRUE);

    // link shader program
    printf("Shader program setup\n");
    glLinkProgram(sl_shader);
    print_program_infolog(sl_shader);
    glGetProgramiv(sl_shader, GL_LINK_STATUS, &status);
    assert(status == GL_TRUE);

    // get variables location
    sl_lightpos_var = glGetUniformLocation(sl_shader, "LightPos");
    sl_mode_var     = glGetUniformLocation(sl_shader, "Mode");
    sl_draw_performance_var = glGetUniformLocation(sl_shader, "DrawPerformance");
    sl_root_var = glGetUniformLocation(sl_shader, "RootMethod");
    // get texture variable locations
    slt_conemap   = glGetUniformLocation(sl_shader, "TConeMap");
    slt_diffuse   = glGetUniformLocation(sl_shader, "TDiffuse");
    slt_heightmap = glGetUniformLocation(sl_shader, "THeightMap");

    // set shader as active
    glUseProgram(sl_shader);
}

int main(int argc, char **argv)
{
    assert(argv[1]);

    // initialize glut
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    int w = 1800, h = 900;

    // init main window
    glutInitWindowSize(w, h);
    glutCreateWindow("Local Raycasting Demo");
    glutReshapeFunc(on_reshape);
    glutDisplayFunc(render_main);
    glutSpecialFunc(on_speckey);
    glutKeyboardFunc(on_key);
    glutMotionFunc(on_mousemove);
    glutMouseFunc(on_mouseclick);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_TEXTURE_2D);
    glClearColor(0.0,0.0,0.0,1.0);

    glewInit();
    
    const char *glslver = (const char *)glGetString(GL_SHADING_LANGUAGE_VERSION);
    printf("GLSL ver: %s\n", glslver ? glslver : "[null]");
    assert(glewIsSupported("GL_VERSION_2_0"));
    compile_shader();

    load_texture(T_HEIGHTMAP, argv[1]);
    load_texture(T_DIFFUSE, "../img/hrube_512x512.png");
    
    glutMainLoop();
}

void load_texture(int tex, const char *filename)
{
    int tidx = tex - GL_TEXTURE0;
    int wt, ht, channels;
    char fname[strlen(filename)+1];
    GLuint tname;
    
    strcpy(fname, filename);
    bool ok = loadPngImage(fname, wt, ht, channels, &textures[tidx]);
    assert(ok);
    std::cout << "Image loaded " << wt << " " << ht << " channels " << channels << std::endl;

    int type = 0;
    switch (channels) {
        case 4: type = GL_RGBA; break;
        case 3: type = GL_RGB;  break;
        case 1: type = GL_LUMINANCE; break;
        default: assert(false); break;
    }

    glActiveTexture(tex);
    glGenTextures(1, &tname);
    glBindTexture(GL_TEXTURE_2D, tname);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, channels, wt, ht, 0, type, GL_UNSIGNED_BYTE, textures[tidx]);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

    if (tex != T_HEIGHTMAP) return;

    // compute cone map

    //ht /= 2; wt /= 2;
    int cmid = T_CONEMAP - GL_TEXTURE0, rcmid = T_RCONEMAP - GL_TEXTURE0;
    int tsize = sizeof(GLubyte) * wt * ht;
    textures[cmid]  = (GLubyte*) malloc(tsize);
    textures[rcmid] = (GLubyte*) malloc(tsize);

    std::cout << "Computing conemap: |" << std::string('.', wt) << "|\b" << std::string('\b', wt) << std::flush;
    for (int sx = 0; sx < wt; ++sx) for (int sy = 0; sy < ht; ++sy) {
        GLubyte sz = textures[tidx][ht*sx+sy];
        float cone = 1.0;
        for (int tx = 0; tx < wt; ++tx) for (int ty = 0; ty < ht; ++ty) {
            GLubyte tz = textures[tidx][ht*tx+ty];
            float dx = tx - sx, dy = ty - sy, dz = tz - sz;
            if (dz > 0.1) {
                cone = std::min(cone, sqrtf(dx*dx + dy*dy) / (dz * ht / 256.0f));
            }
        }
        textures[cmid][ht*sx+sy] = static_cast<GLubyte>(cone * 255.99f);
        if (sy == ht-1) std::cout << '*' << std::flush;
    }
    std::cout << std::endl;

    glActiveTexture(T_CONEMAP);
    glGenTextures(1, &tname);
    glBindTexture(GL_TEXTURE_2D, tname);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, 1, wt, ht, 0, type, GL_UNSIGNED_BYTE, textures[cmid]);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    
}


// taken from: http://blog.nobel-joergensen.com/2010/11/07/loading-a-png-as-texture-in-opengl-using-libpng/
bool loadPngImage(char *name, int &outWidth, int &outHeight, int &outChannels, GLubyte **outData)
{
    png_structp png_ptr;
    png_infop info_ptr;
    unsigned int sig_read = 0;
    int color_type, interlace_type;
    FILE *fp;

    if ((fp = fopen(name, "rb")) == NULL)
        return false;
    
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
            NULL, NULL, NULL);

    if (png_ptr == NULL) {
        fclose(fp);
        return false;
    }

    info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == NULL) {
        fclose(fp);
        png_destroy_read_struct(&png_ptr, png_infopp_NULL, png_infopp_NULL);
        return false;
    }

    if (setjmp(png_jmpbuf(png_ptr))) {
        png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);
        fclose(fp);
        return false;
    }

    png_init_io(png_ptr, fp);

    png_set_sig_bytes(png_ptr, sig_read);

    png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_STRIP_16 | PNG_TRANSFORM_PACKING | PNG_TRANSFORM_EXPAND, png_voidp_NULL);

    outWidth = info_ptr->width;
    outHeight = info_ptr->height;
    switch (info_ptr->color_type) {
        case PNG_COLOR_TYPE_RGBA:
            outChannels = 4;
            break;
        case PNG_COLOR_TYPE_RGB:
            outChannels = 3;
            break;
        case PNG_COLOR_TYPE_GRAY:
            outChannels = 1;
            break;
        default:
            std::cout << "Color type " << info_ptr->color_type << " not supported" << std::endl;
            png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
            fclose(fp);
            return false;
    }
    unsigned int row_bytes = png_get_rowbytes(png_ptr, info_ptr);
    *outData = (unsigned char*) malloc(row_bytes * outHeight);

    png_bytepp row_pointers = png_get_rows(png_ptr, info_ptr);

    for (int i = 0; i < outHeight; i++) {
        memcpy(*outData+(row_bytes * (outHeight-1-i)), row_pointers[i], row_bytes);
    }
    
    png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);

    fclose(fp);
    return true;
}
