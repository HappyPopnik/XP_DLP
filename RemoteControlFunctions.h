#pragma once
#include <Windows.h>

void SetRegistryValueTo0(HKEY hKey, const wchar_t* subKey, const wchar_t* valueName);
void SetRegistryValueTo1(HKEY hKey, const wchar_t* subKey, const wchar_t* valueName);
void SetRegistryValueTominus1(HKEY hKey, const wchar_t* subKey, const wchar_t* valueName);