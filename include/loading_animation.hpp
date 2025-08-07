#pragma once
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
    void updateMessage(const std::string& message);

private:
    std::string message_;
    bool running_;
};
