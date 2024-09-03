#include "ImGuiDemoWindow.h"

#include "ImGuiWrap/ImGuiWrap.h"

static ImGuiDemoWindow::Options s_options;
static bool s_visible;
static bool s_showHelloWorldWindow = false;
static bool s_showAnotherWindow = false;

void ImGuiDemoWindow::Update(ImVec4& clearColor)
{
	if (s_visible)
		ImGui::ShowDemoWindow(&s_visible);

	if (s_options.showAboutWindow)
		ImGui::ShowAboutWindow(&s_options.showAboutWindow);

	if (s_showHelloWorldWindow)
	{
		static float f = 0.0f;
		static int counter = 0;

		ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

		ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
		ImGui::Checkbox("Demo Window", &s_visible);      // Edit bools storing our window open/close state
		ImGui::Checkbox("Another Window", &s_showAnotherWindow);

		ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
		ImGui::ColorEdit3("clear color", (float*)&clearColor); // Edit 3 floats representing a color

		if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
			counter++;
		ImGui::SameLine();
		ImGui::Text("counter = %d", counter);

		ImGuiIO& io = ImGui::GetIO();
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
		ImGui::End();
	}

	if (s_showAnotherWindow)
	{
		ImGui::Begin("Another Window", &s_showAnotherWindow);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
		ImGui::Text("Hello from another window!");
		if (ImGui::Button("Close Me"))
			s_showAnotherWindow = false;
		ImGui::End();
	}
}

bool ImGuiDemoWindow::IsVisible()
{
	return s_visible;
}

void ImGuiDemoWindow::SetVisible(bool visible)
{
	s_visible = visible;
}

ImGuiDemoWindow::Options& ImGuiDemoWindow::GetOptions()
{
	return s_options;
}
