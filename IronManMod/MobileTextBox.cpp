#include "MobileTextBox.h"
#include "MobileTextures.h"
#include "Utility.h"
#include "Settings.h"
#include "plugin.h"
#include "game_sa\CFont.h"
#include "game_sa\CMessages.h"
#include "game_sa\CText.h"
#include "game_sa\common.h"
#include "game_sa\CSprite2d.h"
#include "game_sa\CTimer.h"
#include "game_sa\CHudColours.h"
#include "game_sa\CMenuManager.h"
#include "game_sa\CHud.h"
#include "game_sa\CCamera.h"
#include "game_sa\CCutsceneMgr.h"
#include "game_sa\CGarages.h"
#include "game_sa\CMenuSystem.h"
#include "game_sa\CStats.h"
#include "CFileLoader.h"

void MobileTextBox::InstallPatches() {
    plugin::patch::RedirectCall(0x58FCFA, MyHelpBox_Draw);
}



float MobileTextBox::GetHelpBoxXShift() {
	unsigned int radarmode;
	unsigned int isradarshown;
	radarmode = plugin::patch::GetInt(0xBA676C, false);
	isradarshown = plugin::patch::GetInt(0xBAA3FB, false);
    if (settings.bRadarTop && (radarmode != 2 || isradarshown != 0)) 
	{
        CVehicle *vehicle = FindPlayerVehicle(-1, 0);
		if (vehicle && vehicle->m_nModelIndex != MODEL_VORTEX && (vehicle->m_nVehicleSubClass == VEHICLE_PLANE || vehicle->m_nVehicleSubClass == VEHICLE_HELI))
		{
			return settings.fTextBoxPosnXWithRadarAndPlane;
		}
		else {
			if (FindPlayerPed(-1) && FindPlayerPed(-1)->m_aWeapons[FindPlayerPed(-1)->m_nActiveWeaponSlot].m_nType == WEAPON_PARACHUTE)
			{
				return settings.fTextBoxPosnXWithRadarAndPlane;
			}
			else 
			{
				return settings.fTextBoxPosnXWithRadar; 
			}
		}
    }
	else
	{
		return settings.vecTextBoxPosn.x; 
	}
}

float fTextBoxYWhenMenuEnabled = -80.0f;

bool StringFind(std::string &str, std::string substring);

bool StringFind(std::string &str, std::string substring) {
	size_t i = str.find(substring);
	size_t len = substring.length();
	if (i != std::string::npos) {
		return true;
	}
	return false;
}

