#include <StdInc.h>
#include "Utility/InitFunction.h"
#include "Functions/Global.h"
#include "MinHook.h"
#include <Utility/Hooking.Patterns.h>
#include <thread>
#include <iostream>
#include <Windowsx.h>
#include <Utility/TouchSerial/MT6.h>
#include <fstream>
#ifdef _M_AMD64

#pragma optimize("", off)
#pragma comment(lib, "Ws2_32.lib")

extern LPCSTR hookPort;
static uintptr_t imageBase;
static unsigned char hasp_buffer[0xD40];
static bool isFreePlay;
static bool isEventMode2P;
static bool isEventMode4P;
static const char *ipaddr;


// Data for IC card, Force Feedback etc OFF.
static unsigned char settingData_W5X[405] = {
	0x1F, 0x8B, 0x08, 0x08, 0x53, 0x6A, 0x8B, 0x5A, 0x00, 0x00, 0x73, 0x65,
	0x74, 0x74, 0x69, 0x6E, 0x67, 0x2E, 0x6C, 0x75, 0x61, 0x00, 0x85, 0x93,
	0xC9, 0x6E, 0xC2, 0x30, 0x10, 0x86, 0xEF, 0x79, 0x0A, 0x5E, 0x80, 0x8A,
	0x90, 0x80, 0xE8, 0xA1, 0x07, 0x08, 0xA4, 0x20, 0x11, 0x81, 0x20, 0x2A,
	0x52, 0x6F, 0xC6, 0x19, 0x88, 0x85, 0x17, 0xE4, 0xD8, 0xAD, 0x78, 0xFB,
	0xDA, 0x59, 0x1D, 0xB5, 0x2A, 0x39, 0x44, 0xF9, 0xBF, 0x59, 0x32, 0x8B,
	0x3D, 0x1C, 0xFE, 0xFF, 0x78, 0xF6, 0x35, 0x28, 0x40, 0x29, 0xC2, 0xAF,
	0x2F, 0x54, 0x23, 0xEF, 0x49, 0xC0, 0xD0, 0xF3, 0x58, 0x84, 0x28, 0x39,
	0xAF, 0x11, 0xCF, 0x28, 0x44, 0xC0, 0x15, 0xC8, 0xC1, 0xDB, 0x20, 0x08,
	0x27, 0xD3, 0x51, 0x6D, 0x9A, 0x63, 0x0C, 0xB4, 0xB5, 0x34, 0x74, 0x21,
	0xD1, 0x0D, 0x7E, 0xD1, 0x44, 0x28, 0x21, 0x5B, 0x3A, 0xF1, 0xFD, 0x9A,
	0xA7, 0x42, 0xE3, 0x7C, 0x0B, 0x17, 0x65, 0xE8, 0x78, 0x14, 0xCE, 0x5C,
	0x7C, 0x20, 0xD7, 0xDC, 0x72, 0x3F, 0x0C, 0x82, 0xA9, 0x6B, 0x48, 0xC5,
	0xFD, 0x2F, 0xBC, 0x10, 0x4A, 0x09, 0xD6, 0x25, 0x12, 0x84, 0x47, 0xB9,
	0x56, 0x60, 0x7D, 0x3D, 0xB6, 0xD0, 0x8F, 0x08, 0xC9, 0x2C, 0x12, 0x85,
	0xCD, 0x19, 0x78, 0xEC, 0x1D, 0x31, 0xA8, 0xD5, 0xD8, 0x7A, 0x73, 0x33,
	0x1B, 0xED, 0x90, 0x58, 0x53, 0x1A, 0x09, 0x2D, 0x8B, 0x86, 0x85, 0x86,
	0x49, 0x80, 0x3D, 0x45, 0x8F, 0x2A, 0xE5, 0x1E, 0x9F, 0x37, 0x59, 0xD5,
	0xE4, 0x06, 0xDB, 0xE4, 0x87, 0x6F, 0x57, 0x7D, 0x00, 0xCF, 0x9A, 0x21,
	0x24, 0x57, 0xD7, 0x1E, 0x0B, 0x89, 0x21, 0x06, 0xC8, 0xCE, 0x08, 0xDF,
	0x2A, 0x74, 0x22, 0xBC, 0x98, 0xF3, 0xEC, 0x00, 0x0C, 0x99, 0xAF, 0x2A,
	0xFF, 0xEA, 0xCB, 0x0C, 0x2C, 0x11, 0x19, 0x54, 0x2E, 0xAD, 0x5C, 0x92,
	0xB2, 0x1E, 0x17, 0x99, 0x42, 0x79, 0x5D, 0x63, 0x44, 0x45, 0x01, 0xE9,
	0xE3, 0x0E, 0x75, 0x63, 0x56, 0x1E, 0x35, 0x37, 0xEA, 0x75, 0x5A, 0xCB,
	0x44, 0xF4, 0x64, 0xAA, 0xC1, 0x95, 0x27, 0xC8, 0x7A, 0xD6, 0x5C, 0xBB,
	0x32, 0x96, 0xC4, 0x95, 0x47, 0xA4, 0x5C, 0xB9, 0x2C, 0x67, 0x63, 0x65,
	0xB9, 0x92, 0x3D, 0xE2, 0x40, 0xAB, 0x52, 0xED, 0xB8, 0x3F, 0x84, 0x15,
	0xBE, 0x51, 0x73, 0xA5, 0x24, 0xC2, 0xAA, 0x03, 0xBB, 0xCB, 0x85, 0x12,
	0x0E, 0x5D, 0xB7, 0x26, 0x1D, 0xBE, 0x19, 0x6A, 0x0E, 0x6D, 0x05, 0x52,
	0xC2, 0xE0, 0x53, 0xF0, 0xA6, 0x35, 0xBB, 0x7B, 0x8B, 0xCC, 0x1F, 0xB7,
	0xF5, 0x41, 0x71, 0x9C, 0xD6, 0x66, 0x71, 0x6D, 0xF0, 0xAC, 0xE3, 0x09,
	0xE1, 0x6E, 0xCE, 0xA3, 0x66, 0x0C, 0xA4, 0x35, 0xF6, 0x02, 0x7A, 0x96,
	0x7E, 0xC8, 0xD3, 0x7B, 0x53, 0xDE, 0xB4, 0xD5, 0x2E, 0x7E, 0xEE, 0xF9,
	0x03, 0x44, 0x94, 0xFB, 0x8E, 0xB5, 0x03, 0x00, 0x00
};

