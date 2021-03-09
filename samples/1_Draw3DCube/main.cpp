#include "framework/framework.h"

struct GraphicsPipeline 
{
	ID3D11InputLayout* m_layout = nullptr;
	ID3D11VertexShader* m_vertexShader = nullptr;
	ID3D11PixelShader* m_fragmentShader = nullptr;
};

struct Vertex 
{
	v3 m_pos;
	v3 m_normal;
};

struct FrameMatrices
{
	m4 view;
	m4 viewProj;
	m4 modelViewProj;
	m4 invViewProj;
};

struct Meshlet 
{
	ID3D11Buffer* m_vertexBuffer = nullptr;
	ID3D11Buffer* m_indexBuffer = nullptr;
	u32 m_vertexCount = 0;
	u32 m_vertexOffset = 0;
	u32 m_indexCount = 0;
	u32 m_indexOffset = 0;
};

bool loadShader(const char* relPath, const framework::Window& window, GraphicsPipeline& outPipeline) 
{
	UniquePtr<char[]> hlslSrc = framework::FileUtils::loadFileContent(relPath);
	size_t srcSize = strlen(hlslSrc.get());
	if (!hlslSrc) 
	{
		return false;
	}
	ID3DBlob* errorMSG = nullptr;
	// Load vertex shader
	{
		ID3DBlob* vertexShaderBlob;
		HRESULT res = D3DCompile(hlslSrc.get(), srcSize, NULL, NULL, NULL, "mainVS", "vs_5_0", 0, 0, &vertexShaderBlob, &errorMSG);
		if (FAILED(res))
		{
			OutputDebugStringA((char*)errorMSG->GetBufferPointer());
			errorMSG->Release();
			return false;
		}
		if (window.getDevice()->CreateVertexShader(vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(), NULL, &outPipeline.m_vertexShader) != S_OK)
		{
			vertexShaderBlob->Release();
			return false;
		}

		D3D11_INPUT_ELEMENT_DESC vertexLayout[2];
		ZeroMemory(vertexLayout, 2 * sizeof(D3D11_INPUT_ELEMENT_DESC));
		vertexLayout[0].SemanticName = "POSITION";
		vertexLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		vertexLayout[0].InputSlot = 0;
		vertexLayout[0].AlignedByteOffset = u32(offsetof(Vertex, m_pos));
		vertexLayout[0].InstanceDataStepRate = D3D11_INPUT_PER_VERTEX_DATA;
		vertexLayout[1].SemanticName = "NORMAL";
		vertexLayout[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
		vertexLayout[1].InputSlot = 0;
		vertexLayout[1].AlignedByteOffset = u32(offsetof(Vertex, m_normal));
		vertexLayout[1].InstanceDataStepRate = D3D11_INPUT_PER_VERTEX_DATA;
		if (FAILED(window.getDevice()->CreateInputLayout(vertexLayout, 2, vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(), &outPipeline.m_layout)))
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
			OutputDebugStringA( (char*)errorMSG->GetBufferPointer() );
            errorMSG->Release();
			return false;
		}
		if (window.getDevice()->CreatePixelShader(pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize(), NULL, &outPipeline.m_fragmentShader) != S_OK)
		{
			pixelShaderBlob->Release();
			return false;
		}
		pixelShaderBlob->Release();
	}
	return true;
}

