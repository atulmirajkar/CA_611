#ifndef MIPS_MEMORY_H
#define	MIPS_MEMORY_H

//#include"FunctoinalUnits.h"
#include"simulator.h"

class cacheBlock{
public:
	bool valid;
	int tag;
	cacheBlock();

};


class InstructionCache
{
private:
	cacheBlock blocks[4];
public:
	InstructionCache();
	//function to check if instruction present in cache
	//if not instruction is brought into cache(just a simulation)
	bool isInstructionCacheHit(int instructionNumber);

	//check if block in cache valid
	bool isBlockValid(int blockNumber);

	//check if tag of block correct
	bool isTagCorrect(int blockNumber,int tagNumber);
};


class DataCacheBlock{

public:
	bool isValid;
	bool isDirty;
	int tag;

	DataCacheBlock();
};

class DataCache{
private:
	DataCacheBlock cacheSet[2][2];
	int LRU[2];
	//DataCacheBlock setOne[2];
public:

	int getLwLatency(int address1,Simulator * simPtr);
	int getSwLatency(int address1,Simulator * simPtr);
};



#endif