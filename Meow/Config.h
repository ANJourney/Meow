#pragma once


#include <fstream>
#include <iostream>
#include <Windows.h>
#include "ReplaceSettings.h"
#include "RulesSettings.h"
#include "variable.h"

extern bool isdiscard;
extern bool ISFIRST;

void Save();
void Load();