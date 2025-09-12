// #include "Framework/Core/VulkanDevice.hpp"
// #include <iostream>
// #include <unordered_set>
// #include "Framework/Core/VulkanTexture.hpp"
// #include "Logging/Logger.hpp"
// #include "VulkanDevice.hpp"

// namespace vkb
// {

//     bool VulkanDevice::IsEnableInstanceExtension(const char *Extension)
//     {
//         return std::find_if(
//                    EnabledInstanceExtensions.begin(),
//                    EnabledInstanceExtensions.end(),
//                    [Extension](const char *enabled_extension)
//                    {
//                        return strcmp(Extension, enabled_extension) == 0;
//                    }) != enabled_extensions.end();
//     }

//     /**
//      * Default constructor
//      *
//      * @param physicalDevice Physical device that is to be used
//      */
//     VulkanDevice::VulkanDevice(VkPhysicalDevice physicalDevice)
//     {
//         assert(physicalDevice);
//         this->physicalDevice = physicalDevice;

//         // Store Properties features, limits and properties of the physical device for later use
//         // VulkanDevice properties also contain limits and sparse properties
//         vkGetPhysicalDeviceProperties(physicalDevice, &properties);
//         // Features should be checked by the examples before using them
//         vkGetPhysicalDeviceFeatures(physicalDevice, &features);
//         // Memory properties are used regularly for creating all kinds of buffers
//         vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memoryProperties);
//         // Queue family properties, used for setting up requested queues upon device creation
//         uint32_t queueFamilyCount;
//         vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);
//         assert(queueFamilyCount > 0);
//         queueFamilyProperties.resize(queueFamilyCount);
//         vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilyProperties.data());

//         // Get list of supported extensions
//         uint32_t extCount = 0;
//         vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extCount, nullptr);
//         if (extCount > 0)
//         {
//             std::vector<VkExtensionProperties> extensions(extCount);
//             if (vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extCount, &extensions.front()) == VK_SUCCESS)
//             {
//                 for (auto &ext : extensions)
//                 {
//                     SupportedExtensions.push_back(ext.extensionName);
//                     // EnabledExtensions.push_back(ext.extensionName);
//                     LOG_INFO("Supported extension: {} (specVersion: {})", ext.extensionName, ext.specVersion)
//                 }
//             }
//         }
//     }

//     /**
//      * Default destructor
//      *
//      * @note Frees the logical device
//      */
//     VulkanDevice::~VulkanDevice()
//     {
//         if (commandPool)
//         {
//             vkDestroyCommandPool(logicalDevice, commandPool, nullptr);
//         }
//         if (logicalDevice)
//         {
//             vkDestroyDevice(logicalDevice, nullptr);
//         }
//     }

//     /**
//      * Get the index of a memory type that has all the requested property bits set
//      *
//      * @param typeBits Bit mask with bits set for each memory type supported by the resource to request for (from VkMemoryRequirements)
//      * @param properties Bit mask of properties for the memory type to request
//      * @param (Optional) memTypeFound Pointer to a bool that is set to true if a matching memory type has been found
//      *
//      * @return Index of the requested memory type
//      *
//      * @throw Throws an exception if memTypeFound is null and no memory type could be found that supports the requested properties
//      */
//     uint32_t VulkanDevice::getMemoryType(uint32_t typeBits, VkMemoryPropertyFlags properties, VkBool32 *memTypeFound) const
//     {
//         for (uint32_t i = 0; i < memoryProperties.memoryTypeCount; i++)
//         {
//             if ((typeBits & 1) == 1)
//             {
//                 if ((memoryProperties.memoryTypes[i].propertyFlags & properties) == properties)
//                 {
//                     if (memTypeFound)
//                     {
//                         *memTypeFound = true;
//                     }
//                     return i;
//                 }
//             }
//             typeBits >>= 1;
//         }

//         if (memTypeFound)
//         {
//             *memTypeFound = false;
//             return 0;
//         }
//         else
//         {
//             throw std::runtime_error("Could not find a matching memory type");
//         }
//     }

//     /**
//      * Get the index of a queue family that supports the requested queue flags
//      * SRS - support VkQueueFlags parameter for requesting multiple flags vs. VkQueueFlagBits for a single flag only
//      *
//      * @param queueFlags Queue flags to find a queue family index for
//      *
//      * @return Index of the queue family index that matches the flags
//      *
//      * @throw Throws an exception if no queue family index could be found that supports the requested flags
//      */
//     uint32_t VulkanDevice::getQueueFamilyIndex(VkQueueFlags queueFlags) const
//     {
//         // Dedicated queue for compute
//         // Try to find a queue family index that supports compute but not graphics
//         if ((queueFlags & VK_QUEUE_COMPUTE_BIT) == queueFlags)
//         {
//             for (uint32_t i = 0; i < static_cast<uint32_t>(queueFamilyProperties.size()); i++)
//             {
//                 if ((queueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) && ((queueFamilyProperties[i].queueFlags &
//                                                                                       VK_QUEUE_GRAPHICS_BIT) == 0))
//                 {
//                     return i;
//                 }
//             }
//         }

//         // Dedicated queue for transfer
//         // Try to find a queue family index that supports transfer but not graphics and compute
//         if ((queueFlags & VK_QUEUE_TRANSFER_BIT) == queueFlags)
//         {
//             for (uint32_t i = 0; i < static_cast<uint32_t>(queueFamilyProperties.size()); i++)
//             {
//                 if ((queueFamilyProperties[i].queueFlags & VK_QUEUE_TRANSFER_BIT) && ((queueFamilyProperties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) == 0) && ((queueFamilyProperties[i].queueFlags & VK_QUEUE_COMPUTE_BIT) == 0))
//                 {
//                     return i;
//                 }
//             }
//         }

