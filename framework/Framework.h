#pragma once

// Hacky way to link these libraries
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

// Include some platform libraries
#include <windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>

// External utilities
#include "external/imgui/imgui.h"
#include "external/ImGuizmo/ImGuizmo.h"

// Custom framework
#include "framework/Debug.h"
#include "framework/Types.h"
#include "framework/Time.h"
#include "framework/Hash.h"
#include "framework/Paths.h"
#include "framework/CommandLine.h"
#include "framework/FileUtils.h"
#include "framework/Window.h"
#include "framework/RenderUtils.h"
#include "framework/Camera.h"