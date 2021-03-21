#pragma once

#include "types.h"

namespace framework
{

	enum class MouseButton : u32
	{
		Left = 0,
		Right,
		Middle
	};

	enum class Keys : u32
	{
		// Numbers
		Key_0 = 0x30, Key_1, Key_2, Key_3, Key_4, Key_5, Key_6, Key_7, Key_8, Key_9,
		// Asci
		A = 0x41, B, C, D, E, F, G, H, I, J, K, L, M, N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
		F1 = 0x70, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
		// Some other keys
		LeftShift = 0xA0, RightShift = 0xA1,
		LeftControl = 0xA2, RightControl = 0xA3,
		Space = 0x20,
		Left = 0x25,
		Up,
		Right,
		Down
	};

	class Window
	{
	public:
		Window(s32 argCount, char** args);
		~Window();

		bool init(const char* windowName, const u32 w, const u32 h, bool vsync = true, bool isFullScreen = false, bool enableDebugDevice = false);

		HWND getWindowHandler() const { return m_hwnd; }

		ID3D11Device* getDevice() const { return m_device; }

		ID3D11DeviceContext* getCtx() const { return m_ctx; }

		ID3D11RenderTargetView* getBackBuffer() const { return m_backBuffer; }

		bool update();

		void present();

		void resize(u32 newW, u32 newH);

		HRESULT handleMessage(UINT message, WPARAM wParam, LPARAM lParam);

		static bool isMouseDown(MouseButton button);

		static f32 getMouseWheel();

		static v2 getMouseCursor();

		static v2 getMouseDelta();

		//https://docs.microsoft.com/en-us/windows/win32/inputdev/virtual-key-codes
		static bool isKeyDown(Keys key);

	protected:

		static constexpr u32 s_BackBufferCount = 3;

		bool initDevice(bool enableDebugDevice = false);
		void initImGui();
		bool updateMouseCursor();
		void updateMousePos();
		bool createImGuiRenderResources();
		void invalidateImGuiRenderResources();
		void drawImGui();

		HWND	  m_hwnd;
		HINSTANCE m_instance;

		std::string m_name;
		u32 m_width;
		u32 m_height;
		bool m_fullscreen;
		bool m_vsync;

		IDXGISwapChain* m_swapchain;
		ID3D11Device* m_device;
		ID3D11DeviceContext* m_ctx;
		ID3D11RenderTargetView* m_backBuffer;

		// Imgui settings
		s64 m_time;
		s64 m_ticksPerSecond;
		ImGuiMouseCursor m_LastMouseCursor;
	};

}