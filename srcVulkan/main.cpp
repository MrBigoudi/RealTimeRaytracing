#include <iostream>

#include "application.hpp"

using namespace vkr;

int main(int argc [[maybe_unused]], char** argv [[maybe_unused]]){
    ApplicationPtr app = ApplicationPtr(new Application());
    app->run();
    exit(EXIT_SUCCESS);
}
