#pragma once

#include <stdint.h>
#include <string>
#include <vector>
#include <map>
#include <memory>

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

typedef std::string String;
typedef std::wstring WString;

extern HINSTANCE GInstance;
extern HWND GWnd;
extern bool GAppShouldRun;

void ShowAlert(const String& msg);

#define checkf(Condition, Msg) { if(!(Condition)) { ShowAlert(String("Assertion failed: ").append(Msg));  __debugbreak(); } }
#define check(Condition) { checkf(Condition, "") }