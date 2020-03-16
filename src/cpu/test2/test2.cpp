// test2.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"
#include <iostream>
#include <fstream>
#include "m6502.h"
#include <chrono>

using namespace std;
using namespace std::chrono;

uint8_t store[0x10000];
void read(m6502_device &cpu) {
	uint16_t adr = cpu.getADDR();
	uint8_t r = store[adr];
	cout << "read  : [" << hex;
	cout.fill('0');
	cout.width(4);
	cout << adr << "]=>";
	cout.width(2);
	cout << (int)r << endl;
	cpu.setDATA(r);
}

void write(m6502_device &cpu) {
	uint16_t adr = cpu.getADDR();
	uint8_t val = cpu.getDATA();
	cout << "write : [" << hex;
	cout.fill('0');
	cout.width(4);
	cout << adr << "]<=";
	cout.width(2);
	cout << (int)val << endl;
	store[adr] = val;
};


const long int MAXCYCLES = 100L;


int main()
{

	ifstream ismos;
	ismos.open(
		"D:\\work\\vs2017\\scratch\\mame-cpu-libs\\mame-cpu-lib-6502-test\\testasm\\test-rom0.mos",
		ifstream::in | ifstream::binary
	);

	if (ismos.fail())
	{
		cerr << "Unable to open mos image\n";
		return 1;
	}

	ismos.read((char *)store + 0xC000, 0x4000);

	ismos.close();

	m6502_device cpu;

	cpu.start();
	cpu.reset();


	for (long int i = 0; i < MAXCYCLES; i++)
	{
		cpu.tick();
		if (cpu.getRNW())
			read(cpu);
		else
			write(cpu);
		cout << cpu;
	}

}