//         // For other queue types or if no separate compute queue is present, return the first one to support the requested flags
//         for (uint32_t i = 0; i < static_cast<uint32_t>(queueFamilyProperties.size()); i++)
//         {
//             if ((queueFamilyProperties[i].queueFlags & queueFlags) == queueFlags)
//             {
//                 return i;
//             }
//         }

//         throw std::runtime_error("Could not find a matching queue family index");
//     }

//     /**
//      * Create the logical device based on the assigned physical device, also gets default queue family indices
//      *
//      * @param enabledFeatures Can be used to enable certain features upon device creation
//      * @param pNextChain Optional chain of pointer to extension structures
//      * @param useSwapChain Set to false for headless rendering to omit the swapchain device extensions
//      * @param requestedQueueTypes Bit flags specifying the queue types to be requested from the device
//      *
//      * @return VkResult of the device creation call
//      */
//     VkResult VulkanDevice::createLogicalDevice(VkPhysicalDeviceFeatures enabledFeatures,
//                                                std::vector<const char *> enabledExtensions, void *pNextChain,
//                                                bool useSwapChain, VkQueueFlags requestedQueueTypes)
//     {
//         // Desired queues need to be requested upon logical device creation
//         // Due to differing queue family configurations of Vulkan implementations this can be a bit tricky, especially if the application
//         // requests different queue types

//         enabledFeatures = features;

//         std::vector<VkDeviceQueueCreateInfo> queueCreateInfos{};

//         // Get queue family indices for the requested queue family types
//         // Note that the indices may overlap depending on the implementation

//         constexpr float defaultQueuePriority(0.0f);

//         // Graphics queue
//         if (requestedQueueTypes & VK_QUEUE_GRAPHICS_BIT)
//         {
//             queueFamilyIndices.graphics = getQueueFamilyIndex(VK_QUEUE_GRAPHICS_BIT);
//             VkDeviceQueueCreateInfo queueInfo{};
//             queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
//             queueInfo.queueFamilyIndex = queueFamilyIndices.graphics;
//             queueInfo.queueCount = 1;
//             queueInfo.pQueuePriorities = &defaultQueuePriority;
//             queueCreateInfos.push_back(queueInfo);
//         }
//         else
//         {
//             queueFamilyIndices.graphics = 0;
//         }

//         // Dedicated compute queue
//         if (requestedQueueTypes & VK_QUEUE_COMPUTE_BIT)
//         {
//             queueFamilyIndices.compute = getQueueFamilyIndex(VK_QUEUE_COMPUTE_BIT);
//             if (queueFamilyIndices.compute != queueFamilyIndices.graphics)
//             {
//                 // If compute family index differs, we need an additional queue create info for the compute queue
//                 VkDeviceQueueCreateInfo queueInfo{};
//                 queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
//                 queueInfo.queueFamilyIndex = queueFamilyIndices.compute;
//                 queueInfo.queueCount = 1;
//                 queueInfo.pQueuePriorities = &defaultQueuePriority;
//                 queueCreateInfos.push_back(queueInfo);
//             }
//         }
//         else
//         {
//             // Else we use the same queue
//             queueFamilyIndices.compute = queueFamilyIndices.graphics;
//         }

//         // Dedicated transfer queue
//         if (requestedQueueTypes & VK_QUEUE_TRANSFER_BIT)
//         {
//             queueFamilyIndices.transfer = getQueueFamilyIndex(VK_QUEUE_TRANSFER_BIT);
//             if ((queueFamilyIndices.transfer != queueFamilyIndices.graphics) && (queueFamilyIndices.transfer !=
//                                                                                  queueFamilyIndices.compute))
//             {
//                 // If transfer family index differs, we need an additional queue create info for the transfer queue
//                 VkDeviceQueueCreateInfo queueInfo{};
//                 queueInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
//                 queueInfo.queueFamilyIndex = queueFamilyIndices.transfer;
//                 queueInfo.queueCount = 1;
//                 queueInfo.pQueuePriorities = &defaultQueuePriority;
//                 queueCreateInfos.push_back(queueInfo);
//             }
//         }
//         else
//         {
//             // Else we use the same queue
//             queueFamilyIndices.transfer = queueFamilyIndices.graphics;
//         }

//         if (useSwapChain)
//         {
//             // If the device will be used for presenting to a display via a swapchain we need to request the swapchain extension
//             EnabledExtensions.push_back(VK_KHR_SWAPCHAIN_EXTENSION_NAME);
//         }

//         VkDeviceCreateInfo deviceCreateInfo = {};
//         deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
//         deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
//         ;
//         deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();
//         deviceCreateInfo.pEnabledFeatures = &enabledFeatures;

//         // If a pNext(Chain) has been passed, we need to add it to the device creation info
//         VkPhysicalDeviceFeatures2 physicalDeviceFeatures2{};
//         if (pNextChain)
//         {
//             physicalDeviceFeatures2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2;
//             physicalDeviceFeatures2.features = enabledFeatures;
//             physicalDeviceFeatures2.pNext = pNextChain;
//             deviceCreateInfo.pEnabledFeatures = nullptr;
//             deviceCreateInfo.pNext = &physicalDeviceFeatures2;
//         }

//         /*if (deviceExtensions.size() > 0)
//         {
//             for (const char *enabledExtension : deviceExtensions)
//             {
//                 if (!IsExtensionSupported(enabledExtension))
//                 {
//                     std::cerr << "Enabled device extension \"" << enabledExtension << "\" is not present at device level\n";
//                 }
//             }

//             deviceCreateInfo.enabledExtensionCount = (uint32_t)deviceExtensions.size();
//             deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();
//         }*/
//         std::vector<const char *> names;

