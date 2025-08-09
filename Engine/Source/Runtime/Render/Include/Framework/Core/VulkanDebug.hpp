/*
 * Vulkan examples debug wrapper
 *
 * Copyright (C) 2016-2023 by Sascha Willems - www.saschawillems.de
 *
 * This code is licensed under the MIT license (MIT) (http://opensource.org/licenses/MIT)
 */

#pragma once
// 1. 确认已包含 volk.h，这是正确的。
#include <volk.h>

#include <math.h>
#include <stdlib.h>
#include <string>
#include <cstring>
#include <fstream>
#include <assert.h>
#include <stdio.h>
#include <vector>
#include <sstream>
#ifdef _WIN32
#include <windows.h>
#include <fcntl.h>
#include <io.h>
#endif
#ifdef __ANDROID__
#include "VulkanAndroid.h"
#endif
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace vks
{
	namespace debug
	{
		// 这些函数的声明保持不变，因为它们管理的是 Debug Messenger 对象的生命周期和配置。
		VKAPI_ATTR VkBool32 VKAPI_CALL debugUtilsMessageCallback(
			VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
			void *pUserData);

		void setupDebugging(VkInstance instance);
		void freeDebugCallback(VkInstance instance);
		void setupDebugingMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &debugUtilsMessengerCI);
	}

	namespace debugutils
	{
		// 2. 移除这个函数的声明！
		//    它的唯一作用是加载函数指针，这个工作现在由 volk 自动完成。
		//    它的实现（.cpp 文件中）现在是空的，所以不应该再暴露在公共接口（头文件）中。
		void setup(VkInstance instance);  // <-- THIS LINE IS REMOVED

		// 这些函数是该模块提供的核心功能，需要保留。
		void cmdBeginLabel(VkCommandBuffer cmdbuffer, std::string caption, glm::vec4 color);
		void cmdEndLabel(VkCommandBuffer cmdbuffer);
	}
}