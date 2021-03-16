#include "framework/framework.h"

#define ENABLE_DEVICE_DEBUG true

struct FrameDataCB
{
	m4 view;
	m4 viewProj;
	m4 invViewProj;
	v3 lightDir;
	f32 pad0;
};

struct DrawcallDataCB 
{
	m4 m_model;
};

struct SurfaceMaterial 
{
	framework::Texture2D m_albedo;
	framework::Texture2D m_normal;
};

struct Node 
{
	m4 m_model;
	u32 m_mesh = -1;
};

struct Meshlet 
{
	u32 m_vertexOffset;
	u32 m_vertexCount;
	u32 m_indexBytesOffset; // In bytes
	u32 m_indexCount;
	u32 m_material;
	bool m_isIndexShort = false;
};

struct Mesh 
{
	Vector<Meshlet> m_meshlets;
};

// -----------------------------------------------------------------------------------------------
// Load GLTF code

static String s_PosAttribName = "POSITION";
static String s_NormalAttribName = "NORMAL";
static String s_TangentAttribName = "TANGENT";
static String s_UvAttribName = "TEXCOORD_0";

struct VertexBuffer0 
{
	v3 m_pos;
};
struct VertexBuffer1 
{
	v3 m_normal;
	v3 m_tangent;
	v2 m_uv;
};

static bool doesFileExist(const std::string &abs_filename, void *) 
{
	return framework::FileUtils::doesFileExist(abs_filename.c_str());
}

static String expandFilePath(const std::string & relPath, void *) 
{
	return framework::Paths::getAssetPath(relPath);
}

static bool readWholeFile(std::vector<unsigned char>* outBuffer, std::string* error, const std::string & filePath, void *)
{
	HANDLE file = framework::FileUtils::openFileForRead(filePath.c_str());
	if (file != INVALID_HANDLE_VALUE)
	{
		u32 fileSize = framework::FileUtils::getFileSize(file);
		outBuffer->resize(fileSize);
		memset(outBuffer->data(), 0, fileSize);
		u32 bytesRead = framework::FileUtils::readBytes(file, fileSize, outBuffer->data());
		VERIFY(bytesRead == fileSize, "Not the entire file was loaded");
		framework::FileUtils::closeFile(file);
		return true;
	}
	return false;
}

static bool writeWholeFile(std::string * error, const std::string & filePath, 
	const std::vector<unsigned char> & data, void * userData)
{
	// Not needed for the purpose of this example
	return false;
}

class GltfScene 
{
public:
	using MeshRenderData = framework::DebugMesh;

	bool loadGLTF(ID3D11Device* device, ID3D11DeviceContext* ctx,const char* fileRelPath);

	ID3D11Buffer* getPackedVertexBuffer() const { return m_vertexBuffer; }
	ID3D11Buffer* getPackedIndexBuffer() const { return m_indexBuffer; }
	const Vector<Mesh>& getMeshes() const { return m_meshes; }
	const Vector<SurfaceMaterial> getMaterials() const { return m_materials; }
	const Vector<Node>& getNodes() const { return m_nodes; }

	u32 getVertexBuff0OffsetBytes(const Meshlet& meshlet) const { return meshlet.m_vertexOffset * static_cast<u32>(sizeof(VertexBuffer0)); }
	u32 getVertexBuff1OffsetBytes(const Meshlet& meshlet) const { return m_vertexBuff1OffsetBytes + meshlet.m_vertexOffset * static_cast<u32>(sizeof(VertexBuffer1)); }

private:

	void setupNodeHierarchy(tinygltf::Model* gltf, s32 nodeIdx, const m4& parentModel = m4(1.0f));
	bool setupGeometry(ID3D11Device* device, tinygltf::Model* gltf);
	bool setupMaterials(ID3D11Device* device, ID3D11DeviceContext* ctx, tinygltf::Model* gltf);
	String resolveTexturePath(const String& relPath) const;


