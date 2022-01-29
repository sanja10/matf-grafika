#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader.h>
#include <rg/Camera.h>
#include <learnopengl/model.h>

#include <iostream>

#include <rg/service_locator.h>
void framebuffer_size_callback(GLFWwindow *window, int width, int height);

void mouse_callback(GLFWwindow *window, double xpos, double ypos);

void scroll_callback(GLFWwindow *window, double xoffset, double yoffset);

void processInput(GLFWwindow *window);

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);

void renderQuad();

unsigned int loadTexture(char const * path);
unsigned int loadCubemap(vector <std::string> faces);

// settings
const unsigned int SCR_WIDTH = 1000;
const unsigned int SCR_HEIGHT = 900;

// camera

float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

struct PointLight {
    glm::vec3 position;
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;

    float constant;
    float linear;
    float quadratic;
};

struct ProgramState {
    glm::vec3 clearColor = glm::vec3(0.5f, 0.9f, 0.2f);
    bool ImGuiEnabled = false;

    // camera
    Camera camera;
    bool CameraMouseMovementUpdateEnabled = true;

    // settings
    glm::vec3 snowManPosition = glm::vec3(-2.0f, -1.5f, -4.0f);
    glm::vec3 treePosition = glm::vec3(3.0f, 1.0f, -8.0f);
    float snowManScale = 2.0f;
    float treeScale = 0.04f;
    float giftScale = 0.5f;
    unsigned int numOfGifts = 4;

    // light settings
    glm::vec3 dirLightDir = glm::vec3(-0.2f, -1.0f, -0.3f);
    glm::vec3 dirLightAmbDiffSpec = glm::vec3(0.3f, 0.3f,0.2f);
    bool spotlightOn = true;
    bool blinn = true;
    bool blinnKeyPressed = false;

    bool bloom = true;
    bool bloomKeyPressed = false;
    float exposure = 1.0f;

    PointLight pointLight;
    ProgramState()
            : camera(glm::vec3(0.0f, -1.0f, 12.0f)) {}

    void SaveToFile(std::string filename);

    void LoadFromFile(std::string filename);
};

void ProgramState::SaveToFile(std::string filename) {
    std::ofstream out(filename);
    out << clearColor.r << '\n'
        << clearColor.g << '\n'
        << clearColor.b << '\n'
        << ImGuiEnabled << '\n'
        << camera.Position.x << '\n'
        << camera.Position.y << '\n'
        << camera.Position.z << '\n'
        << camera.Front.x << '\n'
        << camera.Front.y << '\n'
        << camera.Front.z << '\n';
}

void ProgramState::LoadFromFile(std::string filename) {
    std::ifstream in(filename);
    if (in) {
        in >> clearColor.r
           >> clearColor.g
           >> clearColor.b
           >> ImGuiEnabled
           >> camera.Position.x
           >> camera.Position.y
           >> camera.Position.z
           >> camera.Front.x
           >> camera.Front.y
           >> camera.Front.z;
    }
}

ProgramState *programState;

void DrawImGui(ProgramState *programState);

