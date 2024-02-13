#pragma once
#include <d3d9.h>
#include "../imgui/imgui.h"
#include <string>


namespace gui {
	static std::string GetCurrentTimeAndDate();
	static constexpr const char* pages[] = {
		"MainPage",
		"Esp",
		"Aimbot",
		"Memory",
		"Settings"
	};

	constexpr int HEIGHT = 550;
	constexpr int WIDTH = 599;

	inline bool isRunning = true;

	inline HWND window = nullptr;
	inline WNDCLASSEXA windowClass = { };

	inline POINTS position = { };

	inline PDIRECT3D9 d3d = nullptr;
	inline LPDIRECT3DDEVICE9 device = nullptr;
	inline D3DPRESENT_PARAMETERS presentParameters = { };

	//handle window creation and destruction

	void CreateHWindow(const char* windowName) noexcept;

	void DestroyHWindow() noexcept; 

	//handle device creation and destruction
	bool CreateDevice() noexcept;
	void ResetDevice() noexcept;
	void DestroyDevice() noexcept;

	//handle ImGui creation and destruction
	void CreateImGui() noexcept;
	void DestroyImGui() noexcept;


	//Handle render
	void BeginRender() noexcept;
	void EndRender() noexcept;
	void Render() noexcept;

}

namespace setting{
	inline bool enable = false;
	inline bool load = false;
	inline bool save = false;

}

namespace aimbot {
	inline bool silentEnable = false;
	inline bool enable = false;
	inline float fov = 0.0f;
	inline bool fovEnable = false;
	inline bool autofire = false;
	inline float smooth = 0.0f;
	inline bool visibleCheck = false;
	inline float fovRadius;
	inline bool aimKey = false;
	inline int FovTypes;
	inline ImVec4 fovColor;
	inline int hitBoxes;
}

namespace visual {
	inline bool enable = false;
	inline bool box = false;
	inline bool name = true;
	inline float nameColor;
	inline int healthBarStyle = 0;
	inline bool health = false;
	inline bool skeleton = false;
}

namespace misc {
	inline bool enable = false;
	inline bool noRecoil = false;
	inline bool noSpread = false;
	inline bool noFlash = false;
	inline bool bunnyHop = false;
}

void myStyle();
void drawMainPage();
void drawEspPage();
void drawAimBotPage();
void drawMemoryPage();
void drawSettingsPage();