	ID3D11Buffer* m_vertexBuffer = nullptr;
	ID3D11Buffer* m_indexBuffer = nullptr;
	u32 m_vertexBuff1OffsetBytes; // Delta to apply to calculate offset in bytes for VertexBuff1 for each meshlet
	Vector<Mesh> m_meshes;
	Vector<SurfaceMaterial> m_materials;
	Vector<Node> m_nodes;
	String m_basePath;
};

bool GltfScene::loadGLTF(ID3D11Device* device, ID3D11DeviceContext* ctx,const char* fileRelPath)
{
	String path(fileRelPath);
	std::replace( path.begin(), path.end(), '\\', '/');
	size_t pivot = path.find_last_of('/');
	String prefixPath = "./";
	if (pivot != String::npos) 
	{
		prefixPath = path.substr(0, pivot);
	}

	u32 fileSize = 0;
	m_basePath = prefixPath;
	String absPath = framework::Paths::getAssetPath(fileRelPath);
	UniquePtr<char[]> gltfFileData = framework::FileUtils::loadFileContent(absPath.c_str(), fileSize);
	if (fileSize == 0) 
	{
		printf("GLTF file could not be opened.");
		return 1;
	}

	UniquePtr<tinygltf::Model> model = std::make_unique<tinygltf::Model>();
	String error, warnings;
	tinygltf::TinyGLTF loader;
	tinygltf::FsCallbacks callbacks{};
	callbacks.ExpandFilePath = &expandFilePath;
	callbacks.ReadWholeFile = &readWholeFile;
	callbacks.WriteWholeFile = &writeWholeFile;
	callbacks.FileExists = &doesFileExist;
	loader.SetFsCallbacks(callbacks);
	if (!loader.LoadASCIIFromString(model.get(), &error, &warnings, gltfFileData.get(), fileSize, prefixPath)) 
	{
		printf("ERRORs: %s\n", error.c_str());
		return false;
	}
	if (warnings.size()) 
	{
		printf("WARNINGs: %s\n", warnings.c_str());
	}

	if (model->nodes.size() > 0) 
	{
		setupNodeHierarchy(model.get(), 0);
		if (!setupMaterials(device, ctx, model.get())) 
		{
			printf("Failed to initialize material data");
			return false;
		}
		if (!setupGeometry(device, model.get())) 
		{
			printf("Failed to initialize geometry resources");
			return false;
		}
		return true;
	}
	return false;
}

void GltfScene::setupNodeHierarchy(tinygltf::Model* gltf, s32 nodeIdx, const m4& parentModel)
{
	const tinygltf::Node& gltfNode = gltf->nodes[nodeIdx];
	
	m4 model(1.0f);
	if (gltfNode.matrix.size() > 0) 
	{
		model = glm::make_mat4(gltfNode.matrix.data());
	}
	else 
	{
		if (gltfNode.rotation.size() > 0) 
		{
			model = model * glm::mat4_cast(quat((float)gltfNode.rotation[3], (float)gltfNode.rotation[0], (float)gltfNode.rotation[1], (float)gltfNode.rotation[2]));
		}
		if (gltfNode.scale.size() > 0) 
		{
			model = model * glm::scale(m4(1.0f), v3(gltfNode.scale[0], gltfNode.scale[1], gltfNode.scale[2]));
		}
		if (gltfNode.translation.size() > 0) 
		{
			model = model * glm::translate(m4(1.0f), v3(gltfNode.translation[0], gltfNode.translation[1], gltfNode.translation[2]));
		}
	}

	if (gltfNode.mesh >= 0) 
	{
		m_nodes.push_back(Node());
		Node& node = m_nodes[m_nodes.size() - 1];
		node.m_mesh = static_cast<u32>(gltfNode.mesh);
		node.m_model = model;
	}

	for (const s32 idx : gltfNode.children) 
	{
		setupNodeHierarchy(gltf, idx, model);
	}
}