//         if (EnabledExtensions.size() > 0)
//         {
//             deviceCreateInfo.enabledExtensionCount = (uint32_t)EnabledExtensions.size();
//             names.reserve(EnabledExtensions.size());
//             for (const auto &ext : EnabledExtensions)
//             {
//                 names.push_back(ext.c_str());
//             }
//             deviceCreateInfo.ppEnabledExtensionNames = names.data();
//         }

//         this->enabledFeatures = enabledFeatures;

//         VkResult result = vkCreateDevice(physicalDevice, &deviceCreateInfo, nullptr, &logicalDevice);
//         if (result != VK_SUCCESS)
//         {
//             return result;
//         }

//         // Create a default command pool for graphics command buffers
//         commandPool = createCommandPool(queueFamilyIndices.graphics);

//         // TODO: DebugUtilsExtDebugUtils
//         debugUtils = std::make_unique<vkb::DebugUtilsExtDebugUtils>();
//         return result;
//     }

//     /**
//      * Create a buffer on the device
//      *
//      * @param usageFlags Usage flag bit mask for the buffer (i.e. index, vertex, uniform buffer)
//      * @param memoryPropertyFlags Memory properties for this buffer (i.e. device local, host visible, coherent)
//      * @param size Size of the buffer in byes
//      * @param buffer Pointer to the buffer handle acquired by the function
//      * @param memory Pointer to the memory handle acquired by the function
//      * @param data Pointer to the data that should be copied to the buffer after creation (optional, if not set, no data is copied over)
//      *
//      * @return VK_SUCCESS if buffer handle and memory have been created and (optionally passed) data has been copied
//      */
//     VkResult VulkanDevice::createBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags,
//                                         VkDeviceSize size, VkBuffer *buffer, VkDeviceMemory *memory, void *data)
//     {
//         // Create the buffer handle
//         VkBufferCreateInfo bufferCreateInfo = vks::initializers::bufferCreateInfo(usageFlags, size);
//         bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
//         VK_CHECK_RESULT(vkCreateBuffer(logicalDevice, &bufferCreateInfo, nullptr, buffer));

//         // Create the memory backing up the buffer handle
//         VkMemoryRequirements memReqs;
//         VkMemoryAllocateInfo memAlloc = vks::initializers::memoryAllocateInfo();
//         vkGetBufferMemoryRequirements(logicalDevice, *buffer, &memReqs);
//         memAlloc.allocationSize = memReqs.size;
//         // Find a memory type index that fits the properties of the buffer
//         memAlloc.memoryTypeIndex = getMemoryType(memReqs.memoryTypeBits, memoryPropertyFlags);
//         // If the buffer has VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT set we also need to enable the appropriate flag during allocation
//         VkMemoryAllocateFlagsInfoKHR allocFlagsInfo{};
//         if (usageFlags & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT)
//         {
//             allocFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO_KHR;
//             allocFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
//             memAlloc.pNext = &allocFlagsInfo;
//         }
//         VK_CHECK_RESULT(vkAllocateMemory(logicalDevice, &memAlloc, nullptr, memory));

//         // If a pointer to the buffer data has been passed, map the buffer and copy over the data
//         if (data != nullptr)
//         {
//             void *mapped;
//             VK_CHECK_RESULT(vkMapMemory(logicalDevice, *memory, 0, size, 0, &mapped));
//             memcpy(mapped, data, size);
//             // If host coherency hasn't been requested, do a manual flush to make writes visible
//             if ((memoryPropertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0)
//             {
//                 VkMappedMemoryRange mappedRange = vks::initializers::mappedMemoryRange();
//                 mappedRange.memory = *memory;
//                 mappedRange.offset = 0;
//                 mappedRange.size = size;
//                 vkFlushMappedMemoryRanges(logicalDevice, 1, &mappedRange);
//             }
//             vkUnmapMemory(logicalDevice, *memory);
//         }

//         // Attach the memory to the buffer object
//         VK_CHECK_RESULT(vkBindBufferMemory(logicalDevice, *buffer, *memory, 0));

//         return VK_SUCCESS;
//     }

//     /**
//      * Create a buffer on the device
//      *
//      * @param usageFlags Usage flag bit mask for the buffer (i.e. index, vertex, uniform buffer)
//      * @param memoryPropertyFlags Memory properties for this buffer (i.e. device local, host visible, coherent)
//      * @param buffer Pointer to a vk::Vulkan buffer object
//      * @param size Size of the buffer in bytes
//      * @param data Pointer to the data that should be copied to the buffer after creation (optional, if not set, no data is copied over)
//      *
//      * @return VK_SUCCESS if buffer handle and memory have been created and (optionally passed) data has been copied
//      */
//     VkResult VulkanDevice::createBuffer(VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags,
//                                         Buffer *buffer, VkDeviceSize size, void *data)
//     {
//         buffer->device = logicalDevice;

//         // Create the buffer handle
//         VkBufferCreateInfo bufferCreateInfo = vks::initializers::bufferCreateInfo(usageFlags, size);
//         VK_CHECK_RESULT(vkCreateBuffer(logicalDevice, &bufferCreateInfo, nullptr, &buffer->buffer));

//         // Create the memory backing up the buffer handle
//         VkMemoryRequirements memReqs;
//         VkMemoryAllocateInfo memAlloc = vks::initializers::memoryAllocateInfo();
//         vkGetBufferMemoryRequirements(logicalDevice, buffer->buffer, &memReqs);
//         memAlloc.allocationSize = memReqs.size;
//         // Find a memory type index that fits the properties of the buffer
//         memAlloc.memoryTypeIndex = getMemoryType(memReqs.memoryTypeBits, memoryPropertyFlags);
//         // If the buffer has VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT set we also need to enable the appropriate flag during allocation
//         VkMemoryAllocateFlagsInfoKHR allocFlagsInfo{};
//         if (usageFlags & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT)
//         {
//             allocFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO_KHR;
//             allocFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT_KHR;
//             memAlloc.pNext = &allocFlagsInfo;
//         }
//         VK_CHECK_RESULT(vkAllocateMemory(logicalDevice, &memAlloc, nullptr, &buffer->memory));

