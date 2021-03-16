#define STB_IMAGE_IMPLEMENTATION
#include "framework/Framework.h"

#pragma warning(disable:4996)
#define TINYGLTF_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define TINYGLTF_NO_INCLUDE_STB_IMAGE
//#define TINYGLTF_NO_EXTERNAL_IMAGE 
#include "external/tinygltf/tiny_gltf.h"
#pragma warning(default:4996)

namespace framework
{

	DebugMesh DebugPrims::s_box;
	DebugMesh DebugPrims::s_planeXZ;

	bool DebugPrims::init(ID3D11Device* device)
	{
		return initBox(device) && initPlaneXZ(device);
	}

	bool DebugPrims::initBox(ID3D11Device* device)
	{
		// 4 vertices * 6 faces
		static constexpr u32 s_vertexCount = 4 * 6;
		static DebugVertex s_vertexBufferData[s_vertexCount] =
		{
			// Front
			{v3(-0.5f, 0.5f, 0.5f), v3(0.0f, 0.0f, 1.0f), v2(0.0f, 0.0f)},
			{v3(-0.5f, -0.5f, 0.5f), v3(0.0f, 0.0f, 1.0f), v2(0.0f, 1.0f)},
			{v3(0.5f, -0.5f, 0.5f), v3(0.0f, 0.0f, 1.0f), v2(1.0f, 1.0f)},
			{v3(0.5f, 0.5f, 0.5f), v3(0.0f, 0.0f, 1.0f), v2(1.0f, 0.0f)},
			// Right
			{v3(0.5f, 0.5f, 0.5f), v3(1.0f, 0.0f, 0.0f), v2(0.0f, 0.0f)},
			{v3(0.5f, -0.5f, 0.5f), v3(1.0f, 0.0f, 0.0f), v2(0.0f, 1.0f)},
			{v3(0.5f, -0.5f, -0.5f), v3(1.0f, 0.0f, 0.0f), v2(1.0f, 1.0f)},
			{v3(0.5f, 0.5f, -0.5f), v3(1.0f, 0.0f, 0.0f), v2(1.0f, 0.0f)},
			// Back
			{v3(0.5f, 0.5f, -0.5f), v3(0.0f, 0.0f, -1.0f), v2(0.0f, 0.0f)},
			{v3(0.5f, -0.5f, -0.5f), v3(0.0f, 0.0f, -1.0f), v2(0.0f, 1.0f)},
			{v3(-0.5f, -0.5f, -0.5f), v3(0.0f, 0.0f, -1.0f), v2(1.0f, 1.0f)},
			{v3(-0.5f, 0.5f, -0.5f), v3(0.0f, 0.0f, -1.0f), v2(1.0f, 0.0f)},
			// Left
			{v3(-0.5f, 0.5f, -0.5f), v3(-1.0f, 0.0f, 0.0f), v2(0.0f, 0.0f)},
			{v3(-0.5f, -0.5f, -0.5f), v3(-1.0f, 0.0f, 0.0f), v2(0.0f, 1.0f)},
			{v3(-0.5f, -0.5f, 0.5f), v3(-1.0f, 0.0f, 0.0f), v2(1.0f, 1.0f)},
			{v3(-0.5f, 0.5f, 0.5f), v3(-1.0f, 0.0f, 0.0f), v2(1.0f, 0.0f)},
			// Top
			{v3(-0.5f, 0.5f, -0.5f), v3(0.0f, 1.0f, 0.0f), v2(0.0f, 0.0f)},
			{v3(-0.5f, 0.5f, 0.5f), v3(0.0f, 1.0f, 0.0f), v2(0.0f, 1.0f)},
			{v3(0.5f, 0.5f, 0.5f), v3(0.0f, 1.0f, 0.0f), v2(1.0f, 1.0f)},
			{v3(0.5f, 0.5f, -0.5f), v3(0.0f, 1.0f, 0.0f), v2(1.0f, 0.0f)},
			// Bottom
			{v3(-0.5f, -0.5f, 0.5f), v3(0.0f, -1.0f, 0.0f), v2(0.0f, 0.0f)},
			{v3(-0.5f, -0.5f, -0.5f), v3(0.0f, -1.0f, 0.0f), v2(0.0f, 1.0f)},
			{v3(0.5f, -0.5f, -0.5f), v3(0.0f, -1.0f, 0.0f), v2(1.0f, 1.0f)},
			{v3(0.5f, -0.5f, 0.5f), v3(0.0f, -1.0f, 0.0f), v2(1.0f, 0.0f)},
		};
		// 6 faces * 2 triangles * 3 vertices
		static constexpr u32 s_indexCount = 6 * 2 * 3;
		static u16 s_indexBufferData[s_indexCount] =
		{
			// Front
			0,1,2,
			0,2,3,
			//Right
			4,5,6,
			4,6,7,
			// Back
			8,9,10,
			8,10,11,
			// Left
			12,13,14,
			12,14,15,
			// Top
			16,17,18,
			16,18,19,
			// Bottom
			20,21,22,
			20,22,23
		};

		s_box.m_vertexCount = s_vertexCount;
		s_box.m_vertexOffset = 0;
		s_box.m_indexCount = s_indexCount;
		s_box.m_indexOffset = 0;

		{
			D3D11_BUFFER_DESC bufferDesc;
			bufferDesc.Usage = D3D11_USAGE_DEFAULT;
			bufferDesc.ByteWidth = sizeof(DebugVertex) * s_vertexCount;
			bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			bufferDesc.CPUAccessFlags = 0;
			bufferDesc.MiscFlags = 0;

			// Fill in the subresource data.
			D3D11_SUBRESOURCE_DATA initData;
			initData.pSysMem = s_vertexBufferData;
			initData.SysMemPitch = 0;
			initData.SysMemSlicePitch = 0;

			// Create the vertex buffer.
			HRESULT res = device->CreateBuffer(&bufferDesc, &initData, &s_box.m_vertexBuffer);
			if (FAILED(res))
			{
				return false;
			}
		}

		{
			D3D11_BUFFER_DESC bufferDesc;
			bufferDesc.Usage = D3D11_USAGE_DEFAULT;
			bufferDesc.ByteWidth = sizeof(u16) * s_indexCount;
			bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
			bufferDesc.CPUAccessFlags = 0;
			bufferDesc.MiscFlags = 0;

			// Define the resource data.
			D3D11_SUBRESOURCE_DATA initData;
			initData.pSysMem = s_indexBufferData;
			initData.SysMemPitch = 0;
			initData.SysMemSlicePitch = 0;

			// Create the buffer with the device.
			HRESULT res = device->CreateBuffer(&bufferDesc, &initData, &s_box.m_indexBuffer);
			if (FAILED(res))
			{
				return false;
			}
		}
		return true;
	}

