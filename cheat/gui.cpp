#define _CRT_SECURE_NO_WARNINGS
#include "gui.h"

#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_dx9.h"
#include "../imgui/imgui_impl_win32.h"
#include <ctime>
#include "../imgui/imgui_internal.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(
	HWND window,
	UINT message,
	WPARAM wideParameter,
	LPARAM longParameter
);

long __stdcall WindowProcess(
	HWND window,
	UINT message,
	WPARAM wideParameter,
	LPARAM longParameter)
{
	if (ImGui_ImplWin32_WndProcHandler(window, message, wideParameter, longParameter))
		return true;

	switch (message)
	{
	case WM_SIZE: {
		if (gui::device && wideParameter != SIZE_MINIMIZED)
		{
			gui::presentParameters.BackBufferWidth = LOWORD(longParameter);
			gui::presentParameters.BackBufferHeight = HIWORD(longParameter);
			gui::ResetDevice();
		}
	}return 0;

	case WM_SYSCOMMAND: {
		if ((wideParameter & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
			return 0;
	}break;

	case WM_DESTROY: {
		PostQuitMessage(0);
	}return 0;

	case WM_LBUTTONDOWN: {
		gui::position = MAKEPOINTS(longParameter); // set click points
	}return 0;

	case WM_MOUSEMOVE: {
		if (wideParameter == MK_LBUTTON)
		{
			const auto points = MAKEPOINTS(longParameter);
			auto rect = ::RECT{ };

			GetWindowRect(gui::window, &rect);

			rect.left += points.x - gui::position.x;
			rect.top += points.y - gui::position.y;

			if (gui::position.x >= 0 &&
				gui::position.x <= gui::WIDTH &&
				gui::position.y >= 0 && gui::position.y <= 19)
				SetWindowPos(
					gui::window,
					HWND_TOPMOST,
					rect.left,
					rect.top,
					0, 0,
					SWP_SHOWWINDOW | SWP_NOSIZE | SWP_NOZORDER
				);
		}

	}return 0;

	}

	return DefWindowProc(window, message, wideParameter, longParameter);
}

void gui::CreateHWindow(const char* windowName) noexcept
{
	windowClass.cbSize = sizeof(WNDCLASSEX);
	windowClass.style = CS_CLASSDC;
	windowClass.lpfnWndProc = WindowProcess;
	windowClass.cbClsExtra = 0;
	windowClass.cbWndExtra = 0;
	windowClass.hInstance = GetModuleHandleA(0);
	windowClass.hIcon = 0;
	windowClass.hCursor = 0;
	windowClass.hbrBackground = 0;
	windowClass.lpszMenuName = 0;
	windowClass.lpszClassName = "class001";
	windowClass.hIconSm = 0;

	RegisterClassEx(&windowClass);

	window = CreateWindowEx(
		0,
		"class001",
		windowName,
		WS_POPUP,
		100,
		100,
		WIDTH,
		HEIGHT,
		0,
		0,
		windowClass.hInstance,
		0
	);

	ShowWindow(window, SW_SHOWDEFAULT);
	UpdateWindow(window);
}

void gui::DestroyHWindow() noexcept
{
	DestroyWindow(window);
	UnregisterClass(windowClass.lpszClassName, windowClass.hInstance);
}

bool gui::CreateDevice() noexcept
{
	d3d = Direct3DCreate9(D3D_SDK_VERSION);

	if (!d3d)
		return false;

	ZeroMemory(&presentParameters, sizeof(presentParameters));

	presentParameters.Windowed = TRUE;
	presentParameters.SwapEffect = D3DSWAPEFFECT_DISCARD;
	presentParameters.BackBufferFormat = D3DFMT_UNKNOWN;
	presentParameters.EnableAutoDepthStencil = TRUE;
	presentParameters.AutoDepthStencilFormat = D3DFMT_D16;
	presentParameters.PresentationInterval = D3DPRESENT_INTERVAL_ONE;

	if (d3d->CreateDevice(
		D3DADAPTER_DEFAULT,
		D3DDEVTYPE_HAL,
		window,
		D3DCREATE_HARDWARE_VERTEXPROCESSING,
		&presentParameters,
		&device) < 0)
		return false;

	return true;
}

void gui::ResetDevice() noexcept
{
	ImGui_ImplDX9_InvalidateDeviceObjects();

	const auto result = device->Reset(&presentParameters);

	if (result == D3DERR_INVALIDCALL)
		IM_ASSERT(0);

	ImGui_ImplDX9_CreateDeviceObjects();
}

void gui::DestroyDevice() noexcept
{
	if (device)
	{
		device->Release();
		device = nullptr;
	}

	if (d3d)
	{
		d3d->Release();
		d3d = nullptr;
	}
}

void gui::CreateImGui() noexcept
{
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ::ImGui::GetIO();

	io.IniFilename = NULL;

	myStyle();

	ImGui_ImplWin32_Init(window);
	ImGui_ImplDX9_Init(device);
}

void gui::DestroyImGui() noexcept
{
	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}

void gui::BeginRender() noexcept
{
	MSG message;
	while (PeekMessage(&message, 0, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&message);
		DispatchMessage(&message);

		if (message.message == WM_QUIT)
		{
			isRunning = !isRunning;
			return;
		}
	}

	// Start the Dear ImGui frame
	ImGui_ImplDX9_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
}

void gui::EndRender() noexcept
{
	ImGui::EndFrame();

	device->SetRenderState(D3DRS_ZENABLE, FALSE);
	device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	device->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);

	device->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_RGBA(0, 0, 0, 255), 1.0f, 0);

	if (device->BeginScene() >= 0)
	{
		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
		device->EndScene();
	}

	const auto result = device->Present(0, 0, 0, 0);

	// Handle loss of D3D9 device
	if (result == D3DERR_DEVICELOST && device->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
		ResetDevice();
}

std::string gui::GetCurrentTimeAndDate()
{
	std::time_t now = std::time(nullptr);
	std::tm* timeinfo = std::localtime(&now);
	char buffer[80];
	std::strftime(buffer, 80, "%H:%M - %b-%d-%Y", timeinfo);
	return std::string(buffer);
}



void drawMainPage()
{
	//we will create main page here where at bottom there will be textbox and users can live chat with other cheat users

}

// Function to draw the ESP page
void drawEspPage() {
	ImGui::BeginChild("ESPSettings", ImVec2(250, 0), true);
	ImGui::Checkbox("Names", &visual::name);
	ImGui::Checkbox("Health Bars", &visual::health);
	ImGui::EndChild();

	ImGui::SameLine(); // Ensure the following content is on the same line

	ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);

	ImGui::SameLine(); // Ensure the following content is on the same line

	ImGui::BeginChild("ESPSettingsDetails", ImVec2(0, 0), true);

	ImGui::BeginTabBar("##RightContent", ImGuiTabBarFlags_None);

	if (visual::name) {

		ImGui::BeginTabItem("Player Names Color");
		ImGui::ColorEdit4("##PlayerNamesColor", &visual::nameColor);
		ImGui::EndTabItem();

	}

	ImGui::SameLine();
	if (visual::health) {

		ImGui::BeginTabItem("Health Bar Color");
		ImGui::Combo("##HealthBarStyle", &visual::healthBarStyle, "Solid\0Gradient\0");
		ImGui::EndTabItem();
	}
	ImGui::EndTabBar();


	ImGui::EndChild();
}


