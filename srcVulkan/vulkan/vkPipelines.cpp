#include "application.hpp"
#include "gradientPipeline.hpp"

namespace vkr {

void Application::initPipelines(){
    _Slang._Pipelines.emplace_back(PipelinePtr(new GradientPipeline()));
    for(auto pipeline : _Slang._Pipelines){
        pipeline->initDescriptors(_VulkanParameters, _DrawImage, _GlobalDescriptorAllocator);
        pipeline->init(_Slang, _VulkanParameters);
    }
}

void Application::destroyPipelines(){
    for(auto pipeline : _Slang._Pipelines){
        pipeline->destroy(_VulkanParameters);
    }
}

}