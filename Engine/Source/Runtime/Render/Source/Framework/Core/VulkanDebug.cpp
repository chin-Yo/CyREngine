/*
 * Vulkan examples debug wrapper
 *
 * Copyright (C) 2016-2023 by Sascha Willems - www.saschawillems.de
 *
 * This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
 */

#include "Framework/Core/VulkanDebug.hpp"
#include <iostream>

namespace vks
{
	namespace debug
	{
		// 1. 移除所有 PFN_ 和全局函数指针变量
		// PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT; // -> REMOVED
		// PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT; // -> REMOVED
		VkDebugUtilsMessengerEXT debugUtilsMessenger; // 这个状态变量需要保留

		// debugUtilsMessageCallback 函数本身不需要任何改动
		VKAPI_ATTR VkBool32 VKAPI_CALL debugUtilsMessageCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
			void *pUserData)
		{
			// ... (这部分代码保持原样) ...
			// Select prefix depending on flags passed to the callback
			std::string prefix;

			if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
			{
				prefix = "VERBOSE: ";
#if defined(_WIN32)
				// Windows 控制台颜色代码 (可选，但保留)
				// ...
#endif
			}
			else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
			{
				prefix = "INFO: ";
			}
			else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
			{
				prefix = "WARNING: ";
#if defined(_WIN32)
				prefix = "\033[33m" + prefix + "\033[0m";
#endif
			}
			else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
			{
				prefix = "ERROR: ";
#if defined(_WIN32)
				prefix = "\033[31m" + prefix + "\033[0m";
#endif
			}

			// Display message to default output (console/logcat)
			std::stringstream debugMessage;
			if (pCallbackData->pMessageIdName)
			{
				debugMessage << prefix << "[" << pCallbackData->messageIdNumber << "][" << pCallbackData->pMessageIdName << "] : " << pCallbackData->pMessage;
			}
			else
			{
				debugMessage << prefix << "[" << pCallbackData->messageIdNumber << "] : " << pCallbackData->pMessage;
			}

#if defined(__ANDROID__)
			// ... (Android logcat 部分保持原样) ...
#else
			if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
			{
				std::cerr << debugMessage.str() << "\n\n";
			}
			else
			{
				std::cout << debugMessage.str() << "\n\n";
			}
			fflush(stdout);
#endif
			return VK_FALSE;
		}

		// setupDebugingMessengerCreateInfo 函数本身不需要任何改动
		void setupDebugingMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &debugUtilsMessengerCI)
		{
			debugUtilsMessengerCI.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
			debugUtilsMessengerCI.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT; // 建议加上 VERBOSE
			debugUtilsMessengerCI.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;		   // 建议加上 PERFORMANCE
			debugUtilsMessengerCI.pfnUserCallback = debugUtilsMessageCallback;
		}

		void setupDebugging(VkInstance instance)
		{
			// 2. 移除所有 vkGetInstanceProcAddr 调用
			// vkCreateDebugUtilsMessengerEXT = ...; // -> REMOVED
			// vkDestroyDebugUtilsMessengerEXT = ...; // -> REMOVED

			VkDebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCI{};
			setupDebugingMessengerCreateInfo(debugUtilsMessengerCI);

			// 3. 直接像调用普通函数一样调用 Vulkan API
			// 前提是 volkInitialize() 已经在别处被调用过了
			VkResult result = vkCreateDebugUtilsMessengerEXT(instance, &debugUtilsMessengerCI, nullptr, &debugUtilsMessenger);
			assert(result == VK_SUCCESS);
		}

		void freeDebugCallback(VkInstance instance)
		{
			if (debugUtilsMessenger != VK_NULL_HANDLE)
			{
				// 4. 直接调用，不再通过函数指针
				vkDestroyDebugUtilsMessengerEXT(instance, debugUtilsMessenger, nullptr);
			}
		}
	}

	namespace debugutils
	{
		// 1. 移除所有 PFN_ 和全局函数指针变量
		// PFN_vkCmdBeginDebugUtilsLabelEXT vkCmdBeginDebugUtilsLabelEXT{ nullptr }; // -> REMOVED
		// PFN_vkCmdEndDebugUtilsLabelEXT vkCmdEndDebugUtilsLabelEXT{ nullptr }; // -> REMOVED
		// PFN_vkCmdInsertDebugUtilsLabelEXT vkCmdInsertDebugUtilsLabelEXT{ nullptr }; // -> REMOVED

		void setup(VkInstance instance)
		{
			// 2. 这个函数现在是空的，甚至可以被移除！
			// 因为 volk 已经自动加载了所有函数，我们不再需要一个手动的 setup 步骤。
			// 保留一个空的函数体是为了不破坏已有的调用代码，但理想情况下可以删除它和它的调用。
		}

		void cmdBeginLabel(VkCommandBuffer cmdbuffer, std::string caption, glm::vec4 color)
		{
			// 3. 直接调用，但最好检查一下函数指针是否有效
			// 因为这是一个扩展函数，volk 只有在扩展被启用时才会加载它
			if (vkCmdBeginDebugUtilsLabelEXT)
			{
				VkDebugUtilsLabelEXT labelInfo{};
				labelInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_LABEL_EXT;
				labelInfo.pLabelName = caption.c_str();
				memcpy(labelInfo.color, &color[0], sizeof(float) * 4);
				vkCmdBeginDebugUtilsLabelEXT(cmdbuffer, &labelInfo);
			}
		}

		void cmdEndLabel(VkCommandBuffer cmdbuffer)
		{
			// 4. 直接调用，并进行同样的检查
			if (vkCmdEndDebugUtilsLabelEXT)
			{
				vkCmdEndDebugUtilsLabelEXT(cmdbuffer);
			}
		}
	}
}