#include <memory>
#include <vector>
#include <set>
#include <array>
#include <optional>
#include <iostream>
#include <unordered_map>
#include <vulkan/vulkan.hpp>
#include "render/lighting_asset.h"
#include "../../asset/shaders/include/engine_constant.inc.hlsl"
#include "vk_mem_alloc.h"
#include "vk_shader_variables.h"

#include "conf.h"
#include "vk_native.h"
#include "vk_surface.h"
#include "vk_texture.h"
#include "vk_render_texture.h"
#include "vk_ctx.h"
#include <math/math.h>

namespace engine {
namespace vulkan {

struct vk_ctx VK_CTX = {};

#define CTX (*(vk_ctx *)&VK_CTX)

const std::vector<const char *> validationLayers = {
	"VK_LAYER_KHRONOS_validation",
};

const std::vector<const char *> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	VK_KHR_SHADER_NON_SEMANTIC_INFO_EXTENSION_NAME,
};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

struct SwapChainSupportDetails {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> presentModes;
};

static VKAPI_ATTR VkBool32 VKAPI_CALL
debugCallback(VkDebugReportFlagsEXT flags,
	VkDebugReportObjectTypeEXT objType,
	uint64_t obj, size_t location, int32_t code,
	const char *layerPrefix, const char *msg, void *userData)
{
	std::cerr << "validation layer: " << msg << std::endl;
	return VK_FALSE;
}

static bool
check_validation_layer_support()
{
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());
	for (const char *layerName : validationLayers) {
		bool layerFound = false;
		for (const auto &layerProperties : availableLayers) {
			if (strcmp(layerName, layerProperties.layerName) == 0) {
				layerFound = true;
				break;
			}
		}
		if (!layerFound)
			return false;
	}
	return true;
}

static void
setupDebugCallbackArgs(VkDebugReportCallbackCreateInfoEXT &args)
{
	args = {};
	args.sType = VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT;
	args.flags = (VK_DEBUG_REPORT_ERROR_BIT_EXT | VK_DEBUG_REPORT_WARNING_BIT_EXT);
	args.pfnCallback = debugCallback;
}

static VkResult
CreateDebugReportCallbackEXT(
	VkInstance instance,
	const VkDebugReportCallbackCreateInfoEXT *pCreateInfo,
	const VkAllocationCallbacks *pAllocator,
	VkDebugReportCallbackEXT *pCallback)
{
	auto func = (PFN_vkCreateDebugReportCallbackEXT)vkGetInstanceProcAddr(
		instance, "vkCreateDebugReportCallbackEXT");
	if (func != nullptr) {
		return func(instance, pCreateInfo, pAllocator, pCallback);
	} else {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}
static void
DestroyDebugReportCallbackEXT(VkInstance instance,
	VkDebugReportCallbackEXT callback,
	const VkAllocationCallbacks *pAllocator)
{
	auto func = (PFN_vkDestroyDebugReportCallbackEXT)vkGetInstanceProcAddr(
		instance, "vkDestroyDebugReportCallbackEXT");
	if (func != nullptr) {
		func(instance, callback, pAllocator);
	}
}

void setup_debug_callback(VkInstance instance, VkDebugReportCallbackEXT *pCallback)
{
	if (!enableValidationLayers)
		return;
	VkDebugReportCallbackCreateInfoEXT args;
	setupDebugCallbackArgs(args);
	if (CreateDebugReportCallbackEXT(instance, &args, nullptr, pCallback) != VK_SUCCESS)
		std::cerr << "failed to setup debug callback" << std::endl;
}


static VkInstance
create_instance(const char *name, int major, int minor, surface *s)
{
	VkInstance instance;
	if (enableValidationLayers && !check_validation_layer_support()) {
		return VK_NULL_HANDLE;
	}
	VkApplicationInfo appinfo = {};
	appinfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appinfo.pNext = nullptr;
	appinfo.pApplicationName = name;
	appinfo.applicationVersion = VK_MAKE_VERSION(major, minor, 0);
	appinfo.pEngineName = "Dijiang";
	appinfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	appinfo.apiVersion = VK_API_VERSION_1_3;

	VkInstanceCreateInfo createinfo = {};
	createinfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createinfo.pApplicationInfo = &appinfo;
        //createinfo.flags = VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;

	auto extensions = surface_required_extensions(s);
	//extensions.emplace_back(VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME);
	createinfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
	createinfo.ppEnabledExtensionNames = extensions.data();
	createinfo.enabledLayerCount = 0;

	VkDebugReportCallbackCreateInfoEXT debugCreateInfo;
	if (enableValidationLayers) {
		createinfo.enabledLayerCount =
				static_cast<uint32_t>(validationLayers.size());
		createinfo.ppEnabledLayerNames = validationLayers.data();
		setupDebugCallbackArgs(debugCreateInfo);
		createinfo.pNext = &debugCreateInfo;
	} else {
		createinfo.enabledLayerCount = 0;
		createinfo.pNext = nullptr;
	}
	if (vkCreateInstance(&createinfo, nullptr, &instance) != VK_SUCCESS) {
		throw std::runtime_error("failed to crete vkinstance");
	}
	return instance;
}

static bool
checkDeviceExtensionSupport(VkPhysicalDevice device)
{
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount,nullptr);
	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());
	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());
	for (const auto &extension : availableExtensions) {
		requiredExtensions.erase(extension.extensionName);
	}
	return requiredExtensions.empty();
}

