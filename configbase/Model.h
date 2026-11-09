#pragma once
#include "Graficos.h"
#include <memory>
#include <string>
#include <vector>

class Model {
public:
    Model(Graficos& renderer, const std::string& file);
    void draw(Graficos& renderer, const glm::mat4& transform, glm::vec3 color) const;
    std::size_t triangles() const { return triangleCount; }
    std::size_t texturedMaterials() const { return textureCount; }
private:
    struct Material {
        glm::vec3 color = glm::vec3(1);
        GLuint texture = 0;
    };
    struct Part {
        Mesh mesh;
        unsigned material = 0;
    };
    // Una parte por malla conserva el material que le asignamos en Blender.
    std::vector<Material> materials;
    std::vector<std::unique_ptr<Part>> parts;
    std::size_t triangleCount = 0, textureCount = 0;
};


#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <algorithm>
#include <filesystem>
#include <iostream>
#include <stdexcept>
#include <unordered_map>

// [P05][CARGA_OBJ] Assimp abre el archivo indicado en main y lo divide en mallas y materiales.
Model::Model(Graficos& renderer, const std::string& file) {
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(file, aiProcess_Triangulate |
        aiProcess_GenSmoothNormals | aiProcess_PreTransformVertices);
    if (!scene || !scene->mRootNode || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE)
        throw std::runtime_error("Assimp: " + file + ": " + importer.GetErrorString());
    const auto directory = std::filesystem::path(file).parent_path();
    std::unordered_map<std::string, GLuint> loadedTextures;
    materials.resize(std::max(scene->mNumMaterials, 1u));
    for (unsigned i = 0; i < scene->mNumMaterials; ++i) {
        const aiMaterial* source = scene->mMaterials[i];
        auto& material = materials[i];
        aiString name;
        source->Get(AI_MATKEY_NAME, name);
        aiColor3D diffuse(1, 1, 1);
        // Los OBJ sin MTL, como el trofeo, conservan el color indicado por la escena.
        if (std::string(name.C_Str()) != "DefaultMaterial" &&
            source->Get(AI_MATKEY_COLOR_DIFFUSE, diffuse) == AI_SUCCESS)
            material.color = {diffuse.r, diffuse.g, diffuse.b};
        aiString image;
        // [P06][TEXTURA_DEL_OBJ] Lee del MTL la ruta de la imagen; parte de la carpeta del OBJ.
        if (source->GetTexture(aiTextureType_DIFFUSE, 0, &image) == AI_SUCCESS) {
            std::string relative = image.C_Str();
            std::replace(relative.begin(), relative.end(), '\\', '/');
            const std::string path = (directory / relative).lexically_normal().string();
            auto found = loadedTextures.find(path);
            if (found == loadedTextures.end())
                found = loadedTextures.emplace(path, renderer.loadTexture(path)).first;
            material.texture = found->second;
            ++textureCount;
            std::cout << "MATERIAL: " << name.C_Str() << " / textura=" << path << '\n';
        }
    }
    for (unsigned m = 0; m < scene->mNumMeshes; ++m) {
        const aiMesh* source = scene->mMeshes[m];
        auto part = std::make_unique<Part>();
        part->material = source->mMaterialIndex;
        if (materials[part->material].texture && !source->mTextureCoords[0])
            throw std::runtime_error("Modelo texturizado sin coordenadas UV: " + file);
        std::vector<Vertex> vertices;
        std::vector<unsigned> indices;
        for (unsigned i = 0; i < source->mNumVertices; ++i) {
            const auto& p = source->mVertices[i];
            const auto& n = source->mNormals[i];
            glm::vec2 uv(0);
            if (source->mTextureCoords[0]) uv = {source->mTextureCoords[0][i].x, source->mTextureCoords[0][i].y};
            vertices.push_back({{p.x,p.y,p.z},{n.x,n.y,n.z},uv});
        }
        for (unsigned f = 0; f < source->mNumFaces; ++f)
            for (unsigned i = 0; i < source->mFaces[f].mNumIndices; ++i)
                indices.push_back(source->mFaces[f].mIndices[i]);
        if (indices.empty()) continue;
        part->mesh.upload(vertices, indices);
        triangleCount += part->mesh.triangleCount();
        parts.push_back(std::move(part));
    }
    if (parts.empty()) throw std::runtime_error("Modelo sin triangulos: " + file);
    std::cout << "ASSIMP: " << file << " / " << triangleCount << " triangulos / "
              << parts.size() << " partes / " << textureCount << " materiales texturizados\n";
}
// [DIBUJAR_OBJ] La matriz cambia posicion/tamano; el color multiplica el material del modelo.
void Model::draw(Graficos& r, const glm::mat4& transform, glm::vec3 color) const {
    for (const auto& part : parts) {
        const auto& material = materials[part->material];
        r.draw(part->mesh, transform, color * material.color, material.texture);
    }
}
