/**
 * @file loading_animation_utils.hpp
 * @brief Utility functions and helpers for loading animation
 * @version 2.0.0
 * @author Kolosal AI Team
 * @date 2025
 * 
 * Header file for the Kolosal Agent System v2.0.
 * Part of the unified multi-agent AI platform.
 */

#pragma once

#ifndef KOLOSAL_AGENT_INCLUDE_LOADING_ANIMATION_UTILS_HPP_INCLUDED
#define KOLOSAL_AGENT_INCLUDE_LOADING_ANIMATION_UTILS_HPP_INCLUDED
#include <string>

/**
 * @brief Simple loading animation stub for console output
 */
class LoadingAnimation {
public:
    explicit LoadingAnimation(const std::string& message);
    ~LoadingAnimation();
    
    void start();
    void stop();
    void complete(const std::string& message);
    void update_Message(const std::string& message);

private:
    std::string message_;
    bool running_;
};

#endif // KOLOSAL_AGENT_INCLUDE_LOADING_ANIMATION_UTILS_HPP_INCLUDED