static void  
find_queue_families(VkPhysicalDevice device, VkSurfaceKHR surface, int *graphic, int *present) {
	uint32_t count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &count, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamilies(count);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &count, queueFamilies.data());
	int i = 0;
	for (auto const &qf : queueFamilies) {
		if (qf.queueCount > 0) {
			VkBool32 presentSupport = false;
			if (qf.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				*graphic = i;
			}
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
			if (presentSupport)
				*present = i;
		}
		if (*graphic >= 0 && *present >= 0)
			break;
		++i;
	}
	assert(*graphic >= 0 && *present >= 0);
	return ;
}



static bool
isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface)
{
	find_queue_families(device, surface, &VK_CTX.graphicsfamily, &VK_CTX.presentfamily);
	auto extSupport = checkDeviceExtensionSupport(device);
	bool swapChainAdequate = false;
	if (extSupport) {
		auto swapChainSupport = vk_swapchain::query_support(device, surface);
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	}
	VkPhysicalDeviceFeatures supportedFeatures;
	vkGetPhysicalDeviceFeatures(device, &supportedFeatures);
	return VK_CTX.graphicsfamily >= 0 && VK_CTX.presentfamily >= 0 && extSupport && swapChainAdequate && supportedFeatures.samplerAnisotropy;
}


static VkPhysicalDevice
pickPhysicalDevice(VkInstance instance, VkSurfaceKHR surface)
{
	uint32_t devCount = 0;
	VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;
	vkEnumeratePhysicalDevices(instance, &devCount, nullptr);
	if (devCount == 0) {
		throw std::runtime_error("failed find GPUs with vulkan support!");
	}
	std::vector<VkPhysicalDevice> devices(devCount);
	vkEnumeratePhysicalDevices(instance, &devCount, devices.data());
	for (const auto &dev : devices) {
		if (isDeviceSuitable(dev, surface)) {
			physicalDevice = dev;
			break;
		}
	}
	return physicalDevice;
}

static VkDevice
createLogicalDevice(VkPhysicalDevice physicalDevice,
	VkSurfaceKHR surface,
	VkQueue *graphicsQueue, VkQueue *presentQueue)
{
	VkDevice device = VK_NULL_HANDLE;
	std::vector<VkDeviceQueueCreateInfo> queueCreate;
	std::set<int> uniqueQueueFamilies = {CTX.graphicsfamily, CTX.presentfamily};
	float queuePriority = 1.0f;
	for (int qf : uniqueQueueFamilies) {
		auto &x = queueCreate.emplace_back();
		x.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		x.queueFamilyIndex = qf;
		x.queueCount = 1;
		x.pQueuePriorities = &queuePriority;
	}

	VkPhysicalDeviceFeatures deviceFeature = {};
	deviceFeature.samplerAnisotropy = VK_TRUE;

	VkDeviceCreateInfo deviceCreate = {};
	deviceCreate.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	deviceCreate.queueCreateInfoCount =
			static_cast<uint32_t>(queueCreate.size());
	deviceCreate.pQueueCreateInfos = queueCreate.data();
	deviceCreate.pEnabledFeatures = &deviceFeature;
	deviceCreate.enabledExtensionCount =
			static_cast<uint32_t>(deviceExtensions.size());
	deviceCreate.ppEnabledExtensionNames = deviceExtensions.data();

	if (enableValidationLayers) {
		deviceCreate.enabledLayerCount =
				static_cast<uint32_t>(validationLayers.size());
		deviceCreate.ppEnabledLayerNames = validationLayers.data();
	} else {
		deviceCreate.enabledLayerCount = 0;
	}

	if (vkCreateDevice(physicalDevice, &deviceCreate, nullptr, &device) !=
			VK_SUCCESS) {
		throw std::runtime_error("failed to create logical device!");
	}
	vkGetDeviceQueue(device, VK_CTX.graphicsfamily, 0, graphicsQueue);
	vkGetDeviceQueue(device, VK_CTX.presentfamily, 0, presentQueue);
	return device;
}

