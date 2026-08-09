// SPDX-License-Identifier: MIT
// Copyright (c) 2024-2026 Bastian Kuolt. All rights reserved.

#include "GltfLoader.hpp"
#include "io/TextureLoader.hpp"
#include <fastgltf/core.hpp>
#include <fastgltf/types.hpp>
#include <fastgltf/tools.hpp>
#include <spdlog/spdlog.h>
#include <glm/gtc/type_ptr.hpp>
#include <stdexcept>
#include <unordered_map>

namespace bgl::io
{
struct Vertex
{
    glm::vec3 position{0.0f};
    glm::vec3 normal{0.0f, 1.0f, 0.0f};
    glm::vec2 texcoord{0.0f};
};

std::shared_ptr<bgl::gfx::Scene> GltfLoader::loadFromFile(const std::filesystem::path &path)
{
    if (!std::filesystem::exists(path))
    {
        spdlog::error("glTF file does not exist: {}", path.string());
        throw std::runtime_error("glTF file does not exist: " + path.string());
    }

    fastgltf::Parser parser(fastgltf::Extensions::KHR_texture_basisu);

    auto dataBufferResult = fastgltf::GltfDataBuffer::FromPath(path);
    if (dataBufferResult.error() != fastgltf::Error::None)
    {
        spdlog::error("Failed to load glTF data buffer from {}", path.string());
        throw std::runtime_error("Failed to load glTF file data buffer");
    }

    constexpr auto options = fastgltf::Options::LoadExternalBuffers;

    auto assetResult = parser.loadGltf(dataBufferResult.get(), path.parent_path(), options);
    if (assetResult.error() != fastgltf::Error::None)
    {
        spdlog::error("fastgltf error loading {}: {}", path.string(), static_cast<int>(assetResult.error()));
        throw std::runtime_error("Failed to parse glTF asset");
    }

    auto &asset = assetResult.get();
    auto bglScene = std::make_shared<bgl::gfx::Scene>(path.filename().string());

    // 1. Process textures via existing BGL loaders
    std::unordered_map<std::size_t, std::shared_ptr<bgl::gfx::Texture2DArray>> loadedTextures;

    auto getOrLoadTexture = [&](std::size_t imageIndex) -> GLuint {
        if (loadedTextures.contains(imageIndex))
        {
            return loadedTextures[imageIndex] ? loadedTextures[imageIndex]->getHandle() : 0;
        }

        if (imageIndex >= asset.images.size()) return 0;
        const auto &image = asset.images[imageIndex];

        std::unique_ptr<bgl::gfx::Texture2DArray> texArray;

        if (auto *uri = std::get_if<fastgltf::sources::URI>(&image.data))
        {
            auto texPath = path.parent_path() / uri->uri.path();
            texArray = bgl::io::loadTexture(texPath);
        }
        else if (auto *array = std::get_if<fastgltf::sources::Array>(&image.data))
        {
            std::span<const std::byte> memorySpan(
                reinterpret_cast<const std::byte *>(array->bytes.data()),
                array->bytes.size());
            std::string extHint = (array->mimeType == fastgltf::MimeType::JPEG) ? ".jpg" : ".png";
            texArray = bgl::io::loadTexture(memorySpan, extHint);
        }
        else if (auto *vector = std::get_if<fastgltf::sources::Vector>(&image.data))
        {
            std::span<const std::byte> memorySpan(
                reinterpret_cast<const std::byte *>(vector->bytes.data()),
                vector->bytes.size());
            std::string extHint = (vector->mimeType == fastgltf::MimeType::JPEG) ? ".jpg" : ".png";
            texArray = bgl::io::loadTexture(memorySpan, extHint);
        }
        else if (auto *bv = std::get_if<fastgltf::sources::BufferView>(&image.data))
        {
            const auto &bufferView = asset.bufferViews[bv->bufferViewIndex];
            const auto &buffer = asset.buffers[bufferView.bufferIndex];

            std::span<const std::byte> memorySpan;
            if (auto *bufArray = std::get_if<fastgltf::sources::Array>(&buffer.data))
            {
                memorySpan = std::span<const std::byte>(
                    reinterpret_cast<const std::byte *>(bufArray->bytes.data() + bufferView.byteOffset),
                    bufferView.byteLength);
            }
            else if (auto *bufVec = std::get_if<fastgltf::sources::Vector>(&buffer.data))
            {
                memorySpan = std::span<const std::byte>(
                    reinterpret_cast<const std::byte *>(bufVec->bytes.data() + bufferView.byteOffset),
                    bufferView.byteLength);
            }

            if (!memorySpan.empty())
            {
                std::string extHint = (bv->mimeType == fastgltf::MimeType::JPEG) ? ".jpg" : ".png";
                texArray = bgl::io::loadTexture(memorySpan, extHint);
            }
        }

        if (texArray)
        {
            GLuint handle = texArray->getHandle();
            loadedTextures[imageIndex] = std::move(texArray);
            return handle;
        }
        return 0;
    };

    // 2. Convert Meshes using OpenGL 4.6 DSA
    std::vector<std::shared_ptr<bgl::gfx::Mesh>> bglMeshes;
    bglMeshes.reserve(asset.meshes.size());

    for (const auto &gltfMesh : asset.meshes)
    {
        auto bglMesh = std::make_shared<bgl::gfx::Mesh>(std::string(gltfMesh.name));

        for (const auto &prim : gltfMesh.primitives)
        {
            bgl::gfx::Primitive bglPrim{};
            bglPrim.mode = GL_TRIANGLES;

            // Find attributes
            auto posIt = prim.findAttribute("POSITION");
            if (posIt == prim.attributes.end()) continue;

            const auto &posAccessor = asset.accessors[posIt->accessorIndex];
            std::size_t vertexCount = posAccessor.count;
            std::vector<Vertex> vertices(vertexCount);

            // Copy positions
            std::vector<fastgltf::math::fvec3> positions(vertexCount);
            fastgltf::copyFromAccessor<fastgltf::math::fvec3>(asset, posAccessor, positions.data());
            for (std::size_t i = 0; i < vertexCount; ++i)
            {
                vertices[i].position = glm::vec3(positions[i][0], positions[i][1], positions[i][2]);
            }

            // Copy Normals if present
            auto normIt = prim.findAttribute("NORMAL");
            if (normIt != prim.attributes.end())
            {
                std::vector<fastgltf::math::fvec3> normals(vertexCount);
                fastgltf::copyFromAccessor<fastgltf::math::fvec3>(asset, asset.accessors[normIt->accessorIndex], normals.data());
                for (std::size_t i = 0; i < vertexCount; ++i)
                {
                    vertices[i].normal = glm::vec3(normals[i][0], normals[i][1], normals[i][2]);
                }
            }

            // Copy TexCoords if present
            auto texIt = prim.findAttribute("TEXCOORD_0");
            if (texIt != prim.attributes.end())
            {
                std::vector<fastgltf::math::fvec2> texcoords(vertexCount);
                fastgltf::copyFromAccessor<fastgltf::math::fvec2>(asset, asset.accessors[texIt->accessorIndex], texcoords.data());
                for (std::size_t i = 0; i < vertexCount; ++i)
                {
                    vertices[i].texcoord = glm::vec2(texcoords[i][0], texcoords[i][1]);
                }
            }

            // Indices
            std::vector<uint32_t> indices;
            if (prim.indicesAccessor.has_value())
            {
                const auto &idxAccessor = asset.accessors[prim.indicesAccessor.value()];
                indices.resize(idxAccessor.count);
                fastgltf::copyFromAccessor<uint32_t>(asset, idxAccessor, indices.data());
                bglPrim.count = static_cast<GLsizei>(indices.size());
                bglPrim.indexType = GL_UNSIGNED_INT;
            }
            else
            {
                bglPrim.count = static_cast<GLsizei>(vertexCount);
            }

            // OpenGL 4.6 DSA Buffer Creation
            glCreateVertexArrays(1, &bglPrim.vao);
            glCreateBuffers(1, &bglPrim.vbo);

            glNamedBufferStorage(bglPrim.vbo, vertices.size() * sizeof(Vertex), vertices.data(), 0);
            glVertexArrayVertexBuffer(bglPrim.vao, 0, bglPrim.vbo, 0, sizeof(Vertex));

            glEnableVertexArrayAttrib(bglPrim.vao, 0);
            glVertexArrayAttribFormat(bglPrim.vao, 0, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, position));
            glVertexArrayAttribBinding(bglPrim.vao, 0, 0);

            glEnableVertexArrayAttrib(bglPrim.vao, 1);
            glVertexArrayAttribFormat(bglPrim.vao, 1, 3, GL_FLOAT, GL_FALSE, offsetof(Vertex, normal));
            glVertexArrayAttribBinding(bglPrim.vao, 1, 0);

            glEnableVertexArrayAttrib(bglPrim.vao, 2);
            glVertexArrayAttribFormat(bglPrim.vao, 2, 2, GL_FLOAT, GL_FALSE, offsetof(Vertex, texcoord));
            glVertexArrayAttribBinding(bglPrim.vao, 2, 0);

            if (!indices.empty())
            {
                glCreateBuffers(1, &bglPrim.ebo);
                glNamedBufferStorage(bglPrim.ebo, indices.size() * sizeof(uint32_t), indices.data(), 0);
                glVertexArrayElementBuffer(bglPrim.vao, bglPrim.ebo);
            }

            // Material properties
            if (prim.materialIndex.has_value())
            {
                const auto &mat = asset.materials[prim.materialIndex.value()];
                bglPrim.baseColorFactor = glm::vec4(
                    mat.pbrData.baseColorFactor[0],
                    mat.pbrData.baseColorFactor[1],
                    mat.pbrData.baseColorFactor[2],
                    mat.pbrData.baseColorFactor[3]);
                bglPrim.metallicFactor = mat.pbrData.metallicFactor;
                bglPrim.roughnessFactor = mat.pbrData.roughnessFactor;

                if (mat.pbrData.baseColorTexture.has_value())
                {
                    const auto &tex = asset.textures[mat.pbrData.baseColorTexture->textureIndex];
                    if (tex.imageIndex.has_value())
                    {
                        bglPrim.baseColorTexture = getOrLoadTexture(tex.imageIndex.value());
                    }
                }
            }

            bglMesh->addPrimitive(bglPrim);
        }

        bglMeshes.push_back(bglMesh);
    }

