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
    bool blinn = false;
    bool blinnKeyPressed = false;
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

    // build and compile shaders
    // -------------------------
    Shader snowManShader("resources/shaders/model.vs", "resources/shaders/model.fs");
    Shader giftShader("resources/shaders/box.vs", "resources/shaders/box.fs");
    Shader treeShader("resources/shaders/2.model_lighting.vs", "resources/shaders/2.model_lighting.fs");
    Shader skyboxShader("resources/shaders/skybox.vs", "resources/shaders/skybox.fs");
    Shader snowShader("resources/shaders/snowflakeShader.vs", "resources/shaders/snowflakeShader.fs");

    // load textures
    unsigned int giftTexture = loadTexture("resources/textures/wrapPaper.png");
    unsigned int snowflakeTexture = loadTexture("resources/textures/snowflake.png");

    // load models
    // -----------
    Model snowManModel("resources/objects/snowman/snowman_finish.obj", true);
    Model treeModel("resources/objects/Christmas_Tree/Christmas_Tree/12150_Christmas_Tree_V2_L2.obj");

    snowManModel.SetShaderTextureNamePrefix("material.");
    treeModel.SetShaderTextureNamePrefix("material.");

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

    snowManShader.use();
    snowManShader.setInt("diffuseTexture", 0);

    skyboxShader.use();
    skyboxShader.setInt("skybox", 0);
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
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // don't forget to enable shader before setting uniforms
        // enable shader before setting uniforms
        snowManShader.use();
        // view/projection transformations
        glm::mat4 projection = glm::perspective(glm::radians(programState->camera.Zoom),
                                                (float) SCR_WIDTH / (float) SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = programState->camera.GetViewMatrix();
        snowManShader.setMat4("projection", projection);
        snowManShader.setMat4("view", view);

        snowManShader.setVec3("viewPosition", programState->camera.Position);
        snowManShader.setFloat("material.shininess", 32.0f);
        // directional light
        snowManShader.setVec3("dirLight.direction", programState->dirLightDir);
        snowManShader.setVec3("dirLight.ambient", glm::vec3(programState->dirLightAmbDiffSpec.x));
        snowManShader.setVec3("dirLight.diffuse", glm::vec3(programState->dirLightAmbDiffSpec.y));
        snowManShader.setVec3("dirLight.specular", glm::vec3(programState->dirLightAmbDiffSpec.z));

        snowManShader.setVec3("pointLights[0].position", glm::vec3(-5.0f, -10.0f,-5.0f));
        snowManShader.setVec3("pointLights[0].ambient", pointLight.ambient);
        snowManShader.setVec3("pointLights[0].diffuse", pointLight.diffuse);
        snowManShader.setVec3("pointLights[0].specular", pointLight.specular);
        snowManShader.setFloat("pointLights[0].constant", pointLight.constant);
        snowManShader.setFloat("pointLights[0].linear", pointLight.linear);
        snowManShader.setFloat("pointLights[0].quadratic", pointLight.quadratic);

        snowManShader.setVec3("pointLights[1].position", glm::vec3(-10.0f ,110.0f, 1.0f));
        snowManShader.setVec3("pointLights[1].ambient", pointLight.ambient);
        snowManShader.setVec3("pointLights[1].diffuse", pointLight.diffuse);
        snowManShader.setVec3("pointLights[1].specular", pointLight.specular);
        snowManShader.setFloat("pointLights[1].constant", pointLight.constant);
        snowManShader.setFloat("pointLights[1].linear", pointLight.linear);
        snowManShader.setFloat("pointLights[1].quadratic", pointLight.quadratic);

        snowManShader.setBool("blinn", programState->blinn);
        // spotLight
        if (programState->spotlightOn) {
            snowManShader.setVec3("spotLight.position", programState->camera.Position);
            snowManShader.setVec3("spotLight.direction", programState->camera.Front);
            snowManShader.setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
            snowManShader.setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
            snowManShader.setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
            snowManShader.setFloat("spotLight.constant", 1.0f);
            snowManShader.setFloat("spotLight.linear", 0.09);
            snowManShader.setFloat("spotLight.quadratic", 0.032);
            snowManShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
            snowManShader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(15.0f)));
        }else{
            snowManShader.setVec3("spotLight.diffuse", 0.0f, 0.0f, 0.0f);
            snowManShader.setVec3("spotLight.specular", 0.0f, 0.0f, 0.0f);
        }


        // render the loaded model
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model,
                               programState->snowManPosition); // translate it down so it's at the center of the scene
        model = glm::scale(model, glm::vec3(programState->snowManScale));    // it's a bit too big for snowMan scene, so scale it down

        snowManShader.setMat4("model", model);
        snowManModel.Draw(snowManShader);

        treeShader.use();
        pointLight.position = glm::vec3(4.0 * cos(0.9), 4.0f, 4.0 * sin(0.9));
        treeShader.setVec3("pointLight.position", pointLight.position);
        treeShader.setVec3("pointLight.ambient", pointLight.ambient);
        treeShader.setVec3("pointLight.diffuse", pointLight.diffuse);
        treeShader.setVec3("pointLight.specular", pointLight.specular);
        treeShader.setFloat("pointLight.constant", pointLight.constant);
        treeShader.setFloat("pointLight.linear", pointLight.linear);
        treeShader.setFloat("pointLight.quadratic", pointLight.quadratic);
        treeShader.setVec3("viewPosition", programState->camera.Position);
        treeShader.setFloat("material.shininess", 32.0f);

        treeShader.setMat4("projection", projection);
        treeShader.setMat4("view", view);

        // render the loaded model
        glm::mat4 treeModelMatrix = glm::mat4(1.0f);
        treeModelMatrix = glm::translate(treeModelMatrix,
                               programState->treePosition);
        treeModelMatrix = glm::scale(treeModelMatrix, glm::vec3(programState->treeScale));
        treeModelMatrix = glm::rotate(treeModelMatrix, -45.0f, glm::vec3(1.0f, 0.0f, 0.0f));
        treeModelMatrix = glm::rotate(treeModelMatrix, -45.0f, glm::vec3(0.0f, 0.0f, 1.0f));

        treeShader.setMat4("model", treeModelMatrix);
        treeModel.Draw(treeShader);

        giftShader.use();

        giftShader.setMat4("projection", projection);
        giftShader.setMat4("view", view);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, giftTexture);
        glBindVertexArray(giftVAO);

        for (unsigned int i = 0; i < programState->numOfGifts; i++) {
        // render the loaded model
            glm::mat4 giftModel = glm::mat4(1.0f);
            giftModel = glm::translate(giftModel,
                                   giftPositions[i]); // translate it down so it's at the center of the scene
            //giftModel = glm::rotate(giftModel, ((float)tan(glfwGetTime() * 0.2)), glm::vec3(0.0f, 0.0f, 1.0f));
            giftModel = glm::scale(giftModel, glm::vec3(programState->giftScale));
            giftShader.setMat4("model", giftModel);
            glDrawArrays(GL_TRIANGLES, 0, 36);
        }
        glBindVertexArray(0);

        // using blanding for snowflakes- discard techinque
        snowShader.use();
        snowShader.setMat4("projection", projection);
        snowShader.setMat4("view", view);

        glBindVertexArray(transparentVAO);
        glBindTexture(GL_TEXTURE_2D, snowflakeTexture);
        for (unsigned int i = 0; i < snowflakePosition.size(); i++)
        {
            model = glm::mat4(1.0f);
            model = glm::translate(model, snowflakePosition[i]);
            //float coefTranslate = -0.5*tan(glfwGetTime()*0.6);
            model = glm::translate(model, glm::vec3(0.0f,-0.5*tan(glfwGetTime()*0.6), 0.0f));
            float angle= 20*tan(glfwGetTime());
            model=glm::rotate(model,glm::radians(angle),glm::vec3(1.0f,0.5f,0.5f));
            model=glm::scale(model,glm::vec3(0.5f,0.5f,0.5f));
            snowShader.setMat4("model", model);
            glDrawArrays(GL_TRIANGLES, 0, 6);
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

    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS and !programState->blinnKeyPressed)
    {
        programState->blinn = !programState->blinn;
        programState->blinnKeyPressed = true;
    }
    if (glfwGetKey(window, GLFW_KEY_B) == GLFW_RELEASE)
    {
        programState->blinnKeyPressed = false;
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