static VkDescriptorPool
createDescriptorPool(VkDevice device)
{
	VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
	std::array<VkDescriptorPoolSize, 4> poolSize{};
	poolSize[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSize[0].descriptorCount = static_cast<uint32_t>(conf::MAX_FRAMES_IN_FLIGHT * 100);
	poolSize[1].type = VK_DESCRIPTOR_TYPE_SAMPLER;
	poolSize[1].descriptorCount = static_cast<uint32_t>(conf::MAX_FRAMES_IN_FLIGHT * 100);
	poolSize[2].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	poolSize[2].descriptorCount = static_cast<uint32_t>(conf::MAX_FRAMES_IN_FLIGHT * 100);
	poolSize[3].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	poolSize[3].descriptorCount = static_cast<uint32_t>(conf::MAX_FRAMES_IN_FLIGHT * 100);

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSize.size());
	poolInfo.pPoolSizes = poolSize.data();
	poolInfo.maxSets = static_cast<uint32_t>(conf::MAX_FRAMES_IN_FLIGHT * 100); //TODO:
	if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
		return VK_NULL_HANDLE;
	return descriptorPool;
}

static VmaAllocator
vma_init(VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device) {
	VmaAllocator allocator = VK_NULL_HANDLE;
	VmaVulkanFunctions vulkanFunctions = {};
	vulkanFunctions.vkGetInstanceProcAddr = &vkGetInstanceProcAddr;
	vulkanFunctions.vkGetDeviceProcAddr = &vkGetDeviceProcAddr;
	VmaAllocatorCreateInfo allocatorCreateInfo = {};
	allocatorCreateInfo.vulkanApiVersion = VK_API_VERSION_1_2;
	allocatorCreateInfo.physicalDevice = physicalDevice;
	allocatorCreateInfo.device = device;
	allocatorCreateInfo.instance = instance;
	allocatorCreateInfo.pVulkanFunctions = &vulkanFunctions;
	vmaCreateAllocator(&allocatorCreateInfo, &allocator);
	return allocator;
}

static VkCommandPool
createCommandPool(VkPhysicalDevice phydevice, VkDevice logicdevice, VkSurfaceKHR surface)
{
	VkCommandPool commandpool;
	VkCommandPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = CTX.graphicsfamily;
	if (vkCreateCommandPool(logicdevice, &poolInfo, nullptr, &commandpool) != VK_SUCCESS)
		return VK_NULL_HANDLE;
	return commandpool;
}

