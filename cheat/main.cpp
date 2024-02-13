
#include "gui.h"
#include <thread>

int __stdcall wWinMain(
	HINSTANCE instance,
	HINSTANCE previousInstance,
	PWSTR arguments,
	int commandShow)
{
	gui::CreateHWindow("cheat");
	if (!gui::CreateDevice())
		return 0;
	gui::CreateImGui();

	while (gui::isRunning) { //while gui::exit is false, render the 
		gui::BeginRender();
		gui::Render();
		gui::EndRender();
		std::this_thread::sleep_for(std::chrono::milliseconds(5));
	}

	gui::DestroyImGui();
	gui::DestroyDevice();
	gui::DestroyHWindow();
	return EXIT_SUCCESS;
}

