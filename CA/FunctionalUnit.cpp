#include"FunctoinalUnits.h"
#include"Utility.h"
int FunctionalUnit::clockTick = 1;
std::deque<pair<int,SourceLine>> FunctionalUnit::decodedeque;
std::deque<pair<int,SourceLine>> FunctionalUnit::integerALUdeque;
std::deque<pair<int,SourceLine>> FunctionalUnit::memorydeque;
std::deque<pair<int,SourceLine>> FunctionalUnit::fpAdderdeque;
std::deque<pair<int,SourceLine>> FunctionalUnit::fpMultdeque;
std::deque<pair<int,SourceLine>> FunctionalUnit::fpDivdeque;
std::deque<pair<int,SourceLine>> FunctionalUnit::wbdeque;
vector<displayInfo *> FunctionalUnit::completionVector;
pair<bool,int> FunctionalUnit::branchTakenInstrNumberPair;
bool FunctionalUnit::branchTakenSameCycle = false;;
int FunctionalUnit::inFileInstruction = 0;
int displayInfo::instrCacheRequest=0;
int displayInfo::instrCacheHit=0;
int displayInfo::dataCacheRequest=0;
int displayInfo::dataCacheHit=0;
bool PhysicalMemory::isOccupiedByDcache;
bool PhysicalMemory::isOccupiedByIcache;
int PhysicalMemory::dataCacheMissInCycle=-1;
bool PhysicalMemory::isdataCacheMissInSameCycle = false;
bool FunctionalUnit::isExecutionFinished = false;
bool FunctionalUnit::isIFDoneAfterExecFinished = false;
bool FunctionalUnit::isBranchNotTakenOver = false;
displayInfo::displayInfo()
{
	actualInstruction="";
	completionTime = NULL;
	war = false;
	raw = false;
	waw = false;
	structural = false;
	isStructuralHazardBeforeIU = false;
}

FunctionalUnit::FunctionalUnit()
{
		
	numberOfCycles = 0;
	busy =false;
	instructionNumber =0;
}
FunctionalUnit::FunctionalUnit(int numberOfCycles, int noOfInstr)
{
	busy =false;
	instructionNumber =0;
	this->numberOfCycles = numberOfCycles;
	cyclesLeftForInstr = numberOfCycles;
}

bool FunctionalUnit::areAllPipeLinesEmpty()
{
	if(isIFDoneAfterExecFinished && decodedeque.empty() && integerALUdeque.empty() && memorydeque.empty() && fpAdderdeque.empty() &&	fpMultdeque.empty() && fpDivdeque.empty() && wbdeque.empty())
	{
		return true;
	}
	return false;
}
void  FunctionalUnit::displayCompletionTime(const char * outputFilePath)
{

	//open file to write
	ofstream outputFile;
	outputFile.open(outputFilePath);
	string instructionStr = "Instruction";
	instructionStr.resize(25,' ');
	outputFile<<instructionStr<<"\t";
	outputFile<<"IF\t\t";
	outputFile<<"ID\t\t";
	outputFile<<"EX\t\t";
	outputFile<<"Mem\t\t";
	outputFile<<"WB\t\t";
	outputFile<<"RAW\t\t";
	outputFile<<"WAR\t\t";
	outputFile<<"WAW\t\t";
	outputFile<<"Struct\n";

	for(int i=0;i<completionVector.size();i++)
	{
		instructionStr = completionVector[i]->actualInstruction;
		instructionStr.resize(25,' ');
		cout<<endl<<instructionStr<<"\t";
		outputFile<<instructionStr<<"\t";
		for(int j=0;j<5;j++)
		{
			cout<<completionVector[i]->completionTime[j]<<"\t";
			outputFile<<completionVector[i]->completionTime[j]<<"\t\t";
		}
		cout<<completionVector[i]->raw<<"\t";
		cout<<completionVector[i]->war<<"\t";
		cout<<completionVector[i]->waw<<"\t";
		cout<<completionVector[i]->structural<<"\t";

		outputFile<<completionVector[i]->raw<<"\t\t";
		outputFile<<completionVector[i]->war<<"\t\t";
		outputFile<<completionVector[i]->waw<<"\t\t";
		outputFile<<completionVector[i]->structural<<"\t\t";

		outputFile<<endl;
		cout<<endl;
	}

	outputFile<<endl;
	outputFile<<"Total number of requests to instruction cache  "<<completionVector.size()<<endl;
	outputFile<<"Total number of instruction cache hit  "<<displayInfo::instrCacheHit<<endl;
	outputFile<<"Total number of requests to data cache  "<<displayInfo::dataCacheRequest<<endl;
	outputFile<<"Total number of data cache hit  "<<displayInfo::dataCacheHit<<endl;
	outputFile.close();
}


InstrFetch::InstrFetch(int inputNumberOfCycles,int noOfInstr):FunctionalUnit(inputNumberOfCycles, noOfInstr)
{
		isCycleAssigned = false;
		isInstrCacheHit = false;
}

void InstrFetch::checkStatus(Simulator * simPtr)
{
	//initalize the FU
	if(instructionNumber==0)
	{
		instructionNumber = 1;
		inFileInstruction=1;
		busy = true;
		if(!checkCacheAndAssignCyclesLeft(inFileInstruction,simPtr))
		{
			//there was cache miss and the physical memory is still occupied
			return;
		}
		cyclesLeftForInstr--;
	}

	else if(instructionNumber!=0 && cyclesLeftForInstr>0)
	{
			
		busy = true;
		
		if(FunctionalUnit::branchTakenInstrNumberPair.first && FunctionalUnit::isBranchNotTakenOver)
		{
			//+1 because labels storing from 0
			inFileInstruction = branchTakenInstrNumberPair.second + 1;
			FunctionalUnit::branchTakenInstrNumberPair.first = false;
			FunctionalUnit::isBranchNotTakenOver = false;
		}
		//intentionally added this if agter before if
		//for branch instruction we need to waste one cycle in IF
		//this is achieved by his and the before if
		if(FunctionalUnit::branchTakenInstrNumberPair.first)
		{
			FunctionalUnit::isBranchNotTakenOver = true;
		}
		if(!checkCacheAndAssignCyclesLeft(inFileInstruction,simPtr))
		{
			//there was cache miss and the physical memory is still occupied
			return;
		}
		cyclesLeftForInstr--;

	}
	//if numberOfCycles=0 print
	if(cyclesLeftForInstr<=0 && FunctionalUnit::decodedeque.empty())
	{	
		//if will enter unless second HLT is added 
		if(!FunctionalUnit::isIFDoneAfterExecFinished)
		{
			int * arr = new int[5];
			for(int i=0;i<5;i++)
			{
				arr[i] = -1;
			}
			arr[0] =  FunctionalUnit::clockTick;
			//create display info
			displayInfo * di= new displayInfo();
			di->completionTime = arr;

			di->actualInstruction = simPtr->instructionVector[inFileInstruction-1].actualSourceLine;
			if(isInstrCacheHit)
			{
				displayInfo::instrCacheHit++;
				//set isInstrCacheHit to false again
				isInstrCacheHit = false;
			}
		
			completionVector.push_back(di);

			//set FunctionalUnit::isExecutionFinished if isExecutionFinished is set
			if(FunctionalUnit::isExecutionFinished)
			{
				FunctionalUnit::isIFDoneAfterExecFinished = true;
			}
		}
		//if will enter unless 1st hlt is added
		//in case instruction already fetched and stalled for decode queue
		//and there was a jump
		//then the third condition
		if(!FunctionalUnit::isExecutionFinished && !FunctionalUnit::isBranchNotTakenOver && !FunctionalUnit::branchTakenInstrNumberPair.first)
		{
			FunctionalUnit::decodedeque.push_back(make_pair(instructionNumber,simPtr->instructionVector[inFileInstruction-1]));
			instructionNumber++;
			inFileInstruction++;
			cyclesLeftForInstr = numberOfCycles;
			busy = false;
			//test
			isCycleAssigned = false;
			//test
		}
		
		//Current instruction is branch not taken instruction
		//next cycle fetch new instruction
		if(FunctionalUnit::isBranchNotTakenOver)
		{
			cyclesLeftForInstr = numberOfCycles;
			instructionNumber++;
			//test
			isCycleAssigned = false;
			//test
			//inFileInstruction++;
		}

		//as this condition is taken care of in the above if
		//reset the value
		//fetch next instruction
		if(FunctionalUnit::branchTakenInstrNumberPair.first && !FunctionalUnit::isBranchNotTakenOver)
		{
				//FunctionalUnit::branchTakenInstrNumberPair.first = false;
				FunctionalUnit::isBranchNotTakenOver = true;
				cyclesLeftForInstr = numberOfCycles;
				//test
				isCycleAssigned = false;
				//test
				instructionNumber++;
		}
		
	}
}

