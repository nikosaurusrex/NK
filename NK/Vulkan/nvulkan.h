#pragma once

#include <vulkan/vulkan.h>
#include <vma/vk_mem_alloc.h>
#include <winbase.h>

#include "base/base_inc.h"
#include "vulkan/vulkan_core.h"

#define VK_CHECK(call)                                                                             \
    if (call != VK_SUCCESS) {                                                                      \
        LogFatal("Vulkan call failed at %s:%d\n", __FILE__, __LINE__);                             \
    }

#define VK_CHECK_SWAPCHAIN(call)                                                                   \
    do {                                                                                           \
        VkResult cres = call;                                                                      \
        if (cres != VK_SUCCESS && cres != VK_SUBOPTIMAL_KHR && cres != VK_ERROR_OUT_OF_DATE_KHR) { \
            LogFatal("Vulkan call failed at %s:%d\n", __FILE__, __LINE__);                         \
        }                                                                                          \
    } while(0)

struct VulkanState {
    Arena *arena;
    // For all temporary allocations during creation
    // In each scenario where you query the size first and
    // then query a second time returning the actual data
    Arena *temp;
    VkInstance instance;
    VkSurfaceKHR surface;
    VkPhysicalDevice pdevice;
    VkDevice ldevice;
    VkQueue graphics_queue;
    u32 graphics_queue_index;
    VmaAllocator allocator;
};

#define SWAPCHAIN_IMAGE_COUNT 2
struct Swapchain {
    VkSwapchainKHR handle;
    VkSurfaceFormatKHR format;
    VkQueryPool query_pool;

    u32 width;
    u32 height;
    u32 current_semaphore;
    u32 current_image;

    VkSemaphore acquire_semaphore;
    VkSemaphore release_semaphore;
    VkFence frame_fence;
    VkImage images[SWAPCHAIN_IMAGE_COUNT];
};

struct Image {
    VkImage handle;
    VkImageView view;
    VmaAllocation allocation;
    VkFormat format;
};

struct Texture {
    Image image;
    VkDescriptorImageInfo descriptor;
};

struct DescriptorSet {
    VkDescriptorSet handle;
    VkDescriptorSetLayout layout;
    VkDescriptorPool pool;
};

struct Shader {
    const char *path;
    VkShaderStageFlagBits stage;
};

struct Pipeline {
    VkPipeline handle;
    VkPipelineLayout layout;
};

struct Buffer {
    VkBuffer handle;
    VmaAllocation allocation;
};

struct StagingBuffer {
    VkBuffer handle;
    VmaAllocation allocation;
    VmaAllocationInfo allocation_info;
};

VulkanState InitVulkan(Arena *arena, Arena *temp, HINSTANCE hinstance, HWND hwnd);
void DeinitVulkan(VulkanState *state);

VkCommandPool CreateCommandPool(VulkanState *state);
void DestroyCommandPool(VulkanState *state, VkCommandPool cmdpool);

void AllocateCommandBuffers(VulkanState *state, VkCommandPool cmdpool, VkCommandBuffer *buffers, u32 buffers_count);
void FreeCommandBuffers(VulkanState *state, VkCommandPool cmdpool, VkCommandBuffer *buffers, u32 buffers_count);

void BeginCommandBuffer(VulkanState *state, VkCommandBuffer cmdbuf);
void EndCommandBuffer(VulkanState *state, VkCommandBuffer cmdbuf);
void SubmitCommandBuffer(VulkanState *state, VkCommandBuffer cmdbuf);

VkCommandBuffer BeginTempCommandBuffer(VulkanState *state, VkCommandPool cmdpool);
void EndTempCommandBuffer(VulkanState *state, VkCommandPool cmdpool, VkCommandBuffer cmdbuf);

void CreateSwapchain(VulkanState *state, Swapchain *swapchain, VkCommandPool cmdpool);
void DestroySwapchain(VulkanState *state, Swapchain *swapchain);
void UpdateSwapchain(VulkanState *state, Swapchain *swapchain, VkCommandPool cmdpool, b8 vsync);
b32 AcquireSwapchain(VulkanState *state, Swapchain *swapchain, VkCommandPool cmdpool, VkCommandBuffer cmdbuf, Image color_target, Image depth_target);
void PresentSwapchain(VulkanState *state, Swapchain *swapchain, VkCommandBuffer cmdbuf, Image color_target);

Image CreateImage(VulkanState *state, u32 width, u32 height, VkFormat format, u32 mip_levels, VkImageAspectFlags aspect_mask, VkImageUsageFlags usage);
Image CreateDepthImage(VulkanState *state, Swapchain *swapchain, VkCommandPool cmdpool);
void DestroyImage(VulkanState *state, Image image);

Texture CreateTexture(VulkanState *state, u32 width, u32 height, VkFormat format, VkImageAspectFlags aspect_mask, VkImageUsageFlags usage);
Texture CreateTextureFromPixels(VulkanState *state, u32 width, u32 height, u32 channels, VkFormat format, u8 *pixels,
                                VkSamplerCreateInfo sampler_info, VkCommandPool cmdpool);
void DestroyTexture(VulkanState *state, Texture tex);

VkImageMemoryBarrier2 CreateImageBarrier(VkImage image, VkPipelineStageFlags2 src_stage, VkAccessFlags2 src_access, VkImageLayout old_layout,
                                         VkPipelineStageFlags2 dst_stage, VkAccessFlags2 dst_access, VkImageLayout new_layout, VkImageAspectFlags aspect_mask);
void PipelineImageBarriers(VkCommandBuffer cmdbuf, VkDependencyFlags flags, VkImageMemoryBarrier2 *barriers, u32 barriers_count);

VkBufferMemoryBarrier2 CreateBufferBarrier(VkBuffer buffer, VkDeviceSize size, VkPipelineStageFlags2 src_stage, VkAccessFlags2 src_access,
                                         VkPipelineStageFlags2 dst_stage, VkAccessFlags2 dst_access);
void PipelineBufferBarriers(VkCommandBuffer cmdbuf, VkDependencyFlags flags, VkBufferMemoryBarrier2 *barriers, u32 barriers_count);

DescriptorSet CreateDescriptorSet(VulkanState *state, VkDescriptorSetLayoutBinding *bindings, u32 bindings_count);
void FreeDescriptorSet(VulkanState *state, DescriptorSet set);


Pipeline CreatePipeline(VulkanState *state, VkDescriptorSetLayout desc_set_layout, VkPipelineRenderingCreateInfo rendering_info,
                        VkCullModeFlags cull_mode, VkBool32 depth_test, Shader *shaders, u32 shaders_count);
void DestroyPipeline(VulkanState *state, Pipeline pipeline);

void CopyBuffer(VulkanState *state, VkBuffer dst, VkBuffer src, VkDeviceSize size, VkCommandPool cmdpool);
Buffer CreateBuffer(VulkanState *state, VkCommandPool cmdpool, VkBufferUsageFlags usage, VkDeviceSize size, void *data);
StagingBuffer CreateStagingBuffer(VulkanState *state, VkDeviceSize size, void *data);
void DestroyBuffer(VulkanState *state, Buffer buffer);
void DestroyStagingBuffer(VulkanState *state, StagingBuffer buffer);
void UpdateRendererBuffer(Buffer buffer, VkDeviceSize size, void *data, VkCommandBuffer cmdbuf);

VkQueryPool CreateQueryPool(VulkanState *state, uint32_t count, VkQueryType type);
void DestroyQueryPool(VulkanState *state, VkQueryPool pool);
