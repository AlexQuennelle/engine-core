#include "mesh.h"
#include "matrix.h"
#include "shader.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <bgfx/bgfx.h>
#include <print>
#include <ranges>
#include <vector>

bgfx::VertexLayout Vertex::layout;

void ProcessNode(const aiNode* node, const aiScene* scene,
				 std::vector<std::pair<Mesh, Matrix<4>>>& vector,
				 const Matrix<4> transform = Matrix<4>::Identity());

auto LoadModel(const std::string& filepath)
	-> std::vector<std::pair<Mesh, Matrix<4>>>
{
	Assimp::Importer importer;
	const aiScene* scene
		= importer.ReadFile(filepath, 0x0
										  | aiProcess_JoinIdenticalVertices
										  | aiProcess_Triangulate
										  | aiProcess_GenNormals
										  | aiProcess_FixInfacingNormals
										  | aiProcess_SortByPType
										  | aiProcess_OptimizeMeshes
										  | aiProcess_OptimizeGraph);
	if (scene == nullptr)
	{
		std::println("Error: Failed to load {}", filepath);
		return {};
	}

	std::vector<std::pair<Mesh, Matrix<4>>> meshes;
	meshes.reserve(scene->mNumMeshes);

	auto* node = scene->mRootNode;
	ProcessNode(node, scene, meshes);

	return meshes;
}

void ProcessNode(const aiNode* node, const aiScene* scene, // NOLINT
				 std::vector<std::pair<Mesh, Matrix<4>>>& vector,
				 const Matrix<4> transform)
{
	namespace r = std::ranges;
	namespace rv = std::ranges::views;
	if (node->mNumMeshes == 0)
		return;

	std::array<float, 16> newTransformData{};
	std::memcpy(newTransformData.data(), &node->mTransformation,
				sizeof(float) * 16);
	auto transforms = rv::repeat(transform * Matrix<4>(newTransformData));

	auto meshes
		= std::span(node->mMeshes, node->mNumMeshes) // NOLINTNEXTLINE
		  | rv::transform([scene](auto i) { return scene->mMeshes[i]; })
		  | rv::transform([scene](auto mesh) { return Mesh(mesh, scene); });
	auto pairs = rv::zip(meshes, transforms)
				 | r::to<std::vector<std::pair<Mesh, Matrix<4>>>>();
	for (auto& pair : pairs)
	{
		vector.push_back(std::move(pair));
	}

	std::span<aiNode*> children{
		node->mChildren,
		node->mChildren + node->mNumChildren // NOLINT
	};
	for (auto* child : children)
	{
		ProcessNode(child, scene, vector, transform);
	}
}

Mesh::Mesh(const std::array<Vertex, 4>& data,
		   const std::array<uint16_t, 6>& indices)
{
	this->vertexBuffer = bgfx::createVertexBuffer(
		bgfx::makeRef(data.data(), sizeof(data)), Vertex::layout);
	this->indexBuffer = bgfx::createIndexBuffer(
		bgfx::makeRef(indices.data(), sizeof(indices)));
	this->shader = CreateShaderProgram(SHADERS "cubes.vert.bin",
									   SHADERS "cubes.frag.bin");
}
Mesh::Mesh(const std::array<Vertex, 8>& data,
		   const std::array<uint16_t, 36>& indices)
{
	this->vertexBuffer = bgfx::createVertexBuffer(
		bgfx::makeRef(data.data(), sizeof(data)), Vertex::layout);
	this->indexBuffer = bgfx::createIndexBuffer(
		bgfx::makeRef(indices.data(), sizeof(indices)));
	this->shader = CreateShaderProgram(SHADERS "cubes.vert.bin",
									   SHADERS "cubes.frag.bin");
}
// Mesh::Mesh(const std::string& filepath)
// {
// 	Assimp::Importer importer;
// 	const aiScene* scene
// 		= importer.ReadFile(filepath, 0x0
// 										  | aiProcess_JoinIdenticalVertices
// 										  | aiProcess_Triangulate
// 										  | aiProcess_SortByPType
// 										  | aiProcess_FixInfacingNormals
// 										  | aiProcess_OptimizeMeshes
// 										  | aiProcess_OptimizeGraph);
// 	// TODO: Add support for multi-mesh objects
// 	auto* mesh = scene->mMeshes[0]; // NOLINT

// 	// TODO: Add check to ensure colours are present
// 	namespace rv = std::ranges::views;
// 	std::vector<Vertex> vertexData;
// 	auto test = rv::zip(std::span(mesh->mVertices, mesh->mNumVertices),
// 						std::span(mesh->mColors[0], mesh->mNumVertices));
// 	vertexData.reserve(mesh->mNumVertices);
// 	for (auto [position, color] : test)
// 	{
// 		vertexData.push_back({
// 			{.x = position.x, .y = position.y, .z = position.z},
// 			{
// 				.r = static_cast<uint8_t>(std::round(color.r * 255)),
// 				.g = static_cast<uint8_t>(std::round(color.g * 255)),
// 				.b = static_cast<uint8_t>(std::round(color.b * 255)),
// 				.a = static_cast<uint8_t>(std::round(color.a * 255)),
// 			},
// 		});
// 	}