#define HASP_STATUS_OK 0
static unsigned int Hook_hasp_login(int feature_id, void* vendor_code, int hasp_handle) {
#ifdef _DEBUG
	OutputDebugStringA("hasp_login\n");
#endif
	return HASP_STATUS_OK;
}

static unsigned int Hook_hasp_logout(int hasp_handle) {
#ifdef _DEBUG
	OutputDebugStringA("hasp_logout\n");
#endif
	return HASP_STATUS_OK;
}

static unsigned int Hook_hasp_encrypt(int hasp_handle, unsigned char* buffer, unsigned int buffer_size) {
#ifdef _DEBUG
	OutputDebugStringA("hasp_encrypt\n");
#endif
	return HASP_STATUS_OK;
}

static unsigned int Hook_hasp_decrypt(int hasp_handle, unsigned char* buffer, unsigned int buffer_size) {
#ifdef _DEBUG
	OutputDebugStringA("hasp_decrypt\n");
#endif
	return HASP_STATUS_OK;
}

static unsigned int Hook_hasp_get_size(int hasp_handle, int hasp_fileid, unsigned int* hasp_size) {
#ifdef _DEBUG
	OutputDebugStringA("hasp_get_size\n");
#endif
	*hasp_size = 0xD40; // Max addressable size by the game... absmax is 4k
	return HASP_STATUS_OK;
}

static unsigned int Hook_hasp_read(int hasp_handle, int hasp_fileid, unsigned int offset, unsigned int length, unsigned char* buffer) {
#ifdef _DEBUG
	OutputDebugStringA("hasp_read\n");
#endif
	memcpy(buffer, hasp_buffer + offset, length);
	return HASP_STATUS_OK;
}

static unsigned int Hook_hasp_write(int hasp_handle, int hasp_fileid, unsigned int offset, unsigned int length, unsigned char* buffer) {
	return HASP_STATUS_OK;
}

// ******************************************** //
// ************ Debug Data Logging ************ //
// ******************************************** //

// ************* Global Variables ************* //

// **** String Variables

// Debugging event log file
std::string logfile_W5X = "wmmt5dx_errors.txt";

// writeLog(filename: String, message: String): Int
// Given a filename string and a message string, appends
// the message to the given file.
static int writeLog(std::string filename, std::string message)
{
	// Log file to write to
	std::ofstream eventLog;

	// Open the filename provided (append mode)
	eventLog.open(filename, std::ios_base::app);

	// File open success
	if (eventLog.is_open())
	{
		// Write the message to the file
		eventLog << message;

		// Close the log file handle
		eventLog.close();

		// Success
		return 0;
	}
	else // File open failed
	{
		// Failure
		return 1;
	}
}

