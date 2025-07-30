#include "Misc/FileLoader.hpp"
#include <fstream>
#include <filesystem>

std::vector<uint32_t> FileLoader::ReadShaderBinaryU32(const std::string &filename)
{
    // 调用ReadFileBinary获取原始二进制数据
    std::vector<uint8_t> binaryData = ReadFileBinary(filename);

    // 检查数据大小是否是4的倍数(因为我们要转换为uint32_t)
    if (binaryData.size() % sizeof(uint32_t) != 0)
    {
        throw std::runtime_error("Shader binary file size is not a multiple of 4 bytes");
    }

    // 将uint8_t数据转换为uint32_t
    std::vector<uint32_t> result;
    result.resize(binaryData.size() / sizeof(uint32_t));
    std::memcpy(result.data(), binaryData.data(), binaryData.size());

    return result;
}

std::vector<uint8_t> FileLoader::ReadFileBinary(const std::filesystem::path &path)
{
    // 打开文件
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open())
    {
        throw std::runtime_error("Failed to open file: " + path.string());
    }

    // 获取文件大小
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    // 读取文件内容
    std::vector<uint8_t> buffer(size);
    if (!file.read(reinterpret_cast<char *>(buffer.data()), size))
    {
        throw std::runtime_error("Failed to read file: " + path.string());
    }

    return buffer;
}

std::string FileLoader::ReadFileString(const std::filesystem::path &path)
{
    // 调用ReadFileBinary获取二进制数据
    std::vector<uint8_t> binaryData = ReadFileBinary(path);

    // 将二进制数据转换为字符串
    return std::string(reinterpret_cast<const char *>(binaryData.data()), binaryData.size());
}

std::string FileLoader::ReadTextFile(const std::string &filename)
{
    // 调用FileLoader::ReadFileString
    return FileLoader::ReadFileString(filename);
}