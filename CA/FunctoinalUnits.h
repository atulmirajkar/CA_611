#ifndef MIPS_FUNCTIONAL_UNIT_H
#define MIPS_FUNCTIONAL_UNIT_H
#include "simulator.h"
#include"MIPSMemory.h"
#include<string>
#include<deque>
#include<deque>
#include<map>
bool isRAWHazard(Simulator * simPtr,SourceLine srcLine);
bool setLock(Simulator * simPtr,SourceLine srcLine);
void releaseLock(Simulator * simPtr,SourceLine srcLine);
class displayInfo{
public:
	string actualInstruction;
	int * completionTime;
	bool war;
	bool waw;
	bool raw;
	bool structural;
	bool isStructuralHazardBeforeIU;
	static int instrCacheRequest;
	static int instrCacheHit;
	static int dataCacheRequest;
	static int dataCacheHit;

	displayInfo();
};

class FunctionalUnit{
protected:
	int numberOfCycles;
	bool busy;
	int instructionNumber;
	int cyclesLeftForInstr;
	
public:
	static std::deque<pair<int,SourceLine>> decodedeque;
	static std::deque<pair<int,SourceLine>> integerALUdeque;
	static std::deque<pair<int,SourceLine>> memorydeque;
	static std::deque<pair<int,SourceLine>> fpAdderdeque;
	static std::deque<pair<int,SourceLine>> fpMultdeque;
	static std::deque<pair<int,SourceLine>> fpDivdeque;
	static std::deque<pair<int,SourceLine>> wbdeque;
	static vector<displayInfo *> completionVector;
	static pair<bool,int> branchTakenInstrNumberPair;
	static bool branchTakenSameCycle;
	static int inFileInstruction;
	static int clockTick;
	static bool isExecutionFinished;
	static bool isIFDoneAfterExecFinished;
	static bool isBranchNotTakenOver;
	FunctionalUnit();
	FunctionalUnit(int numberOfCycles, int noOfInstr);

	static void  displayCompletionTime(const char * outputFile);
	static bool areAllPipeLinesEmpty();
};


class InstrFetch: protected FunctionalUnit{
	//related to cache	
	int instrCacheCycles;
	bool isCycleAssigned;
	InstructionCache iCache;	
	bool isInstrCacheHit;
public:
	InstrFetch(int inputNumberOfCycles,int noOfInstr);

	void checkStatus(Simulator * simPtr);
	bool checkCacheAndAssignCyclesLeft(int instructionNumber,Simulator * simPtr);
};

class InstrDecode: protected FunctionalUnit{

public:
	
	InstrDecode(int inputNumberOfCycles,int noOfInstr);
	bool isThereAFunctionalUnit(SourceLine instr,Simulator * simPtr);

	bool isTheFunctionalUnitFree(SourceLine instr,Simulator * simPtr);

	//pipeline implementation in the form of deque for each functional unit
	bool push_backToCorrectFunctionalUnit(SourceLine instr,Simulator * simPtr, int instructionNumber);

	void checkStatus(Simulator * simPtr);

	void readRegisters(SourceLine & instr,Simulator * simPtr);
	void processBranchInstruction(SourceLine & instr,Simulator * simPtr);
	bool processHalt(SourceLine & instr,Simulator * simPtr);
	
};

class IntegerAlu:protected FunctionalUnit{
private:
		
public:
	IntegerAlu();
	IntegerAlu(int inputNumberOfCycles,int noOfInstr);


	void checkStatus(Simulator * simPtr);
	void executeIntegerInstruction(SourceLine & instr,Simulator * simPtr);
};

class Memory:protected FunctionalUnit{
	bool isCycleAssigned;
	DataCache dCache;
	bool isDataCacheHit;
public:
	Memory();
	Memory(int inputNumberOfCycles,int noOfInstr);

	void checkStatus(Simulator * simPtr,vector<pair<int,SourceLine>> & writeBackPriorityVector);
	bool checkCacheAndAssignCyclesLeft(SourceLine instr,Simulator * simPtr,int instructionNumber);

};

class IntegerExecPath{
	IntegerAlu intAluFu;
	Memory memFu;
public:
	IntegerExecPath();
	IntegerExecPath(IntegerAlu intAluFu, Memory memFu);
	void checkStatus(Simulator * simPtr,vector<pair<int,SourceLine>> & writeBackPriorityVector);
};

class FPAdderUnit:protected FunctionalUnit{
	bool pipeLined;
public:
	FPAdderUnit();
	FPAdderUnit(int inputNumberOfCycles,bool pipeLined,int noOfInstr);

	void checkStatus(Simulator * simPtr,vector<pair<int,SourceLine>> & writeBackPriorityVector);
};

class FPMultUnit:protected FunctionalUnit{
	bool pipeLined;
public:
	FPMultUnit();
	FPMultUnit(int inputNumberOfCycles, bool pipLined,int noOfInstr);

	void checkStatus(Simulator * simPtr,vector<pair<int,SourceLine>> & writeBackPriorityVector);
};

class FPDivUnit:protected FunctionalUnit{
	bool pipeLined;
public:
	FPDivUnit();
	FPDivUnit(int inputNumberOfCycles,bool pipeLined,int noOfInstr);
	
	void checkStatus(Simulator * simPtr,vector<pair<int,SourceLine>> & writeBackPriorityVector);
};

class InstrExecute
{
	IntegerExecPath intPath;
	FPAdderUnit adderPath;
	FPMultUnit multPath;
	FPDivUnit divPath;
	vector<pair<int,SourceLine>> writeBackPriorityVector;
public:
	InstrExecute(IntegerExecPath intPath,FPAdderUnit adderPath,FPMultUnit multPath,FPDivUnit divPath);

	void checkStatus(Simulator * simPtr);
	//function to check priority of getting more than one instruction to writeback stage
	void pushToWriteBackWithPriority(Simulator * simPtr);
	
	void priorityPushToWriteBack(std::pair<int,SourceLine> numberLinePair1,Simulator * simPtr);
	void compareInstructionPriority(std::pair<int,SourceLine> numberLinePair1,std::pair<int,SourceLine> numberLinePair2,Simulator * simPtr);
};

class WriteBackUnit:protected FunctionalUnit{
public:
	WriteBackUnit(int inputNumberOfCycles,int noOfInstr);
	void checkStatus(Simulator * simPtr);
};


class PhysicalMemory{
public:
	static bool isOccupiedByDcache;
	static bool isOccupiedByIcache;
	static int dataCacheMissInCycle;
	static bool isdataCacheMissInSameCycle;
};

#endif