static BYTE GenerateChecksum(unsigned char* myArray, int index, int length)
{
	BYTE crc = 0;
	for (int i = 0; i < length; i++)
	{
		crc += myArray[index + i];
	}
	return crc & 0xFF;
}

static int ReturnTrue()
{
	return 1;
}

static void GenerateDongleData(bool isTerminal)
{
	memset(hasp_buffer, 0, 0xD40);
	hasp_buffer[0] = 0x01;
	hasp_buffer[0x13] = 0x01;
	hasp_buffer[0x17] = 0x0A;
	hasp_buffer[0x1B] = 0x04;
	hasp_buffer[0x1C] = 0x3B;
	hasp_buffer[0x1D] = 0x6B;
	hasp_buffer[0x1E] = 0x40;
	hasp_buffer[0x1F] = 0x87;

	hasp_buffer[0x23] = 0x01;
	hasp_buffer[0x27] = 0x0A;
	hasp_buffer[0x2B] = 0x04;
	hasp_buffer[0x2C] = 0x3B;
	hasp_buffer[0x2D] = 0x6B;
	hasp_buffer[0x2E] = 0x40;
	hasp_buffer[0x2F] = 0x87;
	if (isTerminal)
	{
		memcpy(hasp_buffer + 0xD00, "276411292430", 12);
		hasp_buffer[0xD3E] = GenerateChecksum(hasp_buffer, 0xD00, 62);
		hasp_buffer[0xD3F] = hasp_buffer[0xD3E] ^ 0xFF;
	}
	else
	{
		memcpy(hasp_buffer + 0xD00, "276413292430", 12);
		hasp_buffer[0xD3E] = GenerateChecksum(hasp_buffer, 0xD00, 62);
		hasp_buffer[0xD3F] = hasp_buffer[0xD3E] ^ 0xFF;
	}
}

static HWND mt6Hwnd;

typedef BOOL(WINAPI* ShowWindow_t)(HWND, int);
static ShowWindow_t pShowWindow;


// Hello Win32 my old friend...
typedef LRESULT(WINAPI* WindowProcedure_t)(HWND, UINT, WPARAM, LPARAM);
static WindowProcedure_t pMaxituneWndProc;

static BOOL gotWindowSize = FALSE;
static unsigned displaySizeX = 0;
static unsigned displaySizeY = 0;
static float scaleFactorX = 0.0f;
static float scaleFactorY = 0.0f;

static LRESULT Hook_WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (!gotWindowSize)
	{
		displaySizeX = GetSystemMetrics(SM_CXSCREEN);
		displaySizeY = GetSystemMetrics(SM_CYSCREEN);
		scaleFactorX = static_cast<float>(displaySizeX) / 1360.0f;
		scaleFactorY = static_cast<float>(displaySizeY) / 768.0f;
		printf("display is %dx%d (scale factor of %f, %f)\n", displaySizeX, displaySizeY, scaleFactorX, scaleFactorY);
		gotWindowSize = TRUE;
	}

	// TODO: Port below to use updated MT6 code
	/*
	if (msg == WM_LBUTTONDOWN ||
		msg == WM_LBUTTONUP)
	{
		unsigned short mx = GET_X_LPARAM(lParam);
		unsigned short my = GET_Y_LPARAM(lParam);

		//unsigned short trueMy = 768 - my;

		float scaledMx = static_cast<float>(mx) / 1360.f;
		float scaledMy = static_cast<float>(my) / 768.f;

		scaledMy = 1.0f - scaledMy;

		scaledMx *= scaleFactorX;
		scaledMy *= scaleFactorY;

		unsigned short trueMx = static_cast<int>(scaledMx * 16383.0f);
		unsigned short trueMy = static_cast<int>(scaledMy * 16383.0f);
		trueMy += 9500; // Cheap hack, todo do the math better!!

		//mx *= (16383 / 1360);
		//trueMy *= (16383 / 1360);

		printf("%d %d\n", trueMx, trueMy);
		mt6SetTouchParams(trueMx, trueMy, msg == WM_LBUTTONDOWN);

		printf("MOUSE %s (%d, %d)\n", msg == WM_LBUTTONDOWN ? "DOWN" : "UP  ", mx, my);
		return 0;
	}
	*/

	return pMaxituneWndProc(hwnd, msg, wParam, lParam);
}

static BOOL Hook_ShowWindow(HWND hwnd, int nCmdShow)
{
	SetWindowLongPtrW(hwnd, -4, (LONG_PTR)Hook_WndProc);
	ShowCursor(1);

	mt6Hwnd = hwnd;
	return pShowWindow(hwnd, nCmdShow);
}

