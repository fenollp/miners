#include <iomanip>
#include <fstream>
#include <stdexcept>
#include <algorithm>

#include "src/resources/shaders.h"
#include "src/include/Scene3D.h"
#include "src/include/GlfwManager.h"

void
Scene3D::load_shaders() {
    const std::vector<tdogl::Shader> shaders{
        tdogl::Shader(shader__vertex_3d, GL_VERTEX_SHADER),
        tdogl::Shader(shader__fragment, GL_FRAGMENT_SHADER)
    };
    program_ = std::make_shared<tdogl::Program>(shaders);
}

void
Scene3D::load_buffers() {
    // make and bind the VAO
    glGenVertexArrays(1, &vao_);
    glBindVertexArray(vao_);

    // make and bind the VBO
    glGenBuffers(1, &vbo_);
    std::cout << "load_buffers:: " << vbo_ << std::endl;
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, Algorithm::vsize(vertices_), vertices_.data(), GL_STATIC_DRAW);
    // connect the xyz to the "vert" attribute of the vertex shader
    glVertexAttribPointer(program_->attrib("vert"), 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(program_->attrib("vert"));

    glGenBuffers(1, &elements_);
    GlfwManager::glProcessErrors();
    std::cout << "load_buffers:: " << elements_ << std::endl;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elements_);
    GlfwManager::glProcessErrors();
    auto size_selected = sizeof (decltype(selected_)::value_type) * selected_.size();
    std::cout << "load_buffers:: size_selected = " << size_selected << std::endl;
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size_selected, selected_.data(), GL_STATIC_DRAW);
    // glBufferData(GL_ELEMENT_ARRAY_BUFFER, size_selected, NULL, GL_STATIC_DRAW);
    GlfwManager::glProcessErrors();

    GLint size = 0;
    glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
    GlfwManager::glProcessErrors();
    std::cout << "size = " << size << std::endl;

    // make and bind the VBO
    glGenBuffers(1, &colors_id_);
    std::cout << "load_buffers:: " << colors_id_ << std::endl;
    glBindBuffer(GL_ARRAY_BUFFER, colors_id_);
    glBufferData(GL_ARRAY_BUFFER, Algorithm::vsize(colors_), colors_.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(program_->attrib("colr"), 3, GL_FLOAT, GL_FALSE, 0, NULL);
    glEnableVertexAttribArray(program_->attrib("colr"));

    // unbind the VAO
    glBindVertexArray(0);
}

void
Scene3D::init(std::shared_ptr<Arguments> args) {
    resize(args->width, args->height);
}

void
Scene3D::unload() {
    if (program_) {
        glDeleteBuffers(1, &vbo_);
        glDeleteBuffers(1, &elements_);
        glDeleteBuffers(1, &colors_id_);
        glDeleteVertexArrays(1, &vao_);
    }
}

void
Scene3D::reload() {
    auto algo = std::static_pointer_cast<Algo3D>(algo_);
    reset_points();
    algo->apply(vertices_, colors_, indices_, width_, height_, depth_)
        || std::cerr << "!apply" << std::endl;
    std::cout << "#indices: " << Manager::size2str(indices_.size()) << std::endl;
    load_buffers();
    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, colors_id_);
    glBufferData(GL_ARRAY_BUFFER, Algorithm::vsize(colors_), colors_.data(), GL_STATIC_DRAW);
    glBindVertexArray(0);
}

void
Scene3D::load(std::shared_ptr<Algorithm> algorithm) {
    Scene::load(algorithm);
    auto algo = std::static_pointer_cast<Algo3D>(algorithm);

    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    load_shaders();
    algo->apply(vertices_, colors_, indices_, width_, height_, depth_)
        || std::cerr << "!apply" << std::endl;
    std::cout << "#indices: " << Manager::size2str(indices_.size()) << std::endl;
    load_buffers();

    // camera_.setPosition(glm::vec3(0, -0.1, 3));
    // camera_.lookAt(glm::vec3(0,0,4));
    // camera_.setNearAndFarPlanes(0.1, 100.);
}

