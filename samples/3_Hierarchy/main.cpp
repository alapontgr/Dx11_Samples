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

struct DepthAttachment 
{
	ID3D11DepthStencilView* m_depthStencilView = nullptr;
	ID3D11Texture2D* m_depthStencilTexture = nullptr;
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

class Node 
{
public:
	
	Node() {}

	Node(const v3& pos, const v3& scale, const framework::DebugMesh* mesh)
		: m_localPos(pos)
		, m_localScale(scale)
		, m_mesh(mesh)
	{}

	~Node(){}

	void draw(ID3D11DeviceContext* ctx, ID3D11Buffer* drawcallCB, const m4& parentModel);

	Node* addChild(UniquePtr<Node>&& child);

	static f32 s_globalRoll;

private:
	v3 m_localPos;
	v3 m_localScale;
	const framework::DebugMesh* m_mesh;
	Vector<UniquePtr<Node>> m_children;
};

f32 Node::s_globalRoll = 0.0f;

void Node::draw(ID3D11DeviceContext* ctx, ID3D11Buffer* drawcallCB, const m4& parentModel) 
{
	m4 model = parentModel * (
		glm::yawPitchRoll(0.0f, 0.0f, glm::radians(s_globalRoll)) *
		glm::scale(m4(1.0f), m_localScale) *
		glm::translate(m4(1.0f), m_localPos));
	if (m_mesh) 
	{
		// Vertex, Index buffers already bound
		// Sampler and Texture2D already bound
		// ...

		updateBatchCB(ctx, drawcallCB, model);	
		ctx->VSSetConstantBuffers(1, 1, &drawcallCB);

		ctx->DrawIndexed(m_mesh->m_indexCount, 0, 0);

	}
	for (const UniquePtr<Node>& child : m_children) 
	{
		child->draw(ctx, drawcallCB, model);
	}
}

Node* Node::addChild(UniquePtr<Node>&& child)
{
	m_children.push_back(std::move(child));
	return m_children[m_children.size() - 1].get();
}

bool createDepthAttachment(ID3D11Device* device, u32 width, u32 height, DepthAttachment& outDepthAttachment) 
{
	D3D11_TEXTURE2D_DESC descDepth;
	descDepth.Width = width;
	descDepth.Height = height;
	descDepth.MipLevels = 1;
	descDepth.ArraySize = 1;
	descDepth.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	descDepth.SampleDesc.Count = 1;
	descDepth.SampleDesc.Quality = 0;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	descDepth.CPUAccessFlags = 0;
	descDepth.MiscFlags = 0;
	HRESULT res = device->CreateTexture2D( &descDepth, NULL, &outDepthAttachment.m_depthStencilTexture );
	if (FAILED(res)) 
	{
		printf("Failed to create texture resource.");
		return false;
	}

	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
	ZeroMemory(&descDSV, sizeof(D3D11_DEPTH_STENCIL_VIEW_DESC));
	descDSV.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0;

	// Create the depth stencil view
	res = device->CreateDepthStencilView( outDepthAttachment.m_depthStencilTexture, // Depth stencil texture
										  &descDSV, // Depth stencil desc
										  &outDepthAttachment.m_depthStencilView );  // [out] Depth stencil view
	if (FAILED(res)) 
	{
		printf("Failed to create Depth Stencil view.");
		return false;
	}
	return true;
}

ID3D11DepthStencilState* createDepthStencilState(ID3D11Device* device) 
{
	D3D11_DEPTH_STENCIL_DESC dsDesc;

	// Depth test parameters
	dsDesc.DepthEnable = true;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS;

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

// -----------------------------------------------------------------------------------------------

int main(int argc, char** argv)
{
	const u32 width = 1280;
	const u32 height = 720;
	static framework::Window s_window(argc, argv);
	s_window.init("3_Hierarchy", width, height, true, false, ENABLE_DEVICE_DEBUG);
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

	framework::Texture2D texture;
	if (!framework::RenderResources::loadTexture2D(device, ctx, "./textures/uv_check.png", DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, texture)) 
	{
		printf("Failed to create texture.");
		return 1;
	}

	ID3D11SamplerState* samplers = framework::RenderResources::createSamplerState(device, D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_CLAMP);
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

	DepthAttachment depthAttachment;
	ID3D11DepthStencilState* depthStencilState = createDepthStencilState(device);

	if (!createDepthAttachment(device, width, height, depthAttachment) || !depthStencilState) 
	{
		return 1;
	}

	f64 lastStamp = framework::Time::getTimeStampS();
	f64 framerate = 0.0f;
	f64 elapsedTime = 0.0f;

	// Build scene
	UniquePtr<Node> root = std::make_unique<Node>(v3(0.0f), v3(1.0f), nullptr);
	root->addChild(std::make_unique<Node>(v3(0.0f), v3(20.0f, 1.0f, 1.0f), &boxMesh));
	Node* anchor = root->addChild(std::make_unique<Node>(v3(10.0f, 0.0f, 1.0f), v3(1.0f), nullptr));
	anchor->addChild(std::make_unique<Node>(v3(0.0f, 0.0f, 0.0), v3(10.0f, 1.0f, 1.0f), &boxMesh));
	anchor = anchor->addChild(std::make_unique<Node>(v3(5.0f, 0.0f, 1.0f), v3(1.0f), nullptr));
	anchor->addChild(std::make_unique<Node>(v3(0.0f), v3(5.0f, 1.0f, 1.0f), &boxMesh));
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
		if (ImGui::Begin("Controls")) 
		{
			ImGui::Text("R-Click + Move mouse: Rotate camera.");
			ImGui::Text("W/A/S/D: Move camera.");
			ImGui::Text("Left/Right: Decrease/Increase camera speed.");
			ImGui::Text("Up/Down: Decrease/Increase camera rotation speed.");
			ImGui::Separator();
			ImGui::DragFloat("Rotation speed", &rotSpeed, 0.1f, 0.0f, 180.0f);;
			ImGui::End();
		}
		// --------------------------------

		f32 delta = static_cast<f32>(elapsedTime) * rotSpeed;
		if (delta > 360.0f) 
		{
			delta = delta - 360.0f;
		}
		Node::s_globalRoll += delta;

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
		updateFrameCB(ctx, frameCB, fpCam, uvOffset, uvScale);

		// Bind shaders and draw batches
		shader.bind(ctx);

		// Draw box
		{
			// Bind vertex and index buffers
			u32 offset = boxMesh.m_vertexOffset;
			u32 vertexStride = static_cast<u32>(sizeof(framework::DebugVertex));
		
			// Bind vertex buffer and index buffer
			ctx->IASetVertexBuffers(0, 1, &boxMesh.m_vertexBuffer, &vertexStride, &offset);
			ctx->IASetIndexBuffer(boxMesh.m_indexBuffer, DXGI_FORMAT_R16_UINT, boxMesh.m_indexOffset);
			ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			// Bind Constant buffers
			ctx->VSSetConstantBuffers(0, 1, &frameCB);
			ctx->PSSetConstantBuffers(0, 1, &frameCB);
			ctx->VSSetConstantBuffers(1, 1, &drawcallCB);

			ctx->PSSetSamplers(0, 1, &samplers);
			ctx->PSSetShaderResources(0, 1, &texture.m_SRV);

			// DrawIndexed as we are using index buffer
			root->draw(ctx, drawcallCB, m4(1.0f));
		}

		s_window.present();
	}

	return 0;
}