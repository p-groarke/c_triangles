#if defined(_WIN32)
	#define VK_USE_PLATFORM_WIN32_KHR 1
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <windows.h>
#include <vulkan/vulkan.h>

void vk_error(VkResult res) {
	if (res >= 0) {
		return;
	}

	const char* err;
	switch (res) {
		case VK_ERROR_OUT_OF_HOST_MEMORY: {
			err = "VK_ERROR_OUT_OF_HOST_MEMORY";
		} break;
		case VK_ERROR_OUT_OF_DEVICE_MEMORY: {
			err = "VK_ERROR_OUT_OF_DEVICE_MEMORY";
		} break;
		case VK_ERROR_INITIALIZATION_FAILED: {
			err = "VK_ERROR_INITIALIZATION_FAILED";
		} break;
		case VK_ERROR_DEVICE_LOST: {
			err = "VK_ERROR_DEVICE_LOST";
		} break;
		case VK_ERROR_MEMORY_MAP_FAILED: {
			err = "VK_ERROR_MEMORY_MAP_FAILED";
		} break;
		case VK_ERROR_LAYER_NOT_PRESENT: {
			err = "VK_ERROR_LAYER_NOT_PRESENT";
		} break;
		case VK_ERROR_EXTENSION_NOT_PRESENT: {
			err = "VK_ERROR_EXTENSION_NOT_PRESENT";
		} break;
		case VK_ERROR_FEATURE_NOT_PRESENT: {
			err = "VK_ERROR_FEATURE_NOT_PRESENT";
		} break;
		case VK_ERROR_INCOMPATIBLE_DRIVER: {
			err = "VK_ERROR_INCOMPATIBLE_DRIVER";
		} break;
		case VK_ERROR_TOO_MANY_OBJECTS: {
			err = "VK_ERROR_TOO_MANY_OBJECTS";
		} break;
		case VK_ERROR_FORMAT_NOT_SUPPORTED: {
			err = "VK_ERROR_FORMAT_NOT_SUPPORTED";
		} break;
		case VK_ERROR_FRAGMENTED_POOL: {
			err = "VK_ERROR_FRAGMENTED_POOL";
		} break;
		case VK_ERROR_SURFACE_LOST_KHR: {
			err = "VK_ERROR_SURFACE_LOST_KHR";
		} break;
		case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR: {
			err = "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
		} break;
		case VK_ERROR_OUT_OF_DATE_KHR: {
			err = "VK_ERROR_OUT_OF_DATE_KHR";
		} break;
		case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR: {
			err = "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
		} break;
		case VK_ERROR_VALIDATION_FAILED_EXT: {
			err = "VK_ERROR_VALIDATION_FAILED_EXT";
		} break;
		case VK_ERROR_INVALID_SHADER_NV: {
			err = "VK_ERROR_INVALID_SHADER_NV";
		} break;
		default: {
			err = "Did no find error code :(";
		}
	}
	printf("Vulkan Error: %s\n", err);
	exit(-1);
}