int main() {
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetKeyCallback(window, key_callback);
    // tell GLFW to capture our mouse
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // tell stb_image.h to flip loaded texture's on the y-axis (before loading model).
    stbi_set_flip_vertically_on_load(true);

    programState = new ProgramState;
    //programState->LoadFromFile("resources/program_state.txt");
    if (programState->ImGuiEnabled) {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }
    // Init Imgui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void) io;



    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    float giftVertices[] = {
            //positions                        //normals                      //texCoords
            -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
            0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f,
            0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,
            0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,
            -0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f,
            -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,

            -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
            0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
            0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
            0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
            -0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
            -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,

            -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
            -0.5f, 0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
            -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
            -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
            -0.5f, -0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
            -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,

            0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
            0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
            0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
            0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
            0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
            0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,


            -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f,
            0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f,
            0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,
            0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,
            -0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f,
            -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f,

            -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
            0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
            0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
            0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
            -0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
            -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f

    };


    glm::vec3 giftPositions[] = {
            glm::vec3(3.0f, 1.0f, -1.0f),
            glm::vec3(3.3f, 1.2f, -1.0f),
            glm::vec3(2.5f, 1.0f, -1.0f),
            glm::vec3(2.0f, 1.2f, -1.0f)
    };

    float skyboxVertices[] = {
            // positions
            -1.0f,  1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            -1.0f,  1.0f, -1.0f,
            1.0f,  1.0f, -1.0f,
            1.0f,  1.0f,  1.0f,
            1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
            1.0f, -1.0f,  1.0f
    };

    // snowflake rectangle
    float transparentVertices[] = {
            // positions                // texture Coords (swapped y coordinates because texture is flipped upside down)
            0.0f,  0.5f,  0.0f,  0.0f,  0.0f,
            0.0f, -0.5f,  0.0f,  0.0f,  1.0f,
            1.0f, -0.5f,  0.0f,  1.0f,  1.0f,

            0.0f,  0.5f,  0.0f,  0.0f,  0.0f,
            1.0f, -0.5f,  0.0f,  1.0f,  1.0f,
            1.0f,  0.5f,  0.0f,  1.0f,  0.0f
    };

    vector<glm::vec3> snowflakePosition
    {
           glm::vec3(-1.0f, 4.0f, -0.45f),
           glm::vec3( 2.5f, 4.5f, 0.55f),
           glm::vec3( 0.5f, 4.0f, 0.7f),
           glm::vec3(1.5f, 4.0f, -2.5f),
           glm::vec3 (4.5f, 4.5f, -0.5f),
           glm::vec3(-2.0f, 4.0f, -0.50f),
           glm::vec3( 2.7f, 3.0f, 0.50f),
           glm::vec3( 0.5f, 3.0f, 0.7f),
           glm::vec3(1.5f, 4.0f, -2.5f),
           glm::vec3 (4.7f, 4.0f, -0.55f)
    };

    float cubeVertices[] = {
            // back face
            -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, // bottom-left
            0.5f, -0.5f, -0.5f,  1.0f, 0.0f, // bottom-right
            0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-right
            0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-right
            -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, // top-left
            -0.5f, -0.5f, -0.5f,  0.0f, 0.0f, // bottom-left
            // front face
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-left
            0.5f,  0.5f,  0.5f,  1.0f, 1.0f, // top-right
            0.5f, -0.5f,  0.5f,  1.0f, 0.0f, // bottom-right
            0.5f,  0.5f,  0.5f,  1.0f, 1.0f, // top-right
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-left
            -0.5f,  0.5f,  0.5f,  0.0f, 1.0f, // top-left
            // left face
            -0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // top-right
            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // bottom-left
            -0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-left
            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // bottom-left
            -0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // top-right
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-right
            // right face
            0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // top-left
            0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-right
            0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // bottom-right
            0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // bottom-right
            0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-left
            0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // top-left
            // bottom face
            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // top-right
            0.5f, -0.5f,  0.5f,  1.0f, 0.0f, // bottom-left
            0.5f, -0.5f, -0.5f,  1.0f, 1.0f, // top-left
            0.5f, -0.5f,  0.5f,  1.0f, 0.0f, // bottom-left
            -0.5f, -0.5f, -0.5f,  0.0f, 1.0f, // top-right
            -0.5f, -0.5f,  0.5f,  0.0f, 0.0f, // bottom-right
            // top face
            -0.5f,  0.5f, -0.5f,  0.0f, 1.0f, // top-left
            0.5f,  0.5f, -0.5f,  1.0f, 1.0f, // top-right
            0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // bottom-right
            0.5f,  0.5f,  0.5f,  1.0f, 0.0f, // bottom-right
            -0.5f,  0.5f,  0.5f,  0.0f, 0.0f, // bottom-left
            -0.5f,  0.5f, -0.5f,  0.0f, 1.0f  // top-left
    };

    vector<glm::vec3> cubePosition
            {
                    glm::vec3(7.0f, 0.0f, -11.5f),
                    glm::vec3( 8.0f, 0.0f, -11.5f),
                    glm::vec3( 9.0f, 0.0f, -11.5f),
                    glm::vec3(7.5f, 1.0f, -11.5f),
                    glm::vec3 (8.5f, 1.0f, -11.5f),
                    glm::vec3(8.0f, 2.0f, -11.5f)
            };


    // gift VAO, VABO
    unsigned int giftVBO, giftVAO;
    glGenVertexArrays(1, &giftVAO);
    glGenBuffers(1, &giftVBO);

    glBindBuffer(GL_ARRAY_BUFFER, giftVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(giftVertices), giftVertices, GL_STATIC_DRAW );


    glBindVertexArray(giftVAO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(3*sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(6*sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);


    //skybox VAO, VBO
    unsigned int skyboxVAO, skyboxVBO;
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);


    //snowflake VAO, VBO
    unsigned int transparentVAO, transparentVBO;
    glGenVertexArrays(1, &transparentVAO);
    glGenBuffers(1, &transparentVBO);
    glBindVertexArray(transparentVAO);
    glBindBuffer(GL_ARRAY_BUFFER, transparentVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(transparentVertices), transparentVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glBindVertexArray(0);


    // configure global opengl state
    // -----------------------------
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    //glFrontFace(GL_CW);

    // build and compile shaders
    // -------------------------
    Shader modelShader("resources/shaders/model.vs", "resources/shaders/model.fs");
    Shader giftShader("resources/shaders/box.vs", "resources/shaders/box.fs");
    Shader skyboxShader("resources/shaders/skybox.vs", "resources/shaders/skybox.fs");
    Shader snowShader("resources/shaders/snowflakeShader.vs", "resources/shaders/snowflakeShader.fs");
    Shader shader("resources/shaders/face_culling.vs", "resources/shaders/face_culling.fs");
    Shader shaderBlur("resources/shaders/blur.vs", "resources/shaders/blur.fs");
    Shader shaderBloom("resources/shaders/bloom.vs", "resources/shaders/bloom.fs");

    // load textures
    unsigned int giftTexture = loadTexture("resources/textures/wrapPaper.png");
    unsigned int iceTexture = loadTexture("resources/textures/ice_texture.jpg");
    unsigned int snowflakeTexture = loadTexture("resources/textures/snowflake.png");

    // load models
    // -----------
    Model snowManModel("resources/objects/snowman/snowman_finish.obj", true);
    Model treeModel("resources/objects/Christmas_Tree/Christmas_Tree/12150_Christmas_Tree_V2_L2.obj");

    snowManModel.SetShaderTextureNamePrefix("material.");
    treeModel.SetShaderTextureNamePrefix("material.");

    //Bloom-------------------------------------------------------------------------------------------------------------
     // configure framebuffers
     // ---------------------------------------
     unsigned int hdrFBO;
    glGenFramebuffers(1, &hdrFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
    // create 2 color buffers (1 for normal rendering, other for brightness threshold values)
    unsigned int colorBuffers[2];
    glGenTextures(2, colorBuffers);
    for (unsigned int i = 0; i < 2; i++)
    {
        glBindTexture(GL_TEXTURE_2D, colorBuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);  // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        // attach texture to framebuffer
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, colorBuffers[i], 0);
    }
    // create and attach depth buffer (renderbuffer)
    unsigned int rboDepth;
    glGenRenderbuffers(1, &rboDepth);
    glBindRenderbuffer(GL_RENDERBUFFER, rboDepth);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, SCR_WIDTH, SCR_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
    unsigned int attachments[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glDrawBuffers(2, attachments);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, colorBuffers[0], 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rboDepth);
    // check if framebuffer is complete
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "Framebuffer not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // ping-pong-framebuffer for blurring
    unsigned int pingpongFBO[2];
    unsigned int pingpongColorbuffers[2];
    glGenFramebuffers(2, pingpongFBO);
    glGenTextures(2, pingpongColorbuffers);
    for (unsigned int i = 0; i < 2; i++)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[i]);
        glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE); // we clamp to the edge as the blur filter would otherwise sample repeated texture values!
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, pingpongColorbuffers[i], 0);
        // also check if framebuffers are complete (no need for depth buffer)
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            std::cout << "Framebuffer not complete!" << std::endl;
    }


    PointLight& pointLight = programState->pointLight;
    pointLight.position = glm::vec3(4.0f, 3.0, -8.0);
    pointLight.ambient = glm::vec3(0.9, 0.9, 0.9);
    pointLight.diffuse = glm::vec3(0.8, 0.8, 0.6);
    pointLight.specular = glm::vec3(1.0, 1.0, 1.0);

    pointLight.constant = 1.0f;
    pointLight.linear = 0.09f;
    pointLight.quadratic = 0.032f;

    vector<std::string> faces {
        FileSystem::getPath("resources/textures/winter/frost_rt.png"),
        FileSystem::getPath("resources/textures/winter/frost_lf.png"),
        FileSystem::getPath("resources/textures/winter/frost_up.png"),
        FileSystem::getPath("resources/textures/winter/frost_dn.png"),
        FileSystem::getPath("resources/textures/winter/frost_ft.png"),
        FileSystem::getPath("resources/textures/winter/frost_bk.png")
    };

    unsigned int cubemapTexture = loadCubemap(faces);

    modelShader.use();
    modelShader.setInt("diffuseTexture", 0);

    skyboxShader.use();
    skyboxShader.setInt("skybox", 0);


    // cube VAO
    unsigned int cubeVAO, cubeVBO;
    glGenVertexArrays(1, &cubeVAO);
    glGenBuffers(1, &cubeVBO);
    glBindVertexArray(cubeVAO);
    glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeVertices), &cubeVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glBindVertexArray(0);



    // shader configuration
    // --------------------
    shader.use();
    shader.setInt("texture1", 0);

    shaderBlur.use();
    shaderBlur.setInt("image", 0);
    shaderBloom.use();
    shaderBloom.setInt("scene", 0);
    shaderBloom.setInt("bloomBlur", 1);


    auto &initServiceLocator = rg::ServiceLocator::Get();
    // draw in wireframe
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // render loop
    // -----------
    rg::ServiceLocator::Get().getEventController().subscribeToEvent(rg::EventType::MouseMoved, &programState->camera);
    rg::ServiceLocator::Get().getEventController().subscribeToEvent(rg::EventType::Keyboard, &programState->camera);

    while (!glfwWindowShouldClose(window)) {
        // per-frame time logic
        // --------------------
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);

        rg::ServiceLocator::Get().getProcessController().update(deltaTime);


        programState->camera.update(deltaTime);

        // render
        // ------
        glClearColor(programState->clearColor.r, programState->clearColor.g, programState->clearColor.b, 1.0f);
        glBindFramebuffer(GL_FRAMEBUFFER, hdrFBO);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        modelShader.use();
        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(programState->camera.Zoom),
                                                (float) SCR_WIDTH / (float) SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = programState->camera.GetViewMatrix();
        modelShader.setMat4("projection", projection);
        modelShader.setMat4("view", view);

        modelShader.setVec3("viewPosition", programState->camera.Position);
        modelShader.setFloat("material.shininess", 32.0f);
        // directional light
        modelShader.setVec3("dirLight.direction", programState->dirLightDir);
        modelShader.setVec3("dirLight.ambient", glm::vec3(programState->dirLightAmbDiffSpec.x));
        modelShader.setVec3("dirLight.diffuse", glm::vec3(programState->dirLightAmbDiffSpec.y));
        modelShader.setVec3("dirLight.specular", glm::vec3(programState->dirLightAmbDiffSpec.z));

        modelShader.setVec3("pointLights[0].position", glm::vec3(-5.0f, -10.0f,-5.0f));
        modelShader.setVec3("pointLights[0].ambient", pointLight.ambient);
        modelShader.setVec3("pointLights[0].diffuse", pointLight.diffuse);
        modelShader.setVec3("pointLights[0].specular", pointLight.specular);
        modelShader.setFloat("pointLights[0].constant", pointLight.constant);
        modelShader.setFloat("pointLights[0].linear", pointLight.linear);
        modelShader.setFloat("pointLights[0].quadratic", pointLight.quadratic);

        modelShader.setVec3("pointLights[1].position", glm::vec3(-10.0f ,110.0f, 1.0f));
        modelShader.setVec3("pointLights[1].ambient", pointLight.ambient);
        modelShader.setVec3("pointLights[1].diffuse", pointLight.diffuse);
        modelShader.setVec3("pointLights[1].specular", pointLight.specular);
        modelShader.setFloat("pointLights[1].constant", pointLight.constant);
        modelShader.setFloat("pointLights[1].linear", pointLight.linear);
        modelShader.setFloat("pointLights[1].quadratic", pointLight.quadratic);

        modelShader.setBool("blinn", programState->blinn);
        // spotLight
        if (programState->spotlightOn) {
            modelShader.setVec3("spotLight.position", programState->camera.Position);
            modelShader.setVec3("spotLight.direction", programState->camera.Front);
            modelShader.setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
            modelShader.setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
            modelShader.setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
            modelShader.setFloat("spotLight.constant", 1.0f);
            modelShader.setFloat("spotLight.linear", 0.09);
            modelShader.setFloat("spotLight.quadratic", 0.032);
            modelShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
            modelShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));
        }else{
            modelShader.setVec3("spotLight.diffuse", 0.0f, 0.0f, 0.0f);
            modelShader.setVec3("spotLight.specular", 0.0f, 0.0f, 0.0f);
        }


        // render the loaded snowman model
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model,
                               programState->snowManPosition); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(programState->snowManScale));    // it's a bit too big for snowMan scene, so scale it down

        modelShader.setMat4("model", model);
        snowManModel.Draw(modelShader);

        modelShader.setVec3("pointLights[0].position", glm::vec3(4.0 * cos(0.9), 4.0f, 4.0 * sin(0.9)));
        // render the loaded tree model
        glm::mat4 treeModelMatrix = glm::mat4(1.0f);
        treeModelMatrix = glm::translate(treeModelMatrix,
                               programState->treePosition);
        treeModelMatrix = glm::scale(treeModelMatrix, glm::vec3(programState->treeScale));
        treeModelMatrix = glm::rotate(treeModelMatrix, -45.0f, glm::vec3(1.0f, 0.0f, 0.0f));
        treeModelMatrix = glm::rotate(treeModelMatrix, -45.0f, glm::vec3(0.0f, 0.0f, 1.0f));

        modelShader.setMat4("model", treeModelMatrix);
        treeModel.Draw(modelShader);

        giftShader.use();

        giftShader.setMat4("projection", projection);
        giftShader.setMat4("view", view);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, giftTexture);
        glBindVertexArray(giftVAO);

        for (unsigned int i = 0; i < programState->numOfGifts; i++) {

            glm::mat4 giftModel = glm::mat4(1.0f);
            giftModel = glm::translate(giftModel,
                                   giftPositions[i]);
            giftModel = glm::scale(giftModel, glm::vec3(programState->giftScale));
            giftShader.setMat4("model", giftModel);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }

        // using blanding for snowflakes- discard
        snowShader.use();
        snowShader.setMat4("projection", projection);
        snowShader.setMat4("view", view);

        glBindVertexArray(transparentVAO);
        glBindTexture(GL_TEXTURE_2D, snowflakeTexture);
        for (unsigned int i = 0; i < snowflakePosition.size(); i++)
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, snowflakePosition[i]);
            model = glm::translate(model, glm::vec3(0.0f,-0.5*tan(glfwGetTime()*0.6), 0.0f));
            float angle= 20*tan(glfwGetTime());
            model=glm::rotate(model,glm::radians(angle),glm::vec3(1.0f,0.5f,0.5f));
            model=glm::scale(model,glm::vec3(0.5f,0.5f,0.5f));
            snowShader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }

        shader.use();
        shader.setMat4("view", view);
        shader.setMat4("projection", projection);

        // cubes
        glBindVertexArray(cubeVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, iceTexture);


        for (unsigned i = 0; i < 6; i++) {
            glCullFace(GL_BACK);
            model = glm::mat4(1.0f);
            model = glm::translate(model, cubePosition[i]);
            shader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
        //skybox
        glDepthFunc(GL_LEQUAL); //change depth function so depth test passes when values are equal to depth buffer's content
        skyboxShader.use();
        view = glm::mat4(glm::mat3(programState->camera.GetViewMatrix())); //remove translation from the view matrix
        skyboxShader.setMat4("view", view);
        skyboxShader.setMat4("projection", projection);


        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        glBindVertexArray(0);
        glDepthFunc(GL_LESS); //set depth function back to default

        if (programState->ImGuiEnabled)
            DrawImGui(programState);

        // blur bright fragments with two-pass Gaussian Blur
        // --------------------------------------------------
        bool horizontal = true, first_iteration = true;
        unsigned int amount = 5;
        shaderBlur.use();
        for (unsigned int i = 0; i < amount; i++)
        {
            glBindFramebuffer(GL_FRAMEBUFFER, pingpongFBO[horizontal]);
            shaderBlur.setInt("horizontal", horizontal);
            glBindTexture(GL_TEXTURE_2D, first_iteration ? colorBuffers[1] : pingpongColorbuffers[!horizontal]);  // bind texture of other framebuffer (or scene if first iteration)
            renderQuad();
            horizontal = !horizontal;
            if (first_iteration)
                first_iteration = false;
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // now render floating point color buffer to 2D quad and tonemap HDR colors to default framebuffer's (clamped) color range
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        shaderBloom.use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, colorBuffers[0]);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, pingpongColorbuffers[!horizontal]);
        shaderBloom.setInt("bloom", programState->bloom);
        shaderBloom.setFloat("exposure", programState->exposure);
        renderQuad();

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
        rg::ServiceLocator::Get().getInputController().update(deltaTime);

    }

    programState->SaveToFile("resources/program_state.txt");
    delete programState;
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glDeleteVertexArrays(1, &giftVAO);
    glDeleteBuffers(1, &giftVBO);
    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteBuffers(1, &skyboxVBO);
    glDeleteVertexArrays(1, &transparentVAO);
    glDeleteBuffers(1, &transparentVBO);
    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window) {
#if 1
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        programState->camera.ProcessKeyboard(RIGHT, deltaTime);

    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS and !programState->blinnKeyPressed)
    {
        programState->blinn = !programState->blinn;
        programState->blinnKeyPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_RELEASE)
    {
        programState->blinnKeyPressed = false;
    }

    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS and !programState->bloomKeyPressed)
    {
        programState->bloom = !programState->bloom;
        programState->bloomKeyPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_RELEASE)
    {
        programState->bloomKeyPressed = false;
    }
