#include "gradientPipeline.hpp"
#include "application.hpp"

#include "dep/slang/source/core/slang-list.h"

namespace vkr {

void GradientPipeline::init(const SlangParameters& slangParamaters, const VulkanAppParameters& vulkanParameters){
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
        slangModule = slangParamaters._Session->loadModule((Pipeline::COMPILED_SHADER_DIRECTORY + "gradient").c_str(), diagnosticsBlob.writeRef());
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
        SlangResult result = slangParamaters._Session->createCompositeComponentType(
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
	computeLayout.pSetLayouts = _DescriptorLayouts.data();
	computeLayout.setLayoutCount = _DescriptorLayouts.size();

	VkResult result = vkCreatePipelineLayout(vulkanParameters._Device, &computeLayout, nullptr, &_PipelineLayout);
    if(result != VK_SUCCESS){
        fprintf(stderr, "Failed to init the backgroud pipelines layouts!\n");
        exit(EXIT_FAILURE);
    }

    //layout code
	VkShaderModule computeDrawShader;
	if (!loadShaderModule(spirvCode, vulkanParameters._Device, &computeDrawShader)){
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
	computePipelineCreateInfo.layout = _PipelineLayout;
	computePipelineCreateInfo.stage = stageinfo;
	
	result = vkCreateComputePipelines(vulkanParameters._Device,VK_NULL_HANDLE,1,&computePipelineCreateInfo, nullptr, &_Pipeline);
    if(result != VK_SUCCESS){
        fprintf(stderr, "Failed to create the compute shader pipelines!\n");
        exit(EXIT_FAILURE);
    }

    vkDestroyShaderModule(vulkanParameters._Device, computeDrawShader, nullptr);
}

void GradientPipeline::destroy(const VulkanAppParameters& vulkanParameters){
    vkDestroyPipelineLayout(vulkanParameters._Device, _PipelineLayout, nullptr);
    vkDestroyPipeline(vulkanParameters._Device, _Pipeline, nullptr);
}

void GradientPipeline::run(const VulkanAppParameters& vulkanParameters, const VkCommandBuffer& cmd){
    // bind the gradient drawing compute pipeline
    vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_COMPUTE, getPipeline());

    // bind the descriptor set containing the draw image for the compute pipeline
    auto descriptorLayouts = getDescriptorLayouts();
    auto descriptorSets = getDescriptors();
    vkCmdBindDescriptorSets(
        cmd, 
        VK_PIPELINE_BIND_POINT_COMPUTE, 
        getPipelineLayout(), 
        0, 
        descriptorSets.size(), 
        descriptorSets.data(), 
        0, 
        nullptr
    );

    // execute the compute pipeline dispatch. We are using 16x16 workgroup size so we need to divide by it
    vkCmdDispatch(
        cmd, 
        std::ceil(vulkanParameters._DrawExtent.width / 16.0), 
        std::ceil(vulkanParameters._DrawExtent.height / 16.0), 
        1
    );
}

void GradientPipeline::initDescriptors(const VulkanAppParameters& vulkanParameters, const AllocatedImage& image, DescriptorAllocator& descriptorAllocator){
	//create a descriptor pool that will hold 10 sets with 1 image each
	std::vector<DescriptorAllocator::PoolSizeRatio> sizes ={
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1 }
	};

	descriptorAllocator.initPool(vulkanParameters._Device, 10, sizes);

	//make the descriptor set layout for our compute draw
	{
		DescriptorLayoutBuilder builder;
		builder.addBinding(0, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE);
		_DescriptorLayouts.push_back(builder.build(vulkanParameters._Device, VK_SHADER_STAGE_COMPUTE_BIT));
	}

    //allocate a descriptor set for our draw image
	_Descriptors.push_back(descriptorAllocator.allocate(vulkanParameters._Device, _DescriptorLayouts[0]));	

	VkDescriptorImageInfo imgInfo{};
	imgInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
	imgInfo.imageView = image._ImageView;
	
	VkWriteDescriptorSet drawImageWrite = {};
	drawImageWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	drawImageWrite.pNext = nullptr;
	
	drawImageWrite.dstBinding = 0;
	drawImageWrite.dstSet = *_Descriptors.data();
	drawImageWrite.descriptorCount = _Descriptors.size();
	drawImageWrite.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	drawImageWrite.pImageInfo = &imgInfo;

	vkUpdateDescriptorSets(vulkanParameters._Device, 1, &drawImageWrite, 0, nullptr);
}

void GradientPipeline::destroyDescriptors(const VulkanAppParameters& vulkanParameters, DescriptorAllocator& descriptorAllocator){
    descriptorAllocator.destroyPool(vulkanParameters._Device);
    for(auto descriptorLayout : _DescriptorLayouts){
        vkDestroyDescriptorSetLayout(vulkanParameters._Device, descriptorLayout, nullptr);
    }
}

}