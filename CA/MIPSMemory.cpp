#include"MIPSMemory.h"


cacheBlock::cacheBlock()
{
	valid = false;
	tag=-1;
}

InstructionCache::InstructionCache()
{

}
//function to check if instruction present in cache
	//if not instruction is brought into cache(just a simulation)
bool InstructionCache::isInstructionCacheHit(int instructionNumber)
{
	//1. subtract 1 from instruction number
	instructionNumber = instructionNumber -1;

	//2. convert instruction number to byte address
	int byteAddress = instructionNumber * 4;

	//3. get blocknumber in cache
	// and with 110000
	//right shift by 4
	int blockNumber = (byteAddress & 48)>>4;

	//4. get tag number 
	// and with 11000000
	//right shift by 6
	int tagNumber = (byteAddress & 192)>>6;

	if(!isBlockValid(blockNumber))
	{
		//if block is invalid
		//set the block number to valid
		//set the correct tag ie we broght that block from memory
		blocks[blockNumber].valid = true;
		blocks[blockNumber].tag = tagNumber;

		//hot to check contention for main memory with data cache
		return false;
	}

	//if valid bit set for that block number check if tag correct
	if(!isTagCorrect(blockNumber,tagNumber))
	{
		//if tag not correct it is a cache miss
		//set the correct tag number ie. we are replacing that block in cache
		blocks[blockNumber].tag = tagNumber;


		//how to check contention for main memory with data cache
		return false;
	}
		
	//block valid, as well as tag matched
	//it is a cach hit
	
	return true;
}

//check if block in cache valid
bool InstructionCache::isBlockValid(int blockNumber)
{
	if(blocks[blockNumber].valid)
	{
		return true;
	}
	return false;
}

bool InstructionCache::isTagCorrect(int blockNumber,int tagNumber)
{
	if(blocks[blockNumber].tag==tagNumber)
	{
		return true;
	}
	return false;
}

DataCacheBlock::DataCacheBlock()
{
	isDirty = false;
	isValid = false;
	tag = -1;
}

int DataCache::getLwLatency(int address1,Simulator *simPtr)
{

	bool isBlockPresent = false;
	int cacheSetNumber = -1;
	//1. subtract 256 from address
	address1 = address1 - 256;

	//2. get block address
	int blockNumber  = (address1 & 16)>> 4;

	//3. get Tag number
	int tagNumber = (address1 & 96) >> 5;

	//first check if this block tag combination present
	int i = 0;
	for(i=0;i<2;i++)
	{
		if(cacheSet[i][blockNumber].isValid && cacheSet[i][blockNumber].tag == tagNumber)
		{
			isBlockPresent = true;
			break;
		}

	}
	//cache hit 
	//if block present we get a cache set
	if(isBlockPresent)
	{
		cacheSetNumber = i;
		LRU[blockNumber] = cacheSetNumber;
		//no need to set dirty bit because we are not writing to cache
		//no need to set valid bit because it is already valid
		//return only number of iCacheAccessCycles
		return simPtr->dCacheAccessCycles;
	}
	//cache miss
	else
	{
		//we need to find a cache set which is not valid 
		for(i=0;i<2;i++)
		{
			if(!cacheSet[i][blockNumber].isValid)
			{
				isBlockPresent = true;
				break;
			}
		}

		if(isBlockPresent)
		{
			//received a block that can be over written
			//we need to bring data from memory
			cacheSetNumber = i;
			LRU[blockNumber] = cacheSetNumber;
			//no need to set dirty bit because we cache is still in consistent state
			//set valid bit because it may be invalid
			cacheSet[cacheSetNumber][blockNumber].isValid = true;
			cacheSet[cacheSetNumber][blockNumber].tag = tagNumber;
			return 2 * (simPtr->dCacheAccessCycles + simPtr->memAccessCycles);
		}
		else
		{
			//we need to use the least recently used approach to get a replacement block in cache
			cacheSetNumber = LRU[blockNumber] ^ 1;
			//we have got not least recently used cacheSetNumber for a given block number
			//we need to check if dirty bit is set for this set block number
			LRU[blockNumber] = cacheSetNumber;
			if(cacheSet[cacheSetNumber][blockNumber].isDirty)
			{
				//write to memory 2*(T+K)
				//then write to cache
				cacheSet[cacheSetNumber][blockNumber].isValid = true;
				//unset the dirty bit
				cacheSet[cacheSetNumber][blockNumber].isDirty = false;
				cacheSet[cacheSetNumber][blockNumber].tag = tagNumber;
				return 2*2*(simPtr->dCacheAccessCycles + simPtr->memAccessCycles);

			}
			else
			{
				//then write to cache
				cacheSet[cacheSetNumber][blockNumber].isValid = true;
				cacheSet[cacheSetNumber][blockNumber].tag = tagNumber;
				return 2*(simPtr->dCacheAccessCycles + simPtr->memAccessCycles);
			}
		}
	}

}

