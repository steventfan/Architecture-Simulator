#include <string>
#include <utility>
#include <vector>

/* Types of possible instruction types */
enum InstructionType {
	NOP = 0,	// NOP. Pipeline bubble.
	ADD,		// Add
	ADDI,		// Add intermediate
	SUB,		// Subtract
	SUBI,		// Subtract intermediate
	MULT,		// Multiply
	DIV,		// Divide
	LW,			// Load word
	SW,			// Store word
	BEQ,		// Branch equal
	BNE,		// Branch not equal
	BEQZ,		// Branch equal 0
	BNEZ		// Branch not equal 0	

};

/* Names of possible instruction types */
const std::string instructionNames[13] = {"*", "ADD", "ADDI", "SUB", "SUBI", "MULT", "DIV", "LW", "SW", "BEQ", "BNE", "BEQZ", "BNEZ"};

enum Stage {
	FETCH = 0,
	DECODE,
	EXEC,
	MEM,
	WB,
	NONE
};

const std::string stageNames[6] = {"FETCH", "DECODE", "EXEC", "MEM", "WB", "NONE"};

/* A Single Register Entry containing register number and register data value */
class Register {
	public:
		Register();
		int dataValue;
		int registerNumber;
		std::string registerName;
};

/* Register file with 16 registers */
extern Register registerFile[16];

class Instruction {
	public:
		Instruction();
		Instruction(std::string);
		InstructionType type; // Type of instruction
		int dest; // Destination register number
		int src1; // Source register number
		int src2; // Source register number
		void printInstruction();
		Stage stage;
};

class Application {
	
	public:
		Application();
		void loadApplication(std::string, int *);
		void printApplication();
		Instruction* getNextInstruction(const bool, const bool, const bool, const int);
		std::vector<Instruction*> instructions;
		int PC; 
};

class PipelineStage {

	public:
		PipelineStage();
		Instruction *inst;
		Stage stageType;
		void clear();
		void addInstruction(Instruction*);
		void printStage();
		std::pair<bool, int> process();
};

class Pipeline {

	public: 
		Pipeline(Application*, bool);
		int cycleTime;
		Application *application;
		PipelineStage pipeline[5];
		void cycle();
		void printPipeline();
		bool done();
		bool hasDependency();
		bool forwarding;
		bool trace;
		bool taken;
};


