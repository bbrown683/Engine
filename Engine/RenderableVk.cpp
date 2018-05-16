/*
MIT License

Copyright (c) 2018 Ben Brown

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "RenderableVk.hpp"
#include "DriverVk.hpp"

RenderableVk::RenderableVk(DriverVk* pDriver) : m_pDriver(pDriver) {}

bool RenderableVk::attachShader(const char* filename, ShaderStage stage) {

    // Uses the C API for reading files.
    // MSVC wants use to use the secure alternative.
    FILE* file;
    file = fopen(filename, "rb");

    if (!file)
        return false;
    // Get the size of the file so we can preallocate 
    // the correct amount for the vector.
    long pos = ftell(file);
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, pos, SEEK_SET);

    std::vector<char> buffer(size + 1);
    fread(buffer.data(), size, 1, file);
    buffer[size] = 0;
    fclose(file);

    vk::ShaderModuleCreateInfo moduleInfo;
    moduleInfo.codeSize = size;
    moduleInfo.pCode = reinterpret_cast<const uint32_t*>(buffer.data());
    auto moduleResult = m_pDriver->getDevice()->createShaderModuleUnique(moduleInfo);
    if (moduleResult.result != vk::Result::eSuccess)
        return false;

    vk::PipelineShaderStageCreateInfo shaderStageInfo;
//    shaderStageInfo.module = moduleResult.value.get(); // Will cause crash...
    shaderStageInfo.pName = "main";
    switch (stage) {
    case ShaderStage::Fragment: shaderStageInfo.stage = vk::ShaderStageFlagBits::eFragment; break;
    case ShaderStage::Geometry: shaderStageInfo.stage = vk::ShaderStageFlagBits::eGeometry; break;
    case ShaderStage::TesselationControl: shaderStageInfo.stage = vk::ShaderStageFlagBits::eTessellationControl; break;
    case ShaderStage::TesselationEvaluation: shaderStageInfo.stage = vk::ShaderStageFlagBits::eTessellationEvaluation; break;
    case ShaderStage::Vertex: shaderStageInfo.stage = vk::ShaderStageFlagBits::eVertex;
    }
    m_ShaderStages.push_back(shaderStageInfo);
    return false;
}

bool RenderableVk::execute() {
    m_pDriver->getPrimaryCommandBuffer()->executeCommands(m_pCommandBuffer.get());
    return true;
}

bool RenderableVk::setIndexBuffer(std::vector<uint16_t> indices) {
    return false;
}

bool RenderableVk::setVertexBuffer(std::vector<uint32_t> vertices) {
    return false;
}
