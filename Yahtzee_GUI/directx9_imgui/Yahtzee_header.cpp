#include<windows.h>
#include<stdio.h>   
#include<tchar.h>
#include <Psapi.h>
#include "psapi.h"
#include <iostream>
#include <thread>
using namespace std;

#include "Yahtzee_header.h"
namespace std {

	double usage::memory_percent() {
		MEMORYSTATUSEX statex;

		statex.dwLength = sizeof(statex);

		GlobalMemoryStatusEx(&statex);

		PROCESS_MEMORY_COUNTERS_EX pmc;
		GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
		SIZE_T physMemUsedByMe = (pmc.WorkingSetSize / 1000) / 1000;

		//int mem_values[8] = { statex.dwMemoryLoad, statex.ullTotalPhys / DIV, statex.ullAvailPhys / DIV, statex.ullTotalPageFile / DIV, statex.ullAvailPageFile / DIV, statex.ullTotalVirtual / DIV, statex.ullAvailVirtual / DIV, statex.ullAvailExtendedVirtual / DIV };
		double total_mem = (statex.ullTotalPhys / 1000) / 1000;		
		double used_mem_per = (physMemUsedByMe / total_mem) * 100;
		//cout << physMemUsedByMe << " " << total_mem << " " << used_mem_per << " " << physMemUsedByMe / total_mem << endl;
		return used_mem_per;
		//virtualMemUsedByMe; all memory im using
	}

	double usage::memory() {
		MEMORYSTATUSEX statex;

		statex.dwLength = sizeof(statex);

		GlobalMemoryStatusEx(&statex);

		PROCESS_MEMORY_COUNTERS_EX pmc;
		GetProcessMemoryInfo(GetCurrentProcess(), (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc));
		SIZE_T physMemUsedByMe = (pmc.WorkingSetSize / 1000) / 1000;
		return physMemUsedByMe;
	}
}