bool InstrFetch::checkCacheAndAssignCyclesLeft(int instructionNumber,Simulator * simPtr)
{
	if(!isCycleAssigned)
	{
		if(iCache.isInstructionCacheHit(instructionNumber))
		{
			cyclesLeftForInstr =1;
			isInstrCacheHit = true;
		}
		else
		{	
			cyclesLeftForInstr = 2 * (simPtr->memAccessCycles + simPtr->iCacheAccessCycles);
		}

		//it is cache miss
		if(cyclesLeftForInstr>simPtr->iCacheAccessCycles)
		{
			
			//if data and instruction cache miss in same cycle - give preference to iCache
			if(PhysicalMemory::isOccupiedByDcache && PhysicalMemory::dataCacheMissInCycle==FunctionalUnit::clockTick)
			{
				PhysicalMemory::isdataCacheMissInSameCycle = true;
				PhysicalMemory::isOccupiedByDcache = false;
				PhysicalMemory::isOccupiedByIcache = true;
			}
			if(PhysicalMemory::isOccupiedByDcache)
			{
				//if occupied we need to stall
				isCycleAssigned = true;
				return false;
			}
			else if(!PhysicalMemory::isOccupiedByDcache)
			{
				PhysicalMemory::isOccupiedByIcache = true;
			}
			
		}
		isCycleAssigned = true;
	}
	//if physical memory ocupied by dcache and we had a miss we need to stall
	if(PhysicalMemory::isOccupiedByDcache && isCycleAssigned && !isInstrCacheHit)
	{
		return false;
	}
	/*else if(!PhysicalMemory::isOccupiedByDcache && isCycleAssigned && !isInstrCacheHit)
	{
		PhysicalMemory::isOccupiedByIcache = true;
	}*/
	//In case of bus contention on the same cyccle 
	//if(PhysicalMemory::isOccupied && PhysicalMemory::dataCacheMissInSameCycle)
	//{
	//	FunctionalUnit::memorydeque.front().second.exStagesLeft++;
	//}
	if(cyclesLeftForInstr==1)
	{
		PhysicalMemory::isOccupiedByIcache = false;
		//test
		//isCycleAssigned = false;
		//test
		if(PhysicalMemory::isdataCacheMissInSameCycle)
		{
			PhysicalMemory::isdataCacheMissInSameCycle = false;
			PhysicalMemory::isOccupiedByDcache = true;
			FunctionalUnit::memorydeque.front().second.exStagesLeft++;
		}
	}

	return true;
}
InstrDecode::InstrDecode(int inputNumberOfCycles,int noOfInstr):FunctionalUnit(inputNumberOfCycles, noOfInstr)
{
		
}
bool InstrDecode::isThereAFunctionalUnit(SourceLine instr,Simulator * simPtr)
{
	std::string functUnit =simPtr->symbolTable[instr.opCode].fuUsed;
	if(functUnit.compare("")==0)
	{
		return false;
	}
	else
	{
		return true;
	}
}

bool InstrDecode::isTheFunctionalUnitFree(SourceLine instr,Simulator * simPtr)
{
	std::string functUnit =simPtr->symbolTable[instr.opCode].fuUsed;
	if(functUnit.compare(FU_INTEGER)==0)
	{
		if(FunctionalUnit::integerALUdeque.empty())
		{
			return true;
		}

	}
	else if(functUnit.compare(FU_FP_ADDER)==0)
	{
		if(FunctionalUnit::fpAdderdeque.empty())
		{
			return true;
		}
	}
	else if(functUnit.compare(FU_FP_MULT)==0)
	{
		if(FunctionalUnit::fpMultdeque.empty())
		{
			return true;
		}
	}
	else if(functUnit.compare(FU_FP_DIV)==0)
	{
		if(FunctionalUnit::fpDivdeque.empty())
		{
			return true;
		}
	}
		
	return false;
}

//pipeline implementation in the form of deque for each functional unit
bool InstrDecode::push_backToCorrectFunctionalUnit(SourceLine instr,Simulator * simPtr, int instructionNumber)
{
	std::string functUnit =simPtr->symbolTable[instr.opCode].fuUsed;
	if(functUnit.compare(FU_INTEGER)==0)
	{
		//memory heirarchy will be implemeted here
		//use exstagesLeft for completion of memory
		//1 for integer ALU
		instr.exStagesLeft = simPtr->memAccessCycles + 1 ;
		if(FunctionalUnit::integerALUdeque.empty())
		{
			FunctionalUnit::integerALUdeque.push_back(make_pair(instructionNumber,instr));
			return true;
		}

	}
	else if(functUnit.compare(FU_FP_ADDER)==0)
	{
		instr.exStagesLeft = simPtr->FPAdderStages;
		if(!(simPtr->isFPAdderPipe) && FunctionalUnit::fpAdderdeque.empty())
		{
			FunctionalUnit::fpAdderdeque.push_back(make_pair(instructionNumber,instr));
			return true;
		}
		else if(simPtr->isFPAdderPipe && ((simPtr->FPAdderStages - FunctionalUnit::fpAdderdeque.size())>0) )
		{
			FunctionalUnit::fpAdderdeque.push_back(make_pair(instructionNumber,instr));
			return true;
		}
	}
	else if(functUnit.compare(FU_FP_MULT)==0)
	{
		instr.exStagesLeft = simPtr->FPMultStages;
		if(!(simPtr->isFPMultPipe) && FunctionalUnit::fpMultdeque.empty())
		{
			FunctionalUnit::fpMultdeque.push_back(make_pair(instructionNumber,instr));
			return true;
		}
		else if(simPtr->isFPMultPipe && ((simPtr->FPMultStages - FunctionalUnit::fpMultdeque.size())>0) )
		{
			FunctionalUnit::fpMultdeque.push_back(make_pair(instructionNumber,instr));
			return true;
		}
	}
	else if(functUnit.compare(FU_FP_DIV)==0)
	{
		instr.exStagesLeft = simPtr->FPDivStages;			
		if(!(simPtr->isFPDivPipe) && FunctionalUnit::fpDivdeque.empty())
		{
			FunctionalUnit::fpDivdeque.push_back(make_pair(instructionNumber,instr));
			return true;
		}
		else if(simPtr->isFPDivPipe && ((simPtr->FPDivStages - FunctionalUnit::fpDivdeque.size())>0))
		{
			FunctionalUnit::fpDivdeque.push_back(make_pair(instructionNumber,instr));
			return true;
		}
	}
	else
	{
		return false;
	}
	return false;
}

