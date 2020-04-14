/*
* How To Run
*
* make
* ./pipesim -i <fileName> -f -r -t --r# <value>
*
* where all flags are optional, # is an integer from 0 to 15, and value is an integer
*/

#include <iostream>
// #include "unistd.h"
#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include "pipeline.h"

using namespace std;

int main(int argc, char *argv[]) {

	int opt;
	bool forwarding = false;
	bool trace = false;
	bool taken = false;
	int registerValues[16] = {0};
	string fileName = "instruction.txt";
	const struct option long_options[] =
		{
			{"r1",	required_argument,	0,	'A'},
			{"r2",	required_argument,	0,	'B'},
			{"r3",	required_argument,	0,	'C'},
			{"r4",	required_argument,	0,	'D'},
			{"r5",	required_argument,	0,	'E'},
			{"r6",	required_argument,	0,	'F'},
			{"r7",	required_argument,	0,	'G'},
			{"r8",	required_argument,	0,	'H'},
			{"r9",	required_argument,	0,	'I'},
			{"r10",	required_argument,	0,	'J'},
			{"r11",	required_argument,	0,	'K'},
			{"r12",	required_argument,	0,	'L'},
			{"r13",	required_argument,	0,	'M'},
			{"r14",	required_argument,	0,	'N'},
			{"r15",	required_argument,	0,	'O'},
			{0, 0, 0, 0}
		};
	int option_index = 0;
	while ((opt = getopt_long(argc, argv, "i:frtA:B:C:D:E:F:G:H:I:J:K:L:M:N:O:", long_options, &option_index)) != EOF)
        switch(opt)
        {
            case 'f': forwarding = true; break;
            case 'i': fileName.assign(optarg); break;
			case 'r': trace = true; break;
			case 't': taken = true; break;
			case 'A': registerValues[1] = stoi(optarg); break;
			case 'B': registerValues[2] = stoi(optarg); break;
			case 'C': registerValues[3] = stoi(optarg); break;
			case 'D': registerValues[4] = stoi(optarg); break;
			case 'E': registerValues[5] = stoi(optarg); break;
			case 'F': registerValues[6] = stoi(optarg); break;
			case 'G': registerValues[7] = stoi(optarg); break;
			case 'H': registerValues[8] = stoi(optarg); break;
			case 'I': registerValues[9] = stoi(optarg); break;
			case 'J': registerValues[10] = stoi(optarg); break;
			case 'K': registerValues[11] = stoi(optarg); break;
			case 'L': registerValues[12] = stoi(optarg); break;
			case 'M': registerValues[13] = stoi(optarg); break;
			case 'N': registerValues[14] = stoi(optarg); break;
			case 'O': registerValues[15] = stoi(optarg); break;
            case '?': fprintf(stderr, "usage is \n -i fileName : to run input file fileName \n -f : for enabling forwarding \n -r : for enabling register trace \n --r# number: where # is an integer from 1 to 15 : to assign register with integer number \n -t: for enabling taken branch prediction \n");
            default: cout << endl; abort();
        }

	cout << "Loading application..." << fileName << endl;
	Application application;
	application.loadApplication(fileName, registerValues);
	cout << "Initializing pipeline..." << endl;
	Pipeline pipeline(&application, trace);
	pipeline.forwarding = forwarding;
	pipeline.taken = taken;

	do {
		pipeline.cycle();
		pipeline.printPipeline();

	} while(!pipeline.done());

	cout << "Completed in " << pipeline.cycleTime - 1 << " cycles" << endl;

	return 0;
}