//         buffer->alignment = memReqs.alignment;
//         buffer->size = size;
//         buffer->usageFlags = usageFlags;
//         buffer->memoryPropertyFlags = memoryPropertyFlags;

//         // If a pointer to the buffer data has been passed, map the buffer and copy over the data
//         if (data != nullptr)
//         {
//             VK_CHECK_RESULT(buffer->map());
//             memcpy(buffer->mapped, data, size);
//             if ((memoryPropertyFlags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT) == 0)
//                 buffer->flush();

//             buffer->unmap();
//         }

//         // Initialize a default descriptor that covers the whole buffer size
//         buffer->setupDescriptor();

//         // Attach the memory to the buffer object
//         return buffer->bind();
//     }

//     /**
//      * Copy buffer data from src to dst using VkCmdCopyBuffer
//      *
//      * @param src Pointer to the source buffer to copy from
//      * @param dst Pointer to the destination buffer to copy to
//      * @param queue Pointer
//      * @param copyRegion (Optional) Pointer to a copy region, if NULL, the whole buffer is copied
//      *
//      * @note Source and destination pointers must have the appropriate transfer usage flags set (TRANSFER_SRC / TRANSFER_DST)
//      */
//     void VulkanDevice::copyBuffer(Buffer *src, Buffer *dst, VkQueue queue, VkBufferCopy *copyRegion)
//     {
//         assert(dst->size <= src->size);
//         assert(src->buffer);
//         VkCommandBuffer copyCmd = createCommandBuffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);
//         VkBufferCopy bufferCopy{};
//         if (copyRegion == nullptr)
//         {
//             bufferCopy.size = src->size;
//         }
//         else
//         {
//             bufferCopy = *copyRegion;
//         }

//         vkCmdCopyBuffer(copyCmd, src->buffer, dst->buffer, 1, &bufferCopy);

//         flushCommandBuffer(copyCmd, queue);
//     }

//     /**
//      * Create a command pool for allocation command buffers from
//      *
//      * @param queueFamilyIndex Family index of the queue to create the command pool for
//      * @param createFlags (Optional) Command pool creation flags (Defaults to VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT)
//      *
//      * @note Command buffers allocated from the created pool can only be submitted to a queue with the same family index
//      *
//      * @return A handle to the created command buffer
//      */
//     VkCommandPool VulkanDevice::createCommandPool(uint32_t queueFamilyIndex, VkCommandPoolCreateFlags createFlags)
//     {
//         VkCommandPoolCreateInfo cmdPoolInfo = {};
//         cmdPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
//         cmdPoolInfo.queueFamilyIndex = queueFamilyIndex;
//         cmdPoolInfo.flags = createFlags;
//         VkCommandPool cmdPool;
//         VK_CHECK_RESULT(vkCreateCommandPool(logicalDevice, &cmdPoolInfo, nullptr, &cmdPool));
//         return cmdPool;
//     }

//     /**
//      * Allocate a command buffer from the command pool
//      *
//      * @param level Level of the new command buffer (primary or secondary)
//      * @param pool Command pool from which the command buffer will be allocated
//      * @param (Optional) begin If true, recording on the new command buffer will be started (vkBeginCommandBuffer) (Defaults to false)
//      *
//      * @return A handle to the allocated command buffer
//      */
//     VkCommandBuffer VulkanDevice::createCommandBuffer(VkCommandBufferLevel level, VkCommandPool pool, bool begin)
//     {
//         VkCommandBufferAllocateInfo cmdBufAllocateInfo = vks::initializers::commandBufferAllocateInfo(pool, level, 1);
//         VkCommandBuffer cmdBuffer;
//         VK_CHECK_RESULT(vkAllocateCommandBuffers(logicalDevice, &cmdBufAllocateInfo, &cmdBuffer));
//         // If requested, also start recording for the new command buffer
//         if (begin)
//         {
//             VkCommandBufferBeginInfo cmdBufInfo = vks::initializers::commandBufferBeginInfo();
//             VK_CHECK_RESULT(vkBeginCommandBuffer(cmdBuffer, &cmdBufInfo));
//         }
//         return cmdBuffer;
//     }

//     VkCommandBuffer VulkanDevice::createCommandBuffer(VkCommandBufferLevel level, bool begin)
//     {
//         return createCommandBuffer(level, commandPool, begin);
//     }

//     /**
//      * Finish command buffer recording and submit it to a queue
//      *
//      * @param commandBuffer Command buffer to flush
//      * @param queue Queue to submit the command buffer to
//      * @param pool Command pool on which the command buffer has been created
//      * @param free (Optional) Free the command buffer once it has been submitted (Defaults to true)
//      *
//      * @note The queue that the command buffer is submitted to must be from the same family index as the pool it was allocated from
//      * @note Uses a fence to ensure command buffer has finished executing
//      */
//     void VulkanDevice::flushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, VkCommandPool pool, bool free)
//     {
//         if (commandBuffer == VK_NULL_HANDLE)
//         {
//             return;
//         }

//         VK_CHECK_RESULT(vkEndCommandBuffer(commandBuffer))