void InstrDecode::checkStatus(Simulator * simPtr)
{
	SourceLine instr;
	//instruction not reached the FU yet
	if(FunctionalUnit::decodedeque.empty())
	{
		return;
	}
	else
	{
		//condition for new instruction to be fetched
		if(cyclesLeftForInstr==numberOfCycles && !FunctionalUnit::decodedeque.empty() )
		{
			
			instructionNumber = FunctionalUnit::decodedeque.front().first;
			instr = FunctionalUnit::decodedeque.front().second;
			if(isRAWHazard(simPtr,instr))
			{
				completionVector[instructionNumber-1]->raw = true;
				return;
			}
			cyclesLeftForInstr--;

			//read registers for the current instruction
			readRegisters(instr,simPtr);

			//if instr changed needs to be pushed back to ID dequeue
			FunctionalUnit::decodedeque.pop_front();
			FunctionalUnit::decodedeque.push_front(make_pair(instructionNumber,instr));
		}
		//current instruction cycles still left
		else if(cyclesLeftForInstr>0)
		{
			cyclesLeftForInstr--;
		}
			
	}
	//cycles for current instruction over and next path empty
	if(cyclesLeftForInstr==0 && !FunctionalUnit::decodedeque.empty())
	{
		//IMP: instr was having null once coming directly in this IF
		instr = FunctionalUnit::decodedeque.front().second;
		
		if(!isThereAFunctionalUnit(instr,simPtr))
		{

			processBranchInstruction(instr,simPtr);
			if(strcmpi(instr.opCode.c_str(),"hlt")==0)
			{
				//if branch taken
				if(processHalt(instr,simPtr))
				{
					completionVector[instructionNumber-1]->completionTime[1] = -1;
				}
				//if branch not taken this is end of execution
				else
				{
					completionVector[instructionNumber-1]->completionTime[1] = FunctionalUnit::clockTick;
					//return false;
					//throw "stop";
					isExecutionFinished = true;
				}
				FunctionalUnit::decodedeque.pop_front();
				cyclesLeftForInstr = numberOfCycles;
				cout<<instr.opCode<<"\t"<<completionVector[instructionNumber-1]->completionTime[0]<<"\t"<<completionVector[instructionNumber-1]->completionTime[1]<<endl;
			}
			else
			{
				FunctionalUnit::decodedeque.pop_front();
				completionVector[instructionNumber-1]->completionTime[1] = FunctionalUnit::clockTick;
				cyclesLeftForInstr = numberOfCycles;
				cout<<instr.opCode<<"\t"<<completionVector[instructionNumber-1]->completionTime[0]<<"\t"<<completionVector[instructionNumber-1]->completionTime[1]<<endl;
			}
			//if a branch or halt instruction return - no work left
			return;
		}

			
		//if push_backToCorrectFunctionalUnit succeeds then proceed with the next instr for decode else stall decode
		//if(!push_backToCorrectFunctionalUnit(simPtr->instructionVector[instructionNumber-1],simPtr,instructionNumber))
 		if(!push_backToCorrectFunctionalUnit(instr,simPtr,instructionNumber))
		{
			completionVector[instructionNumber-1]->structural = true;
			return;
		}
		//if successful lock destination register
		//if lock fails there is WAW hazard - stall in that case
		if(!setLock(simPtr,instr))
		{
			completionVector[instructionNumber-1]->waw = true;
			return;
		}
		FunctionalUnit::decodedeque.pop_front();
		completionVector[instructionNumber-1]->completionTime[1] = FunctionalUnit::clockTick;
		cyclesLeftForInstr = numberOfCycles;
	}
}

bool InstrDecode::processHalt(SourceLine & instr,Simulator * simPtr)
{
	
		if(FunctionalUnit::branchTakenInstrNumberPair.first)
		{
				return true;
		}
		return false;
}

//function processes branch and halt instructions
//if halt instruction and branch was taken  return true
//if halt instruction and branch was not taken  return false
void InstrDecode::processBranchInstruction(SourceLine & instr,Simulator * simPtr)
{	
	if(strcmpi(instr.opCode.c_str(),"bne")==0)
	{
		if(instr.operandValues[1]!=instr.operandValues[2])
		{
			FunctionalUnit::branchTakenInstrNumberPair.first = true;
			FunctionalUnit::branchTakenSameCycle = true;
			FunctionalUnit::branchTakenInstrNumberPair.second = simPtr->labelInstMap[instr.operands[0]];
		}
	}
	else if(strcmpi(instr.opCode.c_str(),"beq")==0)
	{
		if(instr.operandValues[1]==instr.operandValues[2])
		{
			FunctionalUnit::branchTakenInstrNumberPair.first = true;
			FunctionalUnit::branchTakenSameCycle = true;
			FunctionalUnit::branchTakenInstrNumberPair.second = simPtr->labelInstMap[instr.operands[0]];
		}
	}
	else if(strcmpi(instr.opCode.c_str(),"j")==0)
	{
		FunctionalUnit::branchTakenInstrNumberPair.first = true;
		FunctionalUnit::branchTakenSameCycle = true;
		FunctionalUnit::branchTakenInstrNumberPair.second = simPtr->labelInstMap[instr.operands[0]];
	}
	



}
void InstrDecode::readRegisters(SourceLine & instr,Simulator * simPtr)
{
	//if functional unit not integer return
	if(simPtr->symbolTable[instr.opCode].fuUsed!=FU_INTEGER && !(strcmpi(instr.opCode.c_str(),"bne")==0) &&  !(strcmpi(instr.opCode.c_str(),"beq")==0))
	{
		return;
	}

	string destFormat  = simPtr->symbolTable[instr.opCode].destSrcFormat;
	string source1="";
	string source2="";
	string source0= "";
	for(int i=0;i<3;i++)
	{
		//dest_src_src - 012
		//load R1,2(R2)
		//src 2
		//src R2
		//or if instruction is store

		//if store word "sw" we need to read operand at position 0 as well
		//we dont care of store double
		if(destFormat[i]-'0' == 0 && strcmpi(instr.opCode.c_str(),"sw")==0)
		{
			source0 = instr.operands[0];
			if(source0.find("R") != string::npos || source0.find("r") != string::npos)
			{
				source0.erase(0,1);
				source0 = "r" + source0;
				instr.operandValues[0] = simPtr->registerMap[source0];
			}
		}
		if(destFormat[i]-'0' == 1)
		{
			source1 = instr.operands[1];
			if(source1.find("R") != string::npos || source1.find("r") != string::npos)
			{
				source1.erase(0,1);
				source1 = "r" + source1;
				instr.operandValues[1] = simPtr->registerMap[source1];
			}
			else if(Utility::isStringNumber(source1))
			{
				instr.operandValues[1] = atoi(source1.c_str());
			}

			//else throw exception
		}

		else if(destFormat[i]-'0' == 2)
		{
			source2 = instr.operands[2];
			if(source2.find("R") != string::npos || source2.find("r") != string::npos)
			{
				source2.erase(0,1);
				source2 = "r" + source2;
				instr.operandValues[2] = simPtr->registerMap[source2];
			}
			else if(Utility::isStringNumber(source2))
			{
				instr.operandValues[2] = atoi(source2.c_str());
			}

			//else throw exception
		}
		
	}


}


