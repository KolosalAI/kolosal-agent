/**
 * @file loading_animation_utils.cpp
 * @brief Utility functions and helpers for loading animation
 * @version 2.0.0
 * @author Kolosal AI Team
 * @date 2025
 * 
 * Implementation file for the Kolosal Agent System v2.0.
 * Part of the unified multi-agent AI platform.
 */

#include "loading_animation_utils.hpp"
#include <iostream>

LoadingAnimation::LoadingAnimation(const std::string& message) 
    /**
     * @brief Perform message  operation
     * @return : Description of return value
     */
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

void LoadingAnimation::update_Message(const std::string& message) {
    message_ = message;
}

void LoadingAnimation::complete(const std::string& message) {
    if (running_) {
        std::cout << message << std::endl;
        running_ = false;
    }
}
