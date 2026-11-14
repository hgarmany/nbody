#include "toolbar.h"
#include "controls.h"
#include "../physics/physics.h"
#include "../util.h"

void drawToolbar(uint8_t* state, ImFont* font) {
	ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x - 100 - 2 * padding, padding), ImGuiCond_Always);

	ImGui::PushFont(font);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));

	// Create a window without a title or background, to act as a button container
	ImGui::Begin("Sandbox Button", nullptr,
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoDecoration);

	const char* sandboxIcon = "+";

	if (ImGui::Button(sandboxIcon, ImVec2(50, 50)))
		*state = STATE_SANDBOX;

	ImGui::SetWindowSize(ImVec2(50, 50));

	ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x - 50 - padding, padding), ImGuiCond_Always);

	ImGui::PushFont(font);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));

	// Create a window without a title or background, to act as a button container
	ImGui::Begin("Physics Button", nullptr,
		ImGuiWindowFlags_NoTitleBar |
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoDecoration);

	const char* physicsIcon = "\xE2\x9A\x99";  // UTF-8 encoding for ⚙

	if (ImGui::Button(physicsIcon, ImVec2(50, 50)))
		*state = STATE_PHYSICS;

	ImGui::SetWindowSize(ImVec2(50, 50));

	ImGui::End();

	ImGui::PopFont();
	ImGui::PopStyleVar();
	ImGui::PopStyleColor();
}

void drawSandboxDialog(uint8_t* state) {
	// sandbox controller
	bool sandboxOpen = true;
	ImGui::Begin("Sandbox", &sandboxOpen,
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_AlwaysAutoResize);

	float timeStepLog = (float)log10(timeStep);

	ImGui::Checkbox("Physics", &hasPhysics);
	ImGui::Text("Time Step (Logarithmic)");
	ImGui::SliderFloat("##timestep", &timeStepLog, 0, 10);
	ImGui::Checkbox("Trails", &doTrails);

	ImGui::SetWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x - ImGui::GetWindowSize().x - padding, padding), ImGuiCond_Always);

	ImGui::End();
	if (!sandboxOpen)
		*state = STATE_DEFAULT;

	timeStep = pow(10.0, (double)timeStepLog);
}

void drawPhysicsDialog(uint8_t* state) {
	// physics controller
	bool physicsSettingsOpen = true;
	ImGui::Begin("Settings", &physicsSettingsOpen,
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_AlwaysAutoResize);

	float timeStepLog = (float)log10(timeStep);

	ImGui::Checkbox("Physics", &hasPhysics);
	ImGui::Text("Time Step (Logarithmic)");
	ImGui::SliderFloat("##timestep", &timeStepLog, 0, 10);
	ImGui::Checkbox("Trails", &doTrails);

	ImGui::SetWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x - ImGui::GetWindowSize().x - padding, padding), ImGuiCond_Always);

	ImGui::End();
	if (!physicsSettingsOpen)
		*state = STATE_DEFAULT;

	timeStep = pow(10.0, (double)timeStepLog);
}