bool
Scene3D::update(std::shared_ptr<Manager> manager, float elapsedTime) {
    if (manager->args()->spin_shape) {
        degrees_rotated_ += elapsedTime * degrees_per_second_;;
        while (degrees_rotated_ > 360.0f)
            degrees_rotated_ -= 360.0f;
    }

    auto events = manager->getEvents();

    // move position of camera based on WASD keys, and XZ keys for up and down
    if (events->keyDown('S'))
        camera_.offsetPosition(elapsedTime * move_speed_ * -camera_.forward());
    else if (events->keyDown('W'))
        camera_.offsetPosition(elapsedTime * move_speed_ * camera_.forward());
    if (events->keyDown('A'))
        camera_.offsetPosition(elapsedTime * move_speed_ * -camera_.right());
    else if(events->keyDown('D'))
        camera_.offsetPosition(elapsedTime * move_speed_ * camera_.right());
    if (events->keyDown('Z'))
        camera_.offsetPosition(elapsedTime * move_speed_ * -glm::vec3(0,1,0));
    else if (events->keyDown('X'))
        camera_.offsetPosition(elapsedTime * move_speed_ * glm::vec3(0,1,0));
    if (events->keyPressed('O')) {
        camera_.setPosition(glm::vec3(0, -0.1, 3.8));
        camera_.lookAt(glm::vec3(0,0,4));
    }
    if (events->keyPressed(' '))
        manager->args()->spin_shape = !manager->args()->spin_shape;
    if (events->keyPressed('M'))
        manager->args()->move_window = !manager->args()->move_window;
    if (events->keyPressed('.')) { // '>'
        manager->args()->sliding_step_factor *= 2;
    }
    if (events->keyPressed(',')) { // '<'
        manager->args()->sliding_step_factor /= 2;
        if (manager->args()->sliding_step_factor == 0) {
            manager->args()->sliding_step_factor = 1;
        }
    }

    if (manager->args()->move_window || selected_.empty() || manager->slide_window()) {
        if (manager->args()->move_window)
            manager->slide_window_right();
        bool slid = manager->slide_window(selected_, indices_);
        if (manager->args()->move_window && !slid)
            manager->args()->move_window = !manager->args()->move_window;
    }

    auto mouse = manager->getMouse();
    mouse->getCursorPos();
    camera_.offsetOrientation(mouse->sensitivity * mouse->y, mouse->sensitivity * mouse->x);
    // reset the mouse, so it doesn't go out of the window
    mouse->setCursorPos(0, 0);
    mouse->scrollY = 0.0;

    return true;
}

void
Scene3D::render() {
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // bind the program (the shaders)
    program_->use();

    program_->setUniform("camera", camera_.matrix());

    // set the "model" uniform in the vertex shader, based on degreesRotated
    auto rotated = glm::rotate(glm::mat4(), glm::radians(degrees_rotated_), glm::vec3(0,1,0));
    program_->setUniform("model", rotated);

    // bind the VAO
    glBindVertexArray(vao_);

    constexpr size_t SAMPLE = 999UL;

    // const VertIndices sels =

    VertIndices sels;
    sels.reserve(SAMPLE);
    for (int i = 0; i < SAMPLE; ++i) {
        sels.emplace_back(i);
    }
    selected_.assign(sels.begin(), sels.end());
    // const Floats cols =

    Floats cols;
    cols.reserve(4*SAMPLE);
    for (int i = 0; i < 4*SAMPLE; ++i) {
        cols.emplace_back(1.0f);
    }
    colors_.assign(cols.begin(), cols.end());

    // // send newly selected elements
    auto size_selected = sizeof (decltype(selected_)::value_type) * selected_.size();
    std::cout << "render:: size_selected = " << size_selected << std::endl;
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size_selected, selected_.data(), GL_STATIC_DRAW);
    GlfwManager::glProcessErrors();

    std::ostringstream SSsels;
    for (size_t i = 0; i < std::min(selected_.size(), SAMPLE); ++i) {
        // SSsels << ", " << selected_[i];
    }
    std::cout << SSsels.str() << " = SSsels" << std::endl;

    // // draw only the VAO's points we colored
    auto mM = std::minmax_element(selected_.begin(), selected_.end());
    std::cout << "render:: min:" << *mM.first << " max:" << *mM.second << " #:" << selected_.size() << std::endl;
    glDrawRangeElements(GL_POINTS, *mM.first, *mM.second, selected_.size(), GL_UNSIGNED_INT, NULL);
    GlfwManager::glProcessErrors();

    // glDrawArrays(GL_POINTS, 0, n_points_);
    // glDrawElements(GL_POINTS, selected_.size(), GL_UNSIGNED_INT, selected_.data());

    GLint size = 0;
    glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &size);
    GlfwManager::glProcessErrors();
    std::cout << "size = " << size << " == " << 4*selected_.size() << std::endl;

    std::ostringstream SScols;
    SScols << std::setw(12) << "cols:" << colors_.size() <<" ";
    for (size_t i = 0; i < std::min(colors_.size(), 4*SAMPLE); ++i) {
        // SScols << ", " << colors_[i];
    }
    std::cout << SScols.str() << " = SScols" << std::endl;

    // unbind the VAO and the program
    glBindVertexArray(0);
    program_->stopUsing();
}