template<typename AttribT>
static inline void readAttribData(tinygltf::Model* gltf, AttribT* dst, u32 expectedStride, u32 vertexIdx, const tinygltf::Accessor& accesor)
{
	const tinygltf::BufferView& view = gltf->bufferViews[accesor.bufferView];
	u32 stride = static_cast<u32>(glm::max(expectedStride, static_cast<u32>(view.byteStride)));
	unsigned char* src = gltf->buffers[view.buffer].data.data() + accesor.byteOffset + view.byteOffset;
	src += stride * vertexIdx;
	*dst = *(reinterpret_cast<AttribT*>(src));
}

bool GltfScene::setupGeometry(ID3D11Device* device, tinygltf::Model* gltf) 
{
	// Do two iterations:
	// 0: Resolve required sizes and offsets
	// 1: Copy data to buffers

	m_meshes.resize(gltf->meshes.size());
	u32 vertexCount(0);
	u32 indexBufferSize(0);
	for (u32 meshIdx = 0; meshIdx < static_cast<u32>(gltf->meshes.size()); meshIdx++)
	{
		const tinygltf::Mesh& gltfMesh = gltf->meshes[meshIdx];
		const std::vector<tinygltf::Primitive>& prims = gltfMesh.primitives;
		Mesh& mesh = m_meshes[meshIdx];
		mesh.m_meshlets.resize(prims.size());
		for (u32 meshletIdx = 0; meshletIdx < static_cast<u32>(prims.size()); ++meshletIdx) 
		{
			const tinygltf::Primitive& prim = prims[meshletIdx];
			VERIFY(prim.mode == TINYGLTF_MODE_TRIANGLES, "Make sure the primitive is a triangle list");

			Meshlet& meshlet = mesh.m_meshlets[meshletIdx];
			meshlet.m_indexBytesOffset = indexBufferSize;// Offset in bytes (will be used when binding index buffer)
			meshlet.m_vertexOffset = vertexCount;
			meshlet.m_material = prim.material;
			// Find index count
			const tinygltf::Accessor& indexAccesor = gltf->accessors[prim.indices];
			meshlet.m_indexCount = static_cast<u32>(indexAccesor.count);
			// Use 16bit indices when the component type is either byte (we will convert this to u16) or unsigned short
			meshlet.m_isIndexShort = (indexAccesor.componentType == TINYGLTF_COMPONENT_TYPE_SHORT) ||
				(indexAccesor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) ||
				(indexAccesor.componentType == TINYGLTF_COMPONENT_TYPE_BYTE) ||
				(indexAccesor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE);
			indexBufferSize += (meshlet.m_isIndexShort ? 2 : 4) * meshlet.m_indexCount;
			indexBufferSize += (indexBufferSize % 4) != 0 ? 2 : 0; // Guarantee alignment of 4 bytes (alignof(int))

			// Find out the number of vertices
			const auto posIt = prim.attributes.find(s_PosAttribName);
			if (posIt != prim.attributes.end()) // Guaranteed every mesh will have at least this attribute
			{
				const tinygltf::Accessor& posAccesor = gltf->accessors[posIt->second];
				VERIFY(posAccesor.type == TINYGLTF_TYPE_VEC3 && posAccesor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT, "Format for pos not supported");
				meshlet.m_vertexCount = static_cast<u32>(posAccesor.count);
				vertexCount += static_cast<u32>(posAccesor.count);
			}
		}
	}

	m_vertexBuff1OffsetBytes = vertexCount * static_cast<u32>(sizeof(VertexBuffer0)); // Used to calculate offsets correctly during rendering

	// Allocate data for unified buffer memory
	static u32 s_vertexSize = static_cast<u32>(sizeof(VertexBuffer0) + sizeof(VertexBuffer1));
	u32 vertexDataReqSpace = s_vertexSize * vertexCount;
	UniquePtr<char[]> vertexBufferData = UniquePtr<char[]>(new char[vertexDataReqSpace]);
	UniquePtr<char[]> indexBufferData = UniquePtr<char[]>(new char[indexBufferSize]);
	VertexBuffer0* buff0Data = reinterpret_cast<VertexBuffer0*>(vertexBufferData.get());
	VertexBuffer1* buff1Data = reinterpret_cast<VertexBuffer1*>(buff0Data + vertexCount);
	char* indexBuffData = reinterpret_cast<char*>(indexBufferData.get());

	for (u32 meshIdx = 0; meshIdx < static_cast<u32>(gltf->meshes.size()); meshIdx++)
	{
		const tinygltf::Mesh& gltfMesh = gltf->meshes[meshIdx];
		const std::vector<tinygltf::Primitive>& prims = gltfMesh.primitives;
		Mesh& mesh = m_meshes[meshIdx];
		mesh.m_meshlets.resize(prims.size());
		for (u32 meshletIdx = 0; meshletIdx < static_cast<u32>(prims.size()); ++meshletIdx) 
		{
			const tinygltf::Primitive& prim = prims[meshletIdx];
			Meshlet& meshlet = mesh.m_meshlets[meshletIdx];

			// Copy index data
			{
				const tinygltf::Accessor& indexAccesor = gltf->accessors[prim.indices];
				const tinygltf::BufferView& view = gltf->bufferViews[indexAccesor.bufferView];
				bool isInteger = (indexAccesor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) || (indexAccesor.componentType == TINYGLTF_COMPONENT_TYPE_INT);
				bool isShort = (indexAccesor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) || (indexAccesor.componentType == TINYGLTF_COMPONENT_TYPE_SHORT);
				bool isByte = (indexAccesor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) || (indexAccesor.componentType == TINYGLTF_COMPONENT_TYPE_BYTE);
				unsigned char* src = gltf->buffers[view.buffer].data.data() + indexAccesor.byteOffset + view.byteOffset;
				char* dst = indexBuffData + meshlet.m_indexBytesOffset;
				if (isInteger || isShort)
				{
					u32 stride = static_cast<u32>(view.byteStride);
					if (stride)
					{
						u32 bytesToCopy = isInteger ? 4 : 2;
						for (u32 i = 0; i < indexAccesor.count; ++i)
						{
							memcpy(dst, src, bytesToCopy);
							src += stride;
							dst += bytesToCopy;
						}
					}
					else
					{
						u32 bytesToCopy = (isInteger ? 4 : 2) * static_cast<u32>(indexAccesor.count);
						memcpy(dst, src, bytesToCopy);
					}
				}
				else if (isByte)
				{
					// Special conversion needed
					u32 stride = static_cast<u32>(view.byteStride);
					stride = glm::max(stride, 1u);
					for (u32 i = 0; i < indexAccesor.count; ++i)
					{
						u16 element = static_cast<char>(src[0]);
						memcpy(dst, &element, 2);
						src += stride;
					}
				}
			}

			// Fill the data for Vertex Buffer 0
			{
				VertexBuffer0* meshletBuff0 = buff0Data + meshlet.m_vertexOffset;
				const auto posIt = prim.attributes.find(s_PosAttribName);
				if (posIt != prim.attributes.end())
				{
					const tinygltf::Accessor& posAccesor = gltf->accessors[posIt->second];
					const tinygltf::BufferView& view = gltf->bufferViews[posAccesor.bufferView];

					unsigned char* src = gltf->buffers[view.buffer].data.data() + posAccesor.byteOffset + view.byteOffset;
					u32 bytesToCopy = static_cast<u32>(sizeof(v3));
					if (view.byteStride) 
					{
						for (u32 i=0; i<posAccesor.count; ++i) 
						{
							meshletBuff0[i].m_pos = *(reinterpret_cast<v3*>(src));
							src += view.byteStride;
						}
					}
					else 
					{						
						memcpy(&meshletBuff0[0].m_pos[0], src, bytesToCopy * meshlet.m_vertexCount);
					}
				}
			}

			// Fill the data for Vertex Buffer 1
			{
				VertexBuffer1* meshletBuff1 = buff1Data + meshlet.m_vertexOffset;
				const auto normalIt = prim.attributes.find(s_NormalAttribName);
				const auto uvIt = prim.attributes.find(s_UvAttribName);
				const auto tangentIt = prim.attributes.find(s_TangentAttribName);
				for (u32 i=0; i<meshlet.m_vertexCount; ++i) 
				{
					// Fill normals
					meshletBuff1[i].m_normal = v3(0.0f, 0.0f, 1.0f);
					if (normalIt != prim.attributes.end()) 
					{
						const tinygltf::Accessor& accesor = gltf->accessors[normalIt->second];
						VERIFY(accesor.type == TINYGLTF_TYPE_VEC3 && accesor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT, "Format for normal not supported");
						readAttribData<v3>(gltf, &meshletBuff1[i].m_normal, static_cast<u32>(sizeof(v3)), i, accesor);
					}

					// Fill Uvs
					meshletBuff1[i].m_uv = v3(0.0f);
					if (uvIt != prim.attributes.end()) 
					{
						const tinygltf::Accessor& accesor = gltf->accessors[uvIt->second];
						VERIFY(accesor.type == TINYGLTF_TYPE_VEC2 && accesor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT, "Format for uv not supported");
						readAttribData<v2>(gltf, &meshletBuff1[i].m_uv, static_cast<u32>(sizeof(v2)), i, accesor);
					}

					// Fill tangents
					meshletBuff1[i].m_tangent = v3(0.0f);
					if (tangentIt != prim.attributes.end()) 
					{
						const tinygltf::Accessor& accesor = gltf->accessors[tangentIt->second];
						VERIFY((accesor.type == TINYGLTF_TYPE_VEC3 || accesor.type == TINYGLTF_TYPE_VEC4) && accesor.componentType == TINYGLTF_COMPONENT_TYPE_FLOAT, "Format for tangent not supported");
						u32 dataStride = (accesor.type == TINYGLTF_TYPE_VEC3) ? static_cast<u32>(sizeof(v3)) : static_cast<u32>(sizeof(v4));
						readAttribData<v3>(gltf, &meshletBuff1[i].m_tangent, dataStride, i, accesor);
					}
					else if(m_materials[meshlet.m_material].m_normal.m_SRV)
					{
						printf("The material used has a normal map but the geometry lacks tangents.\n");
					}
				}
			}

		} // End iterate meshlets
	} // End iterate meshes

	m_vertexBuffer = framework::RenderResources::createVertexBuffer(device, vertexDataReqSpace, vertexBufferData.get());
	m_indexBuffer = framework::RenderResources::createIndexBuffer(device, indexBufferSize, indexBufferData.get());

	return (m_vertexBuffer && m_indexBuffer);
}