IntegerAlu::IntegerAlu()
{
	
}
IntegerAlu::IntegerAlu(int inputNumberOfCycles,int noOfInstr):FunctionalUnit(inputNumberOfCycles, noOfInstr)
{
	
}


void IntegerAlu::checkStatus(Simulator * simPtr)
{
	SourceLine instr;
	//instruction not reached the FU yet
	if(FunctionalUnit::integerALUdeque.empty())
	{
		return;
	}
		
	else
	{
		//condition for new instruction to be fetched
		//if(cyclesLeftForInstr==numberOfCycles && !FunctionalUnit::integerALUdeque.empty() )
		if(!FunctionalUnit::integerALUdeque.empty() )
		{
			instr = FunctionalUnit::integerALUdeque.front().second;
			instructionNumber = FunctionalUnit::integerALUdeque.front().first;
			// call execute over here
			executeIntegerInstruction(instr,simPtr);
			instr.exStagesLeft--;
			cyclesLeftForInstr=instr.exStagesLeft;
		}
			
		//if first cycle of the instruction
		//execute the actual instruction
		if(cyclesLeftForInstr==(simPtr->memAccessCycles))
		{
			//instruction must have changed and that needs to be pushed in front
			FunctionalUnit::integerALUdeque.pop_front();
			FunctionalUnit::integerALUdeque.push_front(make_pair(instructionNumber,instr));
		}
		if(cyclesLeftForInstr< simPtr->memAccessCycles)
		{
				cyclesLeftForInstr = simPtr->memAccessCycles;
				//reinitialize for mem stage 
				FunctionalUnit::integerALUdeque.front().second.exStagesLeft = simPtr->memAccessCycles;
		}
			
	}
	//exStagesLeft = 1 + simPtr->memAccessCycles
	if(cyclesLeftForInstr==simPtr->memAccessCycles && FunctionalUnit::memorydeque.empty() && !FunctionalUnit::integerALUdeque.empty())
	{
			
		FunctionalUnit::memorydeque.push_back(FunctionalUnit::integerALUdeque.front());
		FunctionalUnit::integerALUdeque.pop_front();
		completionVector[instructionNumber-1]->completionTime[2] = FunctionalUnit::clockTick;
		//cyclesLeftForInstr = numberOfCycles;
	}
	else if(cyclesLeftForInstr==simPtr->memAccessCycles && !FunctionalUnit::memorydeque.empty() && !FunctionalUnit::integerALUdeque.empty())
	{
		//if set set another flag to indicate structural hazard already occurred before IU
		if(completionVector[instructionNumber-1]->structural )
		{
			completionVector[instructionNumber-1]->isStructuralHazardBeforeIU = true;
		}
		completionVector[instructionNumber-1]->structural = true;
	}
}

//function to execute only integer instruction
void IntegerAlu::executeIntegerInstruction(SourceLine & instr,Simulator * simPtr)
{

	if(strcmpi(instr.opCode.c_str(),"lw")==0)
	{
		//from some location in data
		instr.operandValues[0] = simPtr->data[((instr.operandValues[1] + instr.operandValues[2])-256)/4];
	}
	else if(strcmpi(instr.opCode.c_str(),"l.d")==0)
	{
		
	}
	//do we need store word
	else if(strcmpi(instr.opCode.c_str(),"sw")==0)
	{
		//to some location in data
		simPtr->data[((instr.operandValues[1] + instr.operandValues[2])-256)/4] = instr.operandValues[0];
	}
	else if(strcmpi(instr.opCode.c_str(),"s.d")==0)
	{
		
	}
	//do we need to store double??
	else if(strcmpi(instr.opCode.c_str(),"dadd")==0)
	{
		instr.operandValues[0] = instr.operandValues[1] + instr.operandValues[2];
	} 
	else if(strcmpi(instr.opCode.c_str(),"daddi")==0)
	{
		instr.operandValues[0] = instr.operandValues[1] + instr.operandValues[2];
	}
	else if(strcmpi(instr.opCode.c_str(),"dsub")==0)
	{
		instr.operandValues[0] = instr.operandValues[1] - instr.operandValues[2];
	}
	else if(strcmpi(instr.opCode.c_str(),"dsubi")==0)
	{
		instr.operandValues[0] = instr.operandValues[1] - instr.operandValues[2];
	}
	else if(strcmpi(instr.opCode.c_str(),"and")==0)
	{
		instr.operandValues[0] = instr.operandValues[1] & instr.operandValues[2];
	}
	else if(strcmpi(instr.opCode.c_str(),"andi")==0)
	{
		instr.operandValues[0] = instr.operandValues[1] & instr.operandValues[2];
	}
	else if(strcmpi(instr.opCode.c_str(),"or")==0)
	{
		instr.operandValues[0] = instr.operandValues[1] | instr.operandValues[2];
	}
	else if(strcmpi(instr.opCode.c_str(),"ori")==0)
	{
		instr.operandValues[0] = instr.operandValues[1] | instr.operandValues[2];
	}


}

Memory::Memory()
{
	
}
Memory::Memory(int inputNumberOfCycles,int noOfInstr):FunctionalUnit(inputNumberOfCycles, noOfInstr)
{
	isCycleAssigned = false;
	isDataCacheHit = false;
}

