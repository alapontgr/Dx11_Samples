#include "samples/5_Lighting/App.h"

#define ENABLE_DEVICE_DEBUG true

// -----------------------------------------------------------------------------------------------

UberShader::UberShader() 
	: m_lastRetrievedHash(0)
	, m_lastRetrievedShader(nullptr)
{
}

void UberShader::addVariant(u32 hash, UniquePtr<framework::ShaderPipeline>&& variant) 
{
	m_shaderCache[hash] = std::move(variant);
}

framework::ShaderPipeline* UberShader::getShader(u32 hash) 
{
	if (hash && m_lastRetrievedHash == hash) 
	{
		return m_lastRetrievedShader;
	}
	auto entry = m_shaderCache.find(hash);
	if (entry != m_shaderCache.end()) 
	{
		m_lastRetrievedHash = hash;
		m_lastRetrievedShader = entry->second.get();
		return m_lastRetrievedShader;
	}
	return nullptr;
}

bool UberShader::uberize(ID3D11Device* device,
		const String& src, 
		const String* keywords, u32 keywordCount,
		const char* entryVS,
		const char* entryFS,
		D3D11_INPUT_ELEMENT_DESC* vertexAttributes, u32 vertexAttribCount) 
{
	u32 combinationsCount = 1 << keywordCount;
	String defines("");
	String shaderFinalSrc;

	for (u32 i = 0; i < combinationsCount; ++i) 
	{
		u32 hash = 0;
		defines = "";
		u32 bitCount = static_cast<u32>(glm::ceil(glm::log2(static_cast<f32>(i)))) + 1;
		for (u32 j=0; j<bitCount; ++j) 
		{
			if ((i & (1 << j)) != 0) 
			{
				hash |= (1<<j);
				defines += "#define " + keywords[j] + " 1 \n";
			}
		}

		shaderFinalSrc = defines + src;
		UniquePtr<framework::ShaderPipeline> shader = std::make_unique<framework::ShaderPipeline>();
		if (!shader->createGraphicsPipeline(device, shaderFinalSrc.c_str(), shaderFinalSrc.size(), entryVS, entryFS, vertexAttributes, vertexAttribCount)) 
		{
			return false;
		}
		addVariant(hash, std::move(shader));
	}
	return true;
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
	vertexLayout[0].AlignedByteOffset = u32(offsetof(framework::GltfScene::VertexBuffer0, m_pos));
	vertexLayout[0].InstanceDataStepRate = D3D11_INPUT_PER_VERTEX_DATA;
	
	vertexLayout[1].SemanticName = "NORMAL";
	vertexLayout[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	vertexLayout[1].InputSlot = 1;
	vertexLayout[1].AlignedByteOffset = u32(offsetof(framework::GltfScene::VertexBuffer1, m_normal));
	vertexLayout[1].InstanceDataStepRate = D3D11_INPUT_PER_VERTEX_DATA;
	
	vertexLayout[2].SemanticName = "TANGENT";
	vertexLayout[2].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
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

bool loadSurfaceShader(ID3D11Device* device, const char* relPath, UberShader& outShader) 
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
	vertexLayout[2].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	vertexLayout[2].InputSlot = 1;
	vertexLayout[2].AlignedByteOffset = u32(offsetof(framework::GltfScene::VertexBuffer1, m_tangent));
	vertexLayout[2].InstanceDataStepRate = D3D11_INPUT_PER_VERTEX_DATA;

	vertexLayout[3].SemanticName = "TEXCOORD";
	vertexLayout[3].Format = DXGI_FORMAT_R32G32_FLOAT;
	vertexLayout[3].InputSlot = 1;
	vertexLayout[3].AlignedByteOffset = u32(offsetof(framework::GltfScene::VertexBuffer1, m_uv));
	vertexLayout[3].InstanceDataStepRate = D3D11_INPUT_PER_VERTEX_DATA;

	static const u32 s_keywordCount = 2;
	static String s_keywords[] = 
	{
		"NORMAL_MAPPING",
		"DEBUG_NORMALS"
	};

	String absPath = framework::Paths::getAssetPath(relPath);
	UniquePtr<char[]> hlslSrc = framework::FileUtils::loadFileContent(absPath.c_str());
	size_t srcSize = strlen(hlslSrc.get());
	if (!hlslSrc)
	{
		return false;
	}

	return outShader.uberize(device, hlslSrc.get(), s_keywords, s_keywordCount, "mainVS", "mainFS", vertexLayout, s_vertexAttribCount);
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


void App::doGui(DebugConfig& config) 
{
	if (ImGui::Begin("Controls")) 
		{
			ImGui::Text("R-Click + Move mouse: Rotate camera.");
			ImGui::Text("W/A/S/D: Move camera.");
			ImGui::Text("Left/Right: Decrease/Increase camera speed.");
			ImGui::Text("Up/Down: Decrease/Increase camera rotation speed.");
			ImGui::Separator();
			ImGui::Checkbox("Lights edit mode", &config.m_editLights);
			if (config.m_editLights) 
			{
				ImGui::RadioButton("Main (Directional) light", &config.m_editLightIdx, 0); ImGui::SameLine();
				ImGui::RadioButton("Point light", &config.m_editLightIdx, 1); ImGui::SameLine();
				ImGui::RadioButton("Spot light", &config.m_editLightIdx, 2);
				ImGui::RadioButton("Translation", &config.m_editTransformationIdx, 0); ImGui::SameLine();
				ImGui::RadioButton("Rotation", &config.m_editTransformationIdx, 1);
			}
			ImGui::Separator();
			switch (config.m_editLightIdx) 
			{
			case 0:
			{
				ImGui::ColorEdit3("Main light color", &m_mainLightColor[0]);
				ImGui::InputFloat("Main light intensity", &m_mainLightIntensity);
				m_frameCBData.mainLightColor = m_mainLightColor * m_mainLightIntensity;
			}
			break;
			case 1:
			{
				v3 color = m_frameCBData.pointLightColor / m_pointLightIntensity;

				ImGui::InputFloat("Point light radius", &m_frameCBData.pointLightRadius);
				ImGui::ColorEdit3("Point light color", &color[0]);
				ImGui::InputFloat("Point Light Intensity", &m_pointLightIntensity);
				m_frameCBData.pointLightRadius = glm::max(m_frameCBData.pointLightRadius, 0.0f);
				m_frameCBData.pointLightColor = color * m_pointLightIntensity;
			}
			break;
			case 2:
			{
				v3 color = m_frameCBData.spotLightColor / m_spotLightIntensity;

				ImGui::InputFloat("Spot light inner radius", &m_frameCBData.spotLightRadius);
				ImGui::ColorEdit3("Spot light color", &color[0]);
				ImGui::InputFloat("Spot Light Intensity", &m_spotLightIntensity);
				f32 innerAngle = glm::degrees(glm::acos(m_frameCBData.spotLightInnerCone));
				f32 outerAngle = glm::degrees(glm::acos(m_frameCBData.spotLightOuterCone));
				ImGui::SliderFloat("Spot inner angle", &innerAngle, 0.0f, outerAngle);
				ImGui::SliderFloat("Spot outer angle", &outerAngle, 0.0f, 89.9f);
				m_frameCBData.spotLightInnerCone = glm::cos(glm::radians(glm::min(innerAngle, outerAngle)));
				m_frameCBData.spotLightOuterCone = glm::cos(glm::radians(glm::max(outerAngle, innerAngle)));
				m_frameCBData.spotLightColor = color * m_spotLightIntensity;
			}
			break;
			default:
				break;
			}

			ImGui::Separator();

			ImGui::CheckboxFlags("NormalMapping enabled", &config.m_renderingFeaturesMask, 1 << framework::GltfScene::NormalMap);
			ImGui::CheckboxFlags("Debug normals", &config.m_renderingFeaturesMask, s_DebugNormalsFlag);
			ImGui::End();
		}

		const m4 view = m_fpCam.getView();
		const m4 projection = m_fpCam.getProjection();
		if (config.m_editLights) 
		{
			switch (config.m_editLightIdx) 
			{
			case 0:
			{				
				v3 forward(m_fpCam.getForward());
				m_lightModel[3] = v4(m_fpCam.getPos() + forward * 10.0f, 1.0f);
				framework::Gizmo3D::drawRotationGizmo(view, projection, m_lightModel);
				m_frameCBData.lightDir = glm::normalize((m3)m_lightModel * v3(0.0f, -1.0f, 0.0f));
			}
			break;
			case 1:
			{
				m4 model = glm::translate(m4(1.0f), m_frameCBData.pointLightPos);
				framework::Gizmo3D::drawTranslationGizmo(view, projection, model);
				m_frameCBData.pointLightPos = model[3];
			}
			break;
			case 2:
			{
				if (config.m_editTransformationIdx == 0) 
				{
					framework::Gizmo3D::drawTranslationGizmo(view, projection, m_spotModelNoScale);
				}
				else if(config.m_editTransformationIdx == 1)
				{
					framework::Gizmo3D::drawRotationGizmo(view, projection, m_spotModelNoScale);
				}
				m_frameCBData.spotLightPos = m_spotModelNoScale[3];
				m_frameCBData.spotLightDir = glm::normalize(m_spotModelNoScale[2]);
			}
			break;
			default:
				break;
			}
		}
}

void App::drawDebugPrims(DebugConfig& config) 
{
	m_ctx->RSSetState(m_wireRasterState);
	m_debugPrimShader.bind(m_ctx);
	m_ctx->VSSetConstantBuffers(0, 1, &m_frameCB);
	m_ctx->PSSetConstantBuffers(0, 1, &m_frameCB);
	m_ctx->VSSetConstantBuffers(1, 1, &m_drawcallCB);

	if (config.m_editLights) 
	{
		switch (config.m_editLightIdx)
		{
			break;
		case 1:
		{
			m4 model = glm::scale(m4(1.0f), v3(m_frameCBData.pointLightRadius));
			model[3] = v4(m_frameCBData.pointLightPos, 1.0f);
			drawDebugPrim(m_ctx, model, m_drawcallCB, m_debugSphere);
		}
		break;
		case 2:
		{
			m4 translation = glm::translate(m4(1.0f), (v3)m_spotModelNoScale[3]);
			m4 rot = m_spotModelNoScale;
			rot[3] = v4(0.0f, 0.0f, 0.0f, 1.0f);
			m4 model = translation * rot * getSpotlightScale(glm::acos(m_frameCBData.spotLightInnerCone), m_frameCBData.spotLightRadius);
			drawDebugPrim(m_ctx, model, m_drawcallCB, m_debugCone);
			model = translation * rot * getSpotlightScale(glm::acos(m_frameCBData.spotLightOuterCone), m_frameCBData.spotLightRadius);
			drawDebugPrim(m_ctx, model, m_drawcallCB, m_debugCone);
		}
		break;
		default:
			break;
		}
	}
}

s32 App::init() 
{
	const u32 width = 1280;
	const u32 height = 720;
	Window::init("5_Lighting", width, height, true, false, ENABLE_DEVICE_DEBUG);

	// Init shaders
	if (!loadSurfaceShader(m_device, "./shaders/5_ForwardLights.hlsl", m_surfaceShader)) 
	{
		printf("Failed to load and create shader");
		return 1;
	}

	if (!loadShader(m_device, "./shaders/5_DebugPrim.hlsl", m_debugPrimShader)) 
	{
		printf("Failed to load and create shader");
		return 1;
	}

	// Init debug prims
	framework::DebugPrims::createLineSphere(m_device, 2, 2, m_debugSphere);
	framework::DebugPrims::createLineCone(m_device, m_debugCone);

	v3 camPos(0.0f, 5.0f, 20.0f);

	m_fpCam.init(camPos, v3(0.0f), 10, 10.0f);
	m_fpCam.setPerspective(60.0f, f32(width) / f32(height), 0.1f, 1000.0f);

	m_frameCBData.view = m_fpCam.getView();
	m_frameCBData.viewProj = m_fpCam.getViewProj();
	m_frameCBData.invViewProj = m_fpCam.getInvViewProj();
	m_frameCBData.camPosWS = m_fpCam.getPos();

	// Init environment settings
	m_mainLightColor = v3(1.0f);
	m_mainLightIntensity = 1.0f;
	m_pointLightIntensity = 1.0f;
	m_spotLightIntensity = 1.0f;

	{
		m4 lightModel = glm::yawPitchRoll(glm::radians(45.0f), 0.0f, glm::radians(45.0f)) * glm::translate(m4(1.0f), v3(0.0f, 3.0f, -3.0f));
		m_frameCBData.lightDir = glm::normalize((m3)lightModel * v3(0.0f, -1.0f, 0.0f));
		m_frameCBData.mainLightColor = m_mainLightIntensity * m_mainLightColor;
	}
	m_frameCBData.pointLightPos = v3(0.0f, 1.0f, 0.0f);
	m_frameCBData.pointLightRadius = 3.0f;
	m_frameCBData.pointLightColor = v3(m_pointLightIntensity);

	m_frameCBData.spotLightColor = v3(m_spotLightIntensity);
	m_frameCBData.spotLightInnerCone = glm::cos(glm::radians(15.0f));
	m_frameCBData.spotLightOuterCone = glm::cos(glm::radians(30.0f));
	m_frameCBData.spotLightRadius = 3.0f;
	m_frameCBData.spotLightPos = v3(4.0f, 1.5f, 0.0f);
	m_frameCBData.spotLightDir = v3(0.0f, -1.0f, 0.0f);
	m_spotModelNoScale = glm::rotate(m4(1.0f), glm::radians(90.0f), v3(1.0f, 0.0f, 0.0f));
	m_spotModelNoScale[3] = v4(m_frameCBData.spotLightPos, 1.0f);

	m_lightModel = glm::yawPitchRoll(glm::radians(45.0f), 0.0f, glm::radians(45.0f)) * glm::translate(m4(1.0f), v3(0.0f, 3.0f, -3.0f));

	// Graphics resources
	m_frameCB = framework::RenderResources::createConstantBuffer<FrameDataCB>(m_device, m_frameCBData);
	if (!m_frameCB) 
	{
		printf("Failed to create per frame constant buffer");
		return 1;
	}

	m_drawcallCB = framework::RenderResources::createConstantBuffer<DrawcallDataCB>(m_device, {m4(1.0f)});
	if (!m_drawcallCB) 
	{
		printf("Failed to create Per drawcall constant buffer");
		return 1;
	}

	m_samplers = framework::RenderResources::createSamplerState(m_device, D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_WRAP);
	if (!m_samplers) 
	{
		printf("Failed to create sampler state");
		return 1;
	}

	
	// Configure rasterization with a RasterState
	D3D11_RASTERIZER_DESC rasterStateDesc;
	ZeroMemory(&rasterStateDesc, sizeof(D3D11_RASTERIZER_DESC));
	rasterStateDesc.FillMode = D3D11_FILL_SOLID; // Solid geometry
	rasterStateDesc.CullMode = D3D11_CULL_BACK;
	rasterStateDesc.FrontCounterClockwise = true; // OpenGL style (Direct uses clockwise by default)
	if (FAILED(m_device->CreateRasterizerState(&rasterStateDesc, &m_rasterState))) 
	{
		printf("Failed to create Raster State");
		return 1;
	}
		
	ZeroMemory(&rasterStateDesc, sizeof(D3D11_RASTERIZER_DESC));
	rasterStateDesc.FillMode = D3D11_FILL_WIREFRAME; // Solid geometry
	rasterStateDesc.CullMode = D3D11_CULL_BACK;
	rasterStateDesc.FrontCounterClockwise = true; // OpenGL style (Direct uses clockwise by default)
	if (FAILED(m_device->CreateRasterizerState(&rasterStateDesc, &m_wireRasterState))) 
	{
		printf("Failed to create Raster State");
		return 1;
	}

	m_scene = std::make_unique<framework::GltfScene>();
	if (!m_scene->loadGLTF(m_device, m_ctx, "./models/Sponza/glTF/Sponza.gltf")) 
	{
		printf("Failed to load gltf");
		return 1;
	}

	m_depthStencilState = framework::RenderResources::createDepthStencilState(m_device, D3D11_COMPARISON_LESS);
	if (!framework::RenderResources::createDepthAttachment(m_device, width, height, DXGI_FORMAT_D24_UNORM_S8_UINT, m_depthAttachment) || !m_depthStencilState) 
	{
		return 1;
	}

	return 0;
}

s32 App::run() 
{
	if (init()) 
	{
		return 1;
	}

	f64 lastStamp = framework::Time::getTimeStampS();
	f64 framerate = 0.0f;
	f64 elapsedTime = 0.0f;

	const Vector<framework::GltfScene::Mesh>& meshes = m_scene->getMeshes();
	const Vector<framework::GltfScene::Node>& nodes = m_scene->getNodes();
	const Vector<framework::GltfScene::SurfaceMaterial>& materials = m_scene->getMaterials();

	DebugConfig debugConfig;

	// Start frames
	while (update())
	{
		// Calculate elapsed time/framerate/...
		f64 currStamp = framework::Time::getTimeStampS();
		elapsedTime = currStamp - lastStamp;
		lastStamp = currStamp;
		framerate = 1.0 / elapsedTime;

		m_fpCam.update(static_cast<f32>(elapsedTime));

		// Do some UI
		doGui(debugConfig);
		
		m_frameCBData.view = m_fpCam.getView();
		m_frameCBData.viewProj = m_fpCam.getViewProj();;
		m_frameCBData.invViewProj = m_fpCam.getInvViewProj();
		m_frameCBData.camPosWS = m_fpCam.getPos();
		
		// --------------------------------

		ID3D11RenderTargetView* backBuffer = getBackBuffer();
		// Set the back buffer as our RenderTarget
		m_ctx->OMSetRenderTargets(1, &backBuffer, m_depthAttachment.m_depthStencilView);

		// Set the viewport. This configures the area to render
		D3D11_VIEWPORT viewport;
		viewport.TopLeftX = 0.0f;
		viewport.TopLeftY = 0.0f;
		viewport.Width = static_cast<f32>(m_width);
		viewport.Height = static_cast<f32>(m_height);
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		m_ctx->RSSetViewports(1, &viewport);

		// Tell the context how we want to rasterize the following drawcalls
		m_ctx->RSSetState(m_rasterState);

		// Clear the RenderTarget to the desired color
		FLOAT clearColor[] = { 0.0f, 0.0f, 0.0f, 0.0f };
		m_ctx->ClearRenderTargetView(backBuffer, clearColor);

		m_ctx->OMSetDepthStencilState(m_depthStencilState, 0);
		m_ctx->ClearDepthStencilView(m_depthAttachment.m_depthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

		// Update per-frame ConstantBuffer
		updateFrameCB(m_ctx, m_frameCB, m_frameCBData);

		// Bind shaders and draw batches
		framework::ShaderPipeline* currShader = nullptr;
		m_ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		m_ctx->VSSetConstantBuffers(0, 1, &m_frameCB);
		m_ctx->PSSetConstantBuffers(0, 1, &m_frameCB);
		m_ctx->VSSetConstantBuffers(1, 1, &m_drawcallCB);

		// Draw GLTF
		for (const framework::GltfScene::Node& node : nodes) 
		{
			if (node.m_mesh >= 0) 
			{
				updateBatchCB(m_ctx, m_drawcallCB, node.m_model);
				const framework::GltfScene::Mesh& mesh = meshes[node.m_mesh];
				for (const framework::GltfScene::Meshlet& meshlet : mesh.m_meshlets) 
				{
					u32 vertexBubberOffsets[] = {m_scene->getVertexBuff0OffsetBytes(meshlet), m_scene->getVertexBuff1OffsetBytes(meshlet)};
					u32 vertexBubberStrides[] = {static_cast<u32>(sizeof(framework::GltfScene::VertexBuffer0)), static_cast<u32>(sizeof(framework::GltfScene::VertexBuffer1))};
					ID3D11Buffer* vertexBuffers[] = {m_scene->getPackedVertexBuffer(), m_scene->getPackedVertexBuffer()};
					m_ctx->IASetVertexBuffers(0, 2, vertexBuffers, vertexBubberStrides, vertexBubberOffsets);
					m_ctx->IASetIndexBuffer(m_scene->getPackedIndexBuffer(),
						meshlet.m_isIndexShort ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT,
						meshlet.m_indexBytesOffset);

					const framework::GltfScene::SurfaceMaterial& mat = materials[meshlet.m_material];

					u32 hash = debugConfig.m_renderingFeaturesMask & mat.m_hash;
					if ((debugConfig.m_renderingFeaturesMask & s_DebugNormalsFlag) != 0) 
					{
						hash |= s_DebugNormalsFlag;
					}

					framework::ShaderPipeline* shader = m_surfaceShader.getShader(hash); // See how the hash is generated and how we uberize the shader
					VERIFY(shader, "Trying to access null shader");
					if (shader != currShader) 
					{
						currShader = shader;
						currShader->bind(m_ctx);
					}

					ID3D11ShaderResourceView* views[] =	{nullptr, nullptr};
					u32 texToBind = 0;

					if (mat.m_albedo.m_SRV) 
					{						
						views[texToBind++] = mat.m_albedo.m_SRV;
					}
					if (mat.m_normal.m_SRV) 
					{
						views[texToBind++] = mat.m_normal.m_SRV;
					}

					m_ctx->PSSetSamplers(0, 1, &m_samplers);
					m_ctx->PSSetShaderResources(0, texToBind, views);

					m_ctx->DrawIndexed(meshlet.m_indexCount, 0, 0);
				}

			}
		}

		// Draw debug primitives
		drawDebugPrims(debugConfig);

		// Present swapchain
		present();
	}

	return 0;
}
