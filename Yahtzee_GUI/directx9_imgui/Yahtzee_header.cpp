#include<windows.h>
#include<stdio.h>   
#include<tchar.h>
#include <Psapi.h>
#include "psapi.h"
#include <iostream>


#include "Yahtzee_header.h"
namespace std {
	double usage::memory() {
		MEMORYSTATUSEX statex;

		statex.dwLength = sizeof(statex);

		GlobalMemoryStatusEx(&statex);

		

		//_tprintf(TEXT("There is  %*ld percent of memory in use.\n"), WIDTH, statex.dwMemoryLoad);

		/*_tprintf(TEXT("There are %*I64d total Mbytes of physical memory.\n"), WIDTH, statex.ullTotalPhys / DIV);

		_tprintf(TEXT("There are %*I64d free Mbytes of physical memory.\n"), WIDTH, statex.ullAvailPhys / DIV);

		_tprintf(TEXT("There are %*I64d total Mbytes of paging file.\n"), WIDTH, statex.ullTotalPageFile / DIV);

		_tprintf(TEXT("There are %*I64d free Mbytes of paging file.\n"), WIDTH, statex.ullAvailPageFile / DIV);

		_tprintf(TEXT("There are %*I64d total Mbytes of virtual memory.\n"), WIDTH, statex.ullTotalVirtual / DIV);

		_tprintf(TEXT("There are %*I64d free Mbytes of virtual memory.\n"), WIDTH, statex.ullAvailVirtual / DIV);

		_tprintf(TEXT("There are %*I64d free Mbytes of extended memory.\n"), WIDTH, statex.ullAvailExtendedVirtual / DIV);*/
		

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
}