bool createCube(ID3D11Device* device, Meshlet& outMeshlet) 
{
	// 4 vertices * 6 faces
	static constexpr u32 s_vertexCount = 4 * 6;
	static Vertex s_vertexBufferData[s_vertexCount] =
	{
		// Front
		{v3(-0.5f, 0.5f, 0.5f), v3(0.0f, 0.0f, 1.0f)}, 
		{v3(-0.5f, -0.5f, 0.5f), v3(0.0f, 0.0f, 1.0f)},
		{v3(0.5f, -0.5f, 0.5f), v3(0.0f, 0.0f, 1.0f)},
		{v3(0.5f, 0.5f, 0.5f), v3(0.0f, 0.0f, 1.0f)},
		// Right
		{v3(0.5f, 0.5f, 0.5f), v3(1.0f, 0.0f, 0.0f)}, 
		{v3(0.5f, -0.5f, 0.5f), v3(1.0f, 0.0f, 0.0f)},
		{v3(0.5f, -0.5f, -0.5f), v3(1.0f, 0.0f, 0.0f)},
		{v3(0.5f, 0.5f, -0.5f), v3(1.0f, 0.0f, 0.0f)},
		// Back
		{v3(0.5f, 0.5f, -0.5f), v3(0.0f, 0.0f, -1.0f)}, 
		{v3(0.5f, -0.5f, -0.5f), v3(0.0f, 0.0f, -1.0f)},
		{v3(-0.5f, -0.5f, -0.5f), v3(0.0f, 0.0f, -1.0f)},
		{v3(-0.5f, 0.5f, -0.5f), v3(0.0f, 0.0f, -1.0f)},
		// Left
		{v3(-0.5f, 0.5f, -0.5f), v3(-1.0f, 0.0f, 0.0f)}, 
		{v3(-0.5f, -0.5f, -0.5f), v3(-1.0f, 0.0f, 0.0f)},
		{v3(-0.5f, -0.5f, 0.5f), v3(-1.0f, 0.0f, 0.0f)},
		{v3(-0.5f, 0.5f, 0.5f), v3(-1.0f, 0.0f, 0.0f)},
		// Top
		{v3(-0.5f, 0.5f, -0.5f), v3(0.0f, 1.0f, 0.0f)}, 
		{v3(-0.5f, 0.5f, 0.5f), v3(0.0f, 1.0f, 0.0f)},
		{v3(0.5f, 0.5f, 0.5f), v3(0.0f, 1.0f, 0.0f)},
		{v3(0.5f, 0.5f, -0.5f), v3(0.0f, 1.0f, 0.0f)},
		// Bottom
		{v3(-0.5f, -0.5f, 0.5f), v3(0.0f, -1.0f, 0.0f)}, 
		{v3(-0.5f, -0.5f, -0.5f), v3(0.0f, -1.0f, 0.0f)},
		{v3(0.5f, -0.5f, -0.5f), v3(0.0f, -1.0f, 0.0f)},
		{v3(0.5f, -0.5f, 0.5f), v3(0.0f, -1.0f, 0.0f)},
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

	outMeshlet.m_vertexCount = s_vertexCount;
	outMeshlet.m_indexCount = s_indexCount;
	outMeshlet.m_vertexOffset = 0;
	outMeshlet.m_indexOffset = 0;

	{
		D3D11_BUFFER_DESC bufferDesc;
		bufferDesc.Usage            = D3D11_USAGE_DEFAULT;
		bufferDesc.ByteWidth        = sizeof( Vertex ) * s_vertexCount;
		bufferDesc.BindFlags        = D3D11_BIND_VERTEX_BUFFER;
		bufferDesc.CPUAccessFlags   = 0;
		bufferDesc.MiscFlags        = 0;

		// Fill in the subresource data.
		D3D11_SUBRESOURCE_DATA initData;
		initData.pSysMem = s_vertexBufferData;
		initData.SysMemPitch = 0;
		initData.SysMemSlicePitch = 0;

		// Create the vertex buffer.
		HRESULT res = device->CreateBuffer( &bufferDesc, &initData, &outMeshlet.m_vertexBuffer );
		if (FAILED(res)) 
		{
			return false;
		}
	}

	{
		D3D11_BUFFER_DESC bufferDesc;
		bufferDesc.Usage           = D3D11_USAGE_DEFAULT;
		bufferDesc.ByteWidth       = sizeof( u16 ) * s_indexCount;
		bufferDesc.BindFlags       = D3D11_BIND_INDEX_BUFFER;
		bufferDesc.CPUAccessFlags  = 0;
		bufferDesc.MiscFlags       = 0;

		// Define the resource data.
		D3D11_SUBRESOURCE_DATA initData;
		initData.pSysMem = s_indexBufferData;
		initData.SysMemPitch = 0;
		initData.SysMemSlicePitch = 0;

		// Create the buffer with the device.
		HRESULT res = device->CreateBuffer( &bufferDesc, &initData, &outMeshlet.m_indexBuffer );
		if (FAILED(res)) 
		{
			return false;
		}
	}
	return true;
}

ID3D11Buffer* createCameraMatricesConstantBuffer(ID3D11Device* device, const FrameMatrices& matricesData)
{
	// Fill in a buffer description.
	D3D11_BUFFER_DESC cbDesc;
	cbDesc.ByteWidth = sizeof(FrameMatrices);
	cbDesc.Usage = D3D11_USAGE_DYNAMIC;
	cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	cbDesc.MiscFlags = 0;
	cbDesc.StructureByteStride = 0;

	// Fill in the subresource data.
	D3D11_SUBRESOURCE_DATA InitData;
	InitData.pSysMem = &matricesData;
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

int main(int argc, char** argv)
{
	const u32 width = 1280;
	const u32 height = 720;
	static framework::Window s_window(argc, argv);
	s_window.init("1_Draw3DCube", width, height);
	ID3D11Device* device = s_window.getDevice();
	ID3D11DeviceContext* ctx = s_window.getCtx();

	GraphicsPipeline pipeline;
	if (!loadShader("./shaders/1_DisplayVertexNormal.hlsl", s_window, pipeline)) 
	{
		printf("Failed to load shader source.");
		return 1;
	}

	Meshlet meshlet;
	if (!createCube(device, meshlet)) 
	{
		printf("Failed to create geometry");
		return 1;
	}

	v3 camPos(0.0f, 2.5f, 3.0f);
	v3 boxRotAngles(0.0f, 45.0f, 0.0f);
	v3 boxPos(0.0f);
	v3 boxScale(1.0f);

	FrameMatrices matrices;
	m4 proj = glm::perspective(glm::radians(60.0f), f32(width) / f32(height), 0.1f, 1000.0f);
	matrices.view = glm::lookAt(camPos, v3(0.0f), v3(0.0f, 1.0f, 0.0f));
	matrices.viewProj = proj * matrices.view;
	matrices.modelViewProj = matrices.viewProj * glm::rotate(m4(1.0f), glm::radians(boxRotAngles.y), v3(0.0f, 1.0f, 0.0f));
	matrices.invViewProj = glm::inverse(matrices.viewProj);
	ID3D11Buffer* matricesCB = createCameraMatricesConstantBuffer(device, matrices);
	if (!matricesCB) 
	{
		printf("Failed to create matrices Constant Buffer");
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

	f64 lastStamp(0.0);
	f64 framerate = 0.0f;
	f64 elapsedTime = 0.0f;

	while (s_window.update())
	{
		// Do some UI
		if (ImGui::Begin("Controls")) 
		{
			ImGui::DragFloat3("Cam Pos", &camPos[0], 0.01f);
			ImGui::DragFloat3("Box rotation (eulers)", &boxRotAngles[0]);
			ImGui::DragFloat3("Box scale", &boxScale[0], 0.01f);
			ImGui::DragFloat3("Box position", &boxPos[0], 0.1f);
			ImGui::End();
		}
		// Update CB
		m4 translation = glm::translate(m4(1.0f), boxPos);
		m4 scale = glm::scale(m4(1.0f), boxScale);
		m4 rotation =
			glm::rotate(m4(1.0f), glm::radians(boxRotAngles.x), v3(1.0f, 0.0f, 0.0f)) *
			glm::rotate(m4(1.0f), glm::radians(boxRotAngles.y), v3(0.0f, 1.0f, 0.0f)) *
			glm::rotate(m4(1.0f), glm::radians(boxRotAngles.z), v3(0.0f, 0.0f, 1.0f));
		m4 model = rotation * scale * translation;
		matrices.view = glm::lookAt(camPos, v3(0.0f), v3(0.0f, 1.0f, 0.0f));
		matrices.viewProj = proj * matrices.view;
		matrices.modelViewProj = matrices.viewProj * model;
		matrices.invViewProj = glm::inverse(matrices.viewProj);

		// Map to CPU (this is a bad approach for this)
		D3D11_MAPPED_SUBRESOURCE mappedData;
		if (FAILED(ctx->Map(matricesCB, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedData))) 
		{
			printf("Failed to map CB");
			return 1;
		}
		memcpy(mappedData.pData, &matrices, sizeof(FrameMatrices));
		ctx->Unmap(matricesCB, 0);

		// Calculate elapsed time/framerate/...
		f64 currStamp = framework::Time::getTimeStampS();
		elapsedTime = currStamp - lastStamp;
		lastStamp = currStamp;
		framerate = 1.0 / elapsedTime;


		ID3D11DeviceContext* ctx = s_window.getCtx();
		ID3D11RenderTargetView* backBuffer = s_window.getBackBuffer();

		// Set the back buffer as our RenderTarget
		ctx->OMSetRenderTargets(1, &backBuffer, nullptr);

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

		// Bind shaders and draw (Minimum vertex and fragment shaders needed) 
		ctx->VSSetShader(pipeline.m_vertexShader, nullptr, 0);
		ctx->PSSetShader(pipeline.m_fragmentShader, nullptr, 0);

		// Bind vertex and index buffers
		u32 offset = meshlet.m_vertexOffset;
		u32 vertexStride = static_cast<u32>(sizeof(Vertex));
		
		// Bind input layout, vertex buffer and index buffer
		ctx->IASetInputLayout(pipeline.m_layout);
		ctx->IASetVertexBuffers(0, 1, &meshlet.m_vertexBuffer, &vertexStride, &offset);
		ctx->IASetIndexBuffer(meshlet.m_indexBuffer, DXGI_FORMAT_R16_UINT, meshlet.m_indexOffset);
		ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// Bind Constant buffer
		ctx->VSSetConstantBuffers(0, 1, &matricesCB);

		// DrawIndexed as we are using index buffer
		ctx->DrawIndexed(meshlet.m_indexCount, 0, 0);

		s_window.present();
	}

	return 0;
}