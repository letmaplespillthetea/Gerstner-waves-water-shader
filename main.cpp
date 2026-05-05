// ============================================================
//  main.cpp - Water Surface Simulation with ImGui Controls
// ============================================================

#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <vector>

// ImGui includes
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

// Project headers
#include "src/Camera.h"
#include "src/PostProcess.h"
#include "src/Shader.h"
#include "src/SunMesh.h"
#include "src/WaterMesh.h"
#include "src/WaveSystem.h"

// ---- Window configuration ----
static const int WINDOW_WIDTH = 1280;
static const int WINDOW_HEIGHT = 720;
static const char *WINDOW_TITLE =
    "Water Surface Simulation - Gerstner Waves & ImGui";

// ---- Global state ----
static Camera g_camera;
static float g_lastX = WINDOW_WIDTH / 2.0f;
static float g_lastY = WINDOW_HEIGHT / 2.0f;
static bool g_firstMouse = true;
static bool g_rightMouseDown = false;
static bool g_wireframe = false;
static WaveSystem *g_wavesPtr = nullptr;

// ---- Water & Lighting Appearance ----
static glm::vec3 g_lightDir = glm::normalize(glm::vec3(0.0f, 0.15f, -1.0f));
static glm::vec3 g_lightColor = glm::vec3(1.0f, 0.95f, 0.85f);
static glm::vec3 g_shallowClr = glm::vec3(0.1f, 0.5f, 0.6f);
static glm::vec3 g_deepClr = glm::vec3(0.02f, 0.05f, 0.15f);
static float g_ambientStrength = 0.40f;
static float g_bloomIntensity = 1.0f;
static float g_bloomThreshold = 1.0f;
static float g_bloomSoftThreshold = 0.5f;

// ---- Forward declarations ----
GLFWwindow *initWindow();
void initImGui(GLFWwindow *window);
void renderUI(WaveSystem &waves);
void processInput(GLFWwindow *window, float deltaTime);
void framebufferSizeCallback(GLFWwindow *window, int width, int height);
void mouseCallback(GLFWwindow *window, double xPos, double yPos);
void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods);
void scrollCallback(GLFWwindow *window, double xOff, double yOff);
void keyCallback(GLFWwindow *window, int key, int scancode, int action,
                 int mods);

int main() {
  GLFWwindow *window = initWindow();
  if (!window)
    return -1;

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    std::cerr << "Failed to initialize GLAD\n";
    return -1;
  }

  initImGui(window);

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  Shader waterShader("shaders/water.vert", "shaders/water.frag");
  Shader sunShader("shaders/sun.vert", "shaders/sun.frag");
  Shader bloomDownsampleShader("shaders/post.vert",
                               "shaders/bloom_downsample.frag");
  Shader bloomUpsampleShader("shaders/post.vert",
                             "shaders/bloom_upsample.frag");
  Shader postFinalShader("shaders/post.vert", "shaders/post_final.frag");

  WaterMesh waterMesh(200.0f, 200);
  SunMesh sunMesh(30.0f, 64);
  PostProcess postProcessor(WINDOW_WIDTH, WINDOW_HEIGHT);

  WaveSystem waveSystem;
  g_wavesPtr = &waveSystem;

  float prevTime = 0.0f;

  while (!glfwWindowShouldClose(window)) {
    float currentTime = (float)glfwGetTime();
    float deltaTime = currentTime - prevTime;
    prevTime = currentTime;

    glfwPollEvents();
    processInput(window, deltaTime);

    // Start ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    renderUI(waveSystem);

    // ---- 1. Render Scene to HDR Framebuffer ----
    glBindFramebuffer(GL_FRAMEBUFFER, postProcessor.hdrFBO);

    // Check if camera is underwater
    bool isUnderwater = g_camera.position.y < 0.0f;
    if (isUnderwater) {
      glClearColor(0.0f, 0.15f, 0.3f, 1.0f); // Deep underwater color
    } else {
      glClearColor(0.5f, 0.7f, 0.9f, 1.0f); // Sky color
    }
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    int vpW, vpH;
    glfwGetFramebufferSize(window, &vpW, &vpH);
    float aspect = (vpH > 0) ? (float)vpW / (float)vpH : 1.0f;

    glm::mat4 projection =
        glm::perspective(glm::radians(g_camera.fov), aspect, 0.1f, 1000.0f);
    glm::mat4 view = g_camera.getViewMatrix();

    // ---- Draw Sun ----
    sunShader.use();
    glm::mat4 sunModel = glm::mat4(1.0f);
    sunModel =
        glm::translate(sunModel, g_camera.position + g_lightDir * 400.0f);

    sunModel[0][0] = view[0][0];
    sunModel[0][1] = view[1][0];
    sunModel[0][2] = view[2][0];
    sunModel[1][0] = view[0][1];
    sunModel[1][1] = view[1][1];
    sunModel[1][2] = view[2][1];
    sunModel[2][0] = view[0][2];
    sunModel[2][1] = view[1][2];
    sunModel[2][2] = view[2][2];

    sunShader.setMat4("u_model", sunModel);
    sunShader.setMat4("u_view", view);
    sunShader.setMat4("u_projection", projection);
    sunShader.setVec3("u_sunColor", g_lightColor * 3.5f);

    if (g_wireframe)
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    sunMesh.draw();

    // ---- Draw water ----
    waterShader.use();
    waterShader.setMat4("u_model", glm::mat4(1.0f));
    waterShader.setMat4("u_view", view);
    waterShader.setMat4("u_projection", projection);
    waterShader.setFloat("u_time", currentTime);
    waterShader.setVec3("u_cameraPos", g_camera.position);
    waterShader.setVec3("u_lightDir", g_lightDir);
    waterShader.setVec3("u_lightColor", g_lightColor);
    waterShader.setVec3("u_shallowColor", g_shallowClr);
    waterShader.setVec3("u_deepColor", g_deepClr);
    waterShader.setFloat("u_ambientStrength", g_ambientStrength);

    waveSystem.uploadToShader(waterShader);
    waterMesh.draw();
    if (g_wireframe)
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // ---- 2. Render Bloom (Progressive Downsample/Upsample) ----
    postProcessor.renderBloom(bloomDownsampleShader, bloomUpsampleShader,
                              g_bloomThreshold, g_bloomSoftThreshold);

    // ---- 3. Final Composite to Screen ----
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    postFinalShader.use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, postProcessor.colorBuffer);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, postProcessor.mipChain[0].texture);
    postFinalShader.setInt("u_scene", 0);
    postFinalShader.setInt("u_bloomBlur", 1);
    postFinalShader.setFloat("u_bloomIntensity", g_bloomIntensity);
    postFinalShader.setInt("u_isUnderwater", isUnderwater ? 1 : 0);
    postProcessor.renderQuad(); // Render ImGui
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);
  }

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwTerminate();
  return 0;
}