void MobileTextBox::MyHelpBox_Draw() {
	if (boolvars.menuisactive == false && boolvars.alphafad==0) {
		if (CHud::m_pHelpMessage[0]) {
			//if (IsMenuEnabled()) {
				if (fTextBoxYWhenMenuEnabled < 0.0f)
					fTextBoxYWhenMenuEnabled += 5.0f;
			/*}
			else
				fTextBoxYWhenMenuEnabled = -80.0f;*/
			if (!CMessages::StringCompare(CHud::m_pHelpMessage, CHud::m_pLastHelpMessage, 400)) {
				switch (CHud::m_nHelpMessageState) {
				case 0: // STATE_FREE_TO_USE
					CHud::m_nHelpMessageState = 2; // STATE_FADE_OUT
					CHud::m_nHelpMessageTimer = 0;
					CHud::m_nHelpMessageFadeTimer = 0;
					CMessages::StringCopy(CHud::m_pHelpMessageToPrint, CHud::m_pHelpMessage, 400);
					CFont::SetOrientation(ALIGN_CENTER);
					CFont::SetJustify(false);
					CFont::SetFontStyle(FONT_MENU);
					CFont::SetBackground(true, true);
					CFont::SetDropShadowPosition(0);
					//CFont::SetEdge(2);
					CFont::SetScale(SCREEN_MULTIPLIER(settings.vecTextBoxFontScale.x), SCREEN_MULTIPLIER(settings.vecTextBoxFontScale.y));
					/*if (IsMenuEnabled())
						CHud::m_fHelpMessageTime = 4;
					else {*/
						//CFont::SetWrapx(SCREEN_COORD(GetHelpBoxXShift() + settings.fTextBoxWidth - settings.fTextBoxBorderSize) - 4.0f);
						CHud::m_fHelpMessageTime = CFont::GetNumberLines(SCREEN_COORD(GetHelpBoxXShift() + settings.fTextBoxBorderSize) + 4.0f,
							SCREEN_COORD(settings.vecTextBoxPosn.y), CHud::m_pHelpMessageToPrint) + 3;
					//}
					CFont::SetWrapx(SCREEN_WIDTH);
					plugin::CallMethod<0x506EA0>(0xB6BC90, 32, 0.0f, 1.0f);
					break;
				case 1:
				case 2:
				case 3:
				case 4:
					CHud::m_nHelpMessageState = 4; // STATE_REPLACE
					CHud::m_nHelpMessageTimer = 5;
					break;
				default:
					break;
				}
				CMessages::StringCopy(CHud::m_pLastHelpMessage, CHud::m_pHelpMessage, 400);
			}
			float alpha = 200.0f;
			if (CHud::m_nHelpMessageState) {
				switch (CHud::m_nHelpMessageState) {
				case 2: // STATE_FADE_OUT
					if (!TheCamera.m_bWideScreenOn) {
						CHud::m_nHelpMessageFadeTimer += 2 * (CTimer::ms_fTimeStep * 0.02f * 1000.0f);
						if (CHud::m_nHelpMessageFadeTimer > 0) {
							CHud::m_nHelpMessageFadeTimer = 0;
							CHud::m_nHelpMessageState = 1; // STATE_IDLE
						}
						alpha = CHud::m_nHelpMessageFadeTimer * 0.001f * 200.0f;
					}
					break;
				case 3: // STATE_FADE_IN
					CHud::m_nHelpMessageFadeTimer -= 2 * (CTimer::ms_fTimeStep * 0.02f * 1000.0f);
					if (CHud::m_nHelpMessageFadeTimer < 0 || TheCamera.m_bWideScreenOn) {
						CHud::m_nHelpMessageFadeTimer = 0;
						CHud::m_nHelpMessageState = 0; // STATE_FREE_TO_USE
					}
					alpha = CHud::m_nHelpMessageFadeTimer * 0.001f * 200.0f;
					break;
				case 4: // STATE_REPLACE
					CHud::m_nHelpMessageFadeTimer -= 2 * (CTimer::ms_fTimeStep * 0.02f * 1000.0f);
					if (CHud::m_nHelpMessageFadeTimer < 0) {
						CHud::m_nHelpMessageFadeTimer = 0;
						CHud::m_nHelpMessageState = 2; // FADE_OUT
						CMessages::StringCopy(CHud::m_pHelpMessageToPrint, CHud::m_pLastHelpMessage, 400);
					}
					alpha = CHud::m_nHelpMessageFadeTimer * 0.001f * 200.0f;
					break;
				case 1: // STATE_IDLE
					alpha = 255.0;
					CHud::m_nHelpMessageFadeTimer = 600;
					if (!CHud::m_bHelpMessagePermanent) {
						if (CHud::m_nHelpMessageTimer > CHud::m_fHelpMessageTime * 1000.0f || CHud::m_bHelpMessageQuick && CHud::m_nHelpMessageTimer > 3000) {
							CHud::m_nHelpMessageState = 3; // STATE_FADE_IN
							CHud::m_nHelpMessageFadeTimer = 600;
						}
					}
					break;
				default:
					break;
				}
				if (!CCutsceneMgr::ms_running) {
					CHud::m_nHelpMessageTimer += (CTimer::ms_fTimeStep * 0.02f * 1000.0f);
					CFont::SetAlphaFade(alpha);
					CFont::SetProportional(true);
					CFont::SetScale(SCREEN_MULTIPLIER(settings.vecTextBoxFontScale.x), SCREEN_MULTIPLIER(settings.vecTextBoxFontScale.y));
					if (CHud::m_nHelpMessageStatId) {
						if (TheCamera.m_bWideScreenOn) {
							CFont::SetAlphaFade(255.0f);
							return;
						}
						if (CHud::m_nHelpMessageStatId >= 10) {
							if (CHud::m_nHelpMessageStatId >= 100)
								sprintf(gString, "STAT%d", CHud::m_nHelpMessageStatId);
							else
								sprintf(gString, "STAT0%d", CHud::m_nHelpMessageStatId);
						}
						else
							sprintf(gString, "STAT00%d", CHud::m_nHelpMessageStatId);
						CFont::SetOrientation(ALIGN_LEFT);
						CFont::SetJustify(false);
						CFont::SetFontStyle(FONT_MENU);
						CFont::SetBackground(true, true);
						CFont::SetDropShadowPosition(0);
						CFont::SetBackgroundColor(CRGBA(0, 0, 0, alpha));
						CFont::SetColor(CRGBA(255, 255, 255, 255));

						
						//CFont::SetWrapx(RsGlobal.maximumWidth);
						float posX, posY;
						posY = SCREEN_COORD(settings.vecTextBoxPosn.y);
						posX = CFont::GetStringWidth(TheText.Get(gString), 1, 0) + SCREEN_COORD(GetHelpBoxXShift() + settings.fTextBoxBorderSize) + 4.0f;
						CFont::SetWrapx(SCREEN_COORD(settings.fTextBoxWidthWithProgressBar) + posX + 4.0f);
						//if (IsMenuEnabled()) {
							//posX = SCREEN_COORD_CENTER_X - CFont::GetStringWidth(TheText.Get(gString), 1, 0) / 2.0f;// +SCREEN_COORD(settings.vecTextBoxTextPosnWhenMenuEnabled.x);
							//posY = SCREEN_COORD(settings.vecTextBoxTextPosnWhenMenuEnabled.y);
							//CFont::SetWrapx(SCREEN_WIDTH);
						/*
							unsigned int savedShade;
							unsigned int savedAlpha;
							RwEngineInstance->dOpenDevice.fpRenderStateGet(rwRENDERSTATESHADEMODE, &savedShade);
							RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATESHADEMODE, (void *)2);
							RwEngineInstance->dOpenDevice.fpRenderStateGet(rwRENDERSTATEVERTEXALPHAENABLE, &savedAlpha);
							RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)TRUE);
							CSprite2d::DrawRect(CRect(0.0f, SCREEN_COORD(fTextBoxYWhenMenuEnabled), SCREEN_WIDTH, SCREEN_COORD(fTextBoxYWhenMenuEnabled + settings.fTextBoxHeightWhenMenuEnabled)),
								CRGBA(0, 0, 0, 0), CRGBA(0, 0, 0, 0),
								CRGBA(0, 27, 63, 255), CRGBA(0, 27, 63, 255));
							RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATESHADEMODE, (void *)savedShade);
							RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)savedAlpha);
							*/
							CFont::PrintString(SCREEN_COORD(GetHelpBoxXShift() + settings.fTextBoxBorderSize) + 4.0f, SCREEN_COORD(settings.vecTextBoxPosn.y), TheText.Get(gString));
							//CSprite2d::DrawRect(CRect(0.0f, SCREEN_COORD(fTextBoxYWhenMenuEnabled), SCREEN_WIDTH, SCREEN_COORD(fTextBoxYWhenMenuEnabled + settings.fTextBoxHeightWhenMenuEnabled)), CRGBA(0, 0, 0, 255));
							/*}
						else {
							CFont::SetBackground(true, true);
							CFont::PrintString(SCREEN_COORD(settings.vecTextBoxTextPosnWhenMenuEnabled.x),
								SCREEN_COORD(fTextBoxYWhenMenuEnabled + settings.vecTextBoxTextPosnWhenMenuEnabled.y), TheText.Get(gString));
						
							
						}*/
						float progress = 0.0f;
						if (CHud::m_nHelpMessageStatId == 336)
							progress = plugin::CallMethodAndReturn<unsigned int, 0x5F6AA0>(0xC09928 + FindPlayerPed(-1)->m_pPlayerData->m_nPlayerGroup * 0x2D4);
						else
							progress = CStats::GetStatValue(CHud::m_nHelpMessageStatId);
						if (CHud::m_pHelpMessageToPrint[0] == '+') {
							CFont::SetBackground(false, false);
							DrawProgressBarWithProgressDelta_BilinearOffset(posX + SCREEN_COORD(10.0f), SCREEN_COORD(5.0f * 0.6f) +
								posY + SCREEN_COORD(settings.fTextBoxProgressBarYShift), SCREEN_COORD(settings.vecTextBoxProgressBarSize.x),
								SCREEN_COORD(settings.vecTextBoxProgressBarSize.y), fmaxf((1.0f / CHud::m_nHelpMessageMaxStatValue) * progress * 100.0f, 2.0f),
								HudColour.GetRGB(HUD_COLOUR_WHITE, alpha), fmaxf((1.0f / CHud::m_nHelpMessageMaxStatValue) * CHud::m_fHelpMessageStatUpdateValue * 100.0f, 3.0f),
								HudColour.GetRGB(HUD_COLOUR_GREEN, alpha));
							CFont::PrintString(posX + SCREEN_COORD(settings.fTextBoxPlusMinTextXShift), posY, CHud::m_pHelpMessageToPrint);
						}
						else {
							CFont::SetBackground(false, false);
							DrawProgressBarWithProgressDelta_BilinearOffset(posX + SCREEN_COORD(10.0f), SCREEN_COORD(5.0f * 0.6f) +
								posY + SCREEN_COORD(settings.fTextBoxProgressBarYShift), SCREEN_COORD(settings.vecTextBoxProgressBarSize.x),
								SCREEN_COORD(settings.vecTextBoxProgressBarSize.y), fmaxf(1.0f / CHud::m_nHelpMessageMaxStatValue * progress * 100.0f, 2.0f),
								HudColour.GetRGB(HUD_COLOUR_WHITE, alpha), fmaxf((1.0f / CHud::m_nHelpMessageMaxStatValue) * CHud::m_fHelpMessageStatUpdateValue * 100.0f, 3.0f),
								HudColour.GetRGB(HUD_COLOUR_RED, alpha));
							CFont::PrintString(posX + SCREEN_COORD(settings.fTextBoxPlusMinTextXShift), posY, CHud::m_pHelpMessageToPrint);
						}
					}
					else {
						if (CHud::m_BigMessage[0][0] || CHud::m_BigMessage[4][0] || CGarages::MessageIDString[0]) {
							CFont::SetAlphaFade(255.0f);
							return;
						}
						int baseY = 0;
						if (TheCamera.m_bWideScreenOn && !FrontEndMenuManager.m_bWidescreenOn)
							baseY = 56;
						//if (IsMenuEnabled()) {
						std::string strHelpMessage = CHud::m_pHelpMessageToPrint;
						/*(!StringFind(strHelpMessage, "~n~")
						&& !StringFind(strHelpMessage, "~N~"))
						||*/
						if (IsMenuEnabled())
						{
							StringReplace(strHelpMessage, "~n~", " ");
							StringReplace(strHelpMessage, "~N~", " ");
							CFont::SetWrapx(SCREEN_WIDTH);
							CFont::SetBackground(false, false);

							CFont::SetOrientation(ALIGN_CENTER);
							CFont::SetJustify(false);
							CFont::SetFontStyle(FONT_MENU);
							CFont::SetDropShadowPosition(0);
							CFont::SetBackgroundColor(CRGBA(0, 0, 0, alpha));
							CFont::SetColor(CRGBA(255, 255, 255, 255));

							unsigned int savedShade;
							unsigned int savedAlpha;
							RwEngineInstance->dOpenDevice.fpRenderStateGet(rwRENDERSTATESHADEMODE, &savedShade);
							RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATESHADEMODE, (void *)2);
							RwEngineInstance->dOpenDevice.fpRenderStateGet(rwRENDERSTATEVERTEXALPHAENABLE, &savedAlpha);
							RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)TRUE);
							CSprite2d::DrawRect(CRect(0.0f, SCREEN_COORD(fTextBoxYWhenMenuEnabled), SCREEN_WIDTH, SCREEN_COORD(fTextBoxYWhenMenuEnabled + settings.fTextBoxHeightWhenMenuEnabled)),
								CRGBA(0, 0, 0, 0), CRGBA(0, 0, 0, 0),
								CRGBA(0, 0, 0, 255), CRGBA(0, 0, 0, 255));//CRGBA(0, 27, 63, 255), CRGBA(0, 27, 63, 255));
							RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATESHADEMODE, (void *)savedShade);
							RwEngineInstance->dOpenDevice.fpRenderStateSet(rwRENDERSTATEVERTEXALPHAENABLE, (void *)savedAlpha);
							boolvars.bottomtextboxcoord = 0.0f;
							//CSprite2d::DrawRect(CRect(0.0f, SCREEN_COORD(fTextBoxYWhenMenuEnabled), SCREEN_WIDTH, SCREEN_COORD(fTextBoxYWhenMenuEnabled + settings.fTextBoxHeightWhenMenuEnabled)), CRGBA(0, 0, 0, 255));

							CFont::PrintString(SCREEN_COORD_CENTER_X, //SCREEN_COORD(settings.vecTextBoxTextPosnWhenMenuEnabled.x),
								SCREEN_COORD(fTextBoxYWhenMenuEnabled + settings.vecTextBoxTextPosnWhenMenuEnabled.y), const_cast<char *>(strHelpMessage.c_str()));
						}
						else
						{

							CFont::SetOrientation(ALIGN_LEFT);
							CFont::SetJustify(false);
							CFont::SetFontStyle(FONT_MENU);
							CFont::SetDropShadowPosition(0);
							CFont::SetBackgroundColor(CRGBA(0, 0, 0, alpha));
							CFont::SetColor(CRGBA(255, 255, 255, 255));

							if (CHud::m_fHelpMessageBoxWidth == 200.0f)
								CFont::SetWrapx(SCREEN_COORD(GetHelpBoxXShift() + settings.fTextBoxWidth - settings.fTextBoxBorderSize) - 4.0f);
							else
								CFont::SetWrapx(SCREEN_COORD(GetHelpBoxXShift() + (CHud::m_fHelpMessageBoxWidth - settings.fTextBoxBorderSize)) - 4.0f);
							CFont::SetBackground(true, true);
							CFont::PrintString(SCREEN_COORD(GetHelpBoxXShift() + settings.fTextBoxBorderSize) + 4.0f,
								SCREEN_COORD(baseY * 0.6f) + SCREEN_COORD(settings.vecTextBoxPosn.y), CHud::m_pHelpMessageToPrint);
						}
						/*}
						else {
							
						}*/
					}
					CFont::SetWrapx(SCREEN_WIDTH);
					CFont::SetAlphaFade(255.0f);
					return;
				}
			}
		}
		else
			CHud::m_nHelpMessageState = 0;
	}
}

bool MobileTextBox::IsMenuEnabled() {
    return CMenuSystem::MenuInUse[0] || CMenuSystem::MenuInUse[1];
}