bool GltfScene::setupMaterials(ID3D11Device* device, ID3D11DeviceContext* ctx, tinygltf::Model* gltf) 
{
	m_materials.resize(gltf->materials.size());
	for (u32 i = 0; i < static_cast<u32>(gltf->materials.size()); ++i) 
	{
		const tinygltf::Material& gltfMat = gltf->materials[i];
		SurfaceMaterial& material = m_materials[i];
		// Albedo
		{
			s32 albedoIdx = gltfMat.pbrMetallicRoughness.baseColorTexture.index;
			if (albedoIdx >= 0) 
			{
				const tinygltf::Image& albedoImg = gltf->images[gltf->textures[albedoIdx].source];
				material.m_albedo.m_name = resolveTexturePath(albedoImg.uri);
				u32 texelSize = albedoImg.component;
				DXGI_FORMAT format = albedoImg.component == 4 ? DXGI_FORMAT_R8G8B8A8_UNORM_SRGB : DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
				if(!framework::RenderResources::createTexture2D(device, ctx, albedoImg.width, albedoImg.height, texelSize, format, albedoImg.image.data(), material.m_albedo))
				{
					printf("Failed to create albedo\n");
					return false;
				}
			}
		}

		// Normal map
		{
			s32 normalIdx = gltfMat.normalTexture.index;
			if (normalIdx >= 0) 
			{
				const tinygltf::Image& img = gltf->images[gltf->textures[normalIdx].source];
				material.m_normal.m_name = resolveTexturePath(img.uri);
				u32 texelSize = img.component;
				DXGI_FORMAT format = img.component == 4 ? DXGI_FORMAT_R8G8B8A8_UNORM : DXGI_FORMAT_B8G8R8X8_UNORM;
				if(!framework::RenderResources::createTexture2D(device, ctx, img.width, img.height, texelSize, format, img.image.data(), material.m_normal))
				{
					printf("Failed to create normal map\n");
					return false;
				}
			}
		}
	}
	return true;
}

