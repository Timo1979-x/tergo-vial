#include <windows.h>
#include <setupapi.h>
#include <hidsdi.h>
#include <hidpi.h>
#include <dbt.h>
#include <msctf.h> // Заголовок для Text Services Framework
#include <fstream>
#include <string>
#include <vector>
#include <iomanip>
#include <sstream>

//#include "Helpers.h"
//
//// Функция для записи логов
//void LogMessage(const std::wstring& msg) {
//	if (g_logFile.is_open()) {
//		SYSTEMTIME st;
//		GetLocalTime(&st);
//		g_logFile << L"[" << st.wYear << L"-"
//			<< std::setfill(L'0') << std::setw(2) << st.wMonth << L"-"
//			<< std::setfill(L'0') << std::setw(2) << st.wDay << L" "
//			<< std::setfill(L'0') << std::setw(2) << st.wHour << L":"
//			<< std::setfill(L'0') << std::setw(2) << st.wMinute << L":"
//			<< std::setfill(L'0') << std::setw(2) << st.wSecond << L"] "
//			<< msg << std::endl;
//	}
//}

const char* langIdToLangName(DWORD langId) {
	switch (langId) {
		case 0x0409: return "EN";
		case 0x0419: return "RU";
		case 0x040C: return "FR";
		case 0x0407: return "DE";
		case 0x0410: return "IT";
		case 0x040A: return "ES";
		case 0x0415: return "PL";
		case 0x040E: return "HU";
		case 0x0405: return "CH";
		case 0x0418: return "RO";
		case 0x0402: return "BG";
		case 0x0809: return "EN";
		case 0x0C09: return "EN";
		case 0x1009: return "EN";
		case 0x1409: return "EN";
		case 0x1809: return "EN";
		case 0x1C09: return "EN";
		case 0x0422: return "UA";
		case 0x0423: return "BY";
		case 0x042B: return "KZ";
		case 0x0404: return "CN";
		case 0x0804: return "CN";
		case 0x0411: return "JP";
		case 0x0412: return "KR";
		case 0x0408: return "GR";
		case 0x040D: return "HE";
		case 0x041F: return "TR";
		default: return "??";
	}
}