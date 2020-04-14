#include "pipeline.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <unordered_map>

Register registerFile[16];
std::unordered_map<int, int> memory;
int branchRegister;

Register::Register(void) {
	dataValue = 0;
	registerNumber = -1;
	registerName = "";
}

int getRegister(const int registerNumber) {
	return registerFile[registerNumber].dataValue;
}

void setRegister(const int registerNumber, const int value) {
	registerFile[registerNumber].dataValue = value;
}

Instruction::Instruction(void) {
	type = NOP;
	dest = -1;
	src1 = -1;
	src2 = -1;
	stage = NONE;
}

Instruction::Instruction(std::string newInst) {
	std::string buf; 
    	std::stringstream ss(newInst); 
	std::vector<std::string> tokens;
	
    	while (ss >> buf){
		tokens.push_back(buf);
	}

	if(tokens[0] == "ADD")
		type = ADD;
	else if(tokens[0] == "ADDI")
		type = ADDI;
	else if(tokens[0] == "SUB")
		type = SUB;
	else if(tokens[0] == "SUBI")
		type = SUBI;
	else if(tokens[0] == "MULT")
		type = MULT;
	else if(tokens[0] == "DIV")
		type = DIV;
	else if(tokens[0] == "LW")
		type = LW;
	else if(tokens[0] == "SW")
		type = SW;
	else if(tokens[0] == "BEQ")
		type = BEQ;
	else if(tokens[0] == "BNE")
		type = BNE;
	else if(tokens[0] == "BEQZ")
		type = BEQZ;
	else if(tokens[0] == "BNEZ")
		type = BNEZ;
	else
		type = NOP;

	dest = -1;
	src1 = -1;
	src2 = -1;

	if(tokens.size() > 1) {
		dest = atoi(tokens[1].erase(0,1).c_str());
	}
	if(tokens.size() > 2) {
		src1 = atoi(tokens[2].erase(0,1).c_str());
	}
	if(tokens.size() > 3 && type) {
		src2 = atoi(tokens[3].erase(0,1).c_str());
	}

	// Store, BEQZ, and BNEZ has 2 source operands and no destination operand
	if (type == SW || type == BEQZ || type == BNEZ) {
		src2 = src1;
		src1 = dest;
		dest = -1;
	}
	else if (type == BEQ || type == BNE) {
		int tempSrc2 = src2;
		src2 = src1;
		src1 = dest;
		dest = tempSrc2;
	}

	stage = NONE;
}

Application::Application(void) {
	PC = 0;
}

void Application::loadApplication(std::string fileName, int *registerValues) {
	std::string sLine = "";
	Instruction *newInstruction;
	std::ifstream infile;
	infile.open(fileName.c_str(), std::ifstream::in);
	
	if ( !infile ) {
		std::cout << "Failed to open file " << fileName << std::endl;
		return;
	}	

	while (!infile.eof())
	{
		getline(infile, sLine);
		if(sLine.empty())
			break;
		newInstruction = new Instruction(sLine);
		instructions.push_back(newInstruction);
	}

	infile.close();
	std::cout << "Read file completed!!" << std::endl;
	std::cout << "Assigning Registers" << std::endl;

	for(int i = 0; i < 16; i++) {
		Register registers;

		registers.dataValue = registerValues[i];
		registers.registerNumber = i;
		registers.registerName = "r" + i;

		registerFile[i] = registers;
	}
	
	printApplication();
}

void Application::printApplication(void) {
	std::cout << "Printing Application: " << std::endl;
	std::vector<Instruction*>::iterator it;
	for(it=instructions.begin(); it < instructions.end(); it++) {
	
		(*it)->printInstruction();
		std::cout << std::endl;
	}
}

Instruction* Application::getNextInstruction(const bool taken, const bool branch, const bool override, const int pcOffset) {
	Instruction *nextInst = NULL;

	if(override) {
		PC = branchRegister;
	}
	else if(branch) {
		if(taken) {
			branchRegister = PC;
			PC += pcOffset - 1;
		}
		else {
			branchRegister = PC + pcOffset - 1;
		}
	}

	if( PC < instructions.size() ) {
		nextInst = instructions[PC];
		PC += 1;
	}
	
	if( nextInst == NULL )
		nextInst = new Instruction();
	
	return nextInst;
}

PipelineStage::PipelineStage(void) {
	inst = new Instruction();
	stageType = NONE;	
}

void PipelineStage::clear() {
	inst = NULL;
}


