// This file is part of Surface Splatting.
//
// Copyright (C) 2010, 2015 by Sebastian Lipponer.
// 
// Surface Splatting is free software: you can redistribute it and / or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// Surface Splatting is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with Surface Splatting. If not, see <http://www.gnu.org/licenses/>.

#include "splat_renderer.hpp"
#include "omicron/renderer/ogl_utils.h"

#include <iostream>
#include <cmath>
#include "omicron/basic/array.h"
#include "omicron/hierarchy/reconstruction_params.h"

using namespace Eigen;

UniformBufferRaycast::UniformBufferRaycast()
    : glUniformBuffer(sizeof(Matrix4f) + sizeof(Vector4f))
{
}

void
UniformBufferRaycast::set_buffer_data(Matrix4f const&
    projection_matrix_inv, GLint const* viewport)
{
    float viewportf[4] = {
        static_cast<float>(viewport[0]),
        static_cast<float>(viewport[1]),
        static_cast<float>(viewport[2]),
        static_cast<float>(viewport[3])
    };

    bind();
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Matrix4f),
        projection_matrix_inv.data());
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(Matrix4f),
        4 * sizeof(float), viewportf);
    unbind();
}

UniformBufferFrustum::UniformBufferFrustum()
    : glUniformBuffer(6 * sizeof(Vector4f))
{
}

void
UniformBufferFrustum::set_buffer_data(Vector4f const* frustum_plane)
{
    bind();
    glBufferSubData(GL_UNIFORM_BUFFER, 0, 6 * sizeof(Vector4f),
        static_cast<void const*>(frustum_plane));
    unbind();
}

UniformBufferParameter::UniformBufferParameter()
    : glUniformBuffer(8 * sizeof(float))
{
}

void
UniformBufferParameter::set_buffer_data(Vector3f const& color, float shininess,
    float radius_scale, float ewa_radius, float epsilon)
{
    bind();
    glBufferSubData(GL_UNIFORM_BUFFER, 0, 3 * sizeof(float), color.data());
    glBufferSubData(GL_UNIFORM_BUFFER, 12, sizeof(float), &shininess);
    glBufferSubData(GL_UNIFORM_BUFFER, 16, sizeof(float), &radius_scale);
    glBufferSubData(GL_UNIFORM_BUFFER, 20, sizeof(float), &ewa_radius);
    glBufferSubData(GL_UNIFORM_BUFFER, 24, sizeof(float), &epsilon);
    unbind();
}
