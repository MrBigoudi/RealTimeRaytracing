#include "application.hpp"

#include "errorHandler.hpp"

namespace vkr {

void Application::initSlang(){
    // First we need to create slang global session with work with the Slang API.
    SlangResult result = slang::createGlobalSession(_Slang._GlobalSession.writeRef());
    cr::ErrorHandler::vulkanError(
        result == 0,
        __FILE__, __LINE__,
        "Failed to create the slang global session!\n"
    ); 

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
    cr::ErrorHandler::vulkanError(
        result == 0,
        __FILE__, __LINE__,
        "Failed to create the slang session!\n"
    ); 
}

void Application::destroySlang(){}

}