	bool DebugPrims::initPlaneXZ(ID3D11Device* device)
	{
		// 4 vertices * 6 faces
		static constexpr u32 s_vertexCount = 4;
		static DebugVertex s_vertexBufferData[s_vertexCount] =
		{
			// Top
			{v3(-0.5f, 0.0f, -0.5f), v3(0.0f, 1.0f, 0.0f), v2(0.0f, 1.0f)},
			{v3(-0.5f, 0.0f, 0.5f), v3(0.0f, 1.0f, 0.0f), v2(0.0f, 0.0f)},
			{v3(0.5f, 0.0f, 0.5f), v3(0.0f, 1.0f, 0.0f), v2(1.0f, 0.0f)},
			{v3(0.5f, 0.0f, -0.5f), v3(0.0f, 1.0f, 0.0f), v2(1.0f, 1.0f)},

		};
		// 6 faces * 2 triangles * 3 vertices
		static constexpr u32 s_indexCount = 6;
		static u16 s_indexBufferData[s_indexCount] =
		{
			// Front
			0,1,2,
			0,2,3,
		};

		s_planeXZ.m_vertexCount = s_vertexCount;
		s_planeXZ.m_vertexOffset = 0;
		s_planeXZ.m_indexCount = s_indexCount;
		s_planeXZ.m_indexOffset = 0;

		{
			D3D11_BUFFER_DESC bufferDesc;
			bufferDesc.Usage = D3D11_USAGE_DEFAULT;
			bufferDesc.ByteWidth = sizeof(DebugVertex) * s_vertexCount;
			bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			bufferDesc.CPUAccessFlags = 0;
			bufferDesc.MiscFlags = 0;

			// Fill in the subresource data.
			D3D11_SUBRESOURCE_DATA initData;
			initData.pSysMem = s_vertexBufferData;
			initData.SysMemPitch = 0;
			initData.SysMemSlicePitch = 0;

			// Create the vertex buffer.
			HRESULT res = device->CreateBuffer(&bufferDesc, &initData, &s_planeXZ.m_vertexBuffer);
			if (FAILED(res))
			{
				return false;
			}
		}

		{
			D3D11_BUFFER_DESC bufferDesc;
			bufferDesc.Usage = D3D11_USAGE_DEFAULT;
			bufferDesc.ByteWidth = sizeof(u16) * s_indexCount;
			bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
			bufferDesc.CPUAccessFlags = 0;
			bufferDesc.MiscFlags = 0;

			// Define the resource data.
			D3D11_SUBRESOURCE_DATA initData;
			initData.pSysMem = s_indexBufferData;
			initData.SysMemPitch = 0;
			initData.SysMemSlicePitch = 0;

			// Create the buffer with the device.
			HRESULT res = device->CreateBuffer(&bufferDesc, &initData, &s_planeXZ.m_indexBuffer);
			if (FAILED(res))
			{
				return false;
			}
		}
		return true;
	}

	// Icosphere

	struct TriangleIndices
	{
		TriangleIndices(int v1, int v2, int v3) : m_v1(v1), m_v2(v2), m_v3(v3) {}

		s32 m_v1;
		s32 m_v2;
		s32 m_v3;
	};

	// return index of point in the middle of p1 and p2
	static s32 getMiddlePoint(s32 p1, s32 p2, Vector<DebugVertex>& vertices, UMap<s64, s32> cache, f32 radius)
	{
		// first check if we have it already
		bool firstIsSmaller = p1 < p2;
		s64 smallerIndex = firstIsSmaller ? p1 : p2;
		s64 greaterIndex = firstIsSmaller ? p2 : p1;
		s64 key = (smallerIndex << 32) + greaterIndex;

		auto it = cache.find(key);
		if (it != cache.end())
		{
			return it->second;
		}

		// not in cache, calculate it
		v3 point1 = vertices[p1].m_pos;
		v3 point2 = vertices[p2].m_pos;
		v3 middle((point1.x + point2.x) / 2.0f, (point1.y + point2.y) / 2.0f, (point1.z + point2.z) / 2.0f);

		// add vertex makes sure point is on unit sphere
		s32 i = static_cast<s32>(vertices.size());
		vertices.push_back({ glm::normalize(middle) * radius, v3(0.0f), v2(0.0f) });

		cache[key] = i;
		return i;
	}

