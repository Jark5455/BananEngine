//
// Created by yashr on 3/13/22.
//

#include "banan_image.h"
#include "banan_buffer.h"

#include <stdexcept>
#include <algorithm>
#include <memory>
#include <cassert>
#include <thread>

#include <stb_image.h>
#include <openexr.h>
#include <ImathBox.h>
#include <ImfRgbaFile.h>
#include <ImfArray.h>

namespace Banan {
    BananImage::BananImage(BananDevice &device, uint32_t width, uint32_t height, uint32_t mipLevels, VkFormat format, VkImageTiling tiling, VkSampleCountFlagBits numSamples, VkImageUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags) : bananDevice{device}, imageFormat{format}, mipLevels{mipLevels} {
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = static_cast<uint32_t>(width);
        imageInfo.extent.height = static_cast<uint32_t>(height);
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = mipLevels;
        imageInfo.arrayLayers = 1;
        imageInfo.format = imageFormat;
        imageInfo.tiling = tiling;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = usageFlags;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.samples = numSamples;

        bananDevice.createImageWithInfo(imageInfo, memoryPropertyFlags, image, memory);
        imageExtent = {width, height};
        imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        createTextureSampler();
        createTextureImageView();
    }

    BananImage::~BananImage() {
        vkDestroySampler(bananDevice.device(), imageSampler, nullptr);
        vkDestroyImageView(bananDevice.device(), imageView, nullptr);
        vkDestroyImage(bananDevice.device(), image, nullptr);
        vkFreeMemory(bananDevice.device(), memory, nullptr);
    }

    VkDescriptorImageInfo BananImage::descriptorInfo() {
        VkDescriptorImageInfo info{};
        info.imageView = imageView;
        info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        info.sampler = imageSampler;
        return info;
    }

