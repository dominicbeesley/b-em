// test1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <fstream>
#include "m6502.h"
#include <chrono>

#include <stdlib.h>

using namespace std;
using namespace std::chrono;


uint8_t store[0x10000];


inline void read(m6502_device& cpu) {
	cpu.setDATA(store[cpu.getADDR()]);
};

inline void write(m6502_device& cpu) {
	store[cpu.getADDR()] = cpu.getDATA();
}



const long int MAXCYCLES = 1000000000L;

void usage(const char *msg) {
	if (msg)
		cerr << msg << "\n";

	cerr << 
"cpu_test1 <rom image>\n"
"\n"
"Expects a rom image 16k based at 0xC000, runs for " << MAXCYCLES << " cycles \n"
"and returns the flat-out clock speed\n\n";

}

int main(int argc, char *argv[])
{

	if (argc < 2)
	{
		usage("Too few arguments");

		return 2;
	}

	ifstream ismos;
	ismos.open(
		argv[1],
		ifstream::in | ifstream::binary
	);

	if (ismos.fail())
	{
		cerr << "Unable to open mos image \"" << argv[1] << "\"\n";
		return 1;
	}

	ismos.read(((char *)store) + 0xC000, 0x4000);

	ismos.close();
	
	m6502_device cpu;

	cpu.start();
	cpu.reset();

	high_resolution_clock::time_point t1 = high_resolution_clock::now();

	for (long int i = 0; i < MAXCYCLES; i++)
	{
		cpu.tick();
		if (cpu.getRNW()) {
			read(cpu);
		}
		else {
			write(cpu);
		}

		//cout << cpu;
	}

	high_resolution_clock::time_point t2 = high_resolution_clock::now();

	duration<double> ts = duration_cast<duration<double>>(t2 - t1);

	cout << MAXCYCLES << " in " << (ts.count() * 1000) << "ms\n";
	cout << "=" << (((double)MAXCYCLES / 1000000.0) / ts.count()) << "MHz\n";
}