inline String GltfScene::resolveTexturePath(const String& relPath) const
{
	return m_basePath + "/" + relPath;
}

// -----------------------------------------------------------------------------------------------

bool loadShader(ID3D11Device* device, const char* relPath, framework::ShaderPipeline& outShader) 
{
	static constexpr u32 s_vertexAttribCount = 4;
	D3D11_INPUT_ELEMENT_DESC vertexLayout[s_vertexAttribCount];
	ZeroMemory(vertexLayout, s_vertexAttribCount * sizeof(D3D11_INPUT_ELEMENT_DESC));
	vertexLayout[0].SemanticName = "POSITION";
	vertexLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexLayout[0].InputSlot = 0;
	vertexLayout[0].AlignedByteOffset = u32(offsetof(VertexBuffer0, m_pos));
	vertexLayout[0].InstanceDataStepRate = D3D11_INPUT_PER_VERTEX_DATA;
	
	vertexLayout[1].SemanticName = "NORMAL";
	vertexLayout[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexLayout[1].InputSlot = 1;
	vertexLayout[1].AlignedByteOffset = u32(offsetof(VertexBuffer1, m_normal));
	vertexLayout[1].InstanceDataStepRate = D3D11_INPUT_PER_VERTEX_DATA;
	
	vertexLayout[2].SemanticName = "TANGENT";
	vertexLayout[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexLayout[2].InputSlot = 1;
	vertexLayout[2].AlignedByteOffset = u32(offsetof(VertexBuffer1, m_tangent));
	vertexLayout[2].InstanceDataStepRate = D3D11_INPUT_PER_VERTEX_DATA;

	vertexLayout[3].SemanticName = "TEXCOORD";
	vertexLayout[3].Format = DXGI_FORMAT_R32G32_FLOAT;
	vertexLayout[3].InputSlot = 1;
	vertexLayout[3].AlignedByteOffset = u32(offsetof(VertexBuffer1, m_uv));
	vertexLayout[3].InstanceDataStepRate = D3D11_INPUT_PER_VERTEX_DATA;
	return outShader.loadGraphicsPipeline(device, relPath, "mainVS", "mainFS", vertexLayout, s_vertexAttribCount);
}

static void updateFrameCB(ID3D11DeviceContext* ctx, ID3D11Buffer* cBuffer, const framework::Camera& cam, v3 lightDir) 
{
	FrameDataCB frameCBData;
	frameCBData.view = cam.getView();
	frameCBData.viewProj = cam.getViewProj();
	frameCBData.invViewProj = cam.getInvViewProj();
	frameCBData.lightDir = lightDir;
	framework::RenderResources::updateMappableCBData(ctx, cBuffer, &frameCBData, sizeof(FrameDataCB));
}

static void updateBatchCB(ID3D11DeviceContext* ctx, ID3D11Buffer* cBuffer, const m4& model) 
{
	DrawcallDataCB drawcallCB;
	drawcallCB.m_model = model;
	framework::RenderResources::updateMappableCBData(ctx, cBuffer, &drawcallCB, sizeof(DrawcallDataCB));
}

// -----------------------------------------------------------------------------------------------

int main(int argc, char** argv)
{
	const u32 width = 1280;
	const u32 height = 720;
	static framework::Window s_window(argc, argv);
	s_window.init("4_LoadingGLTF", width, height, true, false, ENABLE_DEVICE_DEBUG);
	ID3D11Device* device = s_window.getDevice();
	ID3D11DeviceContext* ctx = s_window.getCtx();

	framework::ShaderPipeline shader;
	if (!loadShader(device, "./shaders/4_Lambertian.hlsl", shader)) 
	{
		printf("Failed to load and create shader");
		return 1;
	}

	const framework::DebugMesh& boxMesh = framework::DebugPrims::getBox();

	framework::FirstPersonCamera fpCam;

	v3 camPos(0.0f, 5.0f, 20.0f);
	f32 rotSpeed = 10.0f;
	v2 uvOffset(0.0f);
	v2 uvScale(1.0f);

	fpCam.init(camPos, v3(0.0f), 10, 10.0f);
	fpCam.setPerspective(60.0f, f32(width) / f32(height), 1.0f, 1000.0f);
	
	FrameDataCB frameCBData;
	frameCBData.view = fpCam.getView();
	frameCBData.viewProj = fpCam.getViewProj();
	frameCBData.invViewProj = fpCam.getInvViewProj();
	ID3D11Buffer* frameCB = framework::RenderResources::createConstantBuffer<FrameDataCB>(device, frameCBData);
	if (!frameCB) 
	{
		printf("Failed to create per frame constant buffer");
		return 1;
	}

	ID3D11Buffer* drawcallCB = framework::RenderResources::createConstantBuffer<DrawcallDataCB>(device, {m4(1.0f)});
	if (!drawcallCB) 
	{
		printf("Failed to create Per drawcall constant buffer");
		return 1;
	}

	framework::Texture2D texture;
	if (!framework::RenderResources::loadTexture2D(device, ctx, "./textures/uv_check.png", DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, texture)) 
	{
		printf("Failed to create texture.");
		return 1;
	}

	ID3D11SamplerState* samplers = framework::RenderResources::createSamplerState(device, D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_WRAP);
	if (!samplers) 
	{
		printf("Failed to create sampler state");
		return 1;
	}

	// Configure rasterization with a RasterState
	ID3D11RasterizerState* rasterState = nullptr;
	D3D11_RASTERIZER_DESC rasterStateDesc;
	ZeroMemory(&rasterStateDesc, sizeof(D3D11_RASTERIZER_DESC));
	rasterStateDesc.FillMode = D3D11_FILL_SOLID; // Solid geometry
	rasterStateDesc.CullMode = D3D11_CULL_BACK;
	rasterStateDesc.FrontCounterClockwise = true; // OpenGL style (Direct uses clockwise by default)
	if (FAILED(device->CreateRasterizerState(&rasterStateDesc, &rasterState))) 
	{
		printf("Failed to create Raster State");
		return 1;
	}

	UniquePtr<GltfScene> scene = std::make_unique<GltfScene>();
	if (!scene->loadGLTF(device, ctx, "./models/Sponza/glTF/Sponza.gltf")) 
	{
		printf("Failed to load gltf");
		return 1;
	}

	framework::DepthAttachment depthAttachment;
	ID3D11DepthStencilState* depthStencilState = framework::RenderResources::createDepthStencilState(device, D3D11_COMPARISON_LESS);
	if (!framework::RenderResources::createDepthAttachment(device, width, height, DXGI_FORMAT_D24_UNORM_S8_UINT, depthAttachment) || !depthStencilState) 
	{
		return 1;
	}

	f64 lastStamp = framework::Time::getTimeStampS();
	f64 framerate = 0.0f;
	f64 elapsedTime = 0.0f;

	const Vector<Mesh>& meshes = scene->getMeshes();
	const Vector<Node>& nodes = scene->getNodes();
	const Vector<SurfaceMaterial>& materials = scene->getMaterials();

	m4 lightModel = glm::yawPitchRoll(glm::radians(45.0f), 0.0f, glm::radians(45.0f)) * glm::translate(m4(1.0f), v3(0.0f, 3.0f, -3.0f));

	// Start frames
	while (s_window.update())
	{
		// Calculate elapsed time/framerate/...
		f64 currStamp = framework::Time::getTimeStampS();
		elapsedTime = currStamp - lastStamp;
		lastStamp = currStamp;
		framerate = 1.0 / elapsedTime;

		fpCam.update(static_cast<f32>(elapsedTime));

		// Do some UI
		static bool s_editLightDir = true;
		if (ImGui::Begin("Controls")) 
		{
			ImGui::Text("R-Click + Move mouse: Rotate camera.");
			ImGui::Text("W/A/S/D: Move camera.");
			ImGui::Text("Left/Right: Decrease/Increase camera speed.");
			ImGui::Text("Up/Down: Decrease/Increase camera rotation speed.");
			ImGui::Separator();
			ImGui::Checkbox("Edit light dir", &s_editLightDir);
			ImGui::End();
		}

		if (s_editLightDir) 
		{
			m4 view = fpCam.getView();
			m4 projection = fpCam.getProjection();
			v3 forward(fpCam.getForward());
			lightModel[3] = v4(fpCam.getPos() + forward * 10.0f, 1.0f);
			framework::Gizmo3D::drawRotationGizmo(view, projection, lightModel);
		}
		v3 lightDir = glm::normalize((m3)lightModel * v3(0.0f, -1.0f, 0.0f));
		// --------------------------------

		ID3D11RenderTargetView* backBuffer = s_window.getBackBuffer();
		// Set the back buffer as our RenderTarget
		ctx->OMSetRenderTargets(1, &backBuffer, depthAttachment.m_depthStencilView);

		// Set the viewport. This configures the area to render
		D3D11_VIEWPORT viewport;
		viewport.TopLeftX = 0.0f;
		viewport.TopLeftY = 0.0f;
		viewport.Width = width;
		viewport.Height = height;
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		ctx->RSSetViewports(1, &viewport);

		// Tell the context how we want to rasterize the following drawcalls
		ctx->RSSetState(rasterState);

		// Clear the RenderTarget to the desired color
		FLOAT clearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
		ctx->ClearRenderTargetView(backBuffer, clearColor);
		ctx->ClearDepthStencilView(depthAttachment.m_depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

		// Update per-frame ConstantBuffer
		updateFrameCB(ctx, frameCB, fpCam, lightDir);

		// Bind shaders and draw batches
		shader.bind(ctx);
		ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		ctx->VSSetConstantBuffers(0, 1, &frameCB);
		ctx->PSSetConstantBuffers(0, 1, &frameCB);
		ctx->VSSetConstantBuffers(1, 1, &drawcallCB);

		// Draw GLTF
		for (const Node& node : nodes) 
		{
			if (node.m_mesh >= 0) 
			{
				updateBatchCB(ctx, drawcallCB, node.m_model);
				const Mesh& mesh = meshes[node.m_mesh];
				for (const Meshlet& meshlet : mesh.m_meshlets) 
				{
					u32 vertexBubberOffsets[] = {scene->getVertexBuff0OffsetBytes(meshlet), scene->getVertexBuff1OffsetBytes(meshlet)};
					u32 vertexBubberStrides[] = {static_cast<u32>(sizeof(VertexBuffer0)), static_cast<u32>(sizeof(VertexBuffer1))};
					ID3D11Buffer* vertexBuffers[] = {scene->getPackedVertexBuffer(), scene->getPackedVertexBuffer()};
					ctx->IASetVertexBuffers(0, 2, vertexBuffers, vertexBubberStrides, vertexBubberOffsets);
					ctx->IASetIndexBuffer(scene->getPackedIndexBuffer(),
						meshlet.m_isIndexShort ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT,
						meshlet.m_indexBytesOffset);

					const SurfaceMaterial& mat = materials[meshlet.m_material];
					if (mat.m_albedo.m_SRV) 
					{
						ctx->PSSetSamplers(0, 1, &samplers);
						ctx->PSSetShaderResources(0, 1, &mat.m_albedo.m_SRV);
					}

					ctx->DrawIndexed(meshlet.m_indexCount, 0, 0);
				}

			}
		}

		s_window.present();
	}

	return 0;
}