#pragma once

#include "framework/Framework.h"

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

static constexpr u32 s_DebugNormalsFlag = 1 << framework::GltfScene::COUNT;

struct DebugConfig 
{
	bool m_editLights = true;
	s32 m_editLightIdx = 1; // Pointlight by default
	s32 m_editTransformationIdx = 0; // 0: Translation, 1: Rotation
	u32 m_renderingFeaturesMask = (1<<framework::GltfScene::NormalMap); // Default: Normal mapping enabled
};

class UberShader 
{
public:
	UberShader();

	void addVariant(u32 hash, UniquePtr<framework::ShaderPipeline>&& variant);

	bool uberize(ID3D11Device* device,
		const String& src, 
		const String* keywords, u32 keywordCount,
		const char* entryVS,
		const char* entryFS,
		D3D11_INPUT_ELEMENT_DESC* vertexAttributes, u32 vertexAttribCount);

	framework::ShaderPipeline* getShader(u32 hash);

private:

	UMap<u32, UniquePtr<framework::ShaderPipeline>> m_shaderCache;
	u32 m_lastRetrievedHash;
	framework::ShaderPipeline* m_lastRetrievedShader;
};

class App : public framework::Window
{
public:
	App(int argCount, char** args) : Window(argCount, args) {}

	s32 init();

	void doGui(DebugConfig& config);

	void drawDebugPrims(DebugConfig& config);

	s32 run();

private:

	UberShader m_surfaceShader;
	framework::ShaderPipeline m_debugPrimShader;

	framework::DebugMesh m_debugSphere;
	framework::DebugMesh m_debugCone;

	framework::FirstPersonCamera m_fpCam;
	FrameDataCB m_frameCBData;

	v3 m_mainLightColor;
	f32 m_mainLightIntensity;
	f32 m_pointLightIntensity;
	f32 m_spotLightIntensity;
	m4 m_spotModelNoScale;
	m4 m_lightModel;

	ID3D11Buffer* m_frameCB;
	ID3D11Buffer* m_drawcallCB;
	ID3D11SamplerState* m_samplers;

	ID3D11RasterizerState* m_rasterState = nullptr;
	ID3D11RasterizerState* m_wireRasterState = nullptr;
	UniquePtr<framework::GltfScene> m_scene;

	framework::DepthAttachment m_depthAttachment;
	ID3D11DepthStencilState* m_depthStencilState;
};