void Memory::checkStatus(Simulator * simPtr,vector<pair<int,SourceLine>> & writeBackPriorityVector)
{
	SourceLine instr;
	//instruction not reached the FU yet
	if(FunctionalUnit::memorydeque.empty())
	{
		return;
	}
	else 
	{
		//condition for new instruction to be fetched
		//if(cyclesLeftForInstr==numberOfCycles && !FunctionalUnit::memorydeque.empty() )
		if(!FunctionalUnit::memorydeque.empty())
		{
			instructionNumber = FunctionalUnit::memorydeque.front().first;
			instr = FunctionalUnit::memorydeque.front().second;
			//check cache cycles
			if(!checkCacheAndAssignCyclesLeft(instr,simPtr,instructionNumber))
			{
				//there was cache miss and the physical memory is still occupied
				return;
			}

			FunctionalUnit::memorydeque.front().second.exStagesLeft--;
			cyclesLeftForInstr = FunctionalUnit::memorydeque.front().second.exStagesLeft;
		}
		//current instruction cycles still left
		if(cyclesLeftForInstr<0)
		{
			cyclesLeftForInstr = 0;
		}
			
	}

	//if memory not required by the instruction it will require only one cycle in mem fu
	if((cyclesLeftForInstr==0 && FunctionalUnit::wbdeque.empty() && !FunctionalUnit::memorydeque.empty()) || !(simPtr->symbolTable[instr.opCode].memoryRequired))
	{
		//FunctionalUnit::memorydeque.pop_front();
		//FunctionalUnit::wbdeque.push_back(make_pair(instructionNumber,simPtr->instructionVector[instructionNumber-1]));
		//completionVector[instructionNumber-1]->completionTime[3] = FunctionalUnit::clockTick;

		writeBackPriorityVector.push_back(memorydeque.front());
	}

}

bool Memory::checkCacheAndAssignCyclesLeft(SourceLine instr,Simulator * simPtr,int instructionNumber)
{
	bool instructionMatched = false;
	
	//if exStagesLeft = 0 that means priority was given to some one else
	//no need to enter again
	if(!isCycleAssigned && FunctionalUnit::memorydeque.front().second.exStagesLeft!=0)
	{
		
		if(strcmpi(instr.opCode.c_str(),"lw")==0)
		{
			FunctionalUnit::memorydeque.front().second.exStagesLeft = dCache.getLwLatency((instr.operandValues[1] + instr.operandValues[2]),simPtr);
			//if cache hit
			if(FunctionalUnit::memorydeque.front().second.exStagesLeft == simPtr->dCacheAccessCycles)
			{
				FunctionalUnit::completionVector[instructionNumber-1]->dataCacheHit++;
			}
			FunctionalUnit::completionVector[instructionNumber-1]->dataCacheRequest++;
			instructionMatched = true;
		}
		else if(strcmpi(instr.opCode.c_str(),"l.d")==0)
		{
			//FunctionalUnit::memorydeque.front().second.exStagesLeft = dCache.getLwLatency((instr.operandValues[1] + instr.operandValues[2]),simPtr)
				//+ dCache.getLwLatency((instr.operandValues[1] + instr.operandValues[2]) + 4,simPtr);
			
			int firstWordCycles = dCache.getLwLatency((instr.operandValues[1] + instr.operandValues[2]),simPtr);
			int secondWordCycles = dCache.getLwLatency((instr.operandValues[1] + instr.operandValues[2]) + 4,simPtr);

			if(firstWordCycles == simPtr->dCacheAccessCycles)
			{
				FunctionalUnit::completionVector[instructionNumber-1]->dataCacheHit++;
			}
			if(secondWordCycles == simPtr->dCacheAccessCycles)
			{
				FunctionalUnit::completionVector[instructionNumber-1]->dataCacheHit++;
			}

			FunctionalUnit::memorydeque.front().second.exStagesLeft = firstWordCycles + secondWordCycles;
			FunctionalUnit::completionVector[instructionNumber-1]->dataCacheRequest+=2;
			instructionMatched = true;
		}
		else if(strcmpi(instr.opCode.c_str(),"sw")==0)
		{
			FunctionalUnit::memorydeque.front().second.exStagesLeft = dCache.getSwLatency((instr.operandValues[1] + instr.operandValues[2]),simPtr);	
			//if cache hit
			if(FunctionalUnit::memorydeque.front().second.exStagesLeft == simPtr->dCacheAccessCycles)
			{
				FunctionalUnit::completionVector[instructionNumber-1]->dataCacheHit++;
			}
			FunctionalUnit::completionVector[instructionNumber-1]->dataCacheRequest++;
			instructionMatched = true;
		}
		else if(strcmpi(instr.opCode.c_str(),"s.d")==0)
		{
			//FunctionalUnit::memorydeque.front().second.exStagesLeft = dCache.getSwLatency((instr.operandValues[1] + instr.operandValues[2]),simPtr) + 
			//	dCache.getSwLatency((instr.operandValues[1] + instr.operandValues[2]) + 4,simPtr);
			
			int firstWordCycles = dCache.getSwLatency((instr.operandValues[1] + instr.operandValues[2]),simPtr);
			int secondWordCycles = dCache.getSwLatency((instr.operandValues[1] + instr.operandValues[2]) + 4,simPtr);

			if(firstWordCycles == simPtr->dCacheAccessCycles)
			{
				FunctionalUnit::completionVector[instructionNumber-1]->dataCacheHit++;
			}
			if(secondWordCycles == simPtr->dCacheAccessCycles)
			{
				FunctionalUnit::completionVector[instructionNumber-1]->dataCacheHit++;
			}

			FunctionalUnit::memorydeque.front().second.exStagesLeft = firstWordCycles + secondWordCycles;
			FunctionalUnit::completionVector[instructionNumber-1]->dataCacheRequest+=2;
			instructionMatched = true;
		}

		//if cache hit latency cycles = data cache access cycles
		if(instructionMatched)
		{
			//wrong condition for checking dcache hit
			//assumption for bus contention
			//in case of l.d or s.d the two addresses are checked one after the other ?
			if((FunctionalUnit::memorydeque.front().second.exStagesLeft == simPtr->dCacheAccessCycles && (strcmpi(instr.opCode.c_str(),"sw")==0 || strcmpi(instr.opCode.c_str(),"lw")==0)) ||
				(FunctionalUnit::memorydeque.front().second.exStagesLeft == 2*simPtr->dCacheAccessCycles && (strcmpi(instr.opCode.c_str(),"s.d")==0 || strcmpi(instr.opCode.c_str(),"l.d")==0)))
			{
				isDataCacheHit = true;
			}
			//cache miss - occupy physical memory
			else
			{
				if(PhysicalMemory::isOccupiedByIcache)
				{
					isCycleAssigned = true;
					return false;	
				}
				else
				{
					PhysicalMemory::isOccupiedByDcache = true;
					PhysicalMemory::dataCacheMissInCycle = FunctionalUnit::clockTick;
				}
			}
			isCycleAssigned = true;
		}


	}

	//if physical memory ocupied by dcache we need to stall
	if(PhysicalMemory::isOccupiedByIcache && isCycleAssigned && !isDataCacheHit)
	{
		return false;
	}
	//else if(!PhysicalMemory::isOccupiedByIcache && isCycleAssigned && !isDataCacheHit)
	//{
	//	PhysicalMemory::isOccupiedByDcache = true;
	//	PhysicalMemory::dataCacheMissInCycle = FunctionalUnit::clockTick;
	//}

	if(FunctionalUnit::memorydeque.front().second.exStagesLeft==1)
	{
		isCycleAssigned = false;
		PhysicalMemory::isOccupiedByDcache = false;
		isDataCacheHit = false;
	}
	return true;
}

IntegerExecPath::IntegerExecPath()
{
	
}
IntegerExecPath::IntegerExecPath(IntegerAlu intAluFu, Memory memFu)
{
	this->intAluFu = intAluFu;
	this->memFu = memFu;
}

