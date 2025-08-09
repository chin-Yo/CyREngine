//#include "SubSystems/GLSLCompiler.hpp"
//
//void GlslCompiler::Initialize()
//{
//    glslang::InitializeProcess();
//}
//
//std::vector<unsigned int> GlslCompiler::CompileGLSLToSpv(const std::string &glslCode, EShLanguage shaderType, std::string &errorLog)
//{
//    // 1. 创建着色器对象
//    glslang::TShader shader(shaderType);
//
//    // 2. 设置着色器源码
//    const char *shaderStrings[1];
//    shaderStrings[0] = glslCode.c_str();
//    shader.setStrings(shaderStrings, 1);
//
//    // 3. 设置编译环境
//    // 指定输入语言为 GLSL，目标客户端为 Vulkan 1.3，目标 SPIR-V 版本为 1.3
//    // 你可以根据你的需求调整这些版本号
//    int clientInputSemanticsVersion = 130; // Vulkan 1.1
//    glslang::EShTargetClientVersion vulkanClientVersion = glslang::EShTargetVulkan_1_3;
//    glslang::EShTargetLanguageVersion targetVersion = glslang::EShTargetSpv_1_3;
//
//    shader.setEnvInput(glslang::EShSourceGlsl, shaderType, glslang::EShClientVulkan, clientInputSemanticsVersion);
//    shader.setEnvClient(glslang::EShClientVulkan, vulkanClientVersion);
//    shader.setEnvTarget(glslang::EShTargetSpv, targetVersion);
//
//    // 4. 设置编译选项/消息
//    // EShMsgSpvRules: 启用 SPIR-V 的特定规则
//    // EShMsgVulkanRules: 启用 Vulkan 的特定规则
//    EShMessages messages = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules);
//
//    // 5. 解析 GLSL 代码
//    const TBuiltInResource *resources = GetDefaultResources();
//    if (!shader.parse(resources, 110, false, messages))
//    {
//        errorLog = "GLSL Parsing Failed:\n";
//        errorLog += shader.getInfoLog();
//        errorLog += "\n";
//        errorLog += shader.getInfoDebugLog();
//        return {}; // 返回空 vector 表示失败
//    }
//
//    // 6. 将着色器链接到 Program
//    glslang::TProgram program;
//    program.addShader(&shader);
//
//    if (!program.link(messages))
//    {
//        errorLog = "GLSL Linking Failed:\n";
//        errorLog += program.getInfoLog();
//        errorLog += "\n";
//        errorLog += program.getInfoDebugLog();
//        return {}; // 返回空 vector 表示失败
//    }
//
//    // 7. 从 Program 中间表示生成 SPIR-V
//    std::vector<unsigned int> spv;
//    glslang::SpvOptions spvOptions;
//    spvOptions.validate = true; // 开启验证，确保生成的 SPIR-V 有效
//    glslang::GlslangToSpv(*program.getIntermediate(shaderType), spv, &spvOptions);
//
//    // 8. 成功，返回 SPIR-V
//    errorLog = "Compilation successful.";
//    return spv;
//}