	bool DebugPrims::createIcoSphere(ID3D11Device* device, f32 radius, s32 recursionLevel, DebugMesh& outMesh)
	{
		Vector<DebugVertex> vertices;
		UMap<s64, s32> middlePointIndexCache;
		int index = 0;

		// create 12 vertices of a icosahedron
		f32 t = (1.0f + glm::sqrt(5.0f)) / 2.0f;

		// Initial vertices
		vertices.push_back({ glm::normalize(v3(-1.0f,  t,  0.0f)) * radius, v3(0.0f), v2(0.0f) });
		vertices.push_back({ glm::normalize(v3(1.0f,  t,  0.0f)) * radius, v3(0.0f), v2(0.0f) });
		vertices.push_back({ glm::normalize(v3(-1.0f, -t,  0.0f)) * radius, v3(0.0f), v2(0.0f) });
		vertices.push_back({ glm::normalize(v3(1.0f, -t,  0.0f)) * radius, v3(0.0f), v2(0.0f) });

		vertices.push_back({ glm::normalize(v3(0.0f, -1.0f,  t)) * radius, v3(0.0f), v2(0.0f) });
		vertices.push_back({ glm::normalize(v3(0.0f,  1.0f,  t)) * radius, v3(0.0f), v2(0.0f) });
		vertices.push_back({ glm::normalize(v3(0.0f, -1.0f, -t)) * radius, v3(0.0f), v2(0.0f) });
		vertices.push_back({ glm::normalize(v3(0.0f,  1.0f, -t)) * radius, v3(0.0f), v2(0.0f) });

		vertices.push_back({ glm::normalize(v3(t,  0.0f, -1.0f)) * radius, v3(0.0f), v2(0.0f) });
		vertices.push_back({ glm::normalize(v3(t,  0.0f,  1.0f)) * radius, v3(0.0f), v2(0.0f) });
		vertices.push_back({ glm::normalize(v3(-t,  0.0f, -1.0f)) * radius, v3(0.0f), v2(0.0f) });
		vertices.push_back({ glm::normalize(v3(-t,  0.0f,  1.0f)) * radius, v3(0.0f), v2(0.0f) });

		// Initial faces
		Vector<TriangleIndices> faces;

		// 5 faces around point 0
		faces.push_back(TriangleIndices(0, 11, 5));
		faces.push_back(TriangleIndices(0, 5, 1));
		faces.push_back(TriangleIndices(0, 1, 7));
		faces.push_back(TriangleIndices(0, 7, 10));
		faces.push_back(TriangleIndices(0, 10, 11));

		// 5 adjacent faces 
		faces.push_back(TriangleIndices(1, 5, 9));
		faces.push_back(TriangleIndices(5, 11, 4));
		faces.push_back(TriangleIndices(11, 10, 2));
		faces.push_back(TriangleIndices(10, 7, 6));
		faces.push_back(TriangleIndices(7, 1, 8));

		// 5 faces around point 3
		faces.push_back(TriangleIndices(3, 9, 4));
		faces.push_back(TriangleIndices(3, 4, 2));
		faces.push_back(TriangleIndices(3, 2, 6));
		faces.push_back(TriangleIndices(3, 6, 8));
		faces.push_back(TriangleIndices(3, 8, 9));

		// 5 adjacent faces 
		faces.push_back(TriangleIndices(4, 9, 5));
		faces.push_back(TriangleIndices(2, 4, 11));
		faces.push_back(TriangleIndices(6, 2, 10));
		faces.push_back(TriangleIndices(8, 6, 7));
		faces.push_back(TriangleIndices(9, 8, 1));

		// refine triangles
		for (s32 i = 0; i < recursionLevel; i++)
		{
			Vector<TriangleIndices> faces2;
			for (const TriangleIndices& tri : faces)
			{
				// replace triangle by 4 triangles
				s32 a = getMiddlePoint(tri.m_v1, tri.m_v2, vertices, middlePointIndexCache, radius);
				s32 b = getMiddlePoint(tri.m_v2, tri.m_v3, vertices, middlePointIndexCache, radius);
				s32 c = getMiddlePoint(tri.m_v3, tri.m_v1, vertices, middlePointIndexCache, radius);

				faces2.push_back(TriangleIndices(tri.m_v1, a, c));
				faces2.push_back(TriangleIndices(tri.m_v2, b, a));
				faces2.push_back(TriangleIndices(tri.m_v3, c, b));
				faces2.push_back(TriangleIndices(a, b, c));
			}
			faces = faces2;
		}

		// Calculate normals and uvs
		for (size_t i = 0; i < vertices.size(); ++i)
		{
			vertices[i].m_normal = glm::normalize(vertices[i].m_pos);
			vertices[i].m_uv = v2(
				glm::clamp((atan2(vertices[i].m_normal.z, vertices[i].m_normal.x) / glm::pi<f32>() + 1.0f) * .5f, 0.0f, 1.0f),
				0.5f - glm::asin(vertices[i].m_normal.y) / glm::pi<f32>());
		}

		Vector<u16> indices;
		indices.resize(faces.size() * 3);
		for (size_t i = 0; i < faces.size(); ++i)
		{
			const TriangleIndices& tri = faces[i];
			indices[i * 3] = tri.m_v1;
			indices[i * 3 + 1] = tri.m_v2;
			indices[i * 3 + 2] = tri.m_v3;
		}

		outMesh.m_indexCount = static_cast<u32>(indices.size());
		outMesh.m_vertexCount = static_cast<u32>(vertices.size());
		outMesh.m_vertexBuffer = RenderResources::createVertexBuffer(device, static_cast<u32>(sizeof(DebugVertex) * vertices.size()), vertices.data());
		outMesh.m_indexBuffer = RenderResources::createIndexBuffer(device, static_cast<u32>(sizeof(u16) * indices.size()), indices.data());
		return outMesh.m_indexBuffer && outMesh.m_vertexBuffer;
	}