typedef void (WINAPI* OutputDebugStringA_t)(LPCSTR);
static void Hook_OutputDebugStringA(LPCSTR str)
{
	printf("debug> %s", str);
}

typedef int (WINAPI *BIND)(SOCKET, CONST SOCKADDR *, INT);
static BIND pbind = NULL;

static unsigned int WINAPI Hook_bind(SOCKET s, const sockaddr *addr, int namelen) {
	sockaddr_in bindAddr = { 0 };
	bindAddr.sin_family = AF_INET;
	bindAddr.sin_addr.s_addr = inet_addr("192.168.96.20");
	bindAddr.sin_port = htons(50765);
	if (addr == (sockaddr*)&bindAddr) {
		sockaddr_in bindAddr2 = { 0 };
		bindAddr2.sin_family = AF_INET;
		bindAddr2.sin_addr.s_addr = inet_addr(ipaddr);
		bindAddr2.sin_port = htons(50765);
		return pbind(s, (sockaddr*)&bindAddr2, namelen);
	}
	else {
		return pbind(s, addr, namelen);
		
	}
}

extern int* ffbOffset;
extern int* ffbOffset2;
extern int* ffbOffset3;
extern int* ffbOffset4;

static DWORD WINAPI Wmmt5FfbCollector(void* ctx)
{
	uintptr_t imageBase = (uintptr_t)GetModuleHandleA(0);
	while (true)
	{
		//*ffbOffset = *(DWORD *)(imageBase + 0x196F188);
		//*ffbOffset2 = *(DWORD *)(imageBase + 0x196F18c);
		//*ffbOffset3 = *(DWORD *)(imageBase + 0x196F190);
		//*ffbOffset4 = *(DWORD *)(imageBase + 0x196F194);
		Sleep(10);
	}
}

static void PathFix() {
	auto chars = { 'F', 'G' , 'J' };

	for (auto cha : chars) {
		auto patterns = hook::pattern(va("%02X 3A 2F", cha));

		if (patterns.size() > 0) {
			for (int i = 0; i < patterns.size(); i++) {
				char* text = patterns.get(i).get<char>(0);
				std::string text_str(text);

				std::string to_replace = va("%c:/", cha);
				std::string replace_with = va("./%c", cha);

				std::string replaced = text_str.replace(0, to_replace.length(), replace_with);

				injector::WriteMemoryRaw(text, (char*)replaced.c_str(), replaced.length() + 1, true);
			}
		}
	}
}

