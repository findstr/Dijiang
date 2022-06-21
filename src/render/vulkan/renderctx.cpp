#include <memory>
#include <vector>
#include <set>
#include <optional>
#include <iostream>
#include <unordered_map>
#include <vulkan/vulkan.hpp>
#include "vk_mem_alloc.h"

#include "conf.h"
#include "vk_surface.h"
#include "vk_texture.h"
#include "renderctx.h"

namespace engine {
namespace vulkan {

renderctx *CTX = nullptr;

struct extent {
	VkSurfaceKHR surface = VK_NULL_HANDLE;
	VkDebugReportCallbackEXT dbgcallback = VK_NULL_HANDLE;
};

static std::unordered_map<const renderctx *, extent> EXT;

const std::vector<const char *> validationLayers = {
	"VK_LAYER_KHRONOS_validation",
};

const std::vector<const char *> deviceExtensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME,
};

#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

struct QueueFamilyIndices {
	int graphicsFamily = -1;
	int presentFamily = -1;
	bool isComplete() { return graphicsFamily >= 0 && presentFamily >= 0; }
};

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

static SwapChainSupportDetails
querySwapChainSupport(VkPhysicalDevice device, VkSurfaceKHR surface)
{
	uint32_t formatCount;
	uint32_t presentModeCount;
	SwapChainSupportDetails details;
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
	if (formatCount != 0) {
		details.formats.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
	}
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);
	if (presentModeCount != 0) {
		details.presentModes.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
	}
	return details;
}

static QueueFamilyIndices
findQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface) {
	QueueFamilyIndices indices;
	uint32_t count = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(device, &count, nullptr);
	std::vector<VkQueueFamilyProperties> queueFamilies(count);
	vkGetPhysicalDeviceQueueFamilyProperties(device, &count, queueFamilies.data());
	int i = 0;
	for (auto const &qf : queueFamilies) {
		if (qf.queueCount > 0) {
			VkBool32 presentSupport = false;
			if (qf.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				indices.graphicsFamily = i;
			}
			vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
			if (presentSupport)
				indices.presentFamily = i;
		}
		if (indices.isComplete())
			break;
		++i;
	}
	return indices;
}



static bool
isDeviceSuitable(VkPhysicalDevice device, VkSurfaceKHR surface)
{
	auto qf = findQueueFamilies(device, surface);
	auto extSupport = checkDeviceExtensionSupport(device);
	bool swapChainAdequate = false;
	if (extSupport) {
		auto swapChainSupport = querySwapChainSupport(device, surface);
		swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
	}
	VkPhysicalDeviceFeatures supportedFeatures;
	vkGetPhysicalDeviceFeatures(device, &supportedFeatures);
	return qf.isComplete() && extSupport && swapChainAdequate && supportedFeatures.samplerAnisotropy;
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
	QueueFamilyIndices indices = findQueueFamilies(physicalDevice, surface);
	std::vector<VkDeviceQueueCreateInfo> queueCreate;
	std::set<int> uniqueQueueFamilies = {indices.graphicsFamily, indices.presentFamily};

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
	vkGetDeviceQueue(device, indices.graphicsFamily, 0, graphicsQueue);
	vkGetDeviceQueue(device, indices.presentFamily, 0, presentQueue);
	return device;
}