int DataCache::getSwLatency(int address1,Simulator * simPtr)
{
	bool isBlockPresent = false;
	int cacheSetNumber  = -1;

	//1. subtract 256 from address
	address1 = address1 - 256;

	//2. get block address
	int blockNumber  = (address1 & 16)>> 4;

	//3. get Tag number
	int tagNumber = (address1 & 96) >> 5;

	//first check if this block tag combination present
	int i = 0;
	for(i=0;i<2;i++)
	{
		//if(cacheSet[i][blockNumber].isValid && cacheSet[i][blockNumber].tag == tagNumber && !cacheSet[i][blockNumber].isDirty)
		if(cacheSet[i][blockNumber].isValid && cacheSet[i][blockNumber].tag == tagNumber)
		{
			isBlockPresent = true;
			break;
		}

	}

	//cache hit
	//if block present we get a cache set
	if(isBlockPresent)
	{
		cacheSetNumber = i;
		LRU[blockNumber] = cacheSetNumber;
		//set dirty bit because we are writing to cache
		cacheSet[cacheSetNumber][blockNumber].isDirty = true;
		//no need to set valid bit because it is already valid
		//return only number of iCacheAccessCycles
		return simPtr->dCacheAccessCycles;
	}
	//cache miss
	else
	{
		//we need to find a cache set which is not valid 
		for(i=0;i<2;i++)
		{
			if(!cacheSet[i][blockNumber].isValid)
			{
				isBlockPresent = true;
				break;
			}
		}

		if(isBlockPresent)
		{
			//received a block that can be over written
			//we just need to write data to cache
			cacheSetNumber = i;
			LRU[blockNumber] = cacheSetNumber;
			//set dirty bit because implementing write back cache
			cacheSet[cacheSetNumber][blockNumber].isDirty= true;
			//set valid bit because it may be invalid
			cacheSet[cacheSetNumber][blockNumber].isValid = true;
			cacheSet[cacheSetNumber][blockNumber].tag = tagNumber;
			return 2*(simPtr->dCacheAccessCycles + simPtr->memAccessCycles);
		}
		else
		{
			//we need to use the least recently used approach to get a replacement block in cache
			cacheSetNumber = LRU[blockNumber] ^ 1;
			//we have got not least recently used cacheSetNumber for a given block number
			//we need to check if dirty bit is set for this set block number
			LRU[blockNumber] = cacheSetNumber;
			if(cacheSet[cacheSetNumber][blockNumber].isDirty)
			{
				//write to memory 2*(T+K)
				//then write to cache
				cacheSet[cacheSetNumber][blockNumber].isValid = true;
				//set the dirty bit
				cacheSet[cacheSetNumber][blockNumber].isDirty = true;
				cacheSet[cacheSetNumber][blockNumber].tag = tagNumber;
				return  2*2*(simPtr->dCacheAccessCycles + simPtr->memAccessCycles);

			}
			else
			{
				//then bring from memory and write to cache 
				cacheSet[cacheSetNumber][blockNumber].isValid = true;
				//set the dirty bit
				cacheSet[cacheSetNumber][blockNumber].isDirty = true;
				cacheSet[cacheSetNumber][blockNumber].tag = tagNumber;
				return 2*(simPtr->dCacheAccessCycles + simPtr->memAccessCycles);
			}
		}

	}
}