void drawAimBotPage()
{
	//we will create aimbot page here where user can enable aimbot and change aimbot settings

}
void drawMemoryPage()
{
	//we will create memory page here where user can view memory and change memory settings

}
void drawSettingsPage()
{
	//we will create settings page here where user can change settings of cheat
	//like loading config,saving config, check for updates etc


}




void gui::Render() noexcept {
	ImGui::SetNextWindowPos(ImVec2(0, 0));
	ImGui::SetNextWindowSize(ImVec2(WIDTH, HEIGHT));

	ImGui::Begin(("Np-Hacks - 0.1 (" + GetCurrentTimeAndDate() + ")").c_str(), &isRunning, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

	static int ActivePage = 0; // Variable to track the active page

	// Page buttons
	for (int i = 0; i < IM_ARRAYSIZE(pages); ++i) {
		// Set button color based on state
		ImGui::PushStyleColor(ImGuiCol_Button, (ActivePage == i) ? ImVec4(0.46f, 0.47f, 0.48f, 1.0f) : ImVec4(0.26f, 0.26f, 0.26f, 1.0f)); // Change color to dark if active

		// Check if button is clicked
		if (ImGui::Button(pages[i], ImVec2(110, 0))) {
			// Update active page
			ActivePage = i;
		}

		// Reset button color
		ImGui::PopStyleColor();
		ImGui::SameLine();
	}

	ImGui::Spacing();
	ImGui::Separator();

	// Render content based on active page
	switch (ActivePage) {
	case 0:
		drawMainPage();
		break;
	case 1:
		drawEspPage();
		break;
	case 2:
		drawAimBotPage();
		break;
	case 3:
		drawMemoryPage();
		break;
	case 4:
		drawSettingsPage();
		break;
	}

	ImGui::End();
}



//lets give custom style to the gui
void myStyle() {
	auto& Style = ImGui::GetStyle();

	Style.WindowTitleAlign = ImVec2(0.5f, 0.5f);

	Style.FrameBorderSize = 0;
	Style.FrameRounding = 0;
	Style.WindowRounding = 0;
	Style.WindowBorderSize = 0;
	Style.FrameRounding = 2;
	Style.ChildRounding = 2;

	Style.Colors[ImGuiCol_TitleBgActive] = ImColor(66, 66, 66);
	Style.Colors[ImGuiCol_TitleBg] = ImColor(66, 66, 66);
	Style.Colors[ImGuiCol_WindowBg] = ImColor(20, 20, 17);
	Style.Colors[ImGuiCol_Button] = ImColor(66, 66, 66);
	Style.Colors[ImGuiCol_ButtonHovered] = ImColor(115, 115, 115);
	Style.Colors[ImGuiCol_ButtonActive] = ImColor(115, 115, 115);
	Style.Colors[ImGuiCol_TabActive] = ImColor(66, 66, 66);
	Style.Colors[ImGuiCol_Tab] = ImColor(66, 66, 66);
	Style.Colors[ImGuiCol_TabHovered] = ImColor(115, 115, 115);

}