	bool DebugPrims::createLineSphere(ID3D11Device* device, u32 res0, u32 res1, DebugMesh& outMesh)
	{
		static constexpr f32 s_radius = 1.0f;
		static constexpr u32 s_linesPerCircle = 20;
		const u32 circleCountPitch = res0;
		const u32 circleCountRoll = res1;
		Vector<DebugVertex> vertices;
		Vector<u16> indices;

		f32 deltaAnglePitch = 180.0f / static_cast<f32>(circleCountPitch);
		f32 deltaAngleRoll = 180.0f / static_cast<f32>(circleCountRoll);
		u16 indexCount(0);
		for (u32 i = 0; i < circleCountPitch; ++i)
		{
			m3 pitchRot = glm::rotate(m4(1.0f), glm::radians(static_cast<f32>(i) * deltaAnglePitch), v3(1.0f, 0.0f, 0.0f));
			for (u32 j = 0; j < circleCountRoll; ++j)
			{
				m3 rot = pitchRot * (m3)glm::rotate(m4(1.0f), glm::radians(static_cast<f32>(j) * deltaAngleRoll), v3(0.0f, 0.0f, 1.0f));
				for (u32 w = 0; w < s_linesPerCircle; ++w)
				{
					f32 rad = glm::radians(360.0f * (static_cast<f32>(w) / static_cast<f32>(s_linesPerCircle)));
					f32 radNext = glm::radians(360.0f * (static_cast<f32>(w + 1) / static_cast<f32>(s_linesPerCircle)));
					vertices.push_back({ rot * s_radius * v3(cos(rad), 0.0f, sin(rad)), v3(0.0f), v2(0.0f) });
					vertices.push_back({ rot * s_radius * v3(cos(radNext), 0.0f, sin(radNext)), v3(0.0f), v2(0.0f) });
					indices.push_back(indexCount++);
					indices.push_back(indexCount++);
				}
			}
		}

		outMesh.m_indexCount = static_cast<u32>(indices.size());
		outMesh.m_vertexCount = static_cast<u32>(vertices.size());
		outMesh.m_vertexBuffer = RenderResources::createVertexBuffer(device, static_cast<u32>(sizeof(DebugVertex) * vertices.size()), vertices.data());
		outMesh.m_indexBuffer = RenderResources::createIndexBuffer(device, static_cast<u32>(sizeof(u16) * indices.size()), indices.data());
		outMesh.m_topology = D3D_PRIMITIVE_TOPOLOGY_LINELIST;
		return outMesh.m_indexBuffer && outMesh.m_vertexBuffer;
	}

	bool DebugPrims::createLineCone(ID3D11Device* device, DebugMesh& outMesh)
	{
		static constexpr f32 s_radius = 1.0f;
		static constexpr u32 s_linesPerCircle = 20;
		static constexpr u32 s_edgeCount = 6;
		Vector<DebugVertex> vertices;
		Vector<u16> indices;

		u16 indexCount(0);
		for (u32 i = 0; i < s_linesPerCircle; ++i)
		{
			f32 rad = glm::radians(360.0f * (static_cast<f32>(i) / static_cast<f32>(s_linesPerCircle)));
			f32 radNext = glm::radians(360.0f * (static_cast<f32>(i + 1) / static_cast<f32>(s_linesPerCircle)));
			vertices.push_back({ v3(cos(rad), sin(rad), 1.0f), v3(0.0f), v2(0.0f) });
			vertices.push_back({ v3(cos(radNext), sin(radNext), 1.0f), v3(0.0f), v2(0.0f) });
			indices.push_back(indexCount++);
			indices.push_back(indexCount++);
		}

		for (u32 i = 0; i < s_edgeCount; ++i)
		{
			f32 rad = glm::radians(360.0f * (static_cast<f32>(i) / static_cast<f32>(s_edgeCount)));
			vertices.push_back({ v3(0.0f), v3(0.0f), v2(0.0f) });
			vertices.push_back({ v3(cos(rad), sin(rad), 1.0f), v3(0.0f), v2(0.0f) });
			indices.push_back(indexCount++);
			indices.push_back(indexCount++);
		}

		outMesh.m_indexCount = static_cast<u32>(indices.size());
		outMesh.m_vertexCount = static_cast<u32>(vertices.size());
		outMesh.m_vertexBuffer = RenderResources::createVertexBuffer(device, static_cast<u32>(sizeof(DebugVertex) * vertices.size()), vertices.data());
		outMesh.m_indexBuffer = RenderResources::createIndexBuffer(device, static_cast<u32>(sizeof(u16) * indices.size()), indices.data());
		outMesh.m_topology = D3D_PRIMITIVE_TOPOLOGY_LINELIST;
		return false;
	}

	// -------------------------------------------------------------------------------------------

	ShaderPipeline::ShaderPipeline()
	{
	}