void initImGui(GLFWwindow *window) {
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;
  ImGui::StyleColorsDark();
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init("#version 130");
}

void renderUI(WaveSystem &waveSystem) {
  // Start the main parameters window
  ImGui::Begin("Water Simulation Controls");

  // Display help/navigation instructions
  if (ImGui::CollapsingHeader("Controls & Help",
                              ImGuiTreeNodeFlags_DefaultOpen)) {
    ImGui::TextColored(ImVec4(1, 1, 0, 1), "Camera Movement:");
    ImGui::BulletText("W, A, S, D : Move Forward/Left/Back/Right");
    ImGui::BulletText("Space      : Move Up");
    ImGui::BulletText("Left Ctrl  : Move Down");

    ImGui::Spacing();
    ImGui::TextColored(ImVec4(1, 1, 0, 1), "Camera Rotation:");
    ImGui::BulletText("Right Click + Drag : Rotate View");
    ImGui::BulletText("Mouse Scroll       : Zoom In/Out");

    ImGui::Spacing();
    ImGui::TextColored(ImVec4(1, 1, 0, 1), "Shortcuts:");
    ImGui::BulletText("F     : Toggle Wireframe");
    ImGui::BulletText("+ / - : Add / Remove Wave");
    ImGui::BulletText("Esc   : Exit Application");
  }

  // System settings (V-Sync, Wireframe)
  if (ImGui::CollapsingHeader("Global Settings",
                              ImGuiTreeNodeFlags_DefaultOpen)) {
    static bool vsync = true;
    if (ImGui::Checkbox("V-Sync", &vsync)) {
      glfwSwapInterval(vsync ? 1 : 0);
    }
    ImGui::Checkbox("Wireframe Mode (F)", &g_wireframe);
  }

  if (ImGui::CollapsingHeader("Appearance", ImGuiTreeNodeFlags_DefaultOpen)) {
    ImGui::ColorEdit3("Shallow Water", &g_shallowClr.x);
    ImGui::ColorEdit3("Deep Water", &g_deepClr.x);
  }

  if (ImGui::CollapsingHeader("Lighting", ImGuiTreeNodeFlags_DefaultOpen)) {
    ImGui::ColorEdit3("Sun Color", &g_lightColor.x);
    if (ImGui::DragFloat3("Sun Direction", &g_lightDir.x, 0.01f, -1.0f, 1.0f)) {
      if (glm::length(g_lightDir) > 0.001f)
        g_lightDir = glm::normalize(g_lightDir);
    }
    ImGui::SliderFloat("Ambient Light", &g_ambientStrength, 0.0f, 1.0f);
  }

  if (ImGui::CollapsingHeader("Post-Processing",
                              ImGuiTreeNodeFlags_DefaultOpen)) {
    ImGui::SliderFloat("Bloom Intensity", &g_bloomIntensity, 0.0f, 10.0f);
    ImGui::SliderFloat("Bloom Threshold", &g_bloomThreshold, 0.0f, 5.0f);
    ImGui::SliderFloat("Bloom Soft Knee", &g_bloomSoftThreshold, 0.0f, 1.0f);
  }

  if (ImGui::CollapsingHeader("Waves", ImGuiTreeNodeFlags_DefaultOpen)) {
    ImGui::Text("Active Waves: %d / %d", (int)waveSystem.waves.size(),
                WaveSystem::MAX_WAVES);

    if (ImGui::Button("Add Wave (+)") &&
        waveSystem.waves.size() < WaveSystem::MAX_WAVES) {
      waveSystem.addWave(
          GerstnerWave(glm::vec2(1.0f, 0.0f), 0.2f, 0.5f, 1.0f, 0.5f));
    }
    ImGui::SameLine();
    if (ImGui::Button("Remove Last Wave (-)") && waveSystem.waves.size() > 1) {
      waveSystem.removeWave((int)waveSystem.waves.size() - 1);
    }

    ImGui::Separator();

    for (int i = 0; i < (int)waveSystem.waves.size(); ++i) {
      std::string label = "Wave " + std::to_string(i);
      if (ImGui::TreeNode(label.c_str())) {
        GerstnerWave &w = waveSystem.waves[i];

        ImGui::DragFloat2("Direction", &w.direction.x, 0.01f, -1.0f, 1.0f);
        // Ensure direction is not zero
        if (glm::length(w.direction) < 0.001f)
          w.direction = glm::vec2(1.0f, 0.0f);

        ImGui::SliderFloat("Amplitude", &w.amplitude, 0.0f, 2.0f);
        ImGui::SliderFloat("Frequency", &w.frequency, 0.0f, 5.0f);
        ImGui::SliderFloat("Speed", &w.speed, 0.0f, 10.0f);
        ImGui::SliderFloat("Steepness", &w.steepness, 0.0f, 1.0f);

        if (ImGui::Button("Delete Wave") && waveSystem.waves.size() > 1) {
          waveSystem.removeWave(i);
          ImGui::TreePop();
          break;
        }

        ImGui::TreePop();
      }
    }
  }

  ImGui::End();
}

