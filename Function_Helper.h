#pragma once

#include <iostream>
#include <windows.h>

bool FileExists(const std::string& name);
bool CopyMyself();
std::string extractFileName(const std::string& filePath);
char generateRandomChar();
std::wstring s2ws(const std::string& str);
int ReadLinesFromFile(const std::string& filename, std::string lines[]);
std::string removeCharacters(std::string input);
std::string ws2s(const std::wstring& wstringconv);
void writeToUSBDevicesFile(std::wstring vid, std::wstring pid, std::wstring serial, std::wofstream& outFile);
const char* GetCurrentDate(const char* format, char* a);
char* GetFutureDate(const char* format, char* a);
int Challange_Response();
void LogWriter(std::string LogDetails);
void LogWriter2(std::string log);
char* convertToChar(const TCHAR* tchararray);