	ShaderPipeline::~ShaderPipeline()
	{
		if (m_layout)
		{
			m_layout->Release();
		}
		if (m_vertexShader)
		{
			m_vertexShader->Release();
		}
		if (m_fragmentShader)
		{
			m_fragmentShader->Release();
		}
	}

	bool ShaderPipeline::loadGraphicsPipeline(ID3D11Device* device,
		const char* srcRelPath,
		const char* entryVS,
		const char* entryFS,
		D3D11_INPUT_ELEMENT_DESC* vertexAttributes, u32 vertexAttribCount)
	{
		String absPath = Paths::getAssetPath(srcRelPath);
		UniquePtr<char[]> hlslSrc = FileUtils::loadFileContent(absPath.c_str());
		size_t srcSize = strlen(hlslSrc.get());
		if (!hlslSrc)
		{
			return false;
		}
		ID3DBlob* errorMSG = nullptr;
		// Load vertex shader
		{
			ID3DBlob* vertexShaderBlob;
			HRESULT res = D3DCompile(hlslSrc.get(), srcSize, NULL, NULL, NULL, entryVS, "vs_5_0", 0, 0, &vertexShaderBlob, &errorMSG);
			if (FAILED(res))
			{
				OutputDebugStringA((char*)errorMSG->GetBufferPointer());
				errorMSG->Release();
				return false;
			}
			if (device->CreateVertexShader(vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(), NULL, &m_vertexShader) != S_OK)
			{
				vertexShaderBlob->Release();
				return false;
			}
			res = device->CreateInputLayout(vertexAttributes, vertexAttribCount, vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(), &m_layout);
			if (FAILED(res))
			{
				return false;
			}
		}

		// Load Fragment/Pixel shader
		{
			ID3DBlob* pixelShaderBlob;
			HRESULT res = D3DCompile(hlslSrc.get(), srcSize, NULL, NULL, NULL, "mainFS", "ps_5_0", 0, 0, &pixelShaderBlob, &errorMSG);
			if (FAILED(res))
			{
				OutputDebugStringA((char*)errorMSG->GetBufferPointer());
				errorMSG->Release();
				return false;
			}
			if (device->CreatePixelShader(pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize(), NULL, &m_fragmentShader) != S_OK)
			{
				pixelShaderBlob->Release();
				return false;
			}
			pixelShaderBlob->Release();
		}
		return true;
	}

	void ShaderPipeline::bind(ID3D11DeviceContext* ctx)
	{
		if (m_layout)
		{
			ctx->IASetInputLayout(m_layout);
		}

		if (m_vertexShader)
		{
			ctx->VSSetShader(m_vertexShader, nullptr, 0);
		}
		if (m_fragmentShader)
		{
			ctx->PSSetShader(m_fragmentShader, nullptr, 0);
		}
	}

	// -----------------------------------------------------------------------------------------

	bool RenderResources::updateMappableCBData(ID3D11DeviceContext* ctx, ID3D11Buffer* cBuffer, const void* data, u32 size)
	{
		D3D11_MAPPED_SUBRESOURCE mappedData;
		if (FAILED(ctx->Map(cBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData)))
		{
			printf("Failed to map CB");
			return false;
		}
		memcpy(mappedData.pData, data, size);
		ctx->Unmap(cBuffer, 0);
		return true;
	}

