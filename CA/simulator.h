#ifndef MIPS_SIMULATOR_H
#define MIPS_SIMULATOR_H
#include<iostream>
#include<map>
#include<string>
#include<vector>
#include <fstream>
#include <istream>
#include<algorithm>
#include<regex>
#include"Utility.h"

using namespace std;

#define OPER 1
#define LABEL 2
//#define REGEX_R_N_R "(r\\d{1,2}),(\\d*)\\((r\\d{1,2})\\)"
#define REGEX_R_N_R "(r[0-9]|r[0][0-9]|r[12][0-9]|r[3][0-1]),([+-]?\\d*)\\((r[0-9]|r[0][0-9]|r[12][0-9]|r[3][0-1])\\)"
#define REGEX_R_R_N "(r[0-9]|r[0][0-9]|r[12][0-9]|r[3][0-1]),(r[0-9]|r[0][0-9]|r[12][0-9]|r[3][0-1]),(\\d*)"
#define REGEX_F_N_R "(f[0-9]|f[0][0-9]|f[12][0-9]|f[3][0-1]),([+-]?\\d*)\\((r[0-9]|r[0][0-9]|r[12][0-9]|r[3][0-1])\\)"
#define REGEX_R_R_R "(r[0-9]|r[0][0-9]|r[12][0-9]|r[3][0-1]),(r[0-9]|r[0][0-9]|r[12][0-9]|r[3][0-1]),(r[0-9]|r[0][0-9]|r[12][0-9]|r[3][0-1])"
#define REGEX_F_F_F "(f[0-9]|f[0][0-9]|f[12][0-9]|f[3][0-1]),(f[0-9]|f[0][0-9]|f[12][0-9]|f[3][0-1]),(f[0-9]|f[0][0-9]|f[12][0-9]|f[3][0-1])"
#define REGEX_R_R_W "(r[0-9]|r[0][0-9]|r[12][0-9]|r[3][0-1]),(r[0-9]|r[0][0-9]|r[12][0-9]|r[3][0-1]),(\\w*)"
#define REGEX_W	"(\\w*)"
#define REGEX_EMPTY ""

#define DEST_SRC_SRC "012"
#define SRC_SRC_DEST "120"
#define EMPTY_FORMAT "999"
#define LABEL_FORMAT "099"
#define FU_INTEGER "int"
#define FU_FP_ADDER "add"
#define FU_FP_MULT "mult"
#define FU_FP_DIV "div"
class SymbolTableAttr
{
//private:
public:
	int symbolType;
	int numberOfExCycles;
	string regex;
	string destSrcFormat;	//dest src1 src2 - 012 , 9 is invalid
	string fuUsed;
	bool memoryRequired;
//public:
	SymbolTableAttr();
	SymbolTableAttr(int symbolType,int numberOfCycles,const char * regex,const char * destSrcFormat,const char * fuToBeUsed,bool memoryReqiored);

};

class SourceLine{
//private:
public:
	string opCode;
	string operands[3];
	string actualSourceLine;
	int exStagesLeft;
	int operandValues[3];	
//public:
	SourceLine();

};

class Simulator{
//private:
public:
	map<string,SymbolTableAttr> symbolTable;

	//access times
	int memAccessCycles;
	int iCacheAccessCycles;
	int dCacheAccessCycles;

	//pipelined or not
	bool isFPAdderPipe;
	bool isFPMultPipe;
	bool isFPDivPipe;

	//FU stages
	int FPAdderStages;
	int FPMultStages;
	int FPDivStages;

	vector<SourceLine> instructionVector;
	map<string,int> labelInstMap;
	map<string,int> registerMap;
	int data[32];
	bool fpRegistersLocked[32];
	bool intRegistersLocked[32];
//public:
	Simulator();
	void readConfig(char * );
	void readInstructions(char *);
	void readRegisters(char *);
	void readData(char *);
	void parseInstruction(SourceLine * ,string );	
	void simulate(const char * outputFile);
	friend class SourceLine;
	friend class SymbolTableAttr;

};

#endif