/* Function Pointers */
#define VK_DEVICE_LEVEL_FUNCTION( fun )										\
	fun = (PFN_##fun)vkGetDeviceProcAddr(vk_data.device, #fun);	\
	if(fun == NULL) {														\
		printf("Could not load device level function: %s\n", #fun);			\
		exit(-1);															\
	}

typedef struct DeviceFunctionPointers {
	PFN_vkGetPhysicalDeviceSurfaceFormatsKHR fpGetPhysicalDeviceSurfaceFormatsKHR;
	PFN_vkCreateWin32SurfaceKHR		fpCreateWin32SurfaceKHR;
	PFN_vkCreateSwapchainKHR		vkCreateSwapchainKHR;
	PFN_vkDestroySwapchainKHR		vkDestroySwapchainKHR;
	PFN_vkGetSwapchainImagesKHR		vkGetSwapchainImagesKHR;
	PFN_vkAcquireNextImageKHR		vkAcquireNextImageKHR;
	PFN_vkQueuePresentKHR			vkQueuePresentKHR;
} DeviceFunctionPointers;

/* Has to be assigned after device creation. */
DeviceFunctionPointers vk_ext_pfn;

/* Data */
const char* app_name = "Super Vulkan Renderer of DOOM 3000";


typedef struct InstanceData {
	VkInstance			instance;
	VkPhysicalDevice	phys_device;
	VkDevice			device;
	VkQueue				queue;
	VkSurfaceKHR		surface;
	VkSwapchainKHR		swapchain;
	VkRenderPass		render_pass;
	uint32_t			queue_family_index;
	VkCommandPool		queue_cmd_pool;
	size_t				queue_cmd_buffers_size;
	VkCommandBuffer*	queue_cmd_buffers;
	size_t				frame_buffers_size;
	VkFramebuffer*		frame_buffers;
	size_t				image_views_size;
	VkImageView*		image_views;

} InstanceData;

InstanceData vk_data = {
	.instance						= VK_NULL_HANDLE
	, .phys_device					= VK_NULL_HANDLE
	, .device						= VK_NULL_HANDLE
	, .queue						= VK_NULL_HANDLE
	, .surface						= VK_NULL_HANDLE
	, .swapchain					= VK_NULL_HANDLE
	, .render_pass					= VK_NULL_HANDLE
	, .queue_family_index			= VK_NULL_HANDLE
	, .queue_cmd_pool				= VK_NULL_HANDLE
	, .queue_cmd_buffers_size		= 0
	, .frame_buffers_size			= 0
};


typedef struct SyncData {
	VkSemaphore		s_image_available;
	VkSemaphore		s_render_finished;
} SyncData;

SyncData vk_sync_data = {
	.s_image_available				= VK_NULL_HANDLE
	, .s_render_finished			= VK_NULL_HANDLE
};


typedef struct SurfaceData {
	VkFormat		color_format;
	VkColorSpaceKHR	color_space;
	VkExtent2D		extent_2d;
} SurfaceData;

SurfaceData vk_surface_data = {
	.color_format					= VK_NULL_HANDLE
	, .color_space					= VK_NULL_HANDLE
	, .extent_2d					= {0}
};


typedef struct ExtensionData {
	const char*		instance_extensions[2];
	const char*		device_extensions[1];
} ExtensionData;

ExtensionData vk_extensions_data  = {
	.instance_extensions = {
		VK_KHR_SURFACE_EXTENSION_NAME
		#if defined(VK_USE_PLATFORM_WIN32_KHR)
			, VK_KHR_WIN32_SURFACE_EXTENSION_NAME
		#endif
	}

	, .device_extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME }
};



HINSTANCE win32_instance = NULL;
HWND win32_window = NULL;
const char* win32_class_name;

void close_window()
{
	DestroyWindow(win32_window);
	UnregisterClass(win32_class_name, win32_instance);
}

LRESULT CALLBACK WindowsEventHandler(HWND hWnd, UINT uMsg, WPARAM wParam,
		LPARAM lParam)
{
	switch (uMsg) {
		case WM_CLOSE:
			close_window();
			return 0;
		case WM_SIZE:
			/* We don't allow resize for now. */
			break;
		default:
			break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

void create_window(uint32_t size_x, uint32_t size_y, const char* name)
{
	assert(size_x > 0 && size_y > 0);
	win32_class_name = name;
	win32_instance = GetModuleHandle(NULL);

	WNDCLASSEX win_class = {
		.cbSize					= sizeof(WNDCLASSEX)
		, .style				= CS_HREDRAW | CS_VREDRAW
		, .lpfnWndProc			= WindowsEventHandler
		, .cbClsExtra			= 0
		, .cbWndExtra			= 0
		, .hInstance			= win32_instance
		, .hIcon				= LoadIcon(NULL, IDI_APPLICATION)
		, .hCursor				= LoadCursor(NULL, IDC_ARROW)
		, .hbrBackground		= (HBRUSH)GetStockObject(WHITE_BRUSH)
		, .lpszMenuName			= NULL
		, .lpszClassName		= win32_class_name
		, .hIconSm				= LoadIcon(NULL, IDI_WINLOGO)
	};

	if (!RegisterClassEx(&win_class)) {
		printf("Could not register window class. Much fail.\n");
		exit(-1);
	}

	DWORD ex_style = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
	DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;

	RECT r = {0, 0, (LONG)size_x, (LONG)size_y};
	AdjustWindowRectEx(&r, style, FALSE, ex_style);
	win32_window = CreateWindowEx(0, win32_class_name, win32_class_name, style,
			CW_USEDEFAULT, CW_USEDEFAULT, r.right - r.left, r.bottom - r. top,
			NULL, NULL, win32_instance, NULL);

	if (win32_window == NULL) {
		printf("Couldn't create the window. Much sad.\n");
		exit(-1);
	}
	//SetWindowLongPtr(win32_window, GWLP_USERDATA, NULL);
	ShowWindow(win32_window, SW_SHOW);
	SetForegroundWindow(win32_window);
	SetFocus(win32_window);
}

/* Monstruous shit */
void init_vk()
{
	/* Get windows surface extension. */
	{
		uint32_t ext_count = 0;
		vk_error(vkEnumerateInstanceExtensionProperties(NULL, &ext_count,
				NULL));

#if defined(_MSC_VER)
		VkExtensionProperties ext_list[32];
#else
		VkExtensionProperties ext_list[ext_count];
#endif
		vk_error(vkEnumerateInstanceExtensionProperties(NULL, &ext_count,
				ext_list));

		bool found = false;
		int ext_names_count = sizeof(vk_extensions_data.instance_extensions)
				/ sizeof(vk_extensions_data.instance_extensions[0]);

		for (int i = 0; i < ext_names_count; ++i) {
			for (int j = 0; j < ext_count; ++j) {
				if (strcmp(vk_extensions_data.instance_extensions[i],
							ext_list[j].extensionName) == 0)
				{
					found = true;
				}
			}
			if (!found) {
				printf("Didn't find required extension! Much sadness was had.\n");
				exit(-1);
			}
		}
	}

	/* Create instance. */
	{
		const VkApplicationInfo AppInfo = {
			.sType					= VK_STRUCTURE_TYPE_APPLICATION_INFO
			, .pNext				= NULL
			, .pApplicationName		= app_name
			, .applicationVersion	= VK_MAKE_VERSION(1, 0, 0)
			, .pEngineName			= app_name
			, .engineVersion		= VK_MAKE_VERSION(1, 0, 0)
			, .apiVersion			= VK_API_VERSION_1_0
		};

		const VkInstanceCreateInfo instance_info = {
			.sType					= VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO
			, .pNext				= NULL
			, .flags				= 0
			, .pApplicationInfo		= &AppInfo
			, .enabledExtensionCount	= sizeof(vk_extensions_data.instance_extensions)
					/ sizeof(vk_extensions_data.instance_extensions[0])
			, .ppEnabledExtensionNames	= vk_extensions_data.instance_extensions
			, .enabledLayerCount	= 0
			, .ppEnabledLayerNames	= NULL
		};

		vk_error(vkCreateInstance(&instance_info, NULL,
				&vk_data.instance));
	}

	/* Get physical device. */
	{
		uint32_t gpu_count = 1;
		vk_error(vkEnumeratePhysicalDevices(vk_data.instance,
				&gpu_count, NULL));
		assert(gpu_count);

#if defined(_MSC_VER)
		VkPhysicalDevice devices[8];
#else
		VkPhysicalDevice devices[gpu_count];
#endif
		vk_error(vkEnumeratePhysicalDevices(vk_data.instance,
				&gpu_count, devices));
		assert(gpu_count >= 1);

		printf("Found %d GPUs\n", gpu_count);
		for (int i = 0; i < gpu_count; ++i) {
			VkPhysicalDeviceProperties props;
			vkGetPhysicalDeviceProperties(devices[i], &props);
			printf("%s\n", props.deviceName);
			printf("    Maximum texture size : %d\n",
					props.limits.maxImageDimension2D);
		}

		vk_data.phys_device = devices[0];
	}

	/* Get Swap Chain extensions */
	{
		uint32_t ext_count = 0;
		vk_error(vkEnumerateDeviceExtensionProperties(vk_data.phys_device,
				NULL, &ext_count, NULL));
		assert(ext_count >= 1);

#if defined(_MSC_VER)
		VkExtensionProperties ext_props[32];
#else
		VkExtensionProperties ext_props[ext_count];
#endif
		vk_error(vkEnumerateDeviceExtensionProperties(vk_data.phys_device,
				NULL, &ext_count, ext_props));

		bool found = false;
		int ext_names_count = sizeof(vk_extensions_data.device_extensions)
				/ sizeof(vk_extensions_data.device_extensions[0]);

		for (int i = 0; i < ext_count; ++i) {
			for (int j = 0; j < ext_names_count; ++j) {
				if (strcmp(ext_props[i].extensionName,
							vk_extensions_data.device_extensions[j])
						== 0)
				{
					found = true;
				}
			}
			if (!found) {
				printf("Didn't find swap chain extension on device.\n");
				exit(-1);
			}
		}
	}

	/* Get available graphics queue. */
	{
		uint32_t queue_fam_count = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(vk_data.phys_device,
				&queue_fam_count, NULL);
		assert(queue_fam_count >= 1 );

#if defined(_MSC_VER)
		VkQueueFamilyProperties queue_fams[32];
#else
		VkQueueFamilyProperties queue_fams[queue_fam_count];
#endif
		vkGetPhysicalDeviceQueueFamilyProperties(vk_data.phys_device,
				&queue_fam_count, queue_fams);

		bool found = false;
		printf("Found %d Queue Families\n", queue_fam_count);
		for (int i = 0; i < queue_fam_count; ++i) {
			if (queue_fams[i].queueCount > 0 &&
					queue_fams[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
			{
				vk_data.queue_family_index = i;
				found = true;
			}
		}

		if (!found) {
			printf("Did not find graphic queue family! Exiting.\n");
			exit(-1);
		}
	}

	/* Create device. */
	{
		float q_priorities[] = { 1.0f };
		const VkDeviceQueueCreateInfo q_create_info = {
			.sType					= VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO
			, .pNext				= NULL
			, .flags				= 0
			, .queueFamilyIndex		= vk_data.queue_family_index
			, .queueCount			= 1
			, .pQueuePriorities		= q_priorities
		};

		const VkDeviceCreateInfo device_create_info = {
			.sType					= VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO
			, .pNext				= NULL
			, .flags				= 0
			, .queueCreateInfoCount	= 1
			, .pQueueCreateInfos	= &q_create_info
			, .enabledLayerCount	= 0
			, .ppEnabledLayerNames	= NULL
			, .enabledExtensionCount	= sizeof(vk_extensions_data.device_extensions)
					/ sizeof(vk_extensions_data.device_extensions[0])
			, .ppEnabledExtensionNames	= vk_extensions_data.device_extensions
			, .pEnabledFeatures		= 0
		};

		vk_error(vkCreateDevice(vk_data.phys_device,
				&device_create_info, VK_NULL_HANDLE,
				&vk_data.device));
	}

	/* Get Device Function Pointers. */
	{
		vk_ext_pfn.VK_DEVICE_LEVEL_FUNCTION(vkCreateSwapchainKHR)
		vk_ext_pfn.VK_DEVICE_LEVEL_FUNCTION(vkDestroySwapchainKHR)
		vk_ext_pfn.VK_DEVICE_LEVEL_FUNCTION(vkGetSwapchainImagesKHR)
		vk_ext_pfn.VK_DEVICE_LEVEL_FUNCTION(vkAcquireNextImageKHR)
		vk_ext_pfn.VK_DEVICE_LEVEL_FUNCTION(vkQueuePresentKHR)
	}

	/* Get queue. */
	{
		vkGetDeviceQueue(vk_data.device,
				vk_data.queue_family_index, 0,
				&vk_data.queue);
	}

	/* Get surface. */
	{
#if defined(VK_USE_PLATFORM_WIN32_KHR)
		vk_ext_pfn.fpCreateWin32SurfaceKHR = (PFN_vkCreateWin32SurfaceKHR)vkGetInstanceProcAddr(
				vk_data.instance, "vkCreateWin32SurfaceKHR");

		VkWin32SurfaceCreateInfoKHR surface_create_info = {
			.sType				= VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR
			, .pNext			= NULL
			, .flags			= 0
			, .hinstance		= win32_instance
			, .hwnd				= win32_window
		};

		vk_error(vk_ext_pfn.fpCreateWin32SurfaceKHR(vk_data.instance,
				&surface_create_info, NULL, &vk_data.surface));
#endif

	}

	/* Create drawing and presentation Semaphores. */
	{
		VkSemaphoreCreateInfo sem_create_info = {
			.sType					= VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
			, .pNext				= NULL
			, .flags				= 0
		};

		vk_error(vkCreateSemaphore(vk_data.device, &sem_create_info,
				NULL, &vk_sync_data.s_image_available));
		vk_error(vkCreateSemaphore(vk_data.device, &sem_create_info,
				NULL, &vk_sync_data.s_render_finished));

	}

	/* Create Swap Chain. */
	{
		/* Example of getting a funtion pointer ourselves. */
		vk_ext_pfn.fpGetPhysicalDeviceSurfaceFormatsKHR =
				(PFN_vkGetPhysicalDeviceSurfaceFormatsKHR)vkGetInstanceProcAddr(
						vk_data.instance,
						"vkGetPhysicalDeviceSurfaceFormatsKHR");

		uint32_t format_count = 0;
		vk_ext_pfn.fpGetPhysicalDeviceSurfaceFormatsKHR(vk_data.phys_device,
				vk_data.surface, &format_count, NULL);
		assert(format_count >= 1);

#if defined(_MSC_VER)
		VkSurfaceFormatKHR surface_formats[32];
#else
		VkSurfaceFormatKHR surface_formats[format_count];
#endif
		vk_ext_pfn.fpGetPhysicalDeviceSurfaceFormatsKHR(vk_data.phys_device,
				vk_data.surface, &format_count, surface_formats);

		/* Only 1 format if the device doesn't care. */
		if (format_count == 1 &&
				surface_formats[0].format == VK_FORMAT_UNDEFINED)
		{
			vk_surface_data.color_format = VK_FORMAT_B8G8R8A8_UNORM;
			vk_surface_data.color_space = VK_COLORSPACE_SRGB_NONLINEAR_KHR;
		} else {
			vk_surface_data.color_format = surface_formats[0].format;
			vk_surface_data.color_space = surface_formats[0].colorSpace;
		}

		/* Get surface capabilities. */
		VkSurfaceCapabilitiesKHR surface_capabilities;
		vk_error(vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
				vk_data.phys_device,
				vk_data.surface, &surface_capabilities));

		/* Select number of image buffers, try min + 1. */
		uint32_t image_count = surface_capabilities.minImageCount + 1;
		if (surface_capabilities.maxImageCount > 0
				&& image_count > surface_capabilities.maxImageCount)
		{
			image_count = surface_capabilities.maxImageCount;
		}
		printf("Selected %d swap chain buffers.\n", image_count);

		/* If width == height == -1, we define size ourselves.
		* This is not arbitrary.
		*/
		vk_surface_data.extent_2d = surface_capabilities.currentExtent;

		if (surface_capabilities.currentExtent.width == -1) {
			vk_surface_data.extent_2d.width = 640;
			vk_surface_data.extent_2d.height = 480;

			if (vk_surface_data.extent_2d.width
					< surface_capabilities.minImageExtent.width)
			{
				vk_surface_data.extent_2d.width =
						surface_capabilities.minImageExtent.width;
			}
			if (vk_surface_data.extent_2d.height
					< surface_capabilities.minImageExtent.height)
			{
				vk_surface_data.extent_2d.height =
						surface_capabilities.minImageExtent.height;
			}
			if (vk_surface_data.extent_2d.width
					> surface_capabilities.maxImageExtent.width)
			{
				vk_surface_data.extent_2d.width =
						surface_capabilities.maxImageExtent.width;
			}
			if (vk_surface_data.extent_2d.height
					> surface_capabilities.maxImageExtent.height)
			{
				vk_surface_data.extent_2d.height =
						surface_capabilities.maxImageExtent.height;
			}
		}

		/* Set the image usage flags.
		* VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT always supported.
		*/
		VkImageUsageFlags image_flags;
		if (surface_capabilities.supportedUsageFlags
				& VK_IMAGE_USAGE_TRANSFER_DST_BIT)
		{
			image_flags = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT
					| VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		} else {
			printf("Could not set the image usage bit. Bits are important.\n");
			exit(-1);
		}

		/* Do we want image transforms, like tablet orientation switching? */
		VkSurfaceTransformFlagBitsKHR transform_flags;
		if (surface_capabilities.supportedTransforms
				& VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
		{
			transform_flags = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
		} else {
			transform_flags = surface_capabilities.currentTransform;
		}

		/* Select presentation mode, MAILBOX best for games. */
		uint32_t p_count = 0;
		vk_error(vkGetPhysicalDeviceSurfacePresentModesKHR(
				vk_data.phys_device,
				vk_data.surface, &p_count, NULL));
		assert(p_count >= 1);

#if defined(_MSC_VER)
		VkPresentModeKHR present_modes[32];
#else
		VkPresentModeKHR present_modes[p_count];
#endif
		vk_error(vkGetPhysicalDeviceSurfacePresentModesKHR(
				vk_data.phys_device,
				vk_data.surface, &p_count,
				present_modes));


		VkPresentModeKHR selected_p_mode = VK_NULL_HANDLE;
		for (int i = 0; i < p_count; ++i) {
			if (present_modes[i] == VK_PRESENT_MODE_MAILBOX_KHR) {
				selected_p_mode = present_modes[i];
			}
		}
		if (selected_p_mode == VK_NULL_HANDLE) {
			for (int i = 0; i < p_count; ++i) {
				if (present_modes[i] == VK_PRESENT_MODE_FIFO_KHR) {
					selected_p_mode = present_modes[i];
				}
			}
		}
		if (selected_p_mode == VK_NULL_HANDLE) {
			printf("Your GPU doesn't support any presentation mode.\n");
			exit(-1);
		}

		VkSwapchainKHR old_swapchain = vk_data.swapchain;

		/* ACTUALLY Create the Swap Chain from HELL. */
		VkSwapchainCreateInfoKHR swapchain_create_info = {
			.sType					= VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR
			, .pNext				= NULL
			, .flags				= 0
			, .surface				= vk_data.surface
			, .minImageCount		= image_count
			, .imageFormat			= vk_surface_data.color_format
			, .imageColorSpace		= vk_surface_data.color_space
			, .imageExtent			= vk_surface_data.extent_2d
			, .imageArrayLayers		= 1
			, .imageUsage			= image_flags
			, .imageSharingMode		= VK_SHARING_MODE_EXCLUSIVE
			, .queueFamilyIndexCount	= 0
			, .pQueueFamilyIndices	= NULL
			, .preTransform			= transform_flags
			, .compositeAlpha		= VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR
			, .presentMode			= selected_p_mode
			, .clipped				= VK_TRUE
			, .oldSwapchain			= old_swapchain
		};

		vk_error(vk_ext_pfn.vkCreateSwapchainKHR(vk_data.device,
				&swapchain_create_info, NULL, &vk_data.swapchain));

		if (old_swapchain != VK_NULL_HANDLE) {
			vk_ext_pfn.vkDestroySwapchainKHR(vk_data.device, old_swapchain, NULL);
		}
	}

	/* Create Command Pool. */
	{
		VkCommandPoolCreateInfo cmd_pool_create_info = {
			.sType					= VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO
			, .pNext				= NULL
			, .flags				= 0
			, .queueFamilyIndex		= vk_data.queue_family_index
		};

		vk_error(vkCreateCommandPool(vk_data.device,
				&cmd_pool_create_info, NULL,
				&vk_data.queue_cmd_pool));
	}

	/* Allocate Command Buffers. */
	{
		uint32_t image_count = 0;
		vk_error(vk_ext_pfn.vkGetSwapchainImagesKHR(vk_data.device,
				vk_data.swapchain, &image_count, NULL));
		assert(image_count >= 1);

		vk_data.queue_cmd_buffers = malloc(sizeof(VkCommandBuffer) * image_count);
		vk_data.queue_cmd_buffers_size = image_count;

		VkCommandBufferAllocateInfo cmd_buffer_allocate_info = {
			.sType					= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO
			, .pNext				= NULL
			, .commandPool			= vk_data.queue_cmd_pool
			, .level				= VK_COMMAND_BUFFER_LEVEL_PRIMARY
			, .commandBufferCount	= image_count
		};

		vk_error(vkAllocateCommandBuffers(vk_data.device,
				&cmd_buffer_allocate_info,
				vk_data.queue_cmd_buffers));
	}

	/* Record Command Buffers. HYPE */
	{
		uint32_t image_count = vk_data.queue_cmd_buffers_size;
		printf("Swapchain image size : %d\n", image_count);

#if defined(_MSC_VER)
		VkImage swapchain_images[32];
#else
		VkImage swapchain_images[image_count];
#endif
		vk_error(vk_ext_pfn.vkGetSwapchainImagesKHR(vk_data.device,
				vk_data.swapchain, &image_count, swapchain_images));

		VkCommandBufferBeginInfo cmd_buffer_begin_info = {
			.sType					= VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO
			, .pNext				= NULL
			, .flags				= VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT
			, .pInheritanceInfo		= NULL
		};

		VkClearColorValue clear_color = {
			{0.0f, 1.0f, 0.0f, 0.0f }
		};

		VkImageSubresourceRange image_subresource_range = {
			.aspectMask				= VK_IMAGE_ASPECT_COLOR_BIT
			, .baseMipLevel			= 0
			, .levelCount			= 1
			, .baseArrayLayer		= 0
			, .layerCount			= 1
		};

		/* TODO : Read up on ImageBarriers and understand them. */
		for (int i = 0; i < image_count; ++i) {
			VkImageMemoryBarrier barrier_from_present_to_clear = {
				.sType				= VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER
				, .pNext			= NULL
				, .srcAccessMask	= VK_ACCESS_MEMORY_READ_BIT
				, .dstAccessMask	= VK_ACCESS_TRANSFER_WRITE_BIT
				, .oldLayout		= VK_IMAGE_LAYOUT_UNDEFINED
				, .newLayout		= VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
				, .srcQueueFamilyIndex	= vk_data.queue_family_index
				, .dstQueueFamilyIndex	= vk_data.queue_family_index
				, .image			= swapchain_images[i]
				, .subresourceRange	= image_subresource_range
			};

			VkImageMemoryBarrier barrier_from_clear_to_present = {
				.sType				= VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER
				, .pNext			= NULL
				, .srcAccessMask	= VK_ACCESS_TRANSFER_WRITE_BIT
				, .dstAccessMask	= VK_ACCESS_MEMORY_READ_BIT
				, .oldLayout		= VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL
				, .newLayout		= VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
				, .srcQueueFamilyIndex	= vk_data.queue_family_index
				, .dstQueueFamilyIndex	= vk_data.queue_family_index
				, .image			= swapchain_images[i]
				, .subresourceRange	= image_subresource_range
			};

			vkBeginCommandBuffer(vk_data.queue_cmd_buffers[i],
					&cmd_buffer_begin_info);
			vkCmdPipelineBarrier(vk_data.queue_cmd_buffers[i],
					VK_PIPELINE_STAGE_TRANSFER_BIT,
					VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL,
					1, &barrier_from_present_to_clear);
			vkCmdClearColorImage(vk_data.queue_cmd_buffers[i],
					swapchain_images[i], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
					&clear_color, 1, &image_subresource_range);
			vkCmdPipelineBarrier(vk_data.queue_cmd_buffers[i],
					VK_PIPELINE_STAGE_TRANSFER_BIT,
					VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, 0, 0, NULL, 0, NULL,
					1, &barrier_from_clear_to_present);
			vk_error(vkEndCommandBuffer(vk_data.queue_cmd_buffers[i]));

		}

	}

}

/* Rendering Pipeline*/
void init_vk_pipeline()
{
	/* Create Render Pass. */
	{
		VkAttachmentDescription attachment_descriptions[] = {
			{
				.flags					= 0
				, .format				= vk_surface_data.color_format
				, .samples				= VK_SAMPLE_COUNT_1_BIT
				, .loadOp				= VK_ATTACHMENT_LOAD_OP_CLEAR
				, .storeOp				= VK_ATTACHMENT_STORE_OP_STORE
				, .stencilLoadOp		= VK_ATTACHMENT_LOAD_OP_DONT_CARE
				, .stencilStoreOp		= VK_ATTACHMENT_STORE_OP_DONT_CARE
				, .initialLayout		= VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
				, .finalLayout			= VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
			}
		};

		VkAttachmentReference color_attachment_references[] = {
			{
				.attachment				= 0
				, .layout				= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
			}
		};

		VkSubpassDescription subpass_descriptions[] = {
			{
				.flags					= 0
				, .pipelineBindPoint	= VK_PIPELINE_BIND_POINT_GRAPHICS
				, .inputAttachmentCount	= 0
				, .pInputAttachments	= NULL
				, .colorAttachmentCount	= 1
				, .pColorAttachments	= color_attachment_references
				, .pResolveAttachments	= NULL
				, .pDepthStencilAttachment	= NULL
				, .preserveAttachmentCount	= 0
				, .pPreserveAttachments	= NULL
			}
		};

		VkRenderPassCreateInfo render_pass_create_info = {
			.sType						= VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO
			, .pNext					= NULL
			, .flags					= 0
			, .attachmentCount			= 1
			, .pAttachments				= attachment_descriptions
			, .subpassCount				= 1
			, .pSubpasses				= subpass_descriptions
			, .dependencyCount			= 0
			, .pDependencies			= NULL
		};

		vk_error(vkCreateRenderPass(vk_data.device, &render_pass_create_info,
				NULL, &vk_data.render_pass));
	}

	/* Create Framebuffer. */
	{
		uint32_t image_count = vk_data.queue_cmd_buffers_size;
#if defined(_MSC_VER)
		VkImage swapchain_images[32];
#else
		VkImage swapchain_images[image_count];
#endif
		vk_error(vk_ext_pfn.vkGetSwapchainImagesKHR(vk_data.device,
				vk_data.swapchain, &image_count, swapchain_images));

		vk_data.image_views = malloc(sizeof(VkImageView) * image_count);
		vk_data.image_views_size = image_count;

		vk_data.frame_buffers = malloc(sizeof(VkFramebuffer) * image_count);
		vk_data.frame_buffers_size = image_count;

		for (int i = 0; i < image_count; ++i) {
			VkImageViewCreateInfo image_view_create_info = {
				.sType					= VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO
				, .pNext				= NULL
				, .flags				= 0
				, .image				= swapchain_images[i]
				, .viewType				= VK_IMAGE_VIEW_TYPE_2D
				, .format				= vk_surface_data.color_format
				, .components			= {
					.r					= VK_COMPONENT_SWIZZLE_IDENTITY
					, .g				= VK_COMPONENT_SWIZZLE_IDENTITY
					, .b				= VK_COMPONENT_SWIZZLE_IDENTITY
					, .a				= VK_COMPONENT_SWIZZLE_IDENTITY
				}
				, .subresourceRange		= {
					.aspectMask			= VK_IMAGE_ASPECT_COLOR_BIT
					, .baseMipLevel		= 0
					, .levelCount		= 1
					, .baseArrayLayer	= 0
					, .layerCount		= 1
				}
			};

			vk_error(vkCreateImageView(vk_data.device, &image_view_create_info,
					NULL, &vk_data.image_views[i]));

			VkFramebufferCreateInfo framebuffer_create_info = {
				.sType					= VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO
				, .pNext				= NULL
				, .flags				= 0
				, .renderPass			= vk_data.render_pass
				, .attachmentCount		= 1
				, .pAttachments			= &vk_data.image_views[i]
				, .width				= 300
				, .height				= 300
				, .layers				= 1
			};

			vk_error(vkCreateFramebuffer(vk_data.device,
					&framebuffer_create_info, NULL, &vk_data.frame_buffers[i]));
		}
	}

	/* Creating Shaders. */
	{
//		uint32_t* vert_code = NULL;
//		size_t vert_size = 0;
//		load_vertex_shader(&vert_code, &vert_size);
	}

}

/* YES, OH YESSSSS FINALLLY! */
void vk_draw()
{
	uint32_t image_index;
	VkResult result = vk_ext_pfn.vkAcquireNextImageKHR(vk_data.device,
			vk_data.swapchain, UINT64_MAX,
			vk_sync_data.s_image_available, VK_NULL_HANDLE, &image_index);

	switch (result) {
		case VK_SUCCESS:
		case VK_SUBOPTIMAL_KHR: // Don't recreate swapchain here, do it later.
			//printf("success\n");
			break;
		case VK_ERROR_OUT_OF_DATE_KHR:
			printf("TODO : Add window resizing.\n");
			return; // TEMPORARY
		default:
			printf("Problem acquiring swapchain image. Eeeek!\n");
			vk_error(result);
			return;
	}

	/* Submit work for free image. */
	VkPipelineStageFlags wait_dst_stage_mask = VK_PIPELINE_STAGE_TRANSFER_BIT;
	VkSubmitInfo submit_info = {
		.sType						= VK_STRUCTURE_TYPE_SUBMIT_INFO
		, .pNext					= NULL
		, .waitSemaphoreCount		= 1
		, .pWaitSemaphores			= &vk_sync_data.s_image_available
		, .pWaitDstStageMask		= &wait_dst_stage_mask
		, .commandBufferCount		= 1
		, .pCommandBuffers			= &vk_data.queue_cmd_buffers[image_index]
		, .signalSemaphoreCount		= 1
		, .pSignalSemaphores		= &vk_sync_data.s_render_finished
	};

	vk_error(vkQueueSubmit(vk_data.queue, 1, &submit_info,
			VK_NULL_HANDLE));

	/* Swap images with the prepared image. */
	VkPresentInfoKHR present_info = {
		.sType						= VK_STRUCTURE_TYPE_PRESENT_INFO_KHR
		, .pNext					= NULL
		, .waitSemaphoreCount		= 1
		, .pWaitSemaphores			= &vk_sync_data.s_render_finished
		, .swapchainCount			= 1
		, .pSwapchains				= &vk_data.swapchain
		, .pImageIndices			= &image_index
		, .pResults					= NULL
	};

	result = vk_ext_pfn.vkQueuePresentKHR(vk_data.queue, &present_info);

	switch (result) {
		case VK_SUCCESS:
			//printf("present success\n");
			break;
		case VK_ERROR_OUT_OF_DATE_KHR:
		case VK_SUBOPTIMAL_KHR:
			printf("TODO : Add window resizing.\n");
			return; // TEMPORARY
		default:
			printf("Problem acquiring swapchain image. Eeeek!\n");
			vk_error(result);
			return;

	}

}

void clear_vk_buffers()
{
	if (vk_data.device == VK_NULL_HANDLE)
		return;

	vkDeviceWaitIdle(vk_data.device);

	if (vk_data.queue_cmd_buffers_size > 0
			&& vk_data.queue_cmd_buffers[0] != VK_NULL_HANDLE)
	{
		/* For example, freeing command pool frees buffers. */
		vkFreeCommandBuffers(vk_data.device, vk_data.queue_cmd_pool,
				vk_data.queue_cmd_buffers_size, vk_data.queue_cmd_buffers);
		free(vk_data.queue_cmd_buffers);
	}

	if (vk_data.queue_cmd_pool != VK_NULL_HANDLE) {
		vkDestroyCommandPool(vk_data.device, vk_data.queue_cmd_pool, NULL);
		vk_data.queue_cmd_pool = VK_NULL_HANDLE;
	}
}

void deinit_vk()
{
	clear_vk_buffers();

	free(vk_data.image_views);
	free(vk_data.frame_buffers);

	if (vk_data.device != VK_NULL_HANDLE) {
		vkDeviceWaitIdle(vk_data.device);

		if (vk_sync_data.s_image_available != VK_NULL_HANDLE) {
			vkDestroySemaphore(vk_data.device, vk_sync_data.s_image_available,
					NULL);
		}
		if (vk_sync_data.s_render_finished != VK_NULL_HANDLE) {
			vkDestroySemaphore(vk_data.device, vk_sync_data.s_render_finished,
					NULL);
		}
		if (vk_data.swapchain != VK_NULL_HANDLE) {
			vk_ext_pfn.vkDestroySwapchainKHR(vk_data.device, vk_data.swapchain,
					NULL);
		}

		vkDestroyDevice(vk_data.device, NULL);
	}

	if (vk_data.surface != VK_NULL_HANDLE) {
		vkDestroySurfaceKHR(vk_data.instance, vk_data.surface, NULL);
	}


	if (vk_data.instance != VK_NULL_HANDLE) {
		vkDestroyInstance(vk_data.instance, NULL);
	}
}

int main(int argc, char** argv) {
	printf("%s - iLLOGIKA\n\n", app_name);

	create_window(512, 512, app_name);
	init_vk();
	init_vk_pipeline();

	uint32_t count_fps = 0;
	time_t last_second = time(NULL);;
	time_t now = time(NULL);

	while (true) {
		vk_draw();

		{
			++count_fps;
			now = time(NULL);
			if (now > last_second) {
				printf("%d fps\n", count_fps);
				last_second = now;
				count_fps = 0;
			}
		}
	}

	deinit_vk();
	printf("\n");
}
