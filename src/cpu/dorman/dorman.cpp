// dorman.cpp : This file contains the 'main' function. Program execution begins and ends there.
//


#include "pch.h"
#include <iostream>
#include <iostream>
#include <fstream>
#include "m6502.h"
#include <chrono>
#include <Windows.h>



using namespace std;
using namespace std::chrono;

HANDLE inh;
HANDLE outh;
bool verby = false;


	uint8_t store[0x10000];
	void read(m6502_device &cpu) {

		uint16_t adr = cpu.getADDR();
		uint8_t r;
		if (adr == 0xf004)
		{
			TCHAR ch;
			DWORD n;
			ReadConsole(inh, &ch, 1, &n, NULL);
			if (n)
				r = (uint8_t)ch;
			else
				r = 0;
		} else
			r = store[adr];
		
		if (verby) {
			cout << "read  : [" << hex;
			cout.fill('0');
			cout.width(4);
			cout << adr << "]=>";
			cout.width(2);
			cout << (int)r << endl;
		}

		cpu.setDATA(r);
	};

	void write(m6502_device &cpu) {
		uint16_t adr = cpu.getADDR();
		uint8_t val = cpu.getDATA();
		if (verby) {
			cout << "write : [" << hex;
			cout.fill('0');
			cout.width(4);
			cout << adr << "]<=";
			cout.width(2);
			cout << (int)val << endl;
		}

		if (adr == 0xf001)
		{
			verby = false;
			TCHAR ch = (TCHAR)val;
			DWORD n;
			WriteConsole(outh, &ch, 1, &n, NULL);
		}
		store[adr] = val;
	}


const long int MAXCYCLES = 1000000000L;


void errorexit(const char *err) {
	cerr << err;
	exit(-1);
}

int main()
{
	inh = GetStdHandle(STD_INPUT_HANDLE);
	outh = GetStdHandle(STD_OUTPUT_HANDLE);

	DWORD m;
	GetConsoleMode(inh, &m);
	m &= ~(ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT);
	SetConsoleMode(inh, m);

	GetConsoleMode(outh, &m);

	if (inh == INVALID_HANDLE_VALUE || outh == INVALID_HANDLE_VALUE)
		errorexit("No Console");

	{
		ifstream ismos;
		ismos.open(
			"D:\\work\\vs2017\\scratch\\mame-cpu-libs\\mame-cpu-lib-6502-test\\testasm\\dorman\\6502_functional_test.bin",
			ifstream::in | ifstream::binary
		);

		if (ismos.fail())
		{
			cerr << "Unable to open mos image\n";
			return 1;
		}

		ismos.read((char *)store + 0x0000, 0x10000);

		ismos.close();
	}

	m6502_device cpu;

	cpu.start();
	cpu.reset();

	high_resolution_clock::time_point t1 = high_resolution_clock::now();

	for (long int i = 0; i < MAXCYCLES; i++)
	{
		cpu.tick();

		if (cpu.getRNW())
			read(cpu);
		else
			write(cpu);

		if (verby) {
			cout << cpu;
		}
		//if (cpu.getPC() == 0x0B12) verby = 1;
	}

	high_resolution_clock::time_point t2 = high_resolution_clock::now();

	duration<double> ts = duration_cast<duration<double>>(t2 - t1);

	cout << MAXCYCLES << " in " << (ts.count() * 1000) << "ms\n";
	cout << "=" << (((double)MAXCYCLES / 1000000.0) / ts.count()) << "MHz\n";
}