void IntegerExecPath::checkStatus(Simulator * simPtr,vector<pair<int,SourceLine>> & writeBackPriorityVector)
{
	//if register fetch busy cannot complete this stage
	memFu.checkStatus(simPtr,writeBackPriorityVector);
	intAluFu.checkStatus(simPtr);

}

FPAdderUnit::FPAdderUnit()
{
	pipeLined = false;
}
FPAdderUnit::FPAdderUnit(int inputNumberOfCycles,bool pipeLined,int noOfInstr):FunctionalUnit(inputNumberOfCycles, noOfInstr)
{
		
	this->pipeLined = pipeLined;
}

void FPAdderUnit::checkStatus(Simulator * simPtr,vector<pair<int,SourceLine>> & writeBackPriorityVector)
{
	SourceLine instr;
	//instruction not reached the FU yet
	if(FunctionalUnit::fpAdderdeque.empty())
	{
		return;
	}
	else
	{
		//condition for new instruction to be fetched
		//if(cyclesLeftForInstr==numberOfCycles && !FunctionalUnit::fpAdderdeque.empty() )
		if(!FunctionalUnit::fpAdderdeque.empty() )
		{
			instructionNumber = FunctionalUnit::fpAdderdeque.front().first;
			instr = FunctionalUnit::fpAdderdeque.front().second;

			std::deque<pair<int,SourceLine>>::iterator it;
			for(it=FunctionalUnit::fpAdderdeque.begin();it<FunctionalUnit::fpAdderdeque.end();it++)
			{
				it->second.exStagesLeft--;
			}
			cyclesLeftForInstr = FunctionalUnit::fpAdderdeque.begin()->second.exStagesLeft;
			if(cyclesLeftForInstr<0)
			{
				cyclesLeftForInstr = 0;
			}
		}
			
	}

	if(cyclesLeftForInstr==0 && FunctionalUnit::wbdeque.empty() && !FunctionalUnit::fpAdderdeque.empty())
	{
		//FunctionalUnit::fpAdderdeque.pop_front();
		//FunctionalUnit::wbdeque.push_back(make_pair(instructionNumber,simPtr->instructionVector[instructionNumber-1]));
		//completionVector[instructionNumber-1]->completionTime[2] = FunctionalUnit::clockTick;
		writeBackPriorityVector.push_back(FunctionalUnit::fpAdderdeque.front());

	}
}

FPMultUnit::FPMultUnit()
{
	pipeLined = false;
}
FPMultUnit::FPMultUnit(int inputNumberOfCycles, bool pipLined,int noOfInstr):FunctionalUnit(inputNumberOfCycles, noOfInstr)
{
		
	this->pipeLined = pipeLined;
}

void FPMultUnit::checkStatus(Simulator * simPtr,vector<pair<int,SourceLine>> & writeBackPriorityVector)
{
	SourceLine instr;
	//instruction not reached the FU yet
	if(FunctionalUnit::fpMultdeque.empty())
	{
		return;
	}
	else 
	{
		//condition for new instruction to be fetched
		if(!FunctionalUnit::fpMultdeque.empty() )
		{
			instructionNumber = FunctionalUnit::fpMultdeque.front().first;
			instr = FunctionalUnit::fpMultdeque.front().second;
			std::deque<pair<int,SourceLine>>::iterator it;
			for(it=FunctionalUnit::fpMultdeque.begin();it<FunctionalUnit::fpMultdeque.end();it++)
			{
				it->second.exStagesLeft--;
			}
			cyclesLeftForInstr = FunctionalUnit::fpMultdeque.begin()->second.exStagesLeft;
			if(cyclesLeftForInstr<0)
			{
				cyclesLeftForInstr = 0;
			}
		}
			
	}

	if(cyclesLeftForInstr==0 && FunctionalUnit::wbdeque.empty() && !FunctionalUnit::fpMultdeque.empty())
	{
		//FunctionalUnit::fpMultdeque.pop_front();
		//FunctionalUnit::wbdeque.push_back(make_pair(instructionNumber,simPtr->instructionVector[instructionNumber-1]));
		//completionVector[instructionNumber-1]->completionTime[2] = FunctionalUnit::clockTick;
		writeBackPriorityVector.push_back(FunctionalUnit::fpMultdeque.front());
	}
}

FPDivUnit::FPDivUnit()
{
	pipeLined = false;
}
FPDivUnit::FPDivUnit(int inputNumberOfCycles,bool pipeLined,int noOfInstr):FunctionalUnit(inputNumberOfCycles, noOfInstr)
{
		
	this->pipeLined = pipeLined;
}

	
void FPDivUnit::checkStatus(Simulator * simPtr,vector<pair<int,SourceLine>> & writeBackPriorityVector)
{
	SourceLine instr;
	//instruction not reached the FU yet
	if(FunctionalUnit::fpDivdeque.empty())
	{
		return;
	}
	else 
	{
		//condition for new instruction to be fetched
		if(!FunctionalUnit::fpDivdeque.empty() )
		{
			instructionNumber = FunctionalUnit::fpDivdeque.front().first;
			instr = FunctionalUnit::fpDivdeque.front().second;
			cyclesLeftForInstr--;
		}

		std::deque<pair<int,SourceLine>>::iterator it;
		for(it=FunctionalUnit::fpDivdeque.begin();it<FunctionalUnit::fpDivdeque.end();it++)
		{
			it->second.exStagesLeft--;
		}
		cyclesLeftForInstr = FunctionalUnit::fpDivdeque.begin()->second.exStagesLeft;
		if(cyclesLeftForInstr<0)
		{
				cyclesLeftForInstr = 0;
		}
			
	}

	if(cyclesLeftForInstr==0 && FunctionalUnit::wbdeque.empty() && !FunctionalUnit::fpDivdeque.empty())
	{
		//FunctionalUnit::fpDivdeque.pop_front();
		//FunctionalUnit::wbdeque.push_back(make_pair(instructionNumber,simPtr->instructionVector[instructionNumber-1]));
		//completionVector[instructionNumber-1]->completionTime[2] = FunctionalUnit::clockTick;
		writeBackPriorityVector.push_back(FunctionalUnit::fpDivdeque.front());
	}
}

InstrExecute::InstrExecute(IntegerExecPath intPath,FPAdderUnit adderPath,FPMultUnit multPath,FPDivUnit divPath)
{
	this->intPath = intPath;
	this->adderPath = adderPath;
	this->multPath = multPath;
	this->divPath = divPath;
}

