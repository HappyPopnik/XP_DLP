typedef struct IUnknown IUknown;

#include "Function_Helper.h"

#include <iostream>
#include <windows.h>
#include <Shlwapi.h>
#include <time.h>
#include <fstream>
#include <string>
#include <vector>
#include <ctime>
#include <random>
#include <sstream>
#include <iomanip>
#include "Sha256.h"
#include <mutex>
#include <tchar.h>
#include <cstring>
#include <cctype>

#pragma comment(lib, "Shlwapi.lib")

std::mutex mtx;

bool FileExists(const std::string& filePath)
{
    std::wstring tempstr = std::wstring(filePath.begin(), filePath.end());
    LPCWSTR wpath = tempstr.c_str();
    GetFileAttributes(wpath);
    if (INVALID_FILE_ATTRIBUTES == GetFileAttributes(wpath) && GetLastError() == ERROR_FILE_NOT_FOUND)
        return false;
    return true;
}

bool CopyMyself()
{    
    wchar_t currentExePath[MAX_PATH];
    GetModuleFileName(NULL, currentExePath, MAX_PATH);
    const wchar_t* destinationFolder = L"c:\\windows\\system32";
    wchar_t destinationPath[MAX_PATH];
    PathCombine(destinationPath, destinationFolder, PathFindFileName(currentExePath));
    if (CopyFile(currentExePath, destinationPath, FALSE))
    {
        std::cout << "successfully copied executable" << std::endl;
        return true;
    }
    DWORD error = GetLastError();
    std::cout << "                                                                                                                                                       couldn't copy File, Error: " << error << std::endl;
    return false;
}

std::string extractFileName(const std::string& filePath)
{
    size_t lastSlash = filePath.find_last_of("\\");
    if (lastSlash != std::string::npos)
    {
        return filePath.substr(lastSlash + 1);
    }
    return filePath;
}

char generateRandomChar()
{
    const char *charset = "0123456789ABCDEF";
    const size_t maxIndex = strlen(charset) - 1;
    int a = rand();
    return charset[a % maxIndex];

}

std::wstring s2ws(const std::string& str)
{
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}

std::string ws2s(const std::wstring& wstringconv)
{
    std::string strconv(wstringconv.begin(), wstringconv.end());
    return strconv;
}

int ReadLinesFromFile(const std::string& filename, std::string lines[])
{
    std::ifstream file(filename);
    if (file.is_open())
    {
        std::string line;
        int line_count = 0;
        while ((std::getline(file, line)) && line_count < 200)
        {
            for (char &c : line)
            {
                std::transform(line.begin(), line.end(), line.begin(), [](unsigned char c) {return std::tolower(c); });
            }
            lines[line_count++] = line;
        }

        file.close();
        return line_count;
    }
    else
    {
        std::cerr << "Unable to open file" << std::endl;
        return -1;
    }
}

std::string removeCharacters(std::string input)
{
    input.erase(0, 2);
    input.erase(4, 2);
    return input;
}

void writeToUSBDevicesFile(std::wstring vid, std::wstring pid, std::wstring serial, std::wofstream& outFile)
{
    char num1 = generateRandomChar();
    std::string num1s(1, num1);
    char num2 = generateRandomChar();
    std::string num2s(1, num2);
    char num3 = generateRandomChar();
    std::string num3s(1, num3);
    char num4 = generateRandomChar();
    std::string num4s(1, num4);
    std::wstring combined = s2ws(num1s) + s2ws(num2s) + vid + s2ws(num3s) + s2ws(num4s) + pid + serial;
    std::wcout << combined << std::endl;

    if (outFile.is_open())
    {
        outFile << combined << L"\n";
        outFile.close();
        LogWriter("New VID - PID was written to file");
    }
    else
    {
        LogWriter("Unable to open file");
    }
}

