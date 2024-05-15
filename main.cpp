#include <iostream>

#include "application.hpp"

int main(int argc [[maybe_unused]], char** argv [[maybe_unused]]){
    Application app{};
    app.run();
    exit(EXIT_SUCCESS);
}