#include "gm_lib.hpp"
#include <limits>

#define URL "ws://127.0.0.1:8765"
#define FILE "output.log"

int main()
{
    std::cout << "Execution will start... Press Enter to exit" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    std::shared_ptr<act::Actuator> actuator = std::make_shared<act::Actuator>(URL, FILE);
    actuator->start();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    actuator->stop();

    std::cout << std::endl;
}