std::pair<bool, int> PipelineStage::process() {	
	// Functionally simulate pipeline stage
	bool branchInst = false;
	int pcOffset = 0;

	switch(stageType) {
		case FETCH: 	// Fetch instruction. PC+4
			//branching
			break;
		case DECODE: 	// Fetch register operands
			switch(this->inst->type) {
				case BEQ:
					branchInst = true;
					pcOffset = this->inst->dest;

					break;
				case BNE:
					branchInst = true;
					pcOffset = this->inst->dest;

					break;
				case BEQZ:
					branchInst = true;
					pcOffset = this->inst->src2;

					break;
				case BNEZ:
					branchInst = true;
					pcOffset = this->inst->src2;

					break;
				default:
					break;
			}

			break;
		case EXEC: 	// Perform ALU operations
			// Run Memory and WB here as well to simulate forwarding
			switch(this->inst->type) {
				case ADD:
					{
						const int result = getRegister(this->inst->src1) + getRegister(this->inst->src2);
						setRegister(this->inst->dest, result);

						break;
					}
				case ADDI:
					{
						const int val = this->inst->src2;
						const int result = getRegister(this->inst->src1) + val;
						setRegister(this->inst->dest, result);

						break;
					}
				case SUB:
					{
						const int result = getRegister(this->inst->src1) - getRegister(this->inst->src2);
						setRegister(this->inst->dest, result);

						break;
					}
				case SUBI:
					{
						const int val = this->inst->src2;
						const int result = getRegister(this->inst->src1) - val;
						setRegister(this->inst->dest, result);

						break;
					}

					break;
				case MULT:
					{
						const int result = getRegister(this->inst->src1) * getRegister(this->inst->src2);
						setRegister(this->inst->dest, result);

						break;
					}
				case DIV:
					{
						const int result = getRegister(this->inst->src1) / getRegister(this->inst->src2);
						setRegister(this->inst->dest, result);

						break;
					}
				case LW:
					{
						const int address = getRegister(this->inst->src1);
						if(memory.find(address) == memory.end()) {
							memory[address] = 0;
						}
						const int result = memory[address];
						setRegister(this->inst->dest, result);

						break;
					}
				case SW:
					{
						const int address = getRegister(this->inst->src2);
						memory[address] = getRegister(this->inst->src1);

						break;
					}
				case BEQ:
					{
						branchInst = true;
						if(getRegister(this->inst->src1) == getRegister(this->inst->src2)) {
							pcOffset = this->inst->dest;
						}

						break;
					}
				case BNE:
					{
						branchInst = true;
						if(getRegister(this->inst->src1) != getRegister(this->inst->src2)) {
							pcOffset = this->inst->dest;
						}
					}
					break;
				case BEQZ:
					{
						branchInst = true;
						if(getRegister(this->inst->src1) == 0) {
							pcOffset = this->inst->src2;
						}
					}
					break;
				case BNEZ:
					{
						branchInst = true;
						if(getRegister(this->inst->src1) != 0) {
							pcOffset = this->inst->src2;
						}
					}
					break;
				default:
					break;
			}

			break;
		default:
			break;
	}	

	return std::make_pair(branchInst, pcOffset);
}

void PipelineStage::addInstruction(Instruction *newInst) {
	inst = newInst;
	inst->stage = stageType;
}

Pipeline::Pipeline(Application *app, bool trace) {
	pipeline[FETCH].stageType = FETCH;
	pipeline[DECODE].stageType = DECODE;
	pipeline[EXEC].stageType = EXEC;
	pipeline[MEM].stageType = MEM;
	pipeline[WB].stageType = WB;
	cycleTime = 0;
	this->trace = trace;

	printPipeline();

	application = app;
	forwarding = false;
	taken = false;
}

bool Pipeline::hasDependency(void) {
	if(pipeline[DECODE].inst->type == NOP)
		return false;

	// Checks if dependency exist between Decode stage and Exec, Mem stage
	// We assume the register file can read/write in the same cycle so no data dependency exist with RAW dependency if an instruction is in Decode and WB.
	for(int i = EXEC; i < WB; i++) {
		if( pipeline[i].inst == NULL )
			continue;		

		if( pipeline[i].inst->type == NOP )
			continue;

		if( forwarding && (pipeline[i].inst->type != LW || i == MEM) )
			continue;

		if( (pipeline[i].inst->dest != -1) && 
		    (pipeline[i].inst->dest == pipeline[DECODE].inst->src1 ||
		     pipeline[i].inst->dest == pipeline[DECODE].inst->src2) ) {
			return true;
		}
	}

	return false;
}

