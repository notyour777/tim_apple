#include "esp.hpp"
#include "aim.hpp"
#include "misc.hpp"

#include "../util/utilFunctions.hpp"

void mainLoop(bool state, MemoryManagement::moduleData client) {
	// Classes
	CCSPlayerController		CCSPlayerController(client.base);
	C_CSPlayerPawn			C_CSPlayerPawn(client.base);
	CGameSceneNode			CGameSceneNode;
	LocalPlayer				localPlayer(client);


	// Shared variables (between features)
	view_matrix_t viewMatrix = MemMan.ReadMem<view_matrix_t>(client.base + offsets::clientDLL["dwViewMatrix"]["value"]);
	Vector3 baseViewAngles = MemMan.ReadMem<Vector3>(client.base + offsets::clientDLL["dwViewAngles"]["value"]);
	DWORD_PTR baseViewAnglesAddy = client.base + offsets::clientDLL["dwViewAngles"]["value"];

	// NOTE: Cheats that only need local player / visuals that don't relate to gameplay
	localPlayer.getPlayerPawn();
	// Aimbot FOV circle
	if (aimConf.fovCircle) {
		ImVec2 p = ImGui::GetWindowPos();
		float screenMidX = GetSystemMetrics(SM_CXSCREEN) / 2.f;
		float screenMidY = GetSystemMetrics(SM_CYSCREEN) / 2.f;

		ImGui::GetBackgroundDrawList()->AddCircle({ screenMidX ,screenMidY }, (aimConf.fov * 10), ImColor(1.f, 1.f, 1.f, 1.f), 0, 1.f);
	}
	// Recoil control
	if (aimConf.rcs) {
		aim::recoilControl(localPlayer, baseViewAnglesAddy);
	}

	// Bunny hop
	if (miscConf.bunnyHop) {
		misc::bunnyHop(client.base, localPlayer.getFlags());
	}

	// Flash
	if (miscConf.flash){
		localPlayer.noFlash();
	}

	// Tigger
	if (aimConf.trigger) {
		aim::triggerBot(localPlayer, client.base);
	}


	// Main loop
	for (int i = 0; i < 64; i++) {

		// Player controller
		CCSPlayerController.id = i;
		CCSPlayerController.getListEntry();
		CCSPlayerController.getController();
		if (CCSPlayerController.value == 0) continue;

		// Player pawn
		C_CSPlayerPawn.value = CCSPlayerController.getC_CSPlayerPawn();
		C_CSPlayerPawn.getListEntry();
		C_CSPlayerPawn.getPlayerPawn();
		C_CSPlayerPawn.getPawnHealth();

		// Game scene node
		CGameSceneNode.value = C_CSPlayerPawn.getCGameSceneNode();
		
		// Checks
		if ((C_CSPlayerPawn.getPawnHealth() <= 0 || C_CSPlayerPawn.getPawnHealth() > 100) || localPlayer.getTeam() == CCSPlayerController.getPawnTeam()) {
			continue;
		}

		// ESP
		if (espConf.state) {
			esp::sharedData::weaponID = C_CSPlayerPawn.getWeaponID();
			esp::sharedData::localOrigin = localPlayer.getOrigin();
			esp::sharedData::entityOrigin = C_CSPlayerPawn.getOrigin();
			esp::sharedData::distance = (int)(utils::getDistance(esp::sharedData::localOrigin, esp::sharedData::entityOrigin)) / 100;

			if (espConf.checkSpotted) {
				if (SharedFunctions::spottedCheck(C_CSPlayerPawn, localPlayer)) {
					esp::boundingBox(C_CSPlayerPawn.getOrigin(), viewMatrix, CCSPlayerController.getPawnName(), C_CSPlayerPawn.pawnHealth, CGameSceneNode.getBoneArray(),true);
				}
			}
			else {
				esp::boundingBox(C_CSPlayerPawn.getOrigin(), viewMatrix, CCSPlayerController.getPawnName(), C_CSPlayerPawn.pawnHealth, CGameSceneNode.getBoneArray(),SharedFunctions::spottedCheck(C_CSPlayerPawn, localPlayer));
			}
		}

		// Aim
		if (aimConf.state) {
			localPlayer.getCameraPos();
			localPlayer.getViewAngles();

			if (aimConf.checkSpotted) {
				if (SharedFunctions::spottedCheck(C_CSPlayerPawn, localPlayer)) {
					aim::aimBot(localPlayer, baseViewAngles, baseViewAnglesAddy, CGameSceneNode.getBoneArray());
				}
			}
			else {
				aim::aimBot(localPlayer, baseViewAngles, baseViewAnglesAddy, CGameSceneNode.getBoneArray());
			}
		}
	}
}