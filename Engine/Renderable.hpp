#pragma once

enum class ShaderStage {
    Fragement,
    Geometry,
    TesselationEvaluation,
    TesselationControl,
    Vertex,
};

/// This object contains most of the render-state associated with a single object.
/// This can include shaders, vertex buffers, index buffers, etc.
class Renderable {
public:
    virtual ~Renderable() {}
    virtual bool attachShader(const char* filename, ShaderStage stage) = 0;
    virtual bool setIndexBuffer(std::vector<uint16_t> indices) = 0;
    virtual bool setVertexBuffer(std::vector<uint32_t> vertices) = 0;
};