	bool RenderResources::loadTexture2D(ID3D11Device* device, ID3D11DeviceContext* ctx, const char* fileRelPath, DXGI_FORMAT format, Texture2D& outTexture)
	{
		String absPath(Paths::getAssetPath(fileRelPath));
		s32 x, y, n;
		static constexpr u32 s_bytesPerTexel = 4;
		unsigned char* data = stbi_load(absPath.c_str(), &x, &y, &n, s_bytesPerTexel);

		if (!data)
		{
			printf("Failed to load resource %s", fileRelPath);
			return false;
		}

		u32 desiredMipMaps = static_cast<u32>(log2f(static_cast<f32>(glm::min(x, y))));;

		D3D11_TEXTURE2D_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
		desc.Width = x;
		desc.Height = y;
		desc.MipLevels = 0;
		desc.ArraySize = 1;
		desc.Format = format;
		desc.SampleDesc.Count = 1;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
		desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
		// Allow staging texture to be visible
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.CPUAccessFlags = 0;

		D3D11_SUBRESOURCE_DATA initialData;
		initialData.pSysMem = data;
		initialData.SysMemPitch = s_bytesPerTexel * x;
		initialData.SysMemSlicePitch = 0;

		HRESULT res = device->CreateTexture2D(&desc, nullptr, &outTexture.m_texture);
		if (FAILED(res))
		{
			printf("Failed to createstaging texture resource");
			stbi_image_free(data);
			return false;
		}
		ctx->UpdateSubresource(outTexture.m_texture, 0, nullptr, data, s_bytesPerTexel * x, 0);

		D3D11_SHADER_RESOURCE_VIEW_DESC descSRV;
		ZeroMemory(&descSRV, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
		descSRV.Format = format;
		descSRV.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		descSRV.Texture2D.MipLevels = -1;
		descSRV.Texture2D.MostDetailedMip = 0;

		res = device->CreateShaderResourceView(outTexture.m_texture, &descSRV, &outTexture.m_SRV);
		if (FAILED(res))
		{
			printf("Failed to create SRV for staging texture");
			stbi_image_free(data);
			return false;
		}
		ctx->GenerateMips(outTexture.m_SRV);
		stbi_image_free(data);
		return true;
	}

	bool RenderResources::createTexture2D(ID3D11Device* device, ID3D11DeviceContext* ctx, u32 w, u32 h, u32 texelSize, DXGI_FORMAT format, const void* data, Texture2D& outTexture)
	{
		u32 desiredMipMaps = static_cast<u32>(log2f(static_cast<f32>(glm::min(w, h))));;

		D3D11_TEXTURE2D_DESC desc;
		ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
		desc.Width = w;
		desc.Height = h;
		desc.MipLevels = 0;
		desc.ArraySize = 1;
		desc.Format = format;
		desc.SampleDesc.Count = 1;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
		desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;
		// Allow staging texture to be visible
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.CPUAccessFlags = 0;

		D3D11_SUBRESOURCE_DATA initialData;
		initialData.pSysMem = data;
		initialData.SysMemPitch = texelSize * w * h;
		initialData.SysMemSlicePitch = 0;

		HRESULT res = device->CreateTexture2D(&desc, nullptr, &outTexture.m_texture);
		ctx->UpdateSubresource(outTexture.m_texture, 0, nullptr, data, texelSize * w, 0);

		D3D11_SHADER_RESOURCE_VIEW_DESC descSRV;
		ZeroMemory(&descSRV, sizeof(D3D11_SHADER_RESOURCE_VIEW_DESC));
		descSRV.Format = format;
		descSRV.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		descSRV.Texture2D.MipLevels = -1;
		descSRV.Texture2D.MostDetailedMip = 0;

		res = device->CreateShaderResourceView(outTexture.m_texture, &descSRV, &outTexture.m_SRV);
		ctx->GenerateMips(outTexture.m_SRV);
		return true;
	}

	ID3D11SamplerState* RenderResources::createSamplerState(ID3D11Device* device, D3D11_FILTER filter, D3D11_TEXTURE_ADDRESS_MODE addressMode)
	{
		ID3D11SamplerState* sampler = nullptr;
		D3D11_SAMPLER_DESC desc;
		ZeroMemory(&desc, sizeof(ID3D11SamplerState));
		desc.Filter = filter;
		desc.AddressU = addressMode;
		desc.AddressV = addressMode;
		desc.AddressW = addressMode;
		desc.MipLODBias = 0.0f;
		desc.MaxAnisotropy = 1;
		desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		desc.MinLOD = 0;
		desc.MaxLOD = D3D11_FLOAT32_MAX;
		device->CreateSamplerState(&desc, &sampler);
		return sampler;
	}

	bool RenderResources::createDepthAttachment(ID3D11Device* device, u32 width, u32 height, DXGI_FORMAT format, DepthAttachment& outDepthAttachment)
	{
		D3D11_TEXTURE2D_DESC descDepth;
		descDepth.Width = width;
		descDepth.Height = height;
		descDepth.MipLevels = 1;
		descDepth.ArraySize = 1;
		descDepth.Format = format;
		descDepth.SampleDesc.Count = 1;
		descDepth.SampleDesc.Quality = 0;
		descDepth.Usage = D3D11_USAGE_DEFAULT;
		descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		descDepth.CPUAccessFlags = 0;
		descDepth.MiscFlags = 0;
		HRESULT res = device->CreateTexture2D(&descDepth, NULL, &outDepthAttachment.m_depthStencilTexture);
		if (FAILED(res))
		{
			printf("Failed to create texture resource.");
			return false;
		}

		D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
		ZeroMemory(&descDSV, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
		descDSV.Format = format;
		descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
		descDSV.Texture2D.MipSlice = 0;

		// Create the depth stencil view
		res = device->CreateDepthStencilView(outDepthAttachment.m_depthStencilTexture, // Depth stencil texture
			&descDSV, // Depth stencil desc
			&outDepthAttachment.m_depthStencilView);  // [out] Depth stencil view
		if (FAILED(res))
		{
			printf("Failed to create Depth Stencil view.");
			return false;
		}
		return true;
	}

	ID3D11DepthStencilState* RenderResources::createDepthStencilState(ID3D11Device* device, D3D11_COMPARISON_FUNC func)
	{
		D3D11_DEPTH_STENCIL_DESC dsDesc;

		// Depth test parameters
		dsDesc.DepthEnable = true;
		dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		dsDesc.DepthFunc = func;

		// Stencil test parameters
		dsDesc.StencilEnable = false;

		// Create depth stencil state
		ID3D11DepthStencilState* pDSState;
		HRESULT res = device->CreateDepthStencilState(&dsDesc, &pDSState);
		if (FAILED(res))
		{
			printf("Failed to create depth stencil state");
		}
		return pDSState;
	}

	ID3D11Buffer* RenderResources::createVertexBuffer(ID3D11Device* device, u32 bufferSize, void* initialData)
	{
		D3D11_BUFFER_DESC bufferDesc;
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		bufferDesc.ByteWidth = bufferSize;
		bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bufferDesc.CPUAccessFlags = 0;
		bufferDesc.MiscFlags = 0;

		// Fill in the subresource data.
		D3D11_SUBRESOURCE_DATA initData;
		initData.pSysMem = initialData;
		initData.SysMemPitch = 0;
		initData.SysMemSlicePitch = 0;

		// Create the vertex buffer.
		ID3D11Buffer* result(nullptr);
		HRESULT res = device->CreateBuffer(&bufferDesc, &initData, &result);
		if (FAILED(res))
		{
			printf("Failed to create vertex buffer");
		}
		return result;
	}

	ID3D11Buffer* RenderResources::createIndexBuffer(ID3D11Device* device, u32 bufferSize, void* initialData)
	{
		D3D11_BUFFER_DESC bufferDesc;
		bufferDesc.Usage = D3D11_USAGE_DEFAULT;
		bufferDesc.ByteWidth = bufferSize;
		bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bufferDesc.CPUAccessFlags = 0;
		bufferDesc.MiscFlags = 0;

		// Define the resource data.
		D3D11_SUBRESOURCE_DATA initData;
		initData.pSysMem = initialData;
		initData.SysMemPitch = 0;
		initData.SysMemSlicePitch = 0;

		// Create the buffer with the device.
		ID3D11Buffer* result(nullptr);
		HRESULT res = device->CreateBuffer(&bufferDesc, &initData, &result);
		if (FAILED(res))
		{
			printf("Failed to create index buffer");
		}
		return result;
	}

	ID3D11Buffer* RenderResources::createConstantBufferRaw(ID3D11Device* device, const void* data, const u32 size)
	{
		// Fill in a buffer description.
		D3D11_BUFFER_DESC cbDesc;
		cbDesc.ByteWidth = size;
		cbDesc.Usage = D3D11_USAGE_DYNAMIC;
		cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		cbDesc.MiscFlags = 0;
		cbDesc.StructureByteStride = 0;

		// Fill in the subresource data.
		D3D11_SUBRESOURCE_DATA InitData;
		InitData.pSysMem = data;
		InitData.SysMemPitch = 0;
		InitData.SysMemSlicePitch = 0;

		// Create the buffer.
		ID3D11Buffer* cb = nullptr;
		if (!FAILED(device->CreateBuffer(&cbDesc, &InitData, &cb)))
		{
			return cb;
		}
		return nullptr;
	}

	// -----------------------------------------------------------------------------------------

	static ImGuizmo::OPERATION g_mCurrentGizmoOperation(ImGuizmo::TRANSLATE);
	bool g_useWindow = false;
	float g_camDistance = 1.f;
	void Gizmo3D::editTransform(f32* cameraView, f32* cameraProjection, f32* matrix, bool editTransformDecomposition)
	{
		static ImGuizmo::MODE mCurrentGizmoMode(ImGuizmo::LOCAL);
		static bool useSnap = false;
		static float snap[3] = { 1.f, 1.f, 1.f };
		static float bounds[] = { -0.5f, -0.5f, -0.5f, 0.5f, 0.5f, 0.5f };
		static float boundsSnap[] = { 0.1f, 0.1f, 0.1f };
		static bool boundSizing = false;
		static bool boundSizingSnap = false;

		if (editTransformDecomposition)
		{
			if (ImGui::IsKeyPressed(90))
				g_mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
			if (ImGui::IsKeyPressed(69))
				g_mCurrentGizmoOperation = ImGuizmo::ROTATE;
			if (ImGui::IsKeyPressed(82)) // r Key
				g_mCurrentGizmoOperation = ImGuizmo::SCALE;
			if (ImGui::RadioButton("Translate", g_mCurrentGizmoOperation == ImGuizmo::TRANSLATE))
				g_mCurrentGizmoOperation = ImGuizmo::TRANSLATE;
			ImGui::SameLine();
			if (ImGui::RadioButton("Rotate", g_mCurrentGizmoOperation == ImGuizmo::ROTATE))
				g_mCurrentGizmoOperation = ImGuizmo::ROTATE;
			ImGui::SameLine();
			if (ImGui::RadioButton("Scale", g_mCurrentGizmoOperation == ImGuizmo::SCALE))
				g_mCurrentGizmoOperation = ImGuizmo::SCALE;
			float matrixTranslation[3], matrixRotation[3], matrixScale[3];
			ImGuizmo::DecomposeMatrixToComponents(matrix, matrixTranslation, matrixRotation, matrixScale);
			ImGui::InputFloat3("Tr", matrixTranslation);
			ImGui::InputFloat3("Rt", matrixRotation);
			ImGui::InputFloat3("Sc", matrixScale);
			ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, matrix);

			if (g_mCurrentGizmoOperation != ImGuizmo::SCALE)
			{
				if (ImGui::RadioButton("Local", mCurrentGizmoMode == ImGuizmo::LOCAL))
					mCurrentGizmoMode = ImGuizmo::LOCAL;
				ImGui::SameLine();
				if (ImGui::RadioButton("World", mCurrentGizmoMode == ImGuizmo::WORLD))
					mCurrentGizmoMode = ImGuizmo::WORLD;
			}
			if (ImGui::IsKeyPressed(83))
				useSnap = !useSnap;
			ImGui::Checkbox("", &useSnap);
			ImGui::SameLine();

			switch (g_mCurrentGizmoOperation)
			{
			case ImGuizmo::TRANSLATE:
				ImGui::InputFloat3("Snap", &snap[0]);
				break;
			case ImGuizmo::ROTATE:
				ImGui::InputFloat("Angle Snap", &snap[0]);
				break;
			case ImGuizmo::SCALE:
				ImGui::InputFloat("Scale Snap", &snap[0]);
				break;
			}
			ImGui::Checkbox("Bound Sizing", &boundSizing);
			if (boundSizing)
			{
				ImGui::PushID(3);
				ImGui::Checkbox("", &boundSizingSnap);
				ImGui::SameLine();
				ImGui::InputFloat3("Snap", boundsSnap);
				ImGui::PopID();
			}
		}

		ImGuiIO& io = ImGui::GetIO();
		float viewManipulateRight = io.DisplaySize.x;
		float viewManipulateTop = 0;
		if (g_useWindow)
		{
			ImGui::SetNextWindowSize(ImVec2(800, 400));
			ImGui::SetNextWindowPos(ImVec2(400, 20));
			ImGui::PushStyleColor(ImGuiCol_WindowBg, (ImVec4)ImColor(0.35f, 0.3f, 0.3f));
			ImGui::Begin("Gizmo", 0, ImGuiWindowFlags_NoMove);
			ImGuizmo::SetDrawlist();
			float windowWidth = (float)ImGui::GetWindowWidth();
			float windowHeight = (float)ImGui::GetWindowHeight();
			ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, windowWidth, windowHeight);
			viewManipulateRight = ImGui::GetWindowPos().x + windowWidth;
			viewManipulateTop = ImGui::GetWindowPos().y;
		}
		else
		{
			ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);
		}

