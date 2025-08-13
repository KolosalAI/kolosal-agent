#include <thread>
#include <iostream>
int main() {
    std::thread t([](){
        std::cout << \
Thread
works\ << std::endl;
    });
    t.join();
    return 0;
}
