//
// Created by Harrand on 25/01/2019.
//

#define USE_D3D 0
#define ENABLE_STATS 1

#include <MSOC/MaskedOcclusionCulling.h>
#include "msoc.hpp"

MSOC::MSOC(): msoc(MaskedOcclusionCulling::Create()), occluders({})
{
    // temporarily hardcode resolution to 1920x1080
    this->msoc->SetResolution(1920, 1080);
    this->msoc->SetNearClipPlane(tz::graphics::default_near_clip);
    this->msoc->ClearBuffer();
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
    if(std::find(this->occluders.begin(), this->occluders.end(), &occluder) == this->occluders.end())
        this->occluders.push_back(&occluder);
    auto pair = this->get_vertex_data(occluder, camera, viewport_dimensions, true);
    //MaskedOcclusionCulling::CullingResult  res = this->msoc->RenderTriangles(pair.first.data(), pair.second.data(), pair.second.size() / 3, this->get_mvp_matrix(occluder, camera, viewport_dimensions).fill_data().data());
    // verts look like: {x, y, w, x, y, w...}
    MaskedOcclusionCulling::CullingResult res = this->msoc->RenderTriangles(pair.first.data(), pair.second.data(), pair.second.size() / 3, nullptr, MaskedOcclusionCulling::BACKFACE_CW, MaskedOcclusionCulling::CLIP_PLANE_ALL);
    /*
    MaskedOcclusionCulling::OcclusionCullingStatistics stats = this->msoc->GetStatistics();
    std::cout << "=-= Occluders =-=\n";
    std::cout << "rasterized triangles = " << stats.mOccluders.mNumRasterizedTriangles << "\n";
    std::cout << "processed triangles = " << stats.mOccluders.mNumProcessedTriangles << "\n";
    std::cout << "tiles merged = " << stats.mOccluders.mNumTilesMerged << "\n";
    std::cout << "tiles updated = " << stats.mOccluders.mNumTilesUpdated << "\n";
    std::cout << "tiles traversed = " << stats.mOccluders.mNumTilesTraversed << "\n";
     */
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
    if(!require_occluders && std::find(this->occluders.begin(), this->occluders.end(), &object) == this->occluders.end())
        this->register_occluder(object, camera, viewport_dimensions);
    // If we require occluders, do it properly.
    auto pair = this->get_vertex_data(object, camera, viewport_dimensions, true);
    MaskedOcclusionCulling::CullingResult res = this->msoc->TestTriangles(pair.first.data(), pair.second.data(), pair.second.size() / 3, nullptr, MaskedOcclusionCulling::BACKFACE_CW, MaskedOcclusionCulling::CLIP_PLANE_ALL);
    /*
    MaskedOcclusionCulling::OcclusionCullingStatistics stats = this->msoc->GetStatistics();
    std::cout << "=-= Occludees =-=\n";
    std::cout << "rasterized triangles = " << stats.mOccludees.mNumRasterizedTriangles << "\n";
    std::cout << "processed triangles = " << stats.mOccludees.mNumProcessedTriangles << "\n";
    std::cout << "processed rectangles = " << stats.mOccludees.mNumProcessedRectangles << "\n";
    std::cout << "tiles traversed = " << stats.mOccludees.mNumTilesTraversed << "\n";
     */
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
    unsigned int w, h;
    this->msoc->GetResolution(w, h);
    result.width = w;
    result.height = h;
    result.pixels.resize(w * h, PixelDepth{1.0f});
    std::vector<float> data;
    for(PixelDepth pixel : result.pixels)
        data.push_back(pixel.data.data()[0]);
    this->msoc->ComputePixelDepthBuffer(data.data(), true);
    for(std::size_t i = 0; i < result.pixels.size(); i++)
        result.pixels[i].data.underlying_data[0] = data[i];
    return result;
}

void MSOC::clear(const Camera& camera, const Vector2I viewport_dimensions)
{
    this->msoc->ClearBuffer();
    this->render_all_occluders(camera, viewport_dimensions);
}

void MSOC::render_all_occluders(const Camera& camera, const Vector2I viewport_dimensions)
{
    for(const StaticObject* occluder : this->occluders)
        this->register_occluder(*occluder, camera, viewport_dimensions);
}

MaskedOcclusionCulling::VertexLayout MSOC::get_layout()
{
    return {};
}

Matrix4x4 MSOC::get_mvp_matrix(const StaticObject& occluder, const Camera& camera, const Vector2I viewport_dimensions)
{
    return camera.projection(viewport_dimensions.x, viewport_dimensions.y).transposed() * camera.view().transposed() * occluder.transform.model().transposed();
}

std::pair<std::vector<float>, std::vector<unsigned int>> MSOC::get_vertex_data(const StaticObject& occluder, const Camera& camera, const Vector2I viewport_dimensions, bool transform)
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
        Vector4F pos_transformed = transform ? (mvp * Vector4F{position_modelspace, 1.0f}) : Vector4F{position_modelspace, 1.0f};
        std::array<float, 4> posdata = pos_transformed.data();
        position_data.push_back(posdata[0]);
        position_data.push_back(posdata[1]);
        position_data.push_back(posdata[2]);
        position_data.push_back(posdata[3]);
    }
    return {position_data, indices};
}