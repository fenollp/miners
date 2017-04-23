#pragma once

#include <memory>

#include <Arguments.hh>
#include <Scene.hh>
#include <Events.hh>
#include <Indices.hh>

class Scene;

class Manager {
public:
    Manager(std::shared_ptr<Arguments> args)
        : fullscreen_(false), args_(args), fileIndex_(0), scene_(NULL),
          sliding_window_offset_(0), sliding_window_length_(args_->sliding_window_length)
        {}
    virtual ~Manager() {}

    virtual void loadScene(std::shared_ptr<Scene> scene);
    virtual void init() = 0;
    virtual void run() = 0;

    virtual std::shared_ptr<Events> getEvents() = 0;
    virtual std::shared_ptr<Mouse> getMouse() = 0;

    void loadFile(size_t index);
    void loadFile(const std::string& filename);
    void loadNextFile() { loadFile((fileIndex_ + 1) % args_->input.size()); }
    void loadPrevFile() { loadFile((fileIndex_ - 1) % args_->input.size()); }

    virtual void toggleFullscreen() = 0;

    std::shared_ptr<Arguments> args() const { return args_; }
    std::shared_ptr<Scene> scene() const { return scene_; }

    void slide_window_left() {
        if (sliding_window_offset_ > args_->sliding_step)
            sliding_window_offset_ -= args_->sliding_step;
        else
            sliding_window_offset_ = 0;
    }
    void slide_window_right() { sliding_window_offset_ += args_->sliding_step; }
    void slide_window_up() { sliding_window_length_ += args_->sliding_step; }
    void slide_window_down() {
        if (sliding_window_length_ > args_->sliding_step)
            sliding_window_length_ -= args_->sliding_step;
        else
            sliding_window_length_ = args_->sliding_window_length;;
    }
    virtual void slide_window() = 0;

    void slide_window(VertIndices& selected, const VertIndices& indices) {
        auto previous_size = selected.size();
        auto left = indices.begin() + sliding_window_offset_;
        if (left > indices.end())
            left = indices.begin();
        auto right = std::min(indices.end(), left + sliding_window_length_);
        selected.assign(left, right);
        if (selected.size() != previous_size)
            std::cout << "#selected: " << size2str(selected.size()) << std::endl;
    }

    static std::string  /// 991337 --> "991,337"
    size2str (size_t size) {
        std::string str = std::to_string(size);
        int pos = str.length() - 3;
        while (pos > 0) {
            str.insert(pos, ",");
            pos -= 3;
        }
        return str;
    }

protected:
    bool fullscreen_;
    std::shared_ptr<Arguments> args_;
    size_t fileIndex_;
    std::shared_ptr<Scene> scene_;
    size_t sliding_window_offset_;
    size_t sliding_window_length_;
};

std::shared_ptr<Manager> createManager(const std::string& str, std::shared_ptr<Arguments> args);

using ManagerFactoryFunc = std::function<std::shared_ptr<Manager>(std::shared_ptr<Arguments>)>;
extern const std::map<const std::string, ManagerFactoryFunc> managers;