static VkDescriptorPool
createDescriptorPool(VkDevice device)
{
	VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
	std::array<VkDescriptorPoolSize, 4> poolSize{};
	poolSize[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSize[0].descriptorCount = static_cast<uint32_t>(conf::MAX_FRAMES_IN_FLIGHT * 10);
	poolSize[1].type = VK_DESCRIPTOR_TYPE_SAMPLER;
	poolSize[1].descriptorCount = static_cast<uint32_t>(conf::MAX_FRAMES_IN_FLIGHT * 10);
	poolSize[2].type = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
	poolSize[2].descriptorCount = static_cast<uint32_t>(conf::MAX_FRAMES_IN_FLIGHT * 10);
	poolSize[3].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
	poolSize[3].descriptorCount = static_cast<uint32_t>(conf::MAX_FRAMES_IN_FLIGHT * 10);

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSize.size());
	poolInfo.pPoolSizes = poolSize.data();
	poolInfo.maxSets = static_cast<uint32_t>(conf::MAX_FRAMES_IN_FLIGHT * 5); //TODO:
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

static VkExtent2D
chooseSwapExtent(const VkSurfaceCapabilitiesKHR &cap, int width, int height)
{
	if (cap.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
		return cap.currentExtent;
	} else {
		VkExtent2D ext = {(uint32_t)width, (uint32_t)height};
		ext.width = std::max(cap.minImageExtent.width, std::min(cap.maxImageExtent.width, ext.width));
		ext.height = std::max(cap.minImageExtent.height, std::min(cap.minImageExtent.height, ext.height));
		return ext;
	}
}

static VkSurfaceFormatKHR chooseSwapSurfaceFormat(
			const std::vector<VkSurfaceFormatKHR> &availableFormats) {
		for (const auto &fmt : availableFormats) {
			if (fmt.format == VK_FORMAT_B8G8R8A8_SRGB&&
					fmt.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
				return fmt;
		}
		return availableFormats[0];
	}
static 	VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> &list) {
		VkPresentModeKHR besetMode = VK_PRESENT_MODE_FIFO_KHR;
		for (auto const p : list) {
			if (p == VK_PRESENT_MODE_MAILBOX_KHR)
				return p;
			if (p == VK_PRESENT_MODE_IMMEDIATE_KHR)
				besetMode = p;
		}
		return besetMode;
	}

static VkImageView
texture_new_view(const renderctx *ctx,
	VkImage image,
	VkFormat format,
	VkImageAspectFlags aspectFlags,
	uint32_t mipLevels)
{
	VkImageView imageView;
	VkImageViewCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	createInfo.image = image;
	createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	createInfo.format = format;
	createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
	createInfo.subresourceRange.aspectMask = aspectFlags;
	createInfo.subresourceRange.baseMipLevel = 0;
	createInfo.subresourceRange.levelCount = mipLevels;
	createInfo.subresourceRange.baseArrayLayer = 0;
	createInfo.subresourceRange.layerCount = 1;
	if (vkCreateImageView(ctx->logicdevice, &createInfo, nullptr, &imageView) != VK_SUCCESS)
		return VK_NULL_HANDLE;
	return imageView;
}



static swapchainst
createSwapChain(renderctx *ctx, VkPhysicalDevice phydevice, VkDevice device, VkSurfaceKHR surface, int width, int height)
{
	swapchainst swapchain;
	auto chain = querySwapChainSupport(phydevice, surface);
	auto surfaceFormat = chooseSwapSurfaceFormat(chain.formats);
	auto presentMode = chooseSwapPresentMode(chain.presentModes);
	auto extent = chooseSwapExtent(chain.capabilities, width, height);
	uint32_t imageCount = chain.capabilities.minImageCount + 1;
	if (chain.capabilities.maxImageCount > 0 && imageCount > chain.capabilities.maxImageCount)
		imageCount = chain.capabilities.maxImageCount;
	printf("createSwapChain:%d - %d\n", chain.capabilities.minImageCount, chain.capabilities.maxImageCount);

	VkSwapchainCreateInfoKHR createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface = surface;
	createInfo.minImageCount = imageCount;
	createInfo.imageFormat = surfaceFormat.format;
	createInfo.imageColorSpace = surfaceFormat.colorSpace;
	createInfo.imageExtent = extent;
	createInfo.imageArrayLayers = 1;
	createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	QueueFamilyIndices indices = findQueueFamilies(phydevice, surface);
	uint32_t queueIndices[] = {(uint32_t)indices.graphicsFamily, (uint32_t)indices.presentFamily};
	if (indices.graphicsFamily != indices.presentFamily) {
		createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount = 2;
		createInfo.pQueueFamilyIndices = queueIndices;
	} else {
		createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount = 0;
		createInfo.pQueueFamilyIndices = nullptr;
	}
	createInfo.preTransform = chain.capabilities.currentTransform;
	createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode = presentMode;
	createInfo.clipped = VK_TRUE;
	createInfo.oldSwapchain = VK_NULL_HANDLE;

	if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &swapchain.handle) != VK_SUCCESS)
		throw std::runtime_error("failed to create swap chain");
	swapchain.imageformat = surfaceFormat.format;
	swapchain.extent = extent;

	std::vector<VkImage> images;
	vkGetSwapchainImagesKHR(ctx->logicdevice, swapchain.handle, &imageCount, nullptr);
	images.resize(imageCount);
	vkGetSwapchainImagesKHR(ctx->logicdevice, swapchain.handle, &imageCount, images.data());
	swapchain.imageviews.resize(images.size());
	for (size_t i = 0; i < images.size(); i++) {
		swapchain.imageviews[i] = texture_new_view(ctx, images[i], swapchain.imageformat, VK_IMAGE_ASPECT_COLOR_BIT, 1);
	}
	return std::move(swapchain);
}

