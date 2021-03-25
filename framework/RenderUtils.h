#pragma once

#include "external/stb/stb_image.h"

namespace framework
{

	// Vertex layout used by the DebugMeshes
	struct DebugVertex
	{
		v3 m_pos;
		v3 m_normal;
		v2 m_uv;
	};

	struct DebugMesh
	{
		ID3D11Buffer* m_vertexBuffer = nullptr;
		ID3D11Buffer* m_indexBuffer = nullptr;
		u32 m_vertexCount = 0;
		u32 m_vertexOffset = 0;
		u32 m_indexCount = 0;
		u32 m_indexOffset = 0;
		D3D_PRIMITIVE_TOPOLOGY m_topology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		bool m_indexIsShort = true;
	};

	class DebugPrims
	{
	public:

		static bool init(ID3D11Device* device);

		static const DebugMesh& getBox() { return s_box; };

		static const DebugMesh& getPlaneXZ() { return s_planeXZ; };

		static bool createIcoSphere(ID3D11Device* device, f32 radius, s32 recursionLevel, DebugMesh& outMesh);

		// Creates a 1 unit radius line sphere
		static bool createLineSphere(ID3D11Device* device, u32 res0, u32 res1, DebugMesh& outMesh);

		// Create line cone. h=1 and half angle = 45`
		static bool createLineCone(ID3D11Device* device, DebugMesh& outMesh);

	private:

		static bool initBox(ID3D11Device* device);
		static bool initPlaneXZ(ID3D11Device* device);

		static DebugMesh s_box;
		static DebugMesh s_planeXZ;
	};

	class ShaderPipeline
	{
	public:
		enum class ShaderStage : u32
		{
			Vertex = 0,
			Fragment,
			// ---------
			COUNT
		};

		ShaderPipeline();

		~ShaderPipeline();

		bool loadGraphicsPipeline(ID3D11Device* device,
			const char* srcRelPath,
			const char* entryVS,
			const char* entryFS,
			D3D11_INPUT_ELEMENT_DESC* vertexAttributes, u32 vertexAttribCount);

		bool createGraphicsPipeline(ID3D11Device* device,
			const char* src, const size_t srcSize,
			const char* entryVS,
			const char* entryFS,
			D3D11_INPUT_ELEMENT_DESC* vertexAttributes, u32 vertexAttribCount);

		void bind(ID3D11DeviceContext* ctx);

	private:

		ID3D11InputLayout* m_layout = nullptr;
		ID3D11VertexShader* m_vertexShader = nullptr;
		ID3D11PixelShader* m_fragmentShader = nullptr;
	};

	struct Texture2D
	{
		~Texture2D()
		{
			if (m_SRV)
			{
				m_SRV->Release();
				m_SRV = nullptr;
			}
			if (m_texture)
			{
				m_texture->Release();
				m_texture = nullptr;
			}
		}

		ID3D11Texture2D* m_texture = nullptr;
		ID3D11ShaderResourceView* m_SRV = nullptr;
		String m_name = "<NO NAME>";
	};

	struct DepthAttachment
	{
		~DepthAttachment()
		{
			if (m_depthStencilView)
			{
				m_depthStencilView->Release();
			}
			if (m_depthStencilTexture)
			{
				m_depthStencilTexture->Release();
			}
		}

		ID3D11DepthStencilView* m_depthStencilView = nullptr;
		ID3D11Texture2D* m_depthStencilTexture = nullptr;
	};

	class RenderResources
	{
	public:

		// Constant buffers
		template <typename T>
		static ID3D11Buffer* createConstantBuffer(ID3D11Device* device, const T& cbInitialData)
		{
			return createConstantBufferRaw(device, &cbInitialData, static_cast<u32>(sizeof(T)));
		}
		static bool updateMappableCBData(ID3D11DeviceContext* ctx, ID3D11Buffer* cBuffer, const void* data, u32 size);

		// Texture resources
		static bool loadTexture2D(ID3D11Device* device, ID3D11DeviceContext* ctx, const char* fileRelPath, DXGI_FORMAT format, Texture2D& outTexture);
		static bool createTexture2D(ID3D11Device* device, ID3D11DeviceContext* ctx, u32 w, u32 h, u32 texelSize, DXGI_FORMAT format, const void* data, Texture2D& outTexture);
		static ID3D11SamplerState* createSamplerState(ID3D11Device* device, D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE addressMode);

		// Depth attachments
		static bool createDepthAttachment(ID3D11Device* device, u32 width, u32 height, DXGI_FORMAT format, DepthAttachment& outDepthAttachment);
		static ID3D11DepthStencilState* createDepthStencilState(ID3D11Device* device, D3D11_COMPARISON_FUNC func);

		// Vertex index buffer helpers
		static ID3D11Buffer* createVertexBuffer(ID3D11Device* device, u32 bufferSize, void* initialData);
		static ID3D11Buffer* createIndexBuffer(ID3D11Device* device, u32 bufferSize, void* initialData);

	private:

		static ID3D11Buffer* createConstantBufferRaw(ID3D11Device* device, const void* data, const u32 size);
	};

	class Gizmo3D
	{
	public:

		enum Transformation : u32 
		{
			Translation = 1<<0,
			Rotation = 1<<1,
			Scale = 1<<2,
			All = Translation | Rotation | Scale
		};

		static void editTransform(f32* cameraView, f32* cameraProjection, f32* matrix, bool editTransformDecomposition);
		static void editRotation(f32* cameraView, f32* cameraProjection, f32* matrix, bool editTransformDecomposition);
		static void drawTranslationGizmo(const m4& cameraView, const m4& cameraProjection, m4& model);
		static void drawRotationGizmo(const m4& cameraView, const m4& cameraProjection, m4& model);
		static void drawScaleGizmo(const m4& cameraView, const m4& cameraProjection, m4& model);
	};

	class GltfScene 
	{
	public:
		using MeshRenderData = framework::DebugMesh;

		struct VertexBuffer0 
		{
			v3 m_pos;
		};
		struct VertexBuffer1 
		{
			v3 m_normal;
			v4 m_tangent;
			v2 m_uv;
		};

		struct SurfaceMaterial 
		{
			framework::Texture2D m_albedo;
			framework::Texture2D m_normal;
			u32 m_hash;
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

		enum MaterialHashFlags 
		{
			NormalMap = 0,
			COUNT
		};

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
}