static void
create_engine_descriptor_set() 
{
	std::array<VkDescriptorSetLayoutBinding, 11> bindings;
	auto &b0 = bindings[0];
	b0.binding = ENGINE_PER_FRAME_BINDING;
	b0.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	b0.descriptorCount = 1;
	b0.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	b0.pImmutableSamplers = nullptr;

	auto &b1 = bindings[1];
	b1.binding = ENGINE_PER_CAMERA_BINDING;
	b1.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	b1.descriptorCount = 1;
	b1.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	b1.pImmutableSamplers = nullptr;

	auto &b2 = bindings[2];
	b2.binding = ENGINE_PER_OBJECT_BINDING;
	b2.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	b2.descriptorCount = 1;
	b2.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
	b2.pImmutableSamplers = nullptr;

	auto &b3 = bindings[3];
	b3.binding = ENGINE_BRDF_TEX_BINDING;
	b3.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	b3.descriptorCount = 1;
	b3.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	b3.pImmutableSamplers = nullptr;

	auto &b4 = bindings[4];
	b4.binding = ENGINE_BRDF_TEX_SAMPLER_BINDING;
	b4.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
	b4.descriptorCount = 1;
	b4.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	b4.pImmutableSamplers = nullptr;

	auto &b5 = bindings[5];
	b5.binding = ENGINE_SKYBOX_SPECULAR_BINDING;
	b5.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	b5.descriptorCount = 1;
	b5.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	b5.pImmutableSamplers = nullptr;

	auto &b6 = bindings[6];
	b6.binding = ENGINE_SKYBOX_SPECULAR_SAMPLER_BINDING;
	b6.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
	b6.descriptorCount = 1;
	b6.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	b6.pImmutableSamplers = nullptr;

	auto &b7 = bindings[7];
	b7.binding = ENGINE_SKYBOX_IRRADIANCE_BINDING;
	b7.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	b7.descriptorCount = 1;
	b7.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	b7.pImmutableSamplers = nullptr;

	auto &b8 = bindings[8];
	b8.binding = ENGINE_SKYBOX_IRRADIANCE_SAMPLER_BINDING;
	b8.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
	b8.descriptorCount = 1;
	b8.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	b8.pImmutableSamplers = nullptr;

	auto &b9 = bindings[9];
	b9.binding = ENGINE_SHADOWMAP_BINDING;
	b9.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	b9.descriptorCount = ENGINE_MAX_DIRECTIONAL_LIGHT;
	b9.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	b9.pImmutableSamplers = nullptr;

	auto &b10 = bindings[10];
	b10.binding = ENGINE_SHADOWMAP_SAMPLER_BINDING;
	b10.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
	b10.descriptorCount = 1;
	b10.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
	b10.pImmutableSamplers = nullptr;

	VkDescriptorSetLayoutCreateInfo ci{};
	ci.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	ci.bindingCount = bindings.size();
	ci.pBindings = bindings.data();
	auto result = vkCreateDescriptorSetLayout(CTX.device, &ci, nullptr, &CTX.engine_desc_set_layout);
	assert(result == VK_SUCCESS);
	VkDescriptorSetLayout layouts[conf::MAX_FRAMES_IN_FLIGHT];
	for (int i = 0; i < conf::MAX_FRAMES_IN_FLIGHT; i++) 
		layouts[i] = CTX.engine_desc_set_layout;
	VkDescriptorSetAllocateInfo dsa{};
	dsa.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	dsa.descriptorPool = CTX.descriptorpool;
	dsa.descriptorSetCount = conf::MAX_FRAMES_IN_FLIGHT;
	dsa.pSetLayouts = layouts;
	result = vkAllocateDescriptorSets(
		CTX.device, &dsa, CTX.engine_desc_set);
	assert(result == VK_SUCCESS);
}
	
