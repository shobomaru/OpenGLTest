// TessellationLine.cpp

#include <iostream>
#include <fstream>
#include <memory>
#include <sstream>

#ifdef _WIN32
#	include <GL/glew.h>
#else
#	include <unistd.h>
#	define GL_DO_NOT_WARN_IF_MULTI_GL_VERSION_HEADERS_INCLUDED
#	include <OpenGL/gl3.h>
#endif // #ifdef _WIN32

#include <GLFW/glfw3.h>


#define APP_NAME "TessellationLine"
#define SCREEN_WIDTH (800)
#define SCREEN_HEIGHT (600)

namespace
{
    std::unique_ptr<char[]> loadFile(const char* path)
    {
        auto fp = std::ifstream(path, std::ios::in | std::ios::binary);
        if(!fp)
            throw "File cannot open.";
        
        auto len = fp.seekg(0, std::ios::end).tellg();
        if(len <= 0)
            throw "File is empty.";
        if(sizeof(len) > sizeof(unsigned int) && len > UINT_MAX)
            throw "File is too big.";
        fp.seekg(0, std::ios::beg);
        
        std::unique_ptr<char []> cs(new char[static_cast<unsigned int>(len) + 1U]);
        fp.read(cs.get(), len);
        if(fp.fail())
            throw "Reading file failed.";
        
        if(fp.eof() || fp.gcount() != len)
            throw "File cannot read to the end.";
        
        cs.get()[len] = '\0';
        return cs;
    };
    
    const auto glshader_deletor = [](GLuint *shader) {
        glDeleteShader(*shader);
        delete shader;
    };
    
    std::unique_ptr<GLuint, decltype(glshader_deletor)> loadShader(const char* code, GLenum type)
    {
        GLuint shader = glCreateShader(type);
        glShaderSource(shader, 1, &code, nullptr);
        glCompileShader(shader);
        
        GLint compiled;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
        if(compiled == GL_FALSE)
        {
            char log[1024];
            GLsizei len = sizeof(log) / sizeof(*log);
            
            glGetShaderInfoLog(shader, len, &len, log);
            std::cerr << "glCompileShader() gets error." << std::endl << log;
            
            shader = GL_INVALID_VALUE;
            throw "Shader compile error.";
        }
        
        std::unique_ptr<GLuint, decltype(glshader_deletor)> g(new GLuint(shader), glshader_deletor);
        return g;
    }
}

struct DrawContext
{
    struct
    {
        GLuint tess;
    } prog;
    struct
    {
        GLuint vaoNull;
    } common;
};

static void error_callback(int error, const char* description)
{
    std::cerr << "GLFW gets error(" << error << ")." << std::endl << description;
    throw "GLFW error.";
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
}

static void init(DrawContext &context)
{
#ifdef SHADER_DIR
    // add current diretory to env
    {
#ifdef _WIN32
        static std::string envPath = "";
        if(envPath.size() == 0)
        {
            std::stringstream ss;
            ss << "PATH=" << getenv("PATH") << ":" << SHADER_DIR;
            envPath = ss.str();
            _putenv(envPath.c_str());
        }
#else
        std::stringstream ss;
        ss << getenv("PATH") << ":" << SHADER_DIR;
        setenv("PATH", ss.str().c_str(), 1);
#endif
    }
#endif // SHADER_DIR
    // create shaders
    {
        auto srcVS = loadFile("VertexShader.glsl");
        auto srcFS = loadFile("FragmentShader.glsl");
        auto srcTCS = loadFile("TCS.glsl");
        auto srcTES = loadFile("TES.glsl");
        context.prog.tess = glCreateProgram();
        auto vs = loadShader(srcVS.get(), GL_VERTEX_SHADER);
        auto fs = loadShader(srcFS.get(), GL_FRAGMENT_SHADER);
        auto tcs = loadShader(srcTCS.get(), GL_TESS_CONTROL_SHADER);
        auto tes = loadShader(srcTES.get(), GL_TESS_EVALUATION_SHADER);
        glAttachShader(context.prog.tess, *vs.get());
        glAttachShader(context.prog.tess, *fs.get());
        glAttachShader(context.prog.tess, *tcs.get());
        glAttachShader(context.prog.tess, *tes.get());
        glLinkProgram(context.prog.tess);
        
        GLint linked;
        glGetProgramiv(context.prog.tess, GL_LINK_STATUS, &linked);
        if(linked == GL_FALSE)
        {
            GLsizei len;
            char log[1024];
            
            glGetProgramInfoLog(context.prog.tess, sizeof(log) / sizeof(*log), &len, log);
            std::cerr << "glLinkProgram() gets error." << std::endl << log;
            
            throw "Shader link error.";
        }
    }
    // create common
    {
        glGenVertexArrays(1, &context.common.vaoNull);
    }
}

static void paint(DrawContext &cont)
{
    glClearColor(0.0f, 0.0f, 0.5f, 1.0f);
    glClearDepth(1.0f);
    glClearStencil(0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    
    //glEnable(GL_CULL_FACE);
    //glCullFace(GL_BACK);
    
    glUseProgram(cont.prog.tess);
    glPatchParameteri(GL_PATCH_VERTICES, 2);
    glBindVertexArray(cont.common.vaoNull);
    glDrawArrays(GL_PATCHES, 0, 2);
    
    GLint error = glGetError();
    if(error != GL_NO_ERROR)
    {
        std::cout << "OepnGL gets error(" /*<< std::hex*/ << error << ")." << std::endl;
    }
}

int main(int argc, char* argv[])
{
    glfwSetErrorCallback(error_callback);
    if (!glfwInit())
    {
        return -1;
    }
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_RED_BITS, 8);
    glfwWindowHint(GLFW_GREEN_BITS, 8);
    glfwWindowHint(GLFW_BLUE_BITS, 8);
    glfwWindowHint(GLFW_ALPHA_BITS, 8);
    glfwWindowHint(GLFW_DEPTH_BITS, 24);
    glfwWindowHint(GLFW_STENCIL_BITS, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
    GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, APP_NAME, NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }
    
    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);
    glfwSwapInterval(1);
    
#if _WIN32
    glewExperimental = GL_TRUE;
    auto glewErr = glewInit();
    if (glewErr != GLEW_OK)
    {
        puts((const char*)glewGetErrorString(glewErr));
        return 1;
    }
    // glewInit() occurs glGetError() == GL_INVALID_ENUM
    GLint glErr;
    while((glErr = glGetError()) != GL_NO_ERROR)
    {
    }
#endif
    
    DrawContext cont;
    memset(&cont, 0, sizeof cont);
    
    init(cont);
    
    while (!glfwWindowShouldClose(window))
    {
        paint(cont);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    
    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
