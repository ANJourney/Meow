#include<Windows.h>
#include<string>
#include"Hook.h"
#include"RulesSettings.h"
#include"variable.h"

std::wstring Setpunct = punctlist;
std::wstring Setmoodw = moodwlist;
std::wstring Setbanw = banwlist;

void Initialization_Variable(){
	Setpunct = punctlist;
	Setmoodw = moodwlist;
	Setbanw = banwlist;
}