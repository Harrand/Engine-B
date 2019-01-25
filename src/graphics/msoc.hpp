//
// Created by Harrand on 25/01/2019.
//

#ifndef ENGINE_B_MSOC_HPP
#define ENGINE_B_MSOC_HPP
#include "MSOC/MaskedOcclusionCulling.h"
#include "graphics/static_object.hpp"

class MSOC
{
public:
    MSOC();
    ~MSOC();
    const std::vector<const StaticObject*>& get_occluders() const;
    bool register_occluder(const StaticObject& occluder, const Camera& camera, const Vector2I viewport_dimensions);
    bool is_visible(const StaticObject& object, const Camera& camera, const Vector2I viewport_dimensions, bool require_occluders = true);
    Bitmap<PixelDepth> get_hierarchical_depth_buffer() const;
    void clear();
private:
    MaskedOcclusionCulling::VertexLayout get_layout();
    std::pair<std::vector<float>, std::vector<unsigned int>> get_vertex_data(const StaticObject& occluder, const Camera& camera, const Vector2I viewport_dimensions);
    MaskedOcclusionCulling* msoc;
    std::vector<const StaticObject*> occluders;
};


#endif //ENGINE_B_MSOC_HPP
