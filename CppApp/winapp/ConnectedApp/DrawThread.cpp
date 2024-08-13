#include "DrawThread.h"
#include "GuiMain.h"
#include "../../shared/ImGuiSrc/imgui.h"

void DrawAppWindow(void* common_ptr)
{
	auto common = (CommonObjects*)common_ptr;
	ImGui::Begin("Connected!");
	ImGui::Text("this is our draw callback");
	static char buff[200];
	ImGui::InputText("URL", buff, sizeof(buff));
	ImGui::SameLine();
	if(ImGui::Button("set"))
		common->url = buff;
	ImGui::End();
}

void DrawThread::operator()(CommonObjects& common)
{
	//GuiMain([] { DrawAppWindow(); });
	GuiMain(DrawAppWindow, &common);
	common.exit_flag = true;
}

