//
// Created by Harrand on 25/01/2019.
//

#define USE_D3D 0
#include "MSOC/MaskedOcclusionCulling.h"
#include "msoc.hpp"

MSOC::MSOC(): msoc(MaskedOcclusionCulling::Create()), occluders({})
{
    // temporarily hardcode resolution to 1920x1080
    this->msoc->SetResolution(1920, 1080);
    this->msoc->SetNearClipPlane(tz::graphics::default_near_clip);
    this->clear();
}

MSOC::~MSOC()
{
    MaskedOcclusionCulling::Destroy(this->msoc);
}

const std::vector<const StaticObject*>& MSOC::get_occluders() const
{
    return this->occluders;
}

bool MSOC::register_occluder(const StaticObject& occluder, const Camera& camera, const Vector2I viewport_dimensions)
{
    this->occluders.push_back(&occluder);
    std::cout << "retrieving vertex data...";
    auto pair = this->get_vertex_data(occluder, camera, viewport_dimensions);
    std::cout << "done.\n";
    std::cout << "num positions = " << pair.first.size() << ", num indices = " << pair.second.size() << "\n";
    std::cout << "adding triangles...";
    std::cout << "position data = :\n";
    for(float dat : pair.first)
        std::cout << dat << ", ";
    MaskedOcclusionCulling::CullingResult res = this->msoc->RenderTriangles(pair.first.data(), pair.second.data(), pair.second.size() / 3, nullptr, MaskedOcclusionCulling::BACKFACE_CCW);
    std::cout << "done.\n";
    MaskedOcclusionCulling::OcclusionCullingStatistics stats = this->msoc->GetStatistics();
    std::cout << "number of occluder triangles processed = " << stats.mOccluders.mNumProcessedTriangles << "\n";
    std::cout << "number of occluder triangles rasterised = " << stats.mOccluders.mNumRasterizedTriangles << "\n";
    std::cout << "number of occludee triangles processed = " << stats.mOccludees.mNumProcessedTriangles << "\n";
    std::cout << "number of occludee triangles rasterised = " << stats.mOccludees.mNumRasterizedTriangles << "\n";
    std::cout << "number of occluders (topaz) = " << this->occluders.size() << "\n";
    switch(res)
    {
        case MaskedOcclusionCulling::CullingResult::VISIBLE:
            return true;
        default: // happens if view culled or occluded.
            return false;
    }
}

bool MSOC::is_visible(const StaticObject& object, const Camera& camera, const Vector2I viewport_dimensions, bool require_occluders)
{
    if(!require_occluders)
        return this->register_occluder(object, camera, viewport_dimensions);
    // If we require occluders, do it properly.
    std::cout << "retrieving vertex data...";
    auto pair = this->get_vertex_data(object, camera, viewport_dimensions);
    std::cout << "done.\ntesting triangles...";
    MaskedOcclusionCulling::CullingResult res = this->msoc->TestTriangles(pair.first.data(), pair.second.data(), pair.second.size());
    std::cout << "done.\n";

    MaskedOcclusionCulling::OcclusionCullingStatistics stats = this->msoc->GetStatistics();
    std::cout << "number of occluder triangles processed = " << stats.mOccluders.mNumProcessedTriangles << "\n";
    std::cout << "number of occluder triangles rasterised = " << stats.mOccluders.mNumRasterizedTriangles << "\n";
    std::cout << "number of occludee triangles processed = " << stats.mOccludees.mNumProcessedTriangles << "\n";
    std::cout << "number of occludee triangles rasterised = " << stats.mOccludees.mNumRasterizedTriangles << "\n";

    switch(res)
    {
        case MaskedOcclusionCulling::CullingResult::VISIBLE:
            return true;
        default:
            return false;
    }
}

Bitmap<PixelDepth> MSOC::get_hierarchical_depth_buffer() const
{
    Bitmap<PixelDepth> result;
    result.width = 1920;
    result.height = 1080;
    result.pixels.resize(1920 * 1080, PixelDepth{1});
    std::vector<float> data;
    for(PixelDepth pixel : result.pixels)
        data.push_back(pixel.data.data()[0]);
    this->msoc->ComputePixelDepthBuffer(data.data(), true);
    return result;
}

void MSOC::clear()
{
    this->occluders.clear();
    this->msoc->ClearBuffer();
}

MaskedOcclusionCulling::VertexLayout MSOC::get_layout()
{
    return {};
}

std::pair<std::vector<float>, std::vector<unsigned int>> MSOC::get_vertex_data(const StaticObject& occluder, const Camera& camera, const Vector2I viewport_dimensions)
{
    if(!occluder.get_asset().valid_mesh())
        return {{}, {}};
    //this->msoc->RenderTriangles();
    // MaskedOcclusionCulling::TransformVertices could be used, but will simply do it manually here.
    Matrix4x4 mvp = camera.projection(viewport_dimensions.x, viewport_dimensions.y) * camera.view() * occluder.transform.model();
    std::vector<Vector3F> positions = occluder.get_asset().mesh->get_positions();
    std::vector<float> position_data;
    std::vector<unsigned int> indices = occluder.get_asset().mesh->get_indices();
    // positions is initially in model-space. need to transform before sending it to MSOC.
    for(const Vector3F& position_modelspace : positions)
    {
        std::array<float, 3> posdata = (mvp * Vector4F{position_modelspace, 1.0f}).xyz().data();
        position_data.push_back(posdata[0]);
        position_data.push_back(posdata[1]);
        position_data.push_back(posdata[2]);
    }
    return {position_data, indices};
}