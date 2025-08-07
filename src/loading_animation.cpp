#include "loading_animation.hpp"
#include <iostream>

LoadingAnimation::LoadingAnimation(const std::string& message) 
    : message_(message), running_(false) {
}

LoadingAnimation::~LoadingAnimation() {
    if (running_) {
        stop();
    }
}

void LoadingAnimation::start() {
    if (!running_) {
        std::cout << message_ << "..." << std::endl;
        running_ = true;
    }
}

void LoadingAnimation::stop() {
    if (running_) {
        std::cout << "Done." << std::endl;
        running_ = false;
    }
}

void LoadingAnimation::updateMessage(const std::string& message) {
    message_ = message;
}

void LoadingAnimation::complete(const std::string& message) {
    if (running_) {
        std::cout << message << std::endl;
        running_ = false;
    }
}
