#pragma once
#include <string>

int RegistryUSBExtractor(const std::string& filePath);
bool BuildRegTree();
bool CheckTree();
void Learningmodupdate(const char* subKey, const char* value, int future);