    void BananImage::createTextureImageView() {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = imageFormat;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = mipLevels;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        VkFormat depthFormats[] = {VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D32_SFLOAT, VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D16_UNORM_S8_UINT, VK_FORMAT_D16_UNORM};
        if (std::find(std::begin(depthFormats), std::end(depthFormats), imageFormat) != std::end(depthFormats)) {
            viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        } else {
            viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        }

        if (vkCreateImageView(bananDevice.device(), &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture image view!");
        }
    }

    void BananImage::createTextureSampler() {
        VkSamplerCreateInfo samplerInfo{};
        samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerInfo.magFilter = VK_FILTER_LINEAR;
        samplerInfo.minFilter = VK_FILTER_LINEAR;
        samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
        samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;

        samplerInfo.anisotropyEnable = VK_TRUE;
        samplerInfo.maxAnisotropy = bananDevice.physicalDeviceProperties().limits.maxSamplerAnisotropy;

        samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        samplerInfo.unnormalizedCoordinates = VK_FALSE;
        samplerInfo.compareEnable = VK_FALSE;
        samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

        samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerInfo.mipLodBias = 0.0f;
        samplerInfo.minLod = 0.0f;
        samplerInfo.maxLod = static_cast<float>(mipLevels);

        if (vkCreateSampler(bananDevice.device(), &samplerInfo, nullptr, &imageSampler) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture sampler!");
        }
    }

    void BananImage::transitionLayout(VkCommandBuffer commandBuffer, VkImageLayout oldLayout, VkImageLayout newLayout) {
        bananDevice.transitionImageLayout(commandBuffer, image, imageFormat, oldLayout, newLayout, mipLevels, 1);
        imageLayout = newLayout;
    }

    void BananImage::generateMipMaps(uint32_t m) {
        bananDevice.generateMipMaps(image, imageExtent.width, imageExtent.height, m);
        mipLevels = m;
    }

    VkImage BananImage::getImageHandle() {
        return image;
    }

    VkImageView BananImage::getImageView() {
        return imageView;
    }

    VkSampler BananImage::getImageSampler() {
        return imageSampler;
    }

    VkExtent2D BananImage::getImageExtent() {
        return imageExtent;
    }

    VkFormat BananImage::getImageFormat() {
        return imageFormat;
    }

    VkImageLayout BananImage::getImageLayout() {
        return imageLayout;
    }

     std::shared_ptr<BananImage> BananImage::makeImageFromFilepath(BananDevice &device, const std::string &filepath) {
        size_t pos = filepath.rfind('.');
        std::string extension = filepath.substr(pos, filepath.length() - 1);

        assert(
                extension == ".exr" ||
                extension == ".jpg" ||
                extension == ".jpeg" ||
                extension == ".png" &&
                "File format not supported");

        uint32_t width;
        uint32_t height;
        uint32_t stride;
        uint32_t levels;
        void *data;

        if (extension == ".exr") {
            Imf::setGlobalThreadCount((int) std::thread::hardware_concurrency());
            Imf::Array2D<Imf::Rgba> pixelBuffer = Imf::Array2D<Imf::Rgba>();
            Imf::Array2D<Imf::Rgba> &pixelBufferRef = pixelBuffer;

            Imf::RgbaInputFile in(filepath.c_str());
            Imath::Box2i win = in.dataWindow();
            Imath::V2i dim(win.max.x - win.min.x + 1, win.max.y - win.min.y + 1);

            int dx = win.min.x;
            int dy = win.min.y;

            pixelBufferRef.resizeErase(dim.x, dim.y);

            in.setFrameBuffer(&pixelBufferRef[0][0] - dx - dy * dim.x, 1, dim.x);
            in.readPixels(win.min.y, win.max.y);

            data = malloc(dim.y * dim.y * 8);
            width = dim.x;
            height = dim.y;
            stride = 16;
            levels = static_cast<uint32_t>(std::floor(std::log2(std::max(width, width)))) + 1;

            std::vector<uint16_t> singleChannelPixelBuffer{};
            singleChannelPixelBuffer.reserve(dim.x * dim.y * 4);

            uint32_t index = 0;
            for (int y1 = 0; y1 < dim.y; y1++) {
                for (int x1 = 0; x1 < dim.x; x1++) {
                    singleChannelPixelBuffer[index++] = pixelBufferRef[y1][x1].r.bits();
                    singleChannelPixelBuffer[index++] = pixelBufferRef[y1][x1].g.bits();
                    singleChannelPixelBuffer[index++] = pixelBufferRef[y1][x1].b.bits();
                    singleChannelPixelBuffer[index++] = pixelBufferRef[y1][x1].a.bits();
                }
            }

            memcpy(data, singleChannelPixelBuffer.data(), dim.x * dim.y * 8);
        } else {
            data = stbi_load(filepath.c_str(), (int *) &width, (int *) &height, nullptr, STBI_rgb_alpha);
            levels = static_cast<uint32_t>(std::floor(std::log2(std::max(width, width)))) + 1;
            stride = 8;
        }

        assert(width != 0 && height != 0 && data != nullptr && "something went wrong when loading textures");

        uint32_t pixelCount = height * width;
        uint32_t pixelSize = stride / 2; // stride is in bits, pixel size should be in bytes

        VkFormat format = stride == 16 ? VK_FORMAT_R16G16B16A16_SFLOAT : VK_FORMAT_R8G8B8A8_SRGB;

        BananBuffer stagingBuffer{device, pixelSize, pixelCount, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};
        stagingBuffer.map();
        stagingBuffer.writeToBuffer((void *) data);

        free(data);

        VkCommandBuffer commandBuffer = device.beginSingleTimeCommands();

        auto bananImage = std::make_shared<BananImage>(device, width, height, levels, format, VK_IMAGE_TILING_OPTIMAL, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
        bananImage->transitionLayout(commandBuffer, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
        device.endSingleTimeCommands(commandBuffer);
        device.copyBufferToImage(stagingBuffer.getBuffer(), bananImage->getImageHandle(), width, height, 1);
        device.generateMipMaps(bananImage->getImageHandle(), width, height, levels);

        return bananImage;
    }

    BananImage::BananImage(BananDevice &device, VkFormat format) : bananDevice{device}, imageFormat{format} {

    }

    BananImageArray::BananImageArray(BananDevice &device, uint32_t width, uint32_t height, uint32_t arrayLevels, uint32_t mipLevels, VkFormat format, VkImageTiling tiling, VkSampleCountFlagBits numSamples, VkImageUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags) : BananImage(device, format), arrayLevels{arrayLevels} {
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = static_cast<uint32_t>(width);
        imageInfo.extent.height = static_cast<uint32_t>(height);
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = mipLevels;
        imageInfo.arrayLayers = arrayLevels;
        imageInfo.format = format;
        imageInfo.tiling = tiling;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = usageFlags;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.samples = numSamples;

        bananDevice.createImageWithInfo(imageInfo, memoryPropertyFlags, image, memory);
        imageExtent = {width, height};
        imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        this->mipLevels = mipLevels;

        createTextureSampler();
        createTextureImageView();
    }

    void BananImageArray::createTextureImageView() {
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = image;
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
        viewInfo.format = imageFormat;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = mipLevels;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = arrayLevels;

        VkFormat depthFormats[] = {VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D32_SFLOAT, VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D16_UNORM_S8_UINT, VK_FORMAT_D16_UNORM};
        if (std::find(std::begin(depthFormats), std::end(depthFormats), imageFormat) != std::end(depthFormats)) {
            viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        } else {
            viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        }

        if (vkCreateImageView(bananDevice.device(), &viewInfo, nullptr, &imageView) != VK_SUCCESS) {
            throw std::runtime_error("failed to create texture image view!");
        }
    }

    BananCubemap::BananCubemap(BananDevice &device, uint32_t sideLength, uint32_t mipLevels, VkFormat format, VkImageTiling tiling, VkSampleCountFlagBits numSamples, VkBufferUsageFlags usageFlags, VkMemoryPropertyFlags memoryPropertyFlags) : bananDevice{device}, cubemapImageFormat{format}, mipLevels{mipLevels} {
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = static_cast<uint32_t>(sideLength);
        imageInfo.extent.height = static_cast<uint32_t>(sideLength);
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = mipLevels;
        imageInfo.arrayLayers = 6;
        imageInfo.format = cubemapImageFormat;
        imageInfo.tiling = tiling;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = usageFlags;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.samples = numSamples;
        imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

        bananDevice.createImageWithInfo(imageInfo, memoryPropertyFlags, cubemapImage, memory);

        createTextureSampler();
        createTextureCubemapImageView();
    }

    BananCubemap::~BananCubemap() {
        vkDestroySampler(bananDevice.device(), cubemapImageSampler, nullptr);
        vkDestroyImageView(bananDevice.device(), cubemapImageView, nullptr);
        vkDestroyImage(bananDevice.device(), cubemapImage, nullptr);
        vkFreeMemory(bananDevice.device(), memory, nullptr);
    }

    void BananCubemap::createTextureCubemapImageView() {
        VkImageViewCreateInfo imageViewCreateInfo{};
        imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
        imageViewCreateInfo.format = cubemapImageFormat;
        imageViewCreateInfo.components = { VK_COMPONENT_SWIZZLE_R, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY };
        imageViewCreateInfo.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, mipLevels, 0, 6};
        imageViewCreateInfo.image = cubemapImage;

        VkFormat depthFormats[] = {VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D32_SFLOAT, VK_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D16_UNORM_S8_UINT, VK_FORMAT_D16_UNORM};
        if (std::find(std::begin(depthFormats), std::end(depthFormats), cubemapImageFormat) != std::end(depthFormats)) {
            imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        } else {
            imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        }

        if (vkCreateImageView(bananDevice.device(), &imageViewCreateInfo, nullptr, &cubemapImageView) != VK_SUCCESS) {
            throw std::runtime_error("failed to create cubemap image view");
        }
    }

    void BananCubemap::createTextureSampler() {
        VkSamplerCreateInfo samplerCreateInfo{};
        samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        samplerCreateInfo.magFilter = VK_FILTER_LINEAR;
        samplerCreateInfo.minFilter = VK_FILTER_LINEAR;
        samplerCreateInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
        samplerCreateInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        samplerCreateInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        samplerCreateInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
        samplerCreateInfo.mipLodBias = 0.0f;
        samplerCreateInfo.maxAnisotropy = 1.0f;
        samplerCreateInfo.compareOp = VK_COMPARE_OP_NEVER;
        samplerCreateInfo.minLod = 0.0f;
        samplerCreateInfo.maxLod = 1.0f;
        samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

        if (vkCreateSampler(bananDevice.device(), &samplerCreateInfo, nullptr, &cubemapImageSampler) != VK_SUCCESS) {
            throw std::runtime_error("failed to create cubemap sampler");
        }
    }

    VkDescriptorImageInfo BananCubemap::cubemapDescriptorInfo() {
        VkDescriptorImageInfo info{};
        info.imageView = cubemapImageView;
        info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        info.sampler = cubemapImageSampler;
        return info;
    }

    VkImage BananCubemap::getImageHandle() {
        return cubemapImage;
    }
}