void
vk_ctx_init_lighting()
{
	static struct {
		render::texture *tex;
		int binding;
		int sampler_binding;
	} textures[] = {
		{LIGHTING_ASSET.brdf_texture.get(), ENGINE_BRDF_TEX_BINDING, ENGINE_BRDF_TEX_SAMPLER_BINDING},
		{LIGHTING_ASSET.skybox_specular.get(), ENGINE_SKYBOX_SPECULAR_BINDING, ENGINE_SKYBOX_SPECULAR_SAMPLER_BINDING},
		{LIGHTING_ASSET.skybox_irradiance.get(), ENGINE_SKYBOX_IRRADIANCE_BINDING, ENGINE_SKYBOX_IRRADIANCE_SAMPLER_BINDING},
	};
	int count = sizeof(textures) / sizeof(textures[0]);
	std::array<VkDescriptorImageInfo, sizeof(textures) / sizeof(textures[0])> imageInfo;
	for (int i = 0; i < count; i++) {
		imageInfo[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo[i].imageView = native_of(textures[i].tex).view;
		imageInfo[i].sampler = native_of(textures[i].tex).sampler(textures[i].tex);
	}
	std::array<VkWriteDescriptorSet, (sizeof(textures) / sizeof(textures[0])) * 2 * conf::MAX_FRAMES_IN_FLIGHT> descriptorWrite;
	int k = 0;
	for (int i = 0; i < count; i++) {
		for (int j = 0; j < conf::MAX_FRAMES_IN_FLIGHT; j++) {
			descriptorWrite[k].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite[k].dstSet = VK_CTX.engine_desc_set[j];
			descriptorWrite[k].dstBinding = textures[i].binding;
			descriptorWrite[k].dstArrayElement = 0;
			descriptorWrite[k].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
			descriptorWrite[k].descriptorCount = 1;
			descriptorWrite[k].pImageInfo = &imageInfo[i];
			descriptorWrite[k].pNext = VK_NULL_HANDLE;
			++k;

			descriptorWrite[k].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite[k].dstSet = VK_CTX.engine_desc_set[j];
			descriptorWrite[k].dstBinding = textures[i].sampler_binding;
			descriptorWrite[k].dstArrayElement = 0;
			descriptorWrite[k].descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
			descriptorWrite[k].descriptorCount = 1;
			descriptorWrite[k].pImageInfo = &imageInfo[i];
			descriptorWrite[k].pNext = VK_NULL_HANDLE;
			++k;
		}
	}
	vkUpdateDescriptorSets(VK_CTX.device,
		static_cast<uint32_t>(descriptorWrite.size()),
		descriptorWrite.data(), 0, nullptr);


}

static VkFormat
find_supported_format(
		const std::vector<VkFormat> &candidates,
		VkImageTiling tiling, VkFormatFeatureFlags features)
{
	for (VkFormat fmt : candidates) {
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(VK_CTX.phydevice, fmt, &props);
		if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
			return fmt;
		} else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
			return fmt;
		}
	}
	throw std::runtime_error("failed to find supported format");
}