    // 3. Build Nodes & Scene Graph
    std::vector<std::shared_ptr<bgl::gfx::Node>> bglNodes;
    bglNodes.reserve(asset.nodes.size());

    for (const auto &gltfNode : asset.nodes)
    {
        auto node = std::make_shared<bgl::gfx::Node>(std::string(gltfNode.name));

        if (auto *matrix = std::get_if<fastgltf::math::fmat4x4>(&gltfNode.transform))
        {
            glm::mat4 m = glm::make_mat4(matrix->data());
            node->setLocalMatrix(m);
        }
        else if (auto *trs = std::get_if<fastgltf::TRS>(&gltfNode.transform))
        {
            node->setTranslation(glm::vec3(trs->translation[0], trs->translation[1], trs->translation[2]));
            node->setRotation(glm::quat(trs->rotation[3], trs->rotation[0], trs->rotation[1], trs->rotation[2]));
            node->setScale(glm::vec3(trs->scale[0], trs->scale[1], trs->scale[2]));
        }

        if (gltfNode.meshIndex.has_value() && gltfNode.meshIndex.value() < bglMeshes.size())
        {
            node->addMesh(bglMeshes[gltfNode.meshIndex.value()]);
        }

        bglNodes.push_back(node);
    }

    // Connect node children hierarchy
    for (std::size_t i = 0; i < asset.nodes.size(); ++i)
    {
        for (auto childIdx : asset.nodes[i].children)
        {
            if (childIdx < bglNodes.size())
            {
                bglNodes[i]->addChild(bglNodes[childIdx]);
            }
        }
    }

    // Add root scene nodes
    if (!asset.scenes.empty())
    {
        const auto &scene = asset.scenes[asset.defaultScene.value_or(0)];
        for (auto rootNodeIdx : scene.nodeIndices)
        {
            if (rootNodeIdx < bglNodes.size())
            {
                bglScene->addRootNode(bglNodes[rootNodeIdx]);
            }
        }
    }

    bglScene->updateTransforms();
    spdlog::info("Successfully loaded glTF asset: {}", path.string());
    return bglScene;
}
} // namespace bgl::io