#else

#endif
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
    //glViewport(0,0, 800, 600);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow *window, double xpos, double ypos) {
#if 1
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

    lastX = xpos;
    lastY = ypos;

    if (programState->CameraMouseMovementUpdateEnabled)
        programState->camera.ProcessMouseMovement(xoffset, yoffset);
#else
    rg::ServiceLocator::Get().getInputController().processMouseMovementCallback(xpos, ypos);
#endif
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow *window, double xoffset, double yoffset) {
#if 1
    programState->camera.ProcessMouseScroll(yoffset);
#else
    rg::ServiceLocator::Get().getInputController().processMouseScrollCallback(yoffset);
#endif
}

void DrawImGui(ProgramState *programState) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();


    {
        static float f = 0.0f;
        ImGui::Begin("Hello window");
        ImGui::Text("Hello text");
        ImGui::SliderFloat("Float slider", &f, 0.0, 1.0);
        ImGui::ColorEdit3("Background color", (float *) &programState->clearColor);
        ImGui::DragFloat3("snowMan position", (float*)&programState->snowManPosition);
        ImGui::DragFloat("snowMan scale", &programState->snowManScale, 0.05, 0.1, 4.0);

        ImGui::DragFloat("pointLight.constant", &programState->pointLight.constant, 0.05, 0.0, 1.0);
        ImGui::DragFloat("pointLight.linear", &programState->pointLight.linear, 0.05, 0.0, 1.0);
        ImGui::DragFloat("pointLight.quadratic", &programState->pointLight.quadratic, 0.05, 0.0, 1.0);

        ImGui::End();
    }

    {
        ImGui::Begin("Camera info");
        const Camera& c = programState->camera;
        ImGui::Text("Camera position: (%f, %f, %f)", c.Position.x, c.Position.y, c.Position.z);
        ImGui::Text("(Yaw, Pitch): (%f, %f)", c.Yaw, c.Pitch);
        ImGui::Text("Camera front: (%f, %f, %f)", c.Front.x, c.Front.y, c.Front.z);
        ImGui::Checkbox("Camera mouse update", &programState->CameraMouseMovementUpdateEnabled);
        ImGui::End();
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
#if 1
    if (key == GLFW_KEY_F1 && action == GLFW_PRESS) {
        programState->ImGuiEnabled = !programState->ImGuiEnabled;
        if (programState->ImGuiEnabled) {
            programState->CameraMouseMovementUpdateEnabled = false;
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
        } else {
            glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
        }
    }
#else
    rg::ServiceLocator::Get().getInputController().processKeyCallback(window, key, action);
#endif
}

unsigned int loadTexture(char const * path)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width,height, nrComponents;
    unsigned char *data = stbi_load(path, &width, &height, &nrComponents,0);

    if(data)
    {
        GLenum format;
        if(nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << path << std::endl;
        stbi_image_free(data);

    }
    return textureID;
}

unsigned int loadCubemap(vector <std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for(unsigned int i = 0; i < faces.size(); i++)
    {
        unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels,0);
        if(data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else
        {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

// renderQuad() renders a 1x1 XY quad in NDC
// -----------------------------------------
unsigned int quadVAO = 0;
unsigned int quadVBO;
void renderQuad()
{
    if (quadVAO == 0)
    {
        float quadVertices[] = {
                // positions        // texture Coords
                -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
                -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
                1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
                1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };
        // setup  VAO
        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    }
    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}