static VkCommandPool
createCommandPool(VkPhysicalDevice phydevice, VkDevice logicdevice, VkSurfaceKHR surface)
{
	VkCommandPool commandpool;
	QueueFamilyIndices family = findQueueFamilies(phydevice, surface);
	VkCommandPoolCreateInfo poolInfo = {};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex = family.graphicsFamily;
	if (vkCreateCommandPool(logicdevice, &poolInfo, nullptr, &commandpool) != VK_SUCCESS)
		return VK_NULL_HANDLE;
	return commandpool;
}

int
renderctx_init(renderctx *ctx, const char *name, surface *s, int width, int height)
{
	ctx->instance = create_instance(name, conf::VERSION_MAJOR, conf::VERSION_MINOR, s);
	auto &ext = EXT[ctx];
	setup_debug_callback(ctx->instance,  &ext.dbgcallback);
	surface_bind(s, ctx->instance, &ext.surface);
	ctx->phydevice = pickPhysicalDevice(ctx->instance, ext.surface);
	vkGetPhysicalDeviceProperties(ctx->phydevice, &ctx->properties);
	ctx->logicdevice = createLogicalDevice(ctx->phydevice, ext.surface, &ctx->graphicsqueue, &ctx->presentqueue);
	ctx->descriptorpool = createDescriptorPool(ctx->logicdevice);
	ctx->allocator = vma_init(ctx->instance, ctx->phydevice, ctx->logicdevice);
	ctx->swapchain = createSwapChain(ctx, ctx->phydevice, ctx->logicdevice, ext.surface, width, height);
	ctx->commandpool = createCommandPool(ctx->phydevice, ctx->logicdevice, ext.surface);
	CTX = ctx;
	std::cout << "The GPU has a minimum buffer alignment of " <<
		ctx->properties.limits.minUniformBufferOffsetAlignment << std::endl;
	return 0;
}

static void
cleanupSwapchain(const renderctx *ctx)
{
	for (auto view : ctx->swapchain.imageviews)
		vkDestroyImageView(ctx->logicdevice, view, nullptr);
	vkDestroySwapchainKHR(ctx->logicdevice, ctx->swapchain.handle, nullptr);
}

void
renderctx_cleanup(renderctx *ctx)
{
	cleanupSwapchain(ctx);
	vkDestroyCommandPool(ctx->logicdevice, ctx->commandpool, nullptr);
	vkDestroyDevice(ctx->logicdevice, nullptr);
	if (enableValidationLayers)
		DestroyDebugReportCallbackEXT(ctx->instance, EXT[ctx].dbgcallback, nullptr);
	vkDestroySurfaceKHR(ctx->instance, EXT[ctx].surface, nullptr);
	vkDestroyInstance(ctx->instance, nullptr);
	EXT.erase(ctx);
	CTX = nullptr;
}

const renderctx *
renderctx_get()
{
	return CTX;
}

}}

