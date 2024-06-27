#include "application.hpp"

namespace vkr {

void Application::initSlang(){
    // First we need to create slang global session with work with the Slang API.
    SlangResult result = slang::createGlobalSession(_Slang._GlobalSession.writeRef());
    if(result != 0){
        fprintf(stderr, "Failed to create the slang global session!\n");
        exit(EXIT_FAILURE);
    }

    // Next we create a compilation session to generate SPIRV code from Slang source.
    slang::SessionDesc sessionDesc = {};
    slang::TargetDesc targetDesc = {};
    targetDesc.format = SLANG_SPIRV;
    targetDesc.profile = _Slang._GlobalSession->findProfile("spirv_1_5");
    targetDesc.flags = SLANG_TARGET_FLAG_GENERATE_SPIRV_DIRECTLY;

    sessionDesc.targets = &targetDesc;
    sessionDesc.targetCount = 1;

    // Create session
    result = _Slang._GlobalSession->createSession(sessionDesc, _Slang._Session.writeRef());
    if(result != 0){
        fprintf(stderr, "Failed to create the slang session!\n");
        exit(EXIT_FAILURE);
    }
}

void Application::destroySlang(){}

}