void InstrExecute::checkStatus(Simulator * simPtr)
{
	//if register fetch busy cannot complete this stage
	intPath.checkStatus(simPtr,writeBackPriorityVector);
	adderPath.checkStatus(simPtr,writeBackPriorityVector);
	multPath.checkStatus(simPtr,writeBackPriorityVector);
	divPath.checkStatus(simPtr,writeBackPriorityVector);

	pushToWriteBackWithPriority(simPtr);
	writeBackPriorityVector.clear();
}
//function to check priority of getting more than one instruction to writeback stage
void InstrExecute::pushToWriteBackWithPriority(Simulator * simPtr)
{
	//if no instruction in contention return without doing anything
	if(writeBackPriorityVector.size()==0)
	{
		return;
	}
	if(writeBackPriorityVector.size()==1)
	{
		priorityPushToWriteBack(writeBackPriorityVector[0],simPtr);
		return;
	}
	while(writeBackPriorityVector.size()>1)
	{
		std::pair<int,SourceLine> numberLinePair1 = writeBackPriorityVector[0];
		writeBackPriorityVector.erase(writeBackPriorityVector.begin());
		std::pair<int,SourceLine> numberLinePair2 = writeBackPriorityVector[0];
		writeBackPriorityVector.erase(writeBackPriorityVector.begin());
		compareInstructionPriority(numberLinePair1,numberLinePair2,simPtr);
	}

	priorityPushToWriteBack(writeBackPriorityVector[0],simPtr);

}
	
void InstrExecute::priorityPushToWriteBack(std::pair<int,SourceLine> numberLinePair1,Simulator * simPtr)
{
	if(simPtr->symbolTable[numberLinePair1.second.opCode].fuUsed==FU_INTEGER)
	{
		FunctionalUnit::memorydeque.pop_front();
		FunctionalUnit::wbdeque.push_back(numberLinePair1);
		FunctionalUnit::completionVector[numberLinePair1.first-1]->completionTime[3] = FunctionalUnit::clockTick;

		if(!FunctionalUnit::integerALUdeque.empty())
		{
			FunctionalUnit::memorydeque.push_back(FunctionalUnit::integerALUdeque.front());
			FunctionalUnit::integerALUdeque.pop_front();
			//cannot use numberLinePair1 since this is the next instruction
			FunctionalUnit::completionVector[FunctionalUnit::memorydeque.front().first-1]->completionTime[2] = FunctionalUnit::clockTick;

			//if integer ALU was waiting there was a structural hazard remove that
			//if structural hazard before IU dont unset the structural hazard
			if(FunctionalUnit::completionVector[FunctionalUnit::memorydeque.front().first-1]->completionTime[2] - FunctionalUnit::completionVector[FunctionalUnit::memorydeque.front().first-1]->completionTime[1] == 1 && !FunctionalUnit::completionVector[FunctionalUnit::memorydeque.front().first-1]->isStructuralHazardBeforeIU)
			{
				FunctionalUnit::completionVector[FunctionalUnit::memorydeque.front().first-1]->structural = false;
			}
		}
	}
	else if(simPtr->symbolTable[numberLinePair1.second.opCode].fuUsed==FU_FP_ADDER)
	{
		FunctionalUnit::fpAdderdeque.pop_front();
		FunctionalUnit::wbdeque.push_back(numberLinePair1);
		FunctionalUnit::completionVector[numberLinePair1.first-1]->completionTime[2] = FunctionalUnit::clockTick;
	}
	else if(simPtr->symbolTable[numberLinePair1.second.opCode].fuUsed==FU_FP_MULT)
	{
		FunctionalUnit::fpMultdeque.pop_front();
		FunctionalUnit::wbdeque.push_back(numberLinePair1);
		FunctionalUnit::completionVector[numberLinePair1.first-1]->completionTime[2] = FunctionalUnit::clockTick;
	}
	else
	{
		FunctionalUnit::fpDivdeque.pop_front();
		FunctionalUnit::wbdeque.push_back(numberLinePair1);
		FunctionalUnit::completionVector[numberLinePair1.first-1]->completionTime[2] = FunctionalUnit::clockTick;
	}
}

void InstrExecute::compareInstructionPriority(std::pair<int,SourceLine> numberLinePair1,std::pair<int,SourceLine> numberLinePair2,Simulator * simPtr)
{
	bool isPipeLine1 = false;
	bool isPipeLine2 = false;
	//compare pipeline or not pipelined
	//instruction 1
	if(simPtr->symbolTable[numberLinePair1.second.opCode].fuUsed==FU_INTEGER)
	{
		//isPipeLine1 = false;
		isPipeLine1 = true;
	}
	else if(simPtr->symbolTable[numberLinePair1.second.opCode].fuUsed==FU_FP_ADDER)
	{
		isPipeLine1 = simPtr->isFPAdderPipe;
	}
	else if(simPtr->symbolTable[numberLinePair1.second.opCode].fuUsed==FU_FP_MULT)
	{
		isPipeLine1 = simPtr->isFPMultPipe;
	}
	else
	{
		isPipeLine1 = simPtr->isFPDivPipe;
	}

	//instruction2
	if(simPtr->symbolTable[numberLinePair2.second.opCode].fuUsed==FU_INTEGER)
	{
		//isPipeLine2 = false;
		isPipeLine2 = true;
	}
	else if(simPtr->symbolTable[numberLinePair2.second.opCode].fuUsed==FU_FP_ADDER)
	{
		isPipeLine2 = simPtr->isFPAdderPipe;
	}
	else if(simPtr->symbolTable[numberLinePair2.second.opCode].fuUsed==FU_FP_MULT)
	{
		isPipeLine2 = simPtr->isFPMultPipe;
	}
	else
	{
		isPipeLine2 = simPtr->isFPDivPipe;
	}

	//if instruction 1 is non pipelined and 2 is pipelined
	if(!isPipeLine1 && isPipeLine2)
	{
		
		writeBackPriorityVector.push_back(numberLinePair1);
		FunctionalUnit::completionVector[numberLinePair2.first-1]->structural = true;
		return;
	}
	//if instruction 2 is non pipelined and 1 is pipelined
	else if(isPipeLine1 && !isPipeLine2)
	{
		writeBackPriorityVector.push_back(numberLinePair2);
		FunctionalUnit::completionVector[numberLinePair1.first-1]->structural = true;
		return;
	}

	//next criteria number of EX cycles
	int numberOfEx1 = 0;
	int numberOfEx2 = 0;

	//instruction 1
	if(simPtr->symbolTable[numberLinePair1.second.opCode].fuUsed==FU_INTEGER)
	{
		numberOfEx1=simPtr->memAccessCycles;
	}
	else if(simPtr->symbolTable[numberLinePair1.second.opCode].fuUsed==FU_FP_ADDER)
	{
		numberOfEx1 = simPtr->FPAdderStages;
	}
	else if(simPtr->symbolTable[numberLinePair1.second.opCode].fuUsed==FU_FP_MULT)
	{
		numberOfEx1 = simPtr->FPMultStages;
	}
	else
	{
		numberOfEx1 = simPtr->FPDivStages;
	}

	//instruction 2
	//instruction2
	if(simPtr->symbolTable[numberLinePair2.second.opCode].fuUsed==FU_INTEGER)
	{
		numberOfEx2 = simPtr->memAccessCycles;
	}
	else if(simPtr->symbolTable[numberLinePair2.second.opCode].fuUsed==FU_FP_ADDER)
	{
		numberOfEx2 = simPtr->FPAdderStages;
	}
	else if(simPtr->symbolTable[numberLinePair2.second.opCode].fuUsed==FU_FP_MULT)
	{
		numberOfEx2 = simPtr->FPMultStages;
	}
	else
	{
		numberOfEx2 = simPtr->FPDivStages;
	}

	if(numberOfEx1<numberOfEx2)
	{
		writeBackPriorityVector.push_back(numberLinePair2);	
		FunctionalUnit::completionVector[numberLinePair1.first-1]->structural = true;
		return;
	}
	else if(numberOfEx2<numberOfEx1)
	{
		writeBackPriorityVector.push_back(numberLinePair1);
		FunctionalUnit::completionVector[numberLinePair2.first-1]->structural = true;
		return;
	}

	//3rd criteria which instruction came first
	if(numberLinePair1.first>numberLinePair2.first)
	{
		writeBackPriorityVector.push_back(numberLinePair2);
		FunctionalUnit::completionVector[numberLinePair1.first-1]->structural = true;
	}
	else
	{
		writeBackPriorityVector.push_back(numberLinePair1);
		FunctionalUnit::completionVector[numberLinePair2.first-2]->structural = true;
	}
}