//         VkSubmitInfo submitInfo = vks::initializers::submitInfo();
//         submitInfo.commandBufferCount = 1;
//         submitInfo.pCommandBuffers = &commandBuffer;
//         // Create fence to ensure that the command buffer has finished executing
//         VkFenceCreateInfo fenceInfo = vks::initializers::fenceCreateInfo(VK_FLAGS_NONE);
//         VkFence fence;
//         VK_CHECK_RESULT(vkCreateFence(logicalDevice, &fenceInfo, nullptr, &fence))
//         // Submit to the queue
//         VK_CHECK_RESULT(vkQueueSubmit(queue, 1, &submitInfo, fence))
//         // Wait for the fence to signal that command buffer has finished executing
//         VK_CHECK_RESULT(vkWaitForFences(logicalDevice, 1, &fence, VK_TRUE, DEFAULT_FENCE_TIMEOUT))
//         vkDestroyFence(logicalDevice, fence, nullptr);
//         if (free)
//         {
//             vkFreeCommandBuffers(logicalDevice, pool, 1, &commandBuffer);
//         }
//     }

//     void VulkanDevice::flushCommandBuffer(VkCommandBuffer commandBuffer, VkQueue queue, bool free)
//     {
//         return flushCommandBuffer(commandBuffer, queue, commandPool, free);
//     }

//     /**
//      * Check if an extension is supported by the (physical device)
//      *
//      * @param extension Name of the extension to check
//      *
//      * @return True if the extension is supported (present in the list read at device creation time)
//      */
//     bool VulkanDevice::IsExtensionSupported(std::string extension) const
//     {
//         return (std::find(SupportedExtensions.begin(), SupportedExtensions.end(), extension) != SupportedExtensions.end());
//     }

//     bool VulkanDevice::IsEnableExtension(const char *extension) const
//     {
//         return (std::find(EnabledExtensions.begin(), EnabledExtensions.end(), extension) != EnabledExtensions.end());
//     }

//     /**
//      * Select the best-fit depth format for this device from a list of possible depth (and stencil) formats
//      *
//      * @param checkSamplingSupport Check if the format can be sampled from (e.g. for shader reads)
//      *
//      * @return The depth format that best fits for the current device
//      *
//      * @throw Throws an exception if no depth format fits the requirements
//      */
//     VkFormat VulkanDevice::getSupportedDepthFormat(bool checkSamplingSupport)
//     {
//         // All depth formats may be optional, so we need to find a suitable depth format to use
//         std::vector<VkFormat> depthFormats = {
//             VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D32_SFLOAT, VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D16_UNORM_S8_UINT,
//             VK_FORMAT_D16_UNORM};
//         for (auto &format : depthFormats)
//         {
//             VkFormatProperties formatProperties;
//             vkGetPhysicalDeviceFormatProperties(physicalDevice, format, &formatProperties);
//             // Format must support depth stencil attachment for optimal tiling
//             if (formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
//             {
//                 if (checkSamplingSupport)
//                 {
//                     if (!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT))
//                     {
//                         continue;
//                     }
//                 }
//                 return format;
//             }
//         }
//         throw std::runtime_error("Could not find a matching depth format");
//     }

//     const vkb::DebugUtils &VulkanDevice::GetDebugUtils() const
//     {
//         return *debugUtils;
//     }

//     vkb::ResourceCache &VulkanDevice::get_resource_cache()
//     {
//         return resource_cache;
//     }
// }

