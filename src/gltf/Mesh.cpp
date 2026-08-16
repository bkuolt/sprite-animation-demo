// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian Kuolt. All rights reserved.

#include "Mesh.hpp"
#include <utility>

namespace bgl::gfx
{
Mesh::Mesh(std::string name)
    : _name(std::move(name))
{
}

Mesh::~Mesh()
{
    for (auto &prim : _primitives)
    {
        if (prim.vao)
        {
            glDeleteVertexArrays(1, &prim.vao);
        }
        if (prim.vbo)
        {
            glDeleteBuffers(1, &prim.vbo);
        }
        if (prim.ebo)
        {
            glDeleteBuffers(1, &prim.ebo);
        }
    }
}

Mesh::Mesh(Mesh &&other) noexcept
    : _name(std::move(other._name)),
      _primitives(std::move(other._primitives))
{
    other._primitives.clear();
}

Mesh &Mesh::operator=(Mesh &&other) noexcept
{
    if (this != &other)
    {
        for (auto &prim : _primitives)
        {
            if (prim.vao) glDeleteVertexArrays(1, &prim.vao);
            if (prim.vbo) glDeleteBuffers(1, &prim.vbo);
            if (prim.ebo) glDeleteBuffers(1, &prim.ebo);
        }
        _name = std::move(other._name);
        _primitives = std::move(other._primitives);
        other._primitives.clear();
    }
    return *this;
}

void Mesh::addPrimitive(Primitive primitive)
{
    _primitives.push_back(primitive);
}

const std::vector<Primitive> &Mesh::getPrimitives() const noexcept
{
    return _primitives;
}

const std::string &Mesh::getName() const noexcept
{
    return _name;
}
} // namespace bgl::gfx