static InitFunction Wmmt5DXFunc([]()
{
	// Alloc debug console
	FreeConsole();
	AllocConsole();

	SetConsoleTitle(L"WMMT5DX Console");

	FILE* pNewStdout = nullptr;
	FILE* pNewStderr = nullptr;
	FILE* pNewStdin = nullptr;

	::freopen_s(&pNewStdout, "CONOUT$", "w", stdout);
	::freopen_s(&pNewStderr, "CONOUT$", "w", stderr);
	::freopen_s(&pNewStdin, "CONIN$", "r", stdin);
	std::cout.clear();
	std::cerr.clear();
	std::cin.clear();
	std::wcout.clear();
	std::wcerr.clear();
	std::wcin.clear();

	FILE* fileF = _wfopen(L"Fsetting.lua.gz", L"r");
	if (fileF == NULL)
	{
		FILE* settingsF = _wfopen(L"Fsetting.lua.gz", L"wb");
		fwrite(settingData_W5X, 1, sizeof(settingData_W5X), settingsF);
		fclose(settingsF);
	}
	else
	{
		fclose(fileF);
	}

	FILE* fileG = _wfopen(L"Gsetting.lua.gz", L"r");
	if (fileG == NULL)
	{
		FILE* settingsG = _wfopen(L"Gsetting.lua.gz", L"wb");
		fwrite(settingData_W5X, 1, sizeof(settingData_W5X), settingsG);
		fclose(settingsG);
	}
	else
	{
		fclose(fileG);
	}

	bool isTerminal = false;
	if (ToBool(config["General"]["TerminalMode"]))
	{
		isTerminal = true;
	}
	
	std::string networkip = config["General"]["NetworkAdapterIP"];
	if (!networkip.empty())
	{
		//strcpy(ipaddr, networkip.c_str());
		ipaddr = networkip.c_str();
	}

	hookPort = "COM3";
	imageBase = (uintptr_t)GetModuleHandleA(0);

	MH_Initialize();

	// Hook dongle funcs
	MH_CreateHookApi(L"hasp_windows_x64_98199.dll", "hasp_write", Hook_hasp_write, NULL);
	MH_CreateHookApi(L"hasp_windows_x64_98199.dll", "hasp_read", Hook_hasp_read, NULL);
	MH_CreateHookApi(L"hasp_windows_x64_98199.dll", "hasp_get_size", Hook_hasp_get_size, NULL);
	MH_CreateHookApi(L"hasp_windows_x64_98199.dll", "hasp_decrypt", Hook_hasp_decrypt, NULL);
	MH_CreateHookApi(L"hasp_windows_x64_98199.dll", "hasp_encrypt", Hook_hasp_encrypt, NULL);
	MH_CreateHookApi(L"hasp_windows_x64_98199.dll", "hasp_logout", Hook_hasp_logout, NULL);
	MH_CreateHookApi(L"hasp_windows_x64_98199.dll", "hasp_login", Hook_hasp_login, NULL);
	MH_CreateHookApi(L"WS2_32", "bind", Hook_bind, reinterpret_cast<LPVOID*>(&pbind));

	// Give me the HWND please maxitune
	MH_CreateHookApi(L"user32", "ShowWindow", Hook_ShowWindow, reinterpret_cast<LPVOID*>(&pShowWindow));
	//pMaxituneWndProc = (WindowProcedure_t)(imageBasedxplus + 0xB78B90);
	pMaxituneWndProc = (WindowProcedure_t)(hook::get_pattern("48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 57 48 83 EC 30 8B EA BA EB FF FF FF 49 8B F9 49 8B F0 48 8B D9 FF 15 ? ? ? 00 48 85 C0 74 1D 4C", 0));

	//load banapass emu
	LoadLibraryA(".\\openBanaW5p.dll");

	GenerateDongleData(isTerminal);

	injector::WriteMemory<uint8_t>(imageBase + 0x61773E, 0, true);
	injector::MakeNOP(imageBase + 0x6185DB, 6, true);
	injector::WriteMemory<uint8_t>(hook::get_pattern("83 FA 04 0F 8C 1E 01 00 00 4C 89 44 24 18 4C 89 4C 24 20", 2), 0, true);
		
	// Skip weird camera init that stucks entire pc on certain brands. TESTED ONLY ON 05!!!!
	if (ToBool(config["General"]["WhiteScreenFix"]))
	{
		injector::WriteMemory<DWORD>(hook::get_pattern("48 8B C4 55 57 41 54 41 55 41 56 48 8D 68 A1 48 81 EC 90 00 00 00 48 C7 45 D7 FE FF FF FF 48 89 58 08 48 89 70 18 45 33 F6 4C 89 75 DF 33 C0 48 89 45 E7", 0), 0x90C3C032, true);
	}

	injector::MakeNOP(hook::get_pattern("45 33 C0 BA 65 09 00 00 48 8D 4D B0 E8 ? ? ? ? 48 8B 08", 12), 5);

	{
		auto location = hook::get_pattern<char>("41 3B C7 74 0E 48 8D 8F B8 00 00 00 BA F6 01 00 00 EB 6E 48 8D 8F A0 00 00 00");
		// Patch some jnz
		injector::WriteMemory<uint8_t>(location + 3, 0xEB, true);

		// Skip some jnz
		injector::MakeNOP(location + 0x22, 2);

		// Skip some jnz
		injector::MakeNOP(location + 0x33, 2);
	}

	// Skip DebugBreak on MFStartup fail
	{
		auto location = hook::get_pattern<char>("48 83 EC 28 33 D2 B9 70 00 02 00 E8 ? ? ? ? 85 C0 79 06");
		injector::WriteMemory<uint8_t>(location + 0x12, 0xEB, true);
	}

	if (isTerminal)
	{
		// Patch some func to 1
		safeJMP(hook::get_pattern("0F B6 41 05 2C 30 3C 09 77 04 0F BE C0 C3 83 C8 FF C3"), ReturnTrue);
	}
	else
	{
		{
			auto location = imageBase + 0x8CCEDD;
			injector::MakeNOP(location + 6, 6); // 6
			injector::MakeNOP(location + 0x16, 2); // 0xF
			injector::MakeNOP(location + 0x1C, 2); // 0x15
		}
	}

	PathFix();

	// Enable all print
	injector::MakeNOP(imageBase + 0x77EFB3, 6);

	MH_EnableHook(MH_ALL_HOOKS);

}, GameID::WMMT5DX);
#endif
#pragma optimize("", on)