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

#version 330

#define VISIBILITY_PASS    0
#define BACKFACE_CULLING   0
#define SMOOTH             0
#define EWA_FILTER         0
#define POINTSIZE_METHOD   0

layout(std140, column_major) uniform Camera
{
    mat4 modelview_matrix;
    mat4 projection_matrix;
};

layout(std140, column_major) uniform Raycast
{
    mat4 projection_matrix_inv;
    vec4 viewport;
};

layout(std140) uniform Frustum
{
    vec4 frustum_plane[6];
};

layout(std140) uniform Parameter
{
    vec3 material_color;
    float material_shininess;
    float radius_scale;
    float ewa_radius;
    float epsilon;
};

#define ATTR_CENTER 0
layout(location = ATTR_CENTER) in vec3 c;

#define ATTR_T1 1
layout(location = ATTR_T1) in vec3 u;

#define ATTR_T2 2
layout(location = ATTR_T2) in vec3 v;

out block
{
    flat out vec3 color;
}
Out;

vec3 lighting(vec3 n_eye, vec3 v_eye, vec3 color, float shininess);

void main()
{
    vec4 c_eye = modelview_matrix * vec4(c, 1.0);
    vec3 u_eye = radius_scale * mat3(modelview_matrix) * u;
    vec3 v_eye = radius_scale * mat3(modelview_matrix) * v;
    vec3 n_eye = normalize(cross(u_eye, v_eye));

    Out.color = lighting(n_eye, vec3(c_eye), material_color, material_shininess);
    gl_Position = projection_matrix * c_eye;
    gl_PointSize = 1.0;
}