static VkFormat
find_depth_format()
{
	return find_supported_format(
		{ VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT },
		VK_IMAGE_TILING_OPTIMAL,
		VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

VkPipelineCache
create_pipeline_cache()
{
	VkResult result;
	VkPipelineCache pipeline_cache;
	VkPipelineCacheCreateInfo pipelineCacheInfo;
	pipelineCacheInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	pipelineCacheInfo.pNext = NULL;
	pipelineCacheInfo.initialDataSize = 0;
	pipelineCacheInfo.pInitialData = NULL;
	pipelineCacheInfo.flags         = 0;
	result = vkCreatePipelineCache(VK_CTX.device, &pipelineCacheInfo, NULL, &pipeline_cache);
	assert(result == VK_SUCCESS);
	return pipeline_cache;
}

VkSampleCountFlagBits getMaxUsableSampleCount() 
{
    VkSampleCountFlags counts = VK_CTX.properties.limits.framebufferColorSampleCounts & VK_CTX.properties.limits.framebufferDepthSampleCounts;
    if (counts & VK_SAMPLE_COUNT_64_BIT) { return VK_SAMPLE_COUNT_64_BIT; }
    if (counts & VK_SAMPLE_COUNT_32_BIT) { return VK_SAMPLE_COUNT_32_BIT; }
    if (counts & VK_SAMPLE_COUNT_16_BIT) { return VK_SAMPLE_COUNT_16_BIT; }
    if (counts & VK_SAMPLE_COUNT_8_BIT) { return VK_SAMPLE_COUNT_8_BIT; }
    if (counts & VK_SAMPLE_COUNT_4_BIT) { return VK_SAMPLE_COUNT_4_BIT; }
    if (counts & VK_SAMPLE_COUNT_2_BIT) { return VK_SAMPLE_COUNT_2_BIT; }
    return VK_SAMPLE_COUNT_1_BIT;
}

int
vk_ctx_init(const char *name, surface *s, int width, int height)
{
	CTX.instance = create_instance(name, conf::VERSION_MAJOR, conf::VERSION_MINOR, s);
	auto &ext = CTX;
	setup_debug_callback(CTX.instance,  &ext.dbgcallback);
	surface_bind(s, CTX.instance, &ext.surface);
	CTX.phydevice = pickPhysicalDevice(CTX.instance, ext.surface);
	vkGetPhysicalDeviceProperties(CTX.phydevice, &CTX.properties);
	CTX.device = createLogicalDevice(CTX.phydevice, ext.surface, &CTX.graphicsqueue, &CTX.presentqueue);
	CTX.descriptorpool = createDescriptorPool(CTX.device);
	CTX.allocator = vma_init(CTX.instance, CTX.phydevice, CTX.device);
	CTX.commandpool = createCommandPool(CTX.phydevice, CTX.device, ext.surface);
	CTX.surface = ext.surface;
	CTX.pipeline_cache = create_pipeline_cache();
	CTX.swapchain.init(CTX.instance, CTX.phydevice, CTX.device, CTX.surface, width, height);
	VkResult result;
	VkCommandBufferAllocateInfo cba = {};
	cba.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	cba.commandPool = VK_CTX.commandpool;
	cba.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	cba.commandBufferCount = CTX.cmdbufs.size();;
	result = vkAllocateCommandBuffers(VK_CTX.device, &cba, CTX.cmdbufs.data());
	if (result != VK_SUCCESS) {
		fprintf(stderr, "[render] new_renderframe result:%d\n", result);
		return -1;
	}
	create_engine_descriptor_set();
	CTX.depth_format = find_depth_format();
	std::cout << "The GPU has a minimum buffer alignment of " <<
		CTX.properties.limits.minUniformBufferOffsetAlignment << std::endl;

	CTX.vkCmdBeginDebugUtilsLabelEXT = (PFN_vkCmdBeginDebugUtilsLabelEXT)vkGetInstanceProcAddr(VK_CTX.instance, "vkCmdBeginDebugUtilsLabelEXT");
	CTX.vkCmdEndDebugUtilsLabelEXT = (PFN_vkCmdEndDebugUtilsLabelEXT)vkGetInstanceProcAddr(VK_CTX.instance, "vkCmdEndDebugUtilsLabelEXT");
	CTX.msaaSamples = getMaxUsableSampleCount();

	return 0;

}

void
vk_ctx_frame_begin()
{
	CTX.cmdbuf = CTX.cmdbufs[VK_CTX.frame_index];
}

void 
vk_ctx_frame_end()
{
	CTX.frame_index = (CTX.frame_index + 1) % conf::MAX_FRAMES_IN_FLIGHT;
}

void
vk_ctx_renderpass_begin(render_texture *rt)
{
	if (rt != nullptr) {
		auto vk_rt = (vk_render_texture *)rt;
		VK_CTX.current_renderpass = vk_rt->render_pass;
		VK_CTX.current_framebuffer = vk_rt->framebuffer();
		VK_CTX.enable_msaa = vk_rt->enable_msaa;
		vk_rt->begin();
	} else {
		VK_CTX.current_renderpass = VK_CTX.swapchain.render_pass;
		VK_CTX.current_framebuffer = VK_CTX.swapchain.framebuffer();
		VK_CTX.enable_msaa = false;
		VK_CTX.swapchain.begin();
	}
	if (rt != nullptr) {
		vk_ctx_debug_label_begin("RenderToTexture");
	} else {
		vk_ctx_debug_label_begin("RenderToSwapchain");
	}
}

void
vk_ctx_renderpass_end()
{
	vk_ctx_debug_label_end();
	vkCmdEndRenderPass(VK_CTX.cmdbuf);
}

void
vk_ctx_cleanup()
{
	CTX.swapchain.destroy();
	vkDestroyCommandPool(CTX.device, CTX.commandpool, nullptr);
	vkDestroyDevice(CTX.device, nullptr);
	if (enableValidationLayers)
		DestroyDebugReportCallbackEXT(CTX.instance, CTX.dbgcallback, nullptr);
	vkDestroySurfaceKHR(CTX.instance, CTX.surface, nullptr);
	vkDestroyInstance(CTX.instance, nullptr);
}


void
vk_ctx_debug_label_begin(const char *label)
{
	VkDebugUtilsLabelEXT label_info;
	label_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
	label_info.pNext = nullptr;
	label_info.pLabelName = label;
	label_info.color[0] = 1.0f;
	label_info.color[1] = 1.0f;
	label_info.color[2] = 1.0f;
	label_info.color[3] = 1.0f;
	//VK_CTX.vkCmdBeginDebugUtilsLabelEXT(VK_CTX.cmdbuf, &label_info);
}

void
vk_ctx_debug_label_end()
{
	//VK_CTX.vkCmdEndDebugUtilsLabelEXT(VK_CTX.cmdbuf);
}

}}

