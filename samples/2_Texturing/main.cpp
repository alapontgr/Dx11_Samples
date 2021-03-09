#include "framework/framework.h"

#define ENABLE_DEVICE_DEBUG false

struct FrameDataCB
{
	m4 view;
	m4 viewProj;
	m4 invViewProj;
	v2 uvOffset;
	v2 uvScale;
};

struct DrawcallDataCB 
{
	m4 m_model;
};

bool loadShader(ID3D11Device* device, const char* relPath, framework::ShaderPipeline& outShader) 
{
	static constexpr u32 s_vertexAttribCount = 3;
	D3D11_INPUT_ELEMENT_DESC vertexLayout[s_vertexAttribCount];
	ZeroMemory(vertexLayout, s_vertexAttribCount * sizeof(D3D11_INPUT_ELEMENT_DESC));
	vertexLayout[0].SemanticName = "POSITION";
	vertexLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexLayout[0].InputSlot = 0;
	vertexLayout[0].AlignedByteOffset = u32(offsetof(framework::DebugVertex, m_pos));
	vertexLayout[0].InstanceDataStepRate = D3D11_INPUT_PER_VERTEX_DATA;
	vertexLayout[1].SemanticName = "NORMAL";
	vertexLayout[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexLayout[1].InputSlot = 0;
	vertexLayout[1].AlignedByteOffset = u32(offsetof(framework::DebugVertex, m_normal));
	vertexLayout[1].InstanceDataStepRate = D3D11_INPUT_PER_VERTEX_DATA;
	vertexLayout[2].SemanticName = "TEXCOORD";
	vertexLayout[2].Format = DXGI_FORMAT_R32G32_FLOAT;
	vertexLayout[2].InputSlot = 0;
	vertexLayout[2].AlignedByteOffset = u32(offsetof(framework::DebugVertex, m_uv));
	vertexLayout[2].InstanceDataStepRate = D3D11_INPUT_PER_VERTEX_DATA;
	return outShader.loadGraphicsPipeline(device, relPath, "mainVS", "mainFS", vertexLayout, s_vertexAttribCount);
}

static void updateFrameCB(ID3D11DeviceContext* ctx, ID3D11Buffer* cBuffer, const framework::Camera& cam, const v2& uvOffset, const v2& uvScale) 
{
	FrameDataCB frameCBData;
	frameCBData.view = cam.getView();
	frameCBData.viewProj = cam.getViewProj();
	frameCBData.invViewProj = cam.getInvViewProj();
	frameCBData.uvOffset = uvOffset;
	frameCBData.uvScale = uvScale;
	framework::RenderResources::updateMappableCBData(ctx, cBuffer, &frameCBData, sizeof(FrameDataCB));
}

static void updateBatchCB(ID3D11DeviceContext* ctx, ID3D11Buffer* cBuffer, const m4& model) 
{
	DrawcallDataCB drawcallCB;
	drawcallCB.m_model = model;
	framework::RenderResources::updateMappableCBData(ctx, cBuffer, &drawcallCB, sizeof(DrawcallDataCB));
}

// -----------------------------------------------------------------------------------------------

struct Tex2D 
{
	~Tex2D() 
	{
		if (m_SRV) 
		{
			m_SRV->Release();
		}
		if (m_texture) 
		{
			m_texture->Release();
		}
	}

	ID3D11Texture2D* m_texture = nullptr;
	ID3D11ShaderResourceView* m_SRV = nullptr;
};

bool loadTexture2D(ID3D11Device* device, ID3D11DeviceContext* ctx, const char* fileRelPath, DXGI_FORMAT format, Tex2D& outTexture) 
{
	String absPath(framework::Paths::getAssetPath(fileRelPath));
	s32 x,y,n;
	static constexpr u32 s_bytesPerTexel = 4;
	unsigned char *data = stbi_load(absPath.c_str(), &x, &y, &n, s_bytesPerTexel);

	if (!data) 
	{
		printf("Failed to load resource %s", fileRelPath);
		return false;
	}

	D3D11_TEXTURE2D_DESC desc;
	ZeroMemory(&desc, sizeof(D3D11_TEXTURE2D_DESC));
	desc.Width = x;
	desc.Height = y;
	desc.MipLevels = 0;// static_cast<u32>(log2f(static_cast<f32>(glm::min(x, y))));
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

ID3D11SamplerState* createSamplerState(ID3D11Device* device,
	D3D11_FILTER filter,
	D3D11_TEXTURE_ADDRESS_MODE addressMode) 
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

// -----------------------------------------------------------------------------------------------

int main(int argc, char** argv)
{
	const u32 width = 1280;
	const u32 height = 720;
	static framework::Window s_window(argc, argv);
	s_window.init("2_Texturing", width, height, true, false, ENABLE_DEVICE_DEBUG);
	ID3D11Device* device = s_window.getDevice();
	ID3D11DeviceContext* ctx = s_window.getCtx();

	framework::ShaderPipeline shader;
	if (!loadShader(device, "./shaders/2_TexturedMesh.hlsl", shader)) 
	{
		printf("Failed to load and create shader");
		return 1;
	}

	const framework::DebugMesh& boxMesh = framework::DebugPrims::getBox();

	framework::FirstPersonCamera fpCam;

	v3 camPos(0.0f, 2.5f, 3.0f);
	v3 boxRotAngles(0.0f, 45.0f, 0.0f);
	v3 boxPos(0.0f, 0.5f, 0.0f);
	v3 boxScale(1.0f);
	v2 uvOffset(0.5f);
	v2 uvScale(1.0f);

	fpCam.init(camPos, v3(0.0f), 10, 10.0f);
	fpCam.setPerspective(60.0f, f32(width) / f32(height), 1.0f, 1000.0f);
	
	FrameDataCB frameCBData;
	frameCBData.view = fpCam.getView();
	frameCBData.viewProj = fpCam.getViewProj();
	frameCBData.invViewProj = fpCam.getInvViewProj();
	frameCBData.uvOffset = uvOffset;
	frameCBData.uvScale = uvScale;
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

	Tex2D texture;
	if (!loadTexture2D(device, ctx, "./textures/uv_check.png", DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, texture)) 
	{
		printf("Failed to create texture.");
		return 1;
	}

	ID3D11SamplerState* samplers[6] =
	{
		createSamplerState(device, D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_CLAMP),
		createSamplerState(device, D3D11_FILTER_MIN_MAG_MIP_POINT, D3D11_TEXTURE_ADDRESS_CLAMP),
		createSamplerState(device, D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_WRAP),
		createSamplerState(device, D3D11_FILTER_MIN_MAG_MIP_POINT, D3D11_TEXTURE_ADDRESS_WRAP),
		createSamplerState(device, D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_MIRROR),
		createSamplerState(device, D3D11_FILTER_MIN_MAG_MIP_POINT, D3D11_TEXTURE_ADDRESS_MIRROR)
	};
	if (!samplers[0] || !samplers[1] || !samplers[2] || !samplers[3] || !samplers[4] || !samplers[5]) 
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

	f64 lastStamp(0.0);
	f64 framerate = 0.0f;
	f64 elapsedTime = 0.0f;

	// Start frames

	while (s_window.update())
	{
		// Calculate elapsed time/framerate/...
		f64 currStamp = framework::Time::getTimeStampS();
		elapsedTime = currStamp - lastStamp;
		lastStamp = currStamp;
		framerate = 1.0 / elapsedTime;

		fpCam.update(static_cast<f32>(elapsedTime));

		static s32 s_filter = 0;
		static s32 s_addrMode = 0;

		// Do some UI
		if (ImGui::Begin("Controls")) 
		{
			ImGui::Text("R-Click + Move mouse: Rotate camera.");
			ImGui::Text("W/A/S/D: Move camera.");
			ImGui::Text("Left/Right: Decrease/Increase camera speed.");
			ImGui::Text("Up/Down: Decrease/Increase camera rotation speed.");
			ImGui::Separator();
			ImGui::DragFloat3("Box rotation (eulers)", &boxRotAngles[0]);
			ImGui::DragFloat3("Box scale", &boxScale[0], 0.01f);
			ImGui::DragFloat3("Box position", &boxPos[0], 0.1f);
			ImGui::Separator();
			ImGui::SliderFloat2("UV offset", &uvOffset[0], 0.0f, 1.0f);
			ImGui::SliderFloat2("UV Scale", &uvScale[0], 0.0f, 2.0f);
			ImGui::Separator();
			ImGui::Text("Address mode:");
			ImGui::RadioButton("Clamp", &s_addrMode, 0); ImGui::SameLine();
			ImGui::RadioButton("Wrap", &s_addrMode, 1); ImGui::SameLine();
			ImGui::RadioButton("Mirror", &s_addrMode, 2);
			ImGui::Separator();
			ImGui::Text("Filter mode:");
			ImGui::RadioButton("Bilinear", &s_filter, 0); ImGui::SameLine();
			ImGui::RadioButton("Nearest", &s_filter, 1);

			ImGui::End();
		}
		s32 samplerIdx = s_addrMode * 2 + s_filter;
		// --------------------------------

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

		// Update per-frame ConstantBuffer
		updateFrameCB(ctx, frameCB, fpCam, uvOffset, uvScale);

		// Bind shaders and draw batches
		shader.bind(ctx);

		// Draw box
		{
			// Update drawcall CB
			m4 translation = glm::translate(m4(1.0f), boxPos);
			m4 scale = glm::scale(m4(1.0f), boxScale);
			m4 rotation =
				glm::rotate(m4(1.0f), glm::radians(boxRotAngles.x), v3(1.0f, 0.0f, 0.0f)) *
				glm::rotate(m4(1.0f), glm::radians(boxRotAngles.y), v3(0.0f, 1.0f, 0.0f)) *
				glm::rotate(m4(1.0f), glm::radians(boxRotAngles.z), v3(0.0f, 0.0f, 1.0f));
			m4 model = rotation * scale * translation;
			updateBatchCB(ctx, drawcallCB, model);		

			// Bind vertex and index buffers
			u32 offset = boxMesh.m_vertexOffset;
			u32 vertexStride = static_cast<u32>(sizeof(framework::DebugVertex));
		
			// Bind vertex buffer and index buffer
			ctx->IASetVertexBuffers(0, 1, &boxMesh.m_vertexBuffer, &vertexStride, &offset);
			ctx->IASetIndexBuffer(boxMesh.m_indexBuffer, DXGI_FORMAT_R16_UINT, boxMesh.m_indexOffset);
			ctx->IASetPrimitiveTopology(boxMesh.m_topology);

			// Bind Constant buffers
			ctx->VSSetConstantBuffers(0, 1, &frameCB);
			ctx->PSSetConstantBuffers(0, 1, &frameCB);
			ctx->VSSetConstantBuffers(1, 1, &drawcallCB);

			ctx->PSSetSamplers(0, 1, &samplers[samplerIdx]);
			ctx->PSSetShaderResources(0, 1, &texture.m_SRV);

			// DrawIndexed as we are using index buffer
			ctx->DrawIndexed(boxMesh.m_indexCount, 0, 0);
		}

		s_window.present();
	}

	return 0;
}