/* Copyright (c) 2019-2025, Arm Limited and Contributors
 * Copyright (c) 2019-2025, Sascha Willems
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 the "License";
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "Framework/Core/VulkanDevice.hpp"
#include "Framework/Core/Buffer.hpp"
#include "Framework/Core/CommandPool.hpp"
#include "Framework/Core/PhysicalDevice.hpp"
#include "Framework/Core/Queue.hpp"
#include "Framework/Misc/FencePool.hpp"
#include "Framework/Core/Debug.hpp"

#define VMA_IMPLEMENTATION
#include <vk_mem_alloc.h>

namespace vkb
{
    VulkanDevice::VulkanDevice(PhysicalDevice &gpu,
                               VkSurfaceKHR surface,
                               std::unique_ptr<DebugUtils> &&debug_utils,
                               std::unordered_map<const char *, bool> requested_extensions) : vkb::VulkanResource<VkDevice>{VK_NULL_HANDLE, this}, // Recursive, but valid
                                                                                              debug_utils{std::move(debug_utils)},
                                                                                              gpu{gpu},
                                                                                              resource_cache{*this}
    {
        LOGI("Selected GPU: {}", gpu.get_properties().deviceName);

        // Prepare the device queues
        uint32_t queue_family_properties_count = to_u32(gpu.get_queue_family_properties().size());
        std::vector<VkDeviceQueueCreateInfo> queue_create_infos(queue_family_properties_count, {VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO});
        std::vector<std::vector<float>> queue_priorities(queue_family_properties_count);

        for (uint32_t queue_family_index = 0U; queue_family_index < queue_family_properties_count; ++queue_family_index)
        {
            const VkQueueFamilyProperties &queue_family_property = gpu.get_queue_family_properties()[queue_family_index];

            if (gpu.has_high_priority_graphics_queue())
            {
                uint32_t graphics_queue_family = get_queue_family_index(VK_QUEUE_GRAPHICS_BIT);
                if (graphics_queue_family == queue_family_index)
                {
                    queue_priorities[queue_family_index].reserve(queue_family_property.queueCount);
                    queue_priorities[queue_family_index].push_back(1.0f);
                    for (uint32_t i = 1; i < queue_family_property.queueCount; i++)
                    {
                        queue_priorities[queue_family_index].push_back(0.5f);
                    }
                }
                else
                {
                    queue_priorities[queue_family_index].resize(queue_family_property.queueCount, 0.5f);
                }
            }
            else
            {
                queue_priorities[queue_family_index].resize(queue_family_property.queueCount, 0.5f);
            }

            VkDeviceQueueCreateInfo &queue_create_info = queue_create_infos[queue_family_index];

            queue_create_info.queueFamilyIndex = queue_family_index;
            queue_create_info.queueCount = queue_family_property.queueCount;
            queue_create_info.pQueuePriorities = queue_priorities[queue_family_index].data();
        }

        // Check extensions to enable Vma Dedicated Allocation
        bool can_get_memory_requirements = is_extension_supported("VK_KHR_get_memory_requirements2");
        bool has_dedicated_allocation = is_extension_supported("VK_KHR_dedicated_allocation");

        if (can_get_memory_requirements && has_dedicated_allocation)
        {
            enabled_extensions.push_back("VK_KHR_get_memory_requirements2");
            enabled_extensions.push_back("VK_KHR_dedicated_allocation");

            LOGI("Dedicated Allocation enabled");
        }

        // For performance queries, we also use host query reset since queryPool resets cannot
        // live in the same command buffer as beginQuery
        if (is_extension_supported("VK_KHR_performance_query") &&
            is_extension_supported("VK_EXT_host_query_reset"))
        {
            auto perf_counter_features =
                gpu.get_extension_features<VkPhysicalDevicePerformanceQueryFeaturesKHR>(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PERFORMANCE_QUERY_FEATURES_KHR);
            auto host_query_reset_features =
                gpu.get_extension_features<VkPhysicalDeviceHostQueryResetFeatures>(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_QUERY_RESET_FEATURES);

            if (perf_counter_features.performanceCounterQueryPools && host_query_reset_features.hostQueryReset)
            {
                gpu.add_extension_features<VkPhysicalDevicePerformanceQueryFeaturesKHR>(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PERFORMANCE_QUERY_FEATURES_KHR)
                    .performanceCounterQueryPools = VK_TRUE;
                gpu.add_extension_features<VkPhysicalDeviceHostQueryResetFeatures>(VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_HOST_QUERY_RESET_FEATURES).hostQueryReset =
                    VK_TRUE;
                enabled_extensions.push_back("VK_KHR_performance_query");
                enabled_extensions.push_back("VK_EXT_host_query_reset");
                LOGI("Performance query enabled");
            }
        }

        // Check that extensions are supported before trying to create the device
        std::vector<const char *> unsupported_extensions{};
        for (auto &extension : requested_extensions)
        {
            if (is_extension_supported(extension.first))
            {
                enabled_extensions.emplace_back(extension.first);
            }
            else
            {
                unsupported_extensions.emplace_back(extension.first);
            }
        }

        if (enabled_extensions.size() > 0)
        {
            LOGI("VulkanDevice supports the following requested extensions:");
            for (auto &extension : enabled_extensions)
            {
                LOGI("  \t{}", extension);
            }
        }

        if (unsupported_extensions.size() > 0)
        {
            auto error = false;
            for (auto &extension : unsupported_extensions)
            {
                auto extension_is_optional = requested_extensions[extension];
                if (extension_is_optional)
                {
                    LOGW("Optional device extension {} not available, some features may be disabled", extension);
                }
                else
                {
                    LOGE("Required device extension {} not available, cannot run", extension);
                    error = true;
                }
            }

            if (error)
            {
                throw VulkanException(VK_ERROR_EXTENSION_NOT_PRESENT, "Extensions not present");
            }
        }

        VkDeviceCreateInfo create_info{VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO};

        // Latest requested feature will have the pNext's all set up for device creation.
        create_info.pNext = gpu.get_extension_feature_chain();

        create_info.pQueueCreateInfos = queue_create_infos.data();
        create_info.queueCreateInfoCount = to_u32(queue_create_infos.size());
        create_info.enabledExtensionCount = to_u32(enabled_extensions.size());
        create_info.ppEnabledExtensionNames = enabled_extensions.data();

        const auto requested_gpu_features = gpu.get_requested_features();
        create_info.pEnabledFeatures = &requested_gpu_features;

        VkResult result = vkCreateDevice(gpu.get_handle(), &create_info, nullptr, &GetHandle());

        if (result != VK_SUCCESS)
        {
            throw VulkanException{result, "Cannot create device"};
        }

        queues.resize(queue_family_properties_count);

        for (uint32_t queue_family_index = 0U; queue_family_index < queue_family_properties_count; ++queue_family_index)
        {
            const VkQueueFamilyProperties &queue_family_property = gpu.get_queue_family_properties()[queue_family_index];

            VkBool32 present_supported = gpu.is_present_supported(surface, queue_family_index);

            for (uint32_t queue_index = 0U; queue_index < queue_family_property.queueCount; ++queue_index)
            {
                queues[queue_family_index].emplace_back(*this, queue_family_index, queue_family_property, present_supported, queue_index);
            }
        }

        prepare_memory_allocator();

        command_pool = std::make_unique<vkb::CommandPool>(*this, get_queue_by_flags(VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT, 0).get_family_index());
        fence_pool = std::make_unique<FencePool>(*this);
    }

    VulkanDevice::VulkanDevice(PhysicalDevice &gpu, VkDevice &vulkan_device, VkSurfaceKHR surface) : VulkanResource{vulkan_device},
                                                                                                     gpu{gpu},
                                                                                                     resource_cache{*this}
    {
        debug_utils = std::make_unique<DummyDebugUtils>();
    }

    VulkanDevice::~VulkanDevice()
    {
        resource_cache.clear();

        command_pool.reset();
        fence_pool.reset();

        vkb::shutdown();

        vkDestroyDevice(GetHandle(), nullptr);
    }

    bool VulkanDevice::is_extension_supported(const std::string &requested_extension) const
    {
        return gpu.is_extension_supported(requested_extension);
    }

    bool VulkanDevice::is_enabled(const char *extension) const
    {
        return std::find_if(enabled_extensions.begin(), enabled_extensions.end(),
                            [extension](const char *enabled_extension)
                            {
                                return strcmp(extension, enabled_extension) == 0;
                            }) != enabled_extensions.end();
    }

    const PhysicalDevice &VulkanDevice::get_gpu() const
    {
        return gpu;
    }

    DriverVersion VulkanDevice::get_driver_version() const
    {
        DriverVersion version;

        switch (gpu.get_properties().vendorID)
        {
        case 0x10DE:
        {
            // Nvidia
            version.major = (gpu.get_properties().driverVersion >> 22) & 0x3ff;
            version.minor = (gpu.get_properties().driverVersion >> 14) & 0x0ff;
            version.patch = (gpu.get_properties().driverVersion >> 6) & 0x0ff;
            // Ignoring optional tertiary info in lower 6 bits
            break;
        }
        default:
        {
            version.major = VK_VERSION_MAJOR(gpu.get_properties().driverVersion);
            version.minor = VK_VERSION_MINOR(gpu.get_properties().driverVersion);
            version.patch = VK_VERSION_PATCH(gpu.get_properties().driverVersion);
        }
        }

        return version;
    }

    bool VulkanDevice::is_image_format_supported(VkFormat format) const
    {
        VkImageFormatProperties format_properties;

        auto result = vkGetPhysicalDeviceImageFormatProperties(gpu.get_handle(),
                                                               format,
                                                               VK_IMAGE_TYPE_2D,
                                                               VK_IMAGE_TILING_OPTIMAL,
                                                               VK_IMAGE_USAGE_SAMPLED_BIT,
                                                               0, // no create flags
                                                               &format_properties);
        return result != VK_ERROR_FORMAT_NOT_SUPPORTED;
    }

    uint32_t VulkanDevice::get_memory_type(uint32_t bits, VkMemoryPropertyFlags properties, VkBool32 *memory_type_found) const
    {
        for (uint32_t i = 0; i < gpu.get_memory_properties().memoryTypeCount; i++)
        {
            if ((bits & 1) == 1)
            {
                if ((gpu.get_memory_properties().memoryTypes[i].propertyFlags & properties) == properties)
                {
                    if (memory_type_found)
                    {
                        *memory_type_found = true;
                    }
                    return i;
                }
            }
            bits >>= 1;
        }

        if (memory_type_found)
        {
            *memory_type_found = false;
            return 0;
        }
        else
        {
            throw std::runtime_error("Could not find a matching memory type");
        }
    }

    const Queue &VulkanDevice::get_queue(uint32_t queue_family_index, uint32_t queue_index)
    {
        return queues[queue_family_index][queue_index];
    }

    const Queue &VulkanDevice::get_queue_by_flags(VkQueueFlags required_queue_flags, uint32_t queue_index) const
    {
        for (uint32_t queue_family_index = 0U; queue_family_index < queues.size(); ++queue_family_index)
        {
            Queue const &first_queue = queues[queue_family_index][0];

            VkQueueFlags queue_flags = first_queue.get_properties().queueFlags;
            uint32_t queue_count = first_queue.get_properties().queueCount;

            if (((queue_flags & required_queue_flags) == required_queue_flags) && queue_index < queue_count)
            {
                return queues[queue_family_index][queue_index];
            }
        }

        throw std::runtime_error("Queue not found");
    }

    const Queue &VulkanDevice::get_queue_by_present(uint32_t queue_index) const
    {
        for (uint32_t queue_family_index = 0U; queue_family_index < queues.size(); ++queue_family_index)
        {
            Queue const &first_queue = queues[queue_family_index][0];

            uint32_t queue_count = first_queue.get_properties().queueCount;

            if (first_queue.support_present() && queue_index < queue_count)
            {
                return queues[queue_family_index][queue_index];
            }
        }

        throw std::runtime_error("Queue not found");
    }

    void VulkanDevice::add_queue(size_t global_index, uint32_t family_index, VkQueueFamilyProperties properties, VkBool32 can_present)
    {
        if (queues.size() < global_index + 1)
        {
            queues.resize(global_index + 1);
        }
        queues[global_index].emplace_back(*this, family_index, properties, can_present, 0);
    }

    uint32_t VulkanDevice::get_num_queues_for_queue_family(uint32_t queue_family_index)
    {
        const auto &queue_family_properties = gpu.get_queue_family_properties();
        return queue_family_properties[queue_family_index].queueCount;
    }

    uint32_t VulkanDevice::get_queue_family_index(VkQueueFlagBits queue_flag)
    {
        const auto &queue_family_properties = gpu.get_queue_family_properties();

        // Dedicated queue for compute
        // Try to find a queue family index that supports compute but not graphics
        if (queue_flag & VK_QUEUE_COMPUTE_BIT)
        {
            for (uint32_t i = 0; i < static_cast<uint32_t>(queue_family_properties.size()); i++)
            {
                if ((queue_family_properties[i].queueFlags & queue_flag) && !(queue_family_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT))
                {
                    return i;
                    break;
                }
            }
        }

        // Dedicated queue for transfer
        // Try to find a queue family index that supports transfer but not graphics and compute
        if (queue_flag & VK_QUEUE_TRANSFER_BIT)
        {
            for (uint32_t i = 0; i < static_cast<uint32_t>(queue_family_properties.size()); i++)
            {
                if ((queue_family_properties[i].queueFlags & queue_flag) && !(queue_family_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) && !(queue_family_properties[i].queueFlags & VK_QUEUE_COMPUTE_BIT))
                {
                    return i;
                    break;
                }
            }
        }

        // For other queue types or if no separate compute queue is present, return the first one to support the requested flags
        for (uint32_t i = 0; i < static_cast<uint32_t>(queue_family_properties.size()); i++)
        {
            if (queue_family_properties[i].queueFlags & queue_flag)
            {
                return i;
                break;
            }
        }

        throw std::runtime_error("Could not find a matching queue family index");
    }

    const Queue &VulkanDevice::get_suitable_graphics_queue() const
    {
        for (uint32_t queue_family_index = 0U; queue_family_index < queues.size(); ++queue_family_index)
        {
            Queue const &first_queue = queues[queue_family_index][0];

            uint32_t queue_count = first_queue.get_properties().queueCount;

            if (first_queue.support_present() && 0 < queue_count)
            {
                return queues[queue_family_index][0];
            }
        }

        return get_queue_by_flags(VK_QUEUE_GRAPHICS_BIT, 0);
    }

    void VulkanDevice::copy_buffer(vkb::Buffer &src, vkb::Buffer &dst, VkQueue queue, VkBufferCopy *copy_region)
    {
        assert(dst.get_size() >= src.get_size());
        assert(src.GetHandle());

        VkCommandBuffer command_buffer = create_command_buffer(VK_COMMAND_BUFFER_LEVEL_PRIMARY, true);

        VkBufferCopy buffer_copy{};
        if (copy_region == nullptr)
        {
            buffer_copy.size = src.get_size();
        }
        else
        {
            buffer_copy = *copy_region;
        }

        vkCmdCopyBuffer(command_buffer, src.GetHandle(), dst.GetHandle(), 1, &buffer_copy);

        flush_command_buffer(command_buffer, queue);
    }

    VkCommandPool VulkanDevice::create_command_pool(uint32_t queue_index, VkCommandPoolCreateFlags flags)
    {
        VkCommandPoolCreateInfo command_pool_info = {};
        command_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        command_pool_info.queueFamilyIndex = queue_index;
        command_pool_info.flags = flags;
        VkCommandPool command_pool;
        VK_CHECK_RESULT(vkCreateCommandPool(GetHandle(), &command_pool_info, nullptr, &command_pool));
        return command_pool;
    }

    VkCommandBuffer VulkanDevice::create_command_buffer(VkCommandBufferLevel level, bool begin) const
    {
        assert(command_pool && "No command pool exists in the device");

        VkCommandBufferAllocateInfo cmd_buf_allocate_info{};
        cmd_buf_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        cmd_buf_allocate_info.commandPool = command_pool->get_handle();
        cmd_buf_allocate_info.level = level;
        cmd_buf_allocate_info.commandBufferCount = 1;

        VkCommandBuffer command_buffer;
        VK_CHECK_RESULT(vkAllocateCommandBuffers(GetHandle(), &cmd_buf_allocate_info, &command_buffer));

        // If requested, also start recording for the new command buffer
        if (begin)
        {
            VkCommandBufferBeginInfo command_buffer_info{};
            command_buffer_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            VK_CHECK_RESULT(vkBeginCommandBuffer(command_buffer, &command_buffer_info));
        }

        return command_buffer;
    }

    void VulkanDevice::flush_command_buffer(VkCommandBuffer command_buffer, VkQueue queue, bool free, VkSemaphore signalSemaphore) const
    {
        if (command_buffer == VK_NULL_HANDLE)
        {
            return;
        }

        VK_CHECK_RESULT(vkEndCommandBuffer(command_buffer));

        VkSubmitInfo submit_info{};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &command_buffer;
        if (signalSemaphore)
        {
            submit_info.pSignalSemaphores = &signalSemaphore;
            submit_info.signalSemaphoreCount = 1;
        }

        // Create fence to ensure that the command buffer has finished executing
        VkFenceCreateInfo fence_info{};
        fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fence_info.flags = VK_FLAGS_NONE;

        VkFence fence;
        VK_CHECK_RESULT(vkCreateFence(GetHandle(), &fence_info, nullptr, &fence));

        // Submit to the queue
        VkResult result = vkQueueSubmit(queue, 1, &submit_info, fence);
        // Wait for the fence to signal that command buffer has finished executing
        VK_CHECK_RESULT(vkWaitForFences(GetHandle(), 1, &fence, VK_TRUE, DEFAULT_FENCE_TIMEOUT));

        vkDestroyFence(GetHandle(), fence, nullptr);

        if (command_pool && free)
        {
            vkFreeCommandBuffers(GetHandle(), command_pool->get_handle(), 1, &command_buffer);
        }
    }

    vkb::CommandPool &VulkanDevice::get_command_pool() const
    {
        return *command_pool;
    }

    FencePool &VulkanDevice::get_fence_pool() const
    {
        return *fence_pool;
    }

    void VulkanDevice::create_internal_fence_pool()
    {
        fence_pool = std::make_unique<FencePool>(*this);
    }

    void VulkanDevice::create_internal_command_pool()
    {
        command_pool = std::make_unique<vkb::CommandPool>(*this, get_queue_by_flags(VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT, 0).get_family_index());
    }

    void VulkanDevice::prepare_memory_allocator()
    {
        vkb::InitVma(*this);
    }

    std::shared_ptr<vkb::CommandBuffer> VulkanDevice::request_command_buffer() const
    {
        return command_pool->request_command_buffer();
    }

    VkFence VulkanDevice::request_fence() const
    {
        return fence_pool->request_fence();
    }

    VkResult VulkanDevice::wait_idle() const
    {
        return vkDeviceWaitIdle(GetHandle());
    }

    ResourceCache &VulkanDevice::get_resource_cache()
    {
        return resource_cache;
    }
} // namespace vkb
