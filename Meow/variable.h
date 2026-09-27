#pragma once

#include<Windows.h>
#include<string>
#include<vector>
#include"Hook.h"
#include"RulesSettings.h"

extern std::wstring Setpunct;
extern std::wstring Setmoodw;
extern std::wstring Setbanw;

extern std::vector<ReplaceRule> g_ReplaceRules;

void Initialization_Variable();