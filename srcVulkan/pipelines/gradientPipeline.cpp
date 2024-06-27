#include "application.hpp"

#include "dep/slang/source/core/slang-list.h"


namespace vkr {

void Application::initGradientPipelines(){
    const std::string COMPILED_SHADER_DIRECTORY = std::string(PROJECT_SOURCE_DIR) + "/shaders/compiled/";
    // Once the slang session has been obtained, we can start loading code into it.
    //
    // The simplest way to load code is by calling `loadModule` with the name of a Slang
    // module. A call to `loadModule("hello-world")` will behave more or less as if you
    // wrote:
    //
    //      import hello_world;
    //
    // In a Slang shader file. The compiler will use its search paths to try to locate
    // `hello-world.slang`, then compile and load that file. If a matching module had
    // already been loaded previously, that would be used directly.
    slang::IModule* slangModule = nullptr;
    {
        Slang::ComPtr<slang::IBlob> diagnosticsBlob;
        slangModule = _Slang._Session->loadModule((COMPILED_SHADER_DIRECTORY + "gradient").c_str(), diagnosticsBlob.writeRef());
        if(!slangModule){
            fprintf(stderr, 
            "Failed to load the slang module: %s\n",
            (const char*)diagnosticsBlob->getBufferPointer());
            exit(EXIT_FAILURE);
        }
    }

    // Loading the `hello-world` module will compile and check all the shader code in it,
    // including the shader entry points we want to use. Now that the module is loaded
    // we can look up those entry points by name.
    //
    // Note: If you are using this `loadModule` approach to load your shader code it is
    // important to tag your entry point functions with the `[shader("...")]` attribute
    // (e.g., `[shader("compute")] void computeMain(...)`). Without that information there
    // is no umambiguous way for the compiler to know which functions represent entry
    // points when it parses your code via `loadModule()`.
    //
    Slang::ComPtr<slang::IEntryPoint> entryPoint;
    slangModule->findEntryPointByName("computeMain", entryPoint.writeRef());

    // At this point we have a few different Slang API objects that represent
    // pieces of our code: `module`, `vertexEntryPoint`, and `fragmentEntryPoint`.
    //
    // A single Slang module could contain many different entry points (e.g.,
    // four vertex entry points, three fragment entry points, and two compute
    // shaders), and before we try to generate output code for our target API
    // we need to identify which entry points we plan to use together.
    //
    // Modules and entry points are both examples of *component types* in the
    // Slang API. The API also provides a way to build a *composite* out of
    // other pieces, and that is what we are going to do with our module
    // and entry points.
    //
    Slang::List<slang::IComponentType*> componentTypes;
    componentTypes.add(slangModule);
    componentTypes.add(entryPoint);

    // Actually creating the composite component type is a single operation
    // on the Slang session, but the operation could potentially fail if
    // something about the composite was invalid (e.g., you are trying to
    // combine multiple copies of the same module), so we need to deal
    // with the possibility of diagnostic output.
    //
    Slang::ComPtr<slang::IComponentType> composedProgram;
    {
        Slang::ComPtr<slang::IBlob> diagnosticsBlob;
        SlangResult result = _Slang._Session->createCompositeComponentType(
            componentTypes.getBuffer(),
            componentTypes.getCount(),
            composedProgram.writeRef(),
            diagnosticsBlob.writeRef());
        if(result != 0){
            fprintf(stderr, 
            "Failed to compose the slang program: %s\n",
            (const char*)diagnosticsBlob->getBufferPointer());
            exit(EXIT_FAILURE);
        }
    }

    // Now we can call `composedProgram->getEntryPointCode()` to retrieve the
    // compiled SPIRV code that we will use to create a vulkan compute pipeline.
    // This will trigger the final Slang compilation and spirv code generation.
    Slang::ComPtr<slang::IBlob> spirvCode;
    {
        Slang::ComPtr<slang::IBlob> diagnosticsBlob;
        SlangResult result = composedProgram->getEntryPointCode(
            0, 0, spirvCode.writeRef(), diagnosticsBlob.writeRef());
        if(result != 0){
            fprintf(stderr, 
            "Failed to get the slang compiled program: %s\n",
            (const char*)diagnosticsBlob->getBufferPointer());
            exit(EXIT_FAILURE);
        }
    }


    VkPipelineLayoutCreateInfo computeLayout{};
	computeLayout.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	computeLayout.pNext = nullptr;
	computeLayout.pSetLayouts = &_DrawImageDescriptorLayout;
	computeLayout.setLayoutCount = 1;

	VkResult result = vkCreatePipelineLayout(_VulkanParameters._Device, &computeLayout, nullptr, &_GradientPipelineLayout);
    if(result != VK_SUCCESS){
        fprintf(stderr, "Failed to init the backgroud pipelines layouts!\n");
        exit(EXIT_FAILURE);
    }

    //layout code
	VkShaderModule computeDrawShader;
	if (!loadShaderModule(spirvCode, _VulkanParameters._Device, &computeDrawShader)){
		fprintf(stderr, "Error when building the compute shader!\n");
        exit(EXIT_FAILURE);
	}

	VkPipelineShaderStageCreateInfo stageinfo{};
	stageinfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	stageinfo.pNext = nullptr;
	stageinfo.stage = VK_SHADER_STAGE_COMPUTE_BIT;
	stageinfo.module = computeDrawShader;
	stageinfo.pName = "main";

	VkComputePipelineCreateInfo computePipelineCreateInfo{};
	computePipelineCreateInfo.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
	computePipelineCreateInfo.pNext = nullptr;
	computePipelineCreateInfo.layout = _GradientPipelineLayout;
	computePipelineCreateInfo.stage = stageinfo;
	
	result = vkCreateComputePipelines(_VulkanParameters._Device,VK_NULL_HANDLE,1,&computePipelineCreateInfo, nullptr, &_GradientPipeline);
    if(result != VK_SUCCESS){
        fprintf(stderr, "Failed to create the compute shader pipelines!\n");
        exit(EXIT_FAILURE);
    }

    vkDestroyShaderModule(_VulkanParameters._Device, computeDrawShader, nullptr);
}

void Application::destroyGradientPipelines(){
    vkDestroyPipelineLayout(_VulkanParameters._Device, _GradientPipelineLayout, nullptr);
    vkDestroyPipeline(_VulkanParameters._Device, _GradientPipeline, nullptr);
}

}