void processInput(GLFWwindow *window, float deltaTime) {
  if (ImGui::GetIO().WantCaptureKeyboard)
    return;

  if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    g_camera.processKeyboard(0, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    g_camera.processKeyboard(1, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
    g_camera.processKeyboard(2, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
    g_camera.processKeyboard(3, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
    g_camera.processKeyboard(4, deltaTime);
  if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
    g_camera.processKeyboard(5, deltaTime);
}

void mouseCallback(GLFWwindow *window, double xPos, double yPos) {
  if (ImGui::GetIO().WantCaptureMouse && !g_rightMouseDown)
    return;

  if (!g_rightMouseDown) {
    g_firstMouse = true;
    return;
  }

  if (g_firstMouse) {
    g_lastX = (float)xPos;
    g_lastY = (float)yPos;
    g_firstMouse = false;
  }

  float xOffset = (float)(xPos - g_lastX);
  float yOffset = -(float)(yPos - g_lastY);
  g_lastX = (float)xPos;
  g_lastY = (float)yPos;

  g_camera.processMouseMovement(xOffset, yOffset);
}

void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods) {
  if (ImGui::GetIO().WantCaptureMouse && action == GLFW_PRESS)
    return;

  if (button == GLFW_MOUSE_BUTTON_RIGHT) {
    g_rightMouseDown = (action == GLFW_PRESS);
    if (g_rightMouseDown) {
      glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    } else {
      glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
      g_firstMouse = true;
    }
  }
}

void scrollCallback(GLFWwindow *window, double xOff, double yOff) {
  if (ImGui::GetIO().WantCaptureMouse)
    return;
  g_camera.processMouseScroll((float)yOff);
}

void keyCallback(GLFWwindow *window, int key, int scancode, int action,
                 int mods) {
  if (ImGui::GetIO().WantCaptureKeyboard)
    return;
  if (action != GLFW_PRESS)
    return;

  if (key == GLFW_KEY_ESCAPE)
    glfwSetWindowShouldClose(window, true);
  if (key == GLFW_KEY_F)
    g_wireframe = !g_wireframe;

  if (key == GLFW_KEY_EQUAL && g_wavesPtr) {
    g_wavesPtr->addWave(
        GerstnerWave(glm::vec2(1.0f, 0.0f), 0.2f, 0.5f, 1.0f, 0.5f));
  }
  if (key == GLFW_KEY_MINUS && g_wavesPtr && g_wavesPtr->waves.size() > 1) {
    g_wavesPtr->removeWave((int)g_wavesPtr->waves.size() - 1);
  }
}

void framebufferSizeCallback(GLFWwindow *window, int width, int height) {
  glViewport(0, 0, width, height);
}

GLFWwindow *initWindow() {
  if (!glfwInit())
    return nullptr;
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_SAMPLES, 4);

  GLFWwindow *window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT,
                                        WINDOW_TITLE, nullptr, nullptr);
  if (!window) {
    glfwTerminate();
    return nullptr;
  }

  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);

  glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
  glfwSetCursorPosCallback(window, mouseCallback);
  glfwSetMouseButtonCallback(window, mouseButtonCallback);
  glfwSetScrollCallback(window, scrollCallback);
  glfwSetKeyCallback(window, keyCallback);

  return window;
}