void Pipeline::cycle(void) {
	cycleTime += 1;
	// Check for data hazards
	// NOTE: Technically, data hazards are detected in the Decode stage. If a data hazard is detected, at the end of the cycle we write 0's (NOP) to the pipeline register so that a NOP will be generated in the EXEC stage in the next cycle. 
	// Doing the check here does a dependency check on the instructions in the previous cycle (we haven't advanced the instructions in the pipeline yet). If a dependency exist in the previous cycle, we stall the pipeline in this cycle.
	bool dependencyDetected = hasDependency();

	// WRITEBACK STAGE
	// Mem -> WB Pipeline register
	pipeline[WB].addInstruction(pipeline[MEM].inst);

	// Writeback
	pipeline[WB].process();

	// MEM STAGE
	// Exec -> Mem Pipeline register
	pipeline[MEM].addInstruction(pipeline[EXEC].inst);
	
	// Mem
	pipeline[MEM].process();
	
    // EXEC STAGE
	// Decode -> Exec Pipeline register
	// If dependency detected, stall by inserting NOP instruction
	if(!dependencyDetected)
		pipeline[EXEC].addInstruction(pipeline[DECODE].inst);
	else 
		pipeline[EXEC].addInstruction(new Instruction());
	
	// Exec
	std::pair<bool, int> result = pipeline[EXEC].process();
	bool override = false;

	// DECODE STAGE
	// Fetch -> Decode Pipeline register
	if(!dependencyDetected) {
		if(result.first) {
			if(taken) {
				if(result.second == 0) {
					pipeline[DECODE].addInstruction(new Instruction());
					override = true;
				}
				else {
					pipeline[DECODE].addInstruction(pipeline[FETCH].inst);
				}
			}
			else {
				if(result.second == 0) {
					pipeline[DECODE].addInstruction(pipeline[FETCH].inst);
				}
				else {
					pipeline[DECODE].addInstruction(new Instruction());
					override = true;
				}
			}
		}
		else {
			pipeline[DECODE].addInstruction(pipeline[FETCH].inst);
		}
	}

	// Decode 
	result = pipeline[DECODE].process();
	
	// FETCH STAGE
	// Fetch
	if(!dependencyDetected) {
		pipeline[FETCH].addInstruction(application->getNextInstruction(taken, result.first, override, result.second));
		pipeline[FETCH].process();
	}
}

bool Pipeline::done() {
	for(int i = 0; i < 5; i++) {
		if(pipeline[i].inst->type != NOP)
			return false;
	}

	return true;
}

void Pipeline::printPipeline(void) {
	if(cycleTime == 0) {
		std::cout << "Cycle" << "\tIF" << "\t\tID" << "\t\tEXEC" << "\t\tMEM" << "\t\tWB";
		if(trace) {
			std::cout << "\t\tr0" << "\tr1" << "\tr2" << "\tr3" << "\tr4" << "\tr5" << "\tr6" << "\tr7" << "\tr8" << "\tr9" << "\tr10" << "\tr11" << "\tr12" << "\tr13" << "\tr14" << "\tr15";
		}
		std::cout << std::endl;
	}
	std:: cout << cycleTime; 
	for(int i = 0; i < 5; i++) {
		pipeline[i].printStage();
	}
	if(trace) {
		for(int i = 0; i < 16; i++) {
			std::cout << "\t" << getRegister(i);
		}
		std::cout << std::endl;
	}
	std::cout << std::endl;
}

void PipelineStage::printStage(void) {
	std::cout << "\t";
	inst->printInstruction();
}

void Instruction::printInstruction(void) {
	if(type == NOP)
		std::cout << instructionNames[type] << "         ";
	else if(type == BEQ || type == BNE)
		std::cout << instructionNames[type] << " r" << src1 << " r" << src2 << " #" << dest;
	else if(type == BEQZ || type == BNEZ)
		std::cout << instructionNames[type] << " r" << src1 << " #" << src2;
	else if(type == SW)
		std::cout << instructionNames[type] << " r" << src1 << " r" << src2;
	else if(type == LW)
		std::cout << instructionNames[type] << " r" << dest << " r" << src1;
	else if(type == ADDI || type == SUBI)
		std::cout << instructionNames[type] << " r" << dest << " r" << src1 << " #" << src2;
	else 
		std::cout << instructionNames[type] << " r" << dest << " r" << src1 << " r" << src2;
}