// 	std::vector<uint16_t> indices;
// 	for (uint32_t i{0}; i < mesh->mNumFaces; i++)
// 	{
// 		auto face{mesh->mFaces[i]};
// 		for (int j{0}; j < face.mNumIndices; j++)
// 		{
// 			indices.push_back(static_cast<uint16_t>(face.mIndices[j]));
// 		}
// 	}

// 	this->vertexBuffer = bgfx::createVertexBuffer(
// 		bgfx::copy(vertexData.data(),
// 				   static_cast<uint32_t>(vertexData.size() * sizeof(Vertex))),
// 		Vertex::layout);
// 	this->indexBuffer = bgfx::createIndexBuffer(
// 		bgfx::copy(indices.data(),
// 				   static_cast<uint32_t>(indices.size() * sizeof(uint16_t))));
// 	this->shader = CreateShaderProgram(SHADERS "cubes.vert.bin",
// 									   SHADERS "cubes.frag.bin");
// }
Mesh::Mesh(Mesh&& other) noexcept :
	vertexBuffer(other.vertexBuffer),
	indexBuffer(other.indexBuffer),
	shader(other.shader)
{
	other.shader = BGFX_INVALID_HANDLE;
	other.vertexBuffer = BGFX_INVALID_HANDLE;
	other.indexBuffer = BGFX_INVALID_HANDLE;
};
Mesh::Mesh(const aiMesh* aiMesh, const aiScene* scene) // NOLINT
{
	namespace r = std::ranges;
	namespace rv = std::ranges::views;

	// TODO: Handle Materials
	//		 Handle Bones
	//		 Handle Animations
	//		 Handle AABB
	//		 Handle Name
	//		 Handle Primitive types
	//		 Documentation

	auto ConvertCols = [](aiColor4D col) -> Color
	{
		return {
			.r = static_cast<uint8_t>(std::round(col.r * 255)),
			.g = static_cast<uint8_t>(std::round(col.g * 255)),
			.b = static_cast<uint8_t>(std::round(col.b * 255)),
			.a = static_cast<uint8_t>(std::round(col.a * 255)),
		};
	};
	auto colors = aiMesh->HasVertexColors(0)
					  ? std::span(aiMesh->mColors[0], aiMesh->mNumVertices)
							| rv::transform(ConvertCols)
							| r::to<std::vector<Color>>()
					  : rv::repeat(Color(), aiMesh->mNumVertices)
							| r::to<std::vector<Color>>();
	auto texCoords
		= aiMesh->HasTextureCoords(0)
			  ? std::span(aiMesh->mTextureCoords[0], aiMesh->mNumVertices)
					| rv::transform([](auto uv) { return Vector2(uv.x, uv.y); })
					| r::to<std::vector<Vector2>>()
			  : rv::repeat(Vector2(), aiMesh->mNumVertices)
					| r::to<std::vector<Vector2>>();
	auto vertData = rv::zip(std::span(aiMesh->mVertices, aiMesh->mNumVertices),
							std::span(aiMesh->mNormals, aiMesh->mNumVertices),
							colors, texCoords);
	for (auto [pos, nor, col, uv] : vertData)
	{
		this->vertices.push_back({
			.pos = {.x = pos.x, .y = pos.y, .z = pos.z},
			.nor = std::bit_cast<Normal3>(nor),
			.col = col,
			.UV = uv,
		});
	}
	this->vertexBuffer = bgfx::createVertexBuffer(
		bgfx::makeRef(this->vertices.data(),
					  static_cast<uint32_t>(vertices.size() * sizeof(Vertex))),
		Vertex::layout);

	if (aiMesh->HasFaces())
	{
		for (const auto& face : std::span(aiMesh->mFaces, aiMesh->mNumFaces))
		{
			this->tris.push_back({
				static_cast<uint16_t>(face.mIndices[0]), // NOLINT
				static_cast<uint16_t>(face.mIndices[1]), // NOLINT
				static_cast<uint16_t>(face.mIndices[2]), // NOLINT
			});
		}
		this->indexBuffer = bgfx::createIndexBuffer(bgfx::makeRef(
			this->tris.data(),
			static_cast<uint32_t>(aiMesh->mNumFaces * sizeof(Vertex))));
	}
}
Mesh::~Mesh() { this->Destroy(); }

void Mesh::Draw(bgfx::ProgramHandle& shader) const
{
	bgfx::setVertexBuffer(0, this->vertexBuffer);
	bgfx::setIndexBuffer(this->indexBuffer);
	bgfx::submit(0, shader);
}

auto Mesh::operator=(Mesh&& other) noexcept -> Mesh&
{
	this->Destroy();

	this->shader = other.shader;
	this->vertexBuffer = other.vertexBuffer;
	this->indexBuffer = other.indexBuffer;

	other.shader = BGFX_INVALID_HANDLE;
	other.vertexBuffer = BGFX_INVALID_HANDLE;
	other.indexBuffer = BGFX_INVALID_HANDLE;

	return *this;
}

void Mesh::Destroy()
{
	if (bgfx::isValid(this->shader))
		bgfx::destroy(this->shader);
	if (bgfx::isValid(this->vertexBuffer))
		bgfx::destroy(this->vertexBuffer);
	if (bgfx::isValid(this->indexBuffer))
		bgfx::destroy(this->indexBuffer);
}
