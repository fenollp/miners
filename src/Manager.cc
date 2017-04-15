#include <iostream>

#include <Manager.hh>
#include <GlfwManager.hh>

void
Manager::loadScene(std::shared_ptr<Scene> scene) {
    // XXX delete scene prior to creating a new one
    if (scene_) {
        std::cout << "deleting scene" << std::endl;
    }
    scene_ = scene;
    scene->init(args_);
}

void
Manager::loadFile(const std::string& filename) {
    args_->input.push_back(filename);
    loadFile(args_->input.size() - 1);
}

void
Manager::loadFile(size_t index) {
    if (fileIndex_ == index && scene_)
        return;

    fileIndex_ = index;
    // TODO: remove previous loader instance from scene.
    //       properly define who owns the variable and its longevity
    auto* loader = loaderFromUri(args_->input[index]);
    loader->load();
    auto* range = DataRange::create(args_->range_begin, args_->range_end);

    Algorithm* algo = NULL;
    if (scene_) {
        algo = scene_->algorithm();
        algo->use(loader, range);
        scene_->reload();
    } else {
        algo = createAlgorithm(args_->algo);
        algo->use(loader, range);
        auto self = static_cast<std::shared_ptr<Manager>>(this);
        loadScene(Scene::forAlgo(self, algo));
    }
}

std::shared_ptr<Manager>
createManager(const std::string& str, std::shared_ptr<Arguments> args) {
    auto it = managers.find(str);
    if (it == managers.end()) {
        return NULL;
    }
    return it->second(args);
}

const std::map<const std::string, ManagerFactoryFunc> managers = {
    { "glfw", [](std::shared_ptr<Arguments> args) { return std::make_shared<GlfwManager>(args); } },
};
