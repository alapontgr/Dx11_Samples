#include "framework/framework.h"

#define ENABLE_DEVICE_DEBUG true

struct FrameDataCB
{
	m4 view;
	m4 viewProj;
	m4 invViewProj;
	v3 camPosWS;
    f32 pd00;
	// Lights
	v3 lightDir;
	f32 pad0;
	v3 mainLightColor;
	
	// Point
	f32 pointLightRadius;
	v3 pointLightPos;
	f32 pad1;
	v3 pointLightColor;

	// Spot
	f32 spotLightInnerCone; // cos of angle
	v3 spotLightPos;
	f32 spotLightOuterCone; // cos of angle
	v3 spotLightDir;
	f32 spotLightRadius;
	v3 spotLightColor;
	f32 pad2;
};

struct DrawcallDataCB 
{
	m4 m_model;
};

// -----------------------------------------------------------------------------------------------

bool loadShader(ID3D11Device* device, const char* relPath, framework::ShaderPipeline& outShader) 
{
	static constexpr u32 s_vertexAttribCount = 4;
	D3D11_INPUT_ELEMENT_DESC vertexLayout[s_vertexAttribCount];
	ZeroMemory(vertexLayout, s_vertexAttribCount * sizeof(D3D11_INPUT_ELEMENT_DESC));
	vertexLayout[0].SemanticName = "POSITION";
	vertexLayout[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexLayout[0].InputSlot = 0;
	vertexLayout[0].AlignedByteOffset = u32(offsetof(framework::GltfScene::VertexBuffer0, m_pos));
	vertexLayout[0].InstanceDataStepRate = D3D11_INPUT_PER_VERTEX_DATA;
	
	vertexLayout[1].SemanticName = "NORMAL";
	vertexLayout[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexLayout[1].InputSlot = 1;
	vertexLayout[1].AlignedByteOffset = u32(offsetof(framework::GltfScene::VertexBuffer1, m_normal));
	vertexLayout[1].InstanceDataStepRate = D3D11_INPUT_PER_VERTEX_DATA;
	
	vertexLayout[2].SemanticName = "TANGENT";
	vertexLayout[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexLayout[2].InputSlot = 1;
	vertexLayout[2].AlignedByteOffset = u32(offsetof(framework::GltfScene::VertexBuffer1, m_tangent));
	vertexLayout[2].InstanceDataStepRate = D3D11_INPUT_PER_VERTEX_DATA;

	vertexLayout[3].SemanticName = "TEXCOORD";
	vertexLayout[3].Format = DXGI_FORMAT_R32G32_FLOAT;
	vertexLayout[3].InputSlot = 1;
	vertexLayout[3].AlignedByteOffset = u32(offsetof(framework::GltfScene::VertexBuffer1, m_uv));
	vertexLayout[3].InstanceDataStepRate = D3D11_INPUT_PER_VERTEX_DATA;
	return outShader.loadGraphicsPipeline(device, relPath, "mainVS", "mainFS", vertexLayout, s_vertexAttribCount);
}

static void updateFrameCB(ID3D11DeviceContext* ctx, ID3D11Buffer* cBuffer, const FrameDataCB& frameData) 
{
	framework::RenderResources::updateMappableCBData(ctx, cBuffer, &frameData, sizeof(FrameDataCB));
}

static void updateBatchCB(ID3D11DeviceContext* ctx, ID3D11Buffer* cBuffer, const m4& model) 
{
	DrawcallDataCB drawcallCB;
	drawcallCB.m_model = model;
	framework::RenderResources::updateMappableCBData(ctx, cBuffer, &drawcallCB, sizeof(DrawcallDataCB));
}

static void drawDebugPrim(ID3D11DeviceContext* ctx, const m4& model, ID3D11Buffer* drawcallCB, const framework::DebugMesh& mesh) 
{
	updateBatchCB(ctx, drawcallCB, model);		

	// Bind vertex and index buffers
	u32 offset = mesh.m_vertexOffset;
	u32 vertexStride = static_cast<u32>(sizeof(framework::DebugVertex));
		
	// Bind vertex buffer and index buffer
	ctx->IASetVertexBuffers(0, 1, &mesh.m_vertexBuffer, &vertexStride, &offset);
	ctx->IASetIndexBuffer(mesh.m_indexBuffer, DXGI_FORMAT_R16_UINT, mesh.m_indexOffset);
	ctx->IASetPrimitiveTopology(mesh.m_topology);

	// DrawIndexed as we are using index buffer
	ctx->DrawIndexed(mesh.m_indexCount, 0, 0);
}

static m4 getSpotlightScale(f32 apertureRad, f32 radius) 
{
	f32 t = glm::tan(apertureRad);
	f32 s = t * radius;
	return glm::scale(m4(1.0f), v3(s, s, radius));
}

// -----------------------------------------------------------------------------------------------

int main(int argc, char** argv)
{
	const u32 width = 1280;
	const u32 height = 720;
	static framework::Window s_window(argc, argv);
	s_window.init("5_Lighting", width, height, true, false, ENABLE_DEVICE_DEBUG);
	ID3D11Device* device = s_window.getDevice();
	ID3D11DeviceContext* ctx = s_window.getCtx();

	framework::ShaderPipeline surfaceShader;
	if (!loadShader(device, "./shaders/5_ForwardLights.hlsl", surfaceShader)) 
	{
		printf("Failed to load and create shader");
		return 1;
	}

	framework::ShaderPipeline debugPrimShader;
	if (!loadShader(device, "./shaders/5_DebugPrim.hlsl", debugPrimShader)) 
	{
		printf("Failed to load and create shader");
		return 1;
	}

	framework::DebugMesh debugSphere;
	framework::DebugPrims::createLineSphere(device, 2, 2, debugSphere);

	framework::DebugMesh debugCone;
	framework::DebugPrims::createLineCone(device, debugCone);

	framework::FirstPersonCamera fpCam;

	v3 camPos(0.0f, 5.0f, 20.0f);

	fpCam.init(camPos, v3(0.0f), 10, 10.0f);
	fpCam.setPerspective(60.0f, f32(width) / f32(height), 1.0f, 1000.0f);
	
	FrameDataCB frameCBData;
	frameCBData.view = fpCam.getView();
	frameCBData.viewProj = fpCam.getViewProj();
	frameCBData.invViewProj = fpCam.getInvViewProj();
	frameCBData.camPosWS = fpCam.getPos();

	v3 mainLightColor = v3(1.0f);
	f32 mainLightIntensity = 1.0f;
	{
		m4 lightModel = glm::yawPitchRoll(glm::radians(45.0f), 0.0f, glm::radians(45.0f)) * glm::translate(m4(1.0f), v3(0.0f, 3.0f, -3.0f));
		frameCBData.lightDir = glm::normalize((m3)lightModel * v3(0.0f, -1.0f, 0.0f));
		frameCBData.mainLightColor = mainLightIntensity * mainLightColor;
	}
	frameCBData.pointLightPos = v3(0.0f, 1.0f, 0.0f);
	frameCBData.pointLightRadius = 3.0f;
	f32 pointLightIntensity = 1.0f;
	frameCBData.pointLightColor = v3(pointLightIntensity);

	f32 spotLightIntensity = 1.0f;
	frameCBData.spotLightColor = v3(spotLightIntensity);
	frameCBData.spotLightInnerCone = glm::cos(glm::radians(15.0f));
	frameCBData.spotLightOuterCone = glm::cos(glm::radians(30.0f));
	frameCBData.spotLightRadius = 3.0f;
	frameCBData.spotLightPos = v3(4.0f, 2.0f, 0.0f);
	frameCBData.spotLightDir = v3(0.0f, -1.0f, 0.0f);
	m4 spotModelNoScale = glm::rotate(m4(1.0f), glm::radians(90.0f), v3(1.0f, 0.0f, 0.0f));
	spotModelNoScale[3] = v4(frameCBData.spotLightPos, 1.0f);

	m4 lightModel = glm::yawPitchRoll(glm::radians(45.0f), 0.0f, glm::radians(45.0f)) * glm::translate(m4(1.0f), v3(0.0f, 3.0f, -3.0f));

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

	ID3D11RasterizerState* wireRasterState = nullptr;
	ZeroMemory(&rasterStateDesc, sizeof(D3D11_RASTERIZER_DESC));
	rasterStateDesc.FillMode = D3D11_FILL_WIREFRAME; // Solid geometry
	rasterStateDesc.CullMode = D3D11_CULL_BACK;
	rasterStateDesc.FrontCounterClockwise = true; // OpenGL style (Direct uses clockwise by default)
	if (FAILED(device->CreateRasterizerState(&rasterStateDesc, &wireRasterState))) 
	{
		printf("Failed to create Raster State");
		return 1;
	}

	UniquePtr<framework::GltfScene> scene = std::make_unique<framework::GltfScene>();
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

	const Vector<framework::GltfScene::Mesh>& meshes = scene->getMeshes();
	const Vector<framework::GltfScene::Node>& nodes = scene->getNodes();
	const Vector<framework::GltfScene::SurfaceMaterial>& materials = scene->getMaterials();

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
		static s32 s_editLightIdx = 0;
		static bool s_editLights = false;
		static s32 s_editTransformationIdx = 0; // 0: translation, 1 : rotation
		if (ImGui::Begin("Controls")) 
		{
			ImGui::Text("R-Click + Move mouse: Rotate camera.");
			ImGui::Text("W/A/S/D: Move camera.");
			ImGui::Text("Left/Right: Decrease/Increase camera speed.");
			ImGui::Text("Up/Down: Decrease/Increase camera rotation speed.");
			ImGui::Separator();
			ImGui::Checkbox("Lights edit mode", &s_editLights);
			if (s_editLights) 
			{
				ImGui::RadioButton("Main (Directional) light", &s_editLightIdx, 0); ImGui::SameLine();
				ImGui::RadioButton("Point light", &s_editLightIdx, 1); ImGui::SameLine();
				ImGui::RadioButton("Spot light", &s_editLightIdx, 2);
				ImGui::RadioButton("Translation", &s_editTransformationIdx, 0); ImGui::SameLine();
				ImGui::RadioButton("Rotation", &s_editTransformationIdx, 1);
			}
			ImGui::Separator();
			switch (s_editLightIdx) 
			{
			case 0:
			{
				ImGui::ColorEdit3("Main light color", &mainLightColor[0]);
				ImGui::InputFloat("Main light intensity", &mainLightIntensity);
				frameCBData.mainLightColor = mainLightColor * mainLightIntensity;
			}
			break;
			case 1:
			{
				v3 color = frameCBData.pointLightColor / pointLightIntensity;

				ImGui::InputFloat("Point light radius", &frameCBData.pointLightRadius);
				ImGui::ColorEdit3("Point light color", &color[0]);
				ImGui::InputFloat("Point Light Intensity", &pointLightIntensity);
				frameCBData.pointLightRadius = glm::max(frameCBData.pointLightRadius, 0.0f);
				frameCBData.pointLightColor = color * pointLightIntensity;
			}
			break;
			case 2:
			{
				v3 color = frameCBData.spotLightColor / spotLightIntensity;

				ImGui::InputFloat("Spot light inner radius", &frameCBData.spotLightRadius);
				ImGui::ColorEdit3("Spot light color", &color[0]);
				ImGui::InputFloat("Spot Light Intensity", &spotLightIntensity);
				f32 innerAngle = glm::degrees(glm::acos(frameCBData.spotLightInnerCone));
				f32 outerAngle = glm::degrees(glm::acos(frameCBData.spotLightOuterCone));
				ImGui::SliderFloat("Spot inner angle", &innerAngle, 0.0f, outerAngle);
				ImGui::SliderFloat("Spot outer angle", &outerAngle, 0.0f, 89.9f);
				frameCBData.spotLightInnerCone = glm::cos(glm::radians(glm::min(innerAngle, outerAngle)));
				frameCBData.spotLightOuterCone = glm::cos(glm::radians(glm::max(outerAngle, innerAngle)));
				frameCBData.spotLightColor = color * spotLightIntensity;
			}
			break;
			default:
				break;
			}

			ImGui::Separator();
			ImGui::End();
		}

		m4 view = fpCam.getView();
		m4 projection = fpCam.getProjection();
		if (s_editLights) 
		{
			switch (s_editLightIdx) 
			{
			case 0:
			{				
				v3 forward(fpCam.getForward());
				lightModel[3] = v4(fpCam.getPos() + forward * 10.0f, 1.0f);
				framework::Gizmo3D::drawRotationGizmo(view, projection, lightModel);
				frameCBData.lightDir = glm::normalize((m3)lightModel * v3(0.0f, -1.0f, 0.0f));
			}
			break;
			case 1:
			{
				m4 model = glm::translate(m4(1.0f), frameCBData.pointLightPos);
				framework::Gizmo3D::drawTranslationGizmo(view, projection, model);
				frameCBData.pointLightPos = model[3];
			}
			break;
			case 2:
			{
				if (s_editTransformationIdx == 0) 
				{
					framework::Gizmo3D::drawTranslationGizmo(view, projection, spotModelNoScale);
				}
				else if(s_editTransformationIdx == 1)
				{
					framework::Gizmo3D::drawRotationGizmo(view, projection, spotModelNoScale);
				}
				frameCBData.spotLightPos = spotModelNoScale[3];
				frameCBData.spotLightDir = glm::normalize(spotModelNoScale[2]);
			}
			break;
			default:
				break;
			}
		}
		
		frameCBData.view = view;
		frameCBData.viewProj = fpCam.getViewProj();;
		frameCBData.invViewProj = fpCam.getInvViewProj();
		frameCBData.camPosWS = fpCam.getPos();
		
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
		updateFrameCB(ctx, frameCB, frameCBData);

		// Bind shaders and draw batches
		surfaceShader.bind(ctx);
		ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		ctx->VSSetConstantBuffers(0, 1, &frameCB);
		ctx->PSSetConstantBuffers(0, 1, &frameCB);
		ctx->VSSetConstantBuffers(1, 1, &drawcallCB);

		// Draw GLTF
		for (const framework::GltfScene::Node& node : nodes) 
		{
			if (node.m_mesh >= 0) 
			{
				updateBatchCB(ctx, drawcallCB, node.m_model);
				const framework::GltfScene::Mesh& mesh = meshes[node.m_mesh];
				for (const framework::GltfScene::Meshlet& meshlet : mesh.m_meshlets) 
				{
					u32 vertexBubberOffsets[] = {scene->getVertexBuff0OffsetBytes(meshlet), scene->getVertexBuff1OffsetBytes(meshlet)};
					u32 vertexBubberStrides[] = {static_cast<u32>(sizeof(framework::GltfScene::VertexBuffer0)), static_cast<u32>(sizeof(framework::GltfScene::VertexBuffer1))};
					ID3D11Buffer* vertexBuffers[] = {scene->getPackedVertexBuffer(), scene->getPackedVertexBuffer()};
					ctx->IASetVertexBuffers(0, 2, vertexBuffers, vertexBubberStrides, vertexBubberOffsets);
					ctx->IASetIndexBuffer(scene->getPackedIndexBuffer(),
						meshlet.m_isIndexShort ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT,
						meshlet.m_indexBytesOffset);

					const framework::GltfScene::SurfaceMaterial& mat = materials[meshlet.m_material];
					if (mat.m_albedo.m_SRV) 
					{
						ctx->PSSetSamplers(0, 1, &samplers);
						ctx->PSSetShaderResources(0, 1, &mat.m_albedo.m_SRV);
					}

					ctx->DrawIndexed(meshlet.m_indexCount, 0, 0);
				}

			}
		}

		// Draw debug primitives
		ctx->RSSetState(wireRasterState);
		debugPrimShader.bind(ctx);
		ctx->VSSetConstantBuffers(0, 1, &frameCB);
		ctx->PSSetConstantBuffers(0, 1, &frameCB);
		ctx->VSSetConstantBuffers(1, 1, &drawcallCB);

		if (s_editLights) 
		{
			switch (s_editLightIdx)
			{
				break;
			case 1:
			{
				m4 model = glm::scale(m4(1.0f), v3(frameCBData.pointLightRadius));
				model[3] = v4(frameCBData.pointLightPos, 1.0f);
				drawDebugPrim(ctx, model, drawcallCB, debugSphere);
			}
			break;
			case 2:
			{
				m4 translation = glm::translate(m4(1.0f), (v3)spotModelNoScale[3]);
				m4 rot = spotModelNoScale;
				rot[3] = v4(0.0f, 0.0f, 0.0f, 1.0f);
				m4 model = translation * rot * getSpotlightScale(glm::acos(frameCBData.spotLightInnerCone), frameCBData.spotLightRadius);
				drawDebugPrim(ctx, model, drawcallCB, debugCone);
				model = translation * rot * getSpotlightScale(glm::acos(frameCBData.spotLightOuterCone), frameCBData.spotLightRadius);
				drawDebugPrim(ctx, model, drawcallCB, debugCone);
			}
			break;
			default:
				break;
			}
		}

		s_window.present();
	}

	return 0;
}