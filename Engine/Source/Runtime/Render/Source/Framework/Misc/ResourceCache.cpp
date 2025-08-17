#include "Framework/Misc/ResourceCache.hpp"
#include "Framework/Core/VulkanDevice.hpp"

namespace vkb
{
    ResourceCache::ResourceCache(VulkanDevice& device)
    : device(device)
    {
    }
}