const char* GetCurrentDate(const char* format, char* a)
// This function returns the date, in the format given.
{
    std::time_t currentTime;
    std::time(&currentTime);
    std::tm timeInfo;
    localtime_s(&timeInfo, &currentTime);
    char buffer[20];
    std::strftime(buffer, sizeof(buffer), format, &timeInfo);
    char* currentDaterime = buffer;
    a = (char*)malloc(strlen(currentDaterime) + 1);
    strcpy_s(a, strlen(currentDaterime) + 1, currentDaterime);
    return a;
}

char* GetFutureDate(const char* format, char* a)
// This function returns the date in 1 hour, in the format given.
{
    std::time_t currentTime;
    std::time(&currentTime);
    std::tm timeInfo;
    localtime_s(&timeInfo, &currentTime);
    timeInfo.tm_hour += 1;
    if (timeInfo.tm_hour >= 24)
    {
        timeInfo.tm_hour -= 24;
        timeInfo.tm_mday += 1;
    }
    char buffer[20];
    std::strftime(buffer, sizeof(buffer), format, &timeInfo);
    char* currentDaterime = buffer;
    a = (char*)malloc(strlen(currentDaterime) + 1);
    strcpy_s(a, strlen(currentDaterime) + 1, currentDaterime);
    return a;
}

std::string StringToHex(const std::string& input)
// This function receives a String and converts it into a hex string.
{
    std::stringstream ss;
    ss << std::hex << std::setfill('O');
    for (char ch : input)
    {
        ss << std::setw(2) << static_cast<int>(ch);
    }
    return ss.str();
}

int Challange_Response()
// This function generated the challenge-response key.
{
    // Generate a random number between 100000000 and 999999999
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dist6(10000000, 99999999);
    int randomnumber = dist6(rng);
    std::cout << "Provide Nitur your current date and the following number: " << randomnumber << std::endl;

    // Convert the number to a string
    std::string converter = std::to_string(randomnumber);
    const char* randomnumber_str = converter.c_str();

    //Get current date
    char* pTimestamp = nullptr;
    const char* date = GetCurrentDate("%Y-%m-%d", pTimestamp);
    char* date_char = (char*)date;
    char* rndnum_char = (char*)randomnumber_str;

    // get the hashes of the date and random number, get last 8 characters of each one, convert to hex, and combine.
    long long hx_num_date_int = stoll(StringToHex(SHA256(date_char).substr(SHA256(date_char).size() - 8)));
    long long hx_num_rnd_int = stoll(StringToHex(SHA256(rndnum_char).substr(SHA256(rndnum_char).size() - 8)));
    int combination = (hx_num_date_int % 100000000) + (hx_num_rnd_int % 100000000);
    free(pTimestamp);
    return combination;
}

void LogWriter2(std::string log)
{
    //mtx.lock();
    std::wofstream LogFile{};
    LogFile.open("c:\\windows\\system32\\Errorlog.txt", std::ios::app);
    LogFile << log.c_str() << L"\n";
    LogFile.close();
    //mtx.unlock();
}

void LogWriter(std::string LogDetails)
{
    mtx.lock();
    HKEY hKey_new;
    const char* keypath2 = "SOFTWARE\\Microsoft\\Hardening\\Log";
    const char* LogName;
    LONG createresult = RegCreateKeyExA(HKEY_LOCAL_MACHINE, keypath2, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS, NULL, &hKey_new, NULL);
    if (createresult == ERROR_SUCCESS)
    {
        std::time_t currentTime = std::time(nullptr);
        std::tm localTime;
        localtime_s(&localTime, &currentTime);
        char buffer[20];
        std::strftime(buffer, sizeof(buffer), "%d-%m-%Y %H:%M:%S", &localTime);
        LogName = buffer;
        LONG log_value_result = RegSetValueExA(hKey_new, LogName, 0, REG_SZ, LPBYTE(LogDetails.c_str()), strlen(LogDetails.c_str()) + 1);
    }
    RegCloseKey(hKey_new);
    LogWriter2(LogDetails);
    mtx.unlock();
}

char* convertToChar(const TCHAR* tchararray)
{
    size_t length = wcslen(tchararray);
    char* charArray = new char[length + 1];
    wcstombs(charArray, tchararray, length + 1);
    return charArray;
}