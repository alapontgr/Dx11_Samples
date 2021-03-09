#include "framework/framework.h"

struct GraphicsPipeline 
{
	ID3D11VertexShader* m_vertexShader = nullptr;
	ID3D11PixelShader* m_fragmentShader = nullptr;
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
			OutputDebugStringA( (char*)errorMSG->GetBufferPointer() );
            errorMSG->Release();
			return false; 
		}
		if (window.getDevice()->CreateVertexShader(vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize(), NULL, &outPipeline.m_vertexShader) != S_OK)
		{
			vertexShaderBlob->Release();
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

int main(int argc, char** argv)
{
	const u32 width = 1280;
	const u32 height = 720;
	static framework::Window s_window(argc, argv);
	s_window.init("0_HelloTriangle", width, height);

	GraphicsPipeline pipeline;
	if (!loadShader("./shaders/0_Triangle.hlsl", s_window, pipeline)) 
	{
		printf("Failed to load shader source.");
		return 1;
	}

	ID3D11Device* device = s_window.getDevice();
	ID3D11DeviceContext* ctx = s_window.getCtx();

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
		// Calculate elapsed time/framerate/...
		f64 currStamp = framework::Time::getTimeStampS();
		elapsedTime = currStamp - lastStamp;
		lastStamp = currStamp;
		framerate = 1.0 / elapsedTime;


		ID3D11DeviceContext* ctx = s_window.getCtx();
		ID3D11RenderTargetView* backBuffer = s_window.getBackBuffer();

		// Ignore this for now
		ctx->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr); 

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
		// No vertex buffers. Let the GPU generate some vertices and we'll assign the position in the shader for now.
		ctx->Draw(3, 0);

		s_window.present();
	}

	return 0;
}