		//ImGuizmo::DrawGrid(cameraView, cameraProjection, identityMatrix, 100.f);
		//ImGuizmo::DrawCubes(cameraView, cameraProjection, &objectMatrix[0][0], gizmoCount);
		ImGuizmo::Manipulate(cameraView, cameraProjection, g_mCurrentGizmoOperation, mCurrentGizmoMode, matrix, NULL, useSnap ? &snap[0] : NULL, boundSizing ? bounds : NULL, boundSizingSnap ? boundsSnap : NULL);

		ImGuizmo::ViewManipulate(cameraView, g_camDistance, ImVec2(viewManipulateRight - 128, viewManipulateTop), ImVec2(128, 128), 0x10101010);

		if (g_useWindow)
		{
			ImGui::End();
			ImGui::PopStyleColor(1);
		}
	}


	void Gizmo3D::editRotation(f32* cameraView, f32* cameraProjection, f32* matrix, bool editTransformDecomposition)
	{
		static ImGuizmo::MODE mCurrentGizmoMode(ImGuizmo::LOCAL);

		g_mCurrentGizmoOperation = ImGuizmo::ROTATE;
		if (editTransformDecomposition)
		{
			float matrixTranslation[3], matrixRotation[3], matrixScale[3];
			ImGuizmo::DecomposeMatrixToComponents(matrix, matrixTranslation, matrixRotation, matrixScale);
			ImGui::InputFloat3("Rt", matrixRotation);
			ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, matrix);

			if (g_mCurrentGizmoOperation != ImGuizmo::SCALE)
			{
				if (ImGui::RadioButton("Local", mCurrentGizmoMode == ImGuizmo::LOCAL))
					mCurrentGizmoMode = ImGuizmo::LOCAL;
				ImGui::SameLine();
				if (ImGui::RadioButton("World", mCurrentGizmoMode == ImGuizmo::WORLD))
					mCurrentGizmoMode = ImGuizmo::WORLD;
			}
		}

		ImGuiIO& io = ImGui::GetIO();
		float viewManipulateRight = io.DisplaySize.x;
		float viewManipulateTop = 0;
		ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);

		//ImGuizmo::DrawGrid(cameraView, cameraProjection, identityMatrix, 100.f);
		//ImGuizmo::DrawCubes(cameraView, cameraProjection, &objectMatrix[0][0], gizmoCount);
		ImGuizmo::Manipulate(cameraView, cameraProjection, g_mCurrentGizmoOperation, mCurrentGizmoMode, matrix, nullptr, nullptr, nullptr, nullptr);

		//ImGuizmo::ViewManipulate(cameraView, g_camDistance, ImVec2(viewManipulateRight - 128, viewManipulateTop), ImVec2(128, 128), 0x10101010);
	}

	void Gizmo3D::drawTranslationGizmo(const m4& cameraView, const m4& cameraProjection, m4& model)
	{
		ImGuiIO& io = ImGui::GetIO();
		float viewManipulateRight = io.DisplaySize.x;
		float viewManipulateTop = 0;
		ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);

		ImGuizmo::Manipulate(&cameraView[0][0], &cameraProjection[0][0], ImGuizmo::TRANSLATE, ImGuizmo::LOCAL, &model[0][0], NULL, NULL, nullptr, nullptr);
	}

	void Gizmo3D::drawRotationGizmo(const m4& cameraView, const m4& cameraProjection, m4& model)
	{
		ImGuiIO& io = ImGui::GetIO();
		float viewManipulateRight = io.DisplaySize.x;
		float viewManipulateTop = 0;
		ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);

		ImGuizmo::Manipulate(&cameraView[0][0], &cameraProjection[0][0], ImGuizmo::ROTATE, ImGuizmo::LOCAL, &model[0][0], NULL, NULL, nullptr, nullptr);
	}

	void Gizmo3D::drawScaleGizmo(const m4& cameraView, const m4& cameraProjection, m4& model)
	{
		ImGuiIO& io = ImGui::GetIO();
		float viewManipulateRight = io.DisplaySize.x;
		float viewManipulateTop = 0;
		ImGuizmo::SetRect(0, 0, io.DisplaySize.x, io.DisplaySize.y);

		ImGuizmo::Manipulate(&cameraView[0][0], &cameraProjection[0][0], ImGuizmo::SCALE, ImGuizmo::LOCAL, &model[0][0], NULL, NULL, nullptr, nullptr);
	}

	// -----------------------------------------------------------------------------------------

	static String s_PosAttribName = "POSITION";
	static String s_NormalAttribName = "NORMAL";
	static String s_TangentAttribName = "TANGENT";
	static String s_UvAttribName = "TEXCOORD_0";

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
}