WriteBackUnit::WriteBackUnit(int inputNumberOfCycles,int noOfInstr):FunctionalUnit(inputNumberOfCycles, noOfInstr)
{

}

void WriteBackUnit::checkStatus(Simulator * simPtr)
{
	SourceLine instr;
	//instruction not reached the FU yet
	if(FunctionalUnit::wbdeque.empty())
	{
		return;
	}
	else
	{
		//condition for new instruction to be fetched
		if(cyclesLeftForInstr==numberOfCycles && !FunctionalUnit::wbdeque.empty() )
		{
			instructionNumber = FunctionalUnit::wbdeque.front().first;
			instr = FunctionalUnit::wbdeque.front().second;
			busy = true;
			cyclesLeftForInstr--;
		}
		//current instruction cycles still left
		else if(cyclesLeftForInstr>0)
		{
			cyclesLeftForInstr--;
		}
			
	}
	//if instruction cycles over there is not next pipeline stage
	if(cyclesLeftForInstr==0 && !FunctionalUnit::wbdeque.empty())
	{
			
		completionVector[instructionNumber-1]->completionTime[4] = FunctionalUnit::clockTick;
		cout<<FunctionalUnit::wbdeque.front().second.opCode<<"\t";
		FunctionalUnit::wbdeque.pop_front();
		for(int i=0;i<5;i++)
		{
			cout<<completionVector[instructionNumber-1]->completionTime[i]<<"\t";
		}
		cout<<endl;
		cyclesLeftForInstr = numberOfCycles;
		//release lock if any
		releaseLock(simPtr,instr);
	}
}

//function to release locks
void releaseLock(Simulator * simPtr,SourceLine srcLine)
{
	//if instruction is store there is no locking of registers
	if(strcmpi((srcLine.opCode).c_str(),"sw")==0 ||strcmpi((srcLine.opCode).c_str(),"s.d")==0)
	{
		return;
	}

	string destFormat  = simPtr->symbolTable[srcLine.opCode].destSrcFormat;
	string destStr="";
	for(int i=0;i<3;i++)
	{
		//dest_src_src - 012
		//load R1,2(R2)
		//src 2
		//src R2
		if(destFormat[i]-'0' == 0)
		{
			destStr =  srcLine.operands[destFormat[i]-'0'];

			//floating point register
			if(destStr.find("f") != string::npos || destStr.find("F") != string::npos)
			{
				destStr.erase(0,1);
				simPtr->fpRegistersLocked[atoi(destStr.c_str())] =false;
				
			}

			//integer register
			if(destStr.find("R") != string::npos || destStr.find("r") != string::npos)
			{
				//write back actual value to register 
				destStr.erase(0,1);
				simPtr->registerMap["r"+destStr] = srcLine.operandValues[0];
				simPtr->intRegistersLocked[atoi(destStr.c_str())] = false;
			}
		}
	}



}
//function checks RAW hazard
//source registers are checked if they are locked
//if RAW hazard return true
bool isRAWHazard(Simulator * simPtr,SourceLine srcLine)
{
	string destFormat  = simPtr->symbolTable[srcLine.opCode].destSrcFormat;
	string source1="";
	string source2="";
	for(int i=0;i<3;i++)
	{
		//dest_src_src - 012
		//load R1,2(R2)
		//src 2
		//src R2
		//or if instruction is store
		if(destFormat[i]-'0' == 2 || destFormat[i]-'0' == 1 || strcmpi((srcLine.opCode).c_str(),"sw")==0 || strcmpi((srcLine.opCode).c_str(),"s.d")==0)
		{
			source1 = srcLine.operands[destFormat[i]-'0'];

			//floating point register
			if(source1.find("f") != string::npos || source1.find("F") != string::npos)
			{
				source1.erase(0,1);
				if(simPtr->fpRegistersLocked[atoi(source1.c_str())])
				{
					return true;
				}
			}

			//integer register
			if(source1.find("R") != string::npos || source1.find("r") != string::npos)
			{
				source1.erase(0,1);
				if(simPtr->intRegistersLocked[atoi(source1.c_str())])
				{
					return true;
				}
				
			}
			
			//floating point register
			if(source2.find("f") != string::npos || source2.find("F") != string::npos)
			{
				source2.erase(0,1);
				if(simPtr->fpRegistersLocked[atoi(source1.c_str())])
				{
					return true;
				}
				
			}

			//integer register
			if(source2.find("R") != string::npos || source2.find("r") != string::npos)
			{
				source2.erase(0,1);
				if(simPtr->intRegistersLocked[atoi(source1.c_str())])
				{
					return true;
				}
				
			}
		}
	}
	return false;
}

//FUNTION TO SET LOCK TO DESTINATION REGISTER
//THE FUNCTION ALSO CHECKS WHETHER THERE IS WAW HAZARD - IF YES IT RETURNS FALSE
bool setLock(Simulator * simPtr,SourceLine srcLine)
{

	//if instruction is store no need to lock the registers
	if(strcmpi((srcLine.opCode).c_str(),"sw")==0 ||strcmpi((srcLine.opCode).c_str(),"s.d")==0)
	{
		return true;
	}
	string destFormat  = simPtr->symbolTable[srcLine.opCode].destSrcFormat;
	string destStr="";
	for(int i=0;i<3;i++)
	{
		//dest_src_src - 012
		//load R1,2(R2)
		//src 2
		//src R2
		if(destFormat[i]-'0' == 0)
		{
			destStr =  srcLine.operands[destFormat[i]-'0'];

			//floating point register
			if(destStr.find("f") != string::npos || destStr.find("F") != string::npos)
			{
				destStr.erase(0,1);
				//if already locked you need to wait until it is unlocked
				if(simPtr->fpRegistersLocked[atoi(destStr.c_str())])
				{
					return false;	
				}
				else
				{
					simPtr->fpRegistersLocked[atoi(destStr.c_str())] = true;
					return true;
				}
			}

			//integer register
			if(destStr.find("R") != string::npos || destStr.find("r") != string::npos)
			{
				destStr.erase(0,1);
				if(simPtr->intRegistersLocked[atoi(destStr.c_str())])
				{
					return false;	
				}
				else
				{
					simPtr->intRegistersLocked[atoi(destStr.c_str())] = true;
					return true;
				}
			}
		}
	}

	return true;
}

