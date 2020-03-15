#pragma once

#include <bitset>
#include <memory>

#include "include/GLFW/glfw3.h"  // TODO: fix include path some day
#include "src/include/Manager.h"
#include "src/include/Types.h"

struct GlfwModifiers {
  int shift : 1;
  int ctrl : 1;
  int alt : 1;
  int super : 1;
} __attribute__((packed));

struct GlfwKeyboardState {
  std::bitset<512> keys;
  union {
    int rawmods;
    GlfwModifiers mods;
  };

  void copy(const std::shared_ptr<GlfwKeyboardState>& state) {
    keys = state->keys;
    rawmods = state->rawmods;
  }
};

class GlfwKeyboardEvents : public Events {
 public:
  GlfwKeyboardEvents()
      : current_(std::make_shared<GlfwKeyboardState>()),
        previous_(std::make_shared<GlfwKeyboardState>()) {}
  virtual ~GlfwKeyboardEvents();

  virtual bool keyPressed(int key);
  virtual bool keyReleased(int key);
  virtual bool keyDown(int key);
  virtual bool keyUp(int key);

  virtual void update();

  void process(int key, int scancode, int action, int mods);

  // protected:
  std::shared_ptr<GlfwKeyboardState> current_;
  std::shared_ptr<GlfwKeyboardState> previous_;
};

class GlfwMouse : public Mouse {
 public:
  virtual void getCursorPos();
  virtual void setCursorPos();
};

class GlfwManager : public Manager {
 public:
  GlfwManager(std::shared_ptr<Arguments>& args)
      : Manager(args), window_(nullptr) {
    resetFloats();
  }
  virtual ~GlfwManager() {}

  virtual void init() override;
  virtual void run() override;
  virtual void computeMatricesFromInputs(
      glm::mat4*, glm::mat4*) override;  // TODO: remove from manager api

  virtual std::shared_ptr<Events> getEvents() final { return events_; }
  virtual std::shared_ptr<Mouse> getMouse() final { return mouse_; }

  virtual void ToggleFullscreen() override;
  virtual bool SlideWindow() override;

  GLFWwindow* window() { return window_; }

  static void glProcessErrors(bool quiet = false);

  static std::shared_ptr<GlfwManager> instance(
      std::shared_ptr<Arguments> args) {
    if (instance_ || !args)
      throw std::runtime_error("Bad call to instance of GlfwManager");
    instance_ = std::make_shared<GlfwManager>(args);
    return instance_;
  }

  static std::shared_ptr<GlfwManager> instance() {
    if (!instance_)
      throw std::runtime_error("GlfwManager wasn't previously instanciated");
    return instance_;
  }

  void viewport(int w, int h) {
    viewport_width_ = w;
    viewport_height_ = h;
  }

 private:
  void resetFloats() {
    position_ = glm::vec3(0, 0, 5);
    horizontal_angle_ = 3.14f;
    vertical_angle_ = 0.0f;  // look at the horizon
    initial_fov_ = 45.0f;
  }

 protected:
  static std::shared_ptr<GlfwManager> instance_;
  GLFWwindow* window_;
  std::shared_ptr<GlfwKeyboardEvents> events_;
  std::shared_ptr<GlfwMouse> mouse_;
  int viewport_width_ = 800, viewport_height_ = 600;

 private:
  glm::vec3 position_;
  // horizontal angle : toward -Z
  float horizontal_angle_;
  // vertical angle : 0, look at the horizon
  float vertical_angle_;
  // Initial Field of View
  float initial_fov_;

  float speed_ = 3.0f;                // 3 units / second
  float mouse_sensitivity_ = 0.005f;  // 0.1f;
  // FoV is the level of zoom. 80° = very wide angle, huge deformations.
  // 60° - 45°: standard. 20°: big zoom.
};
