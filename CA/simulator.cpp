#include"simulator.h"
#include"FunctoinalUnits.h"



void tokenize(string str, string delimiter, vector<string> & instance);
void removeWhiteSpace(string & inputString);
string trim(string &s);
void tokenizeFirstOcc(string str, string delimiter, vector<string> & instance);
int getIntFromBinaryString(string line);
void main(int args, char * argv[])
{
	//argv[1] - inst.txt
	//argv[2] - data.txt
	//argv[3] - reg.txt
	//argv[4] - config.txt
	//argv[5] - result.txt
	try{
		Simulator sim;
		sim.readConfig(argv[4]);
		sim.readInstructions(argv[1]);
		sim.readRegisters(argv[3]);
		sim.readData(argv[2]);

		sim.simulate(argv[5]);
	}
	catch(const char * exceptStr)
	{
		cout<<exceptStr<<endl;
		ofstream outputFile;
		outputFile.open(argv[5]);
		outputFile<<exceptStr<<endl;
	}
	catch(string exceptStr)
	{
		cout<<exceptStr<<endl;
		ofstream outputFile;
		outputFile.open(argv[5]);
		outputFile<<exceptStr<<endl;
	}
}

Simulator::Simulator()
{
	for(int i=0;i<32;i++)
	{
		fpRegistersLocked[i]=false;
		intRegistersLocked[i] = false;
	}
}

void Simulator::simulate(const char * outputFile)
{
	//initialize functional units
	InstrFetch IFUnit(1,this->instructionVector.size());
	InstrDecode IDUnit(1,this->instructionVector.size());
	IntegerAlu instrUnit(1,this->instructionVector.size());
	Memory memUnit(this->memAccessCycles,this->instructionVector.size());
	//initialize icache and dcache also

	IntegerExecPath intExecPath(instrUnit,memUnit);
	FPAdderUnit fpAdderPath(this->FPAdderStages,this->isFPAdderPipe,this->instructionVector.size());
	FPMultUnit fpMultPath(this->FPMultStages,this->isFPMultPipe,this->instructionVector.size());
	FPDivUnit fpDivPath(this->FPDivStages,this->isFPDivPipe,this->instructionVector.size());

	InstrExecute exUnit(intExecPath,fpAdderPath,fpMultPath,fpDivPath);
	WriteBackUnit wbUnit(1,this->instructionVector.size());

	//for every clock tick
	//check status of each functional unit

	//CHANGE THIS CONDITION LATER
	
	while(true)
	{
 		wbUnit.checkStatus(this);
		exUnit.checkStatus(this);
		IDUnit.checkStatus(this);
		IFUnit.checkStatus(this);	
		
		FunctionalUnit::clockTick++;
		cout<<FunctionalUnit::clockTick<<endl;
		if(FunctionalUnit::areAllPipeLinesEmpty())
		{
			break;
		}
	}

	FunctionalUnit::displayCompletionTime(outputFile);


}

void Simulator::readData(char * dataFilePath)
{
	ifstream ifs;
	ifs.open(dataFilePath);
	if(ifs.is_open())
	{
		int dataLineNumber = 0;
		string line;
		while(getline(ifs,line))
		{
			//trim and remove white space
			line = trim(line);
			removeWhiteSpace(line);
			this->data[dataLineNumber] = getIntFromBinaryString(line);
			dataLineNumber++;
		}
	}
}
void Simulator::readConfig(char * configFilePath)
{
	int lineNumber = 0;
	string configFilePathStr(configFilePath);
	//instruction map stores values for number of cycles in ex stage(ex + mem)
	this->symbolTable["HLT"]=SymbolTableAttr(OPER,0,REGEX_EMPTY,EMPTY_FORMAT,"",false);
	this->symbolTable["J"]=SymbolTableAttr(OPER,0,REGEX_W,LABEL_FORMAT,"",false);
	this->symbolTable["BEQ"]=SymbolTableAttr(OPER,0,REGEX_R_R_W,SRC_SRC_DEST,"",false);
	this->symbolTable["BNE"]=SymbolTableAttr(OPER,0,REGEX_R_R_W,SRC_SRC_DEST,"",false);
	this->symbolTable["DADD"]=SymbolTableAttr(OPER,2,REGEX_R_R_R,DEST_SRC_SRC,FU_INTEGER,false);
	this->symbolTable["DADDI"]=SymbolTableAttr(OPER,2,REGEX_R_R_N,DEST_SRC_SRC,FU_INTEGER,false);
	this->symbolTable["DSUB"]=SymbolTableAttr(OPER,2,REGEX_R_R_R,DEST_SRC_SRC,FU_INTEGER,false);
	this->symbolTable["DSUBI"]=SymbolTableAttr(OPER,2,REGEX_R_R_N,DEST_SRC_SRC,FU_INTEGER,false);
	this->symbolTable["AND"]=SymbolTableAttr(OPER,2,REGEX_R_R_R,DEST_SRC_SRC,FU_INTEGER,false);
	this->symbolTable["ANDI"]=SymbolTableAttr(OPER,2,REGEX_R_R_R,DEST_SRC_SRC,FU_INTEGER,false);
	this->symbolTable["OR"]=SymbolTableAttr(OPER,2,REGEX_R_R_R,DEST_SRC_SRC,FU_INTEGER,false);
	this->symbolTable["ORI"]=SymbolTableAttr(OPER,0,REGEX_R_R_R,DEST_SRC_SRC,FU_INTEGER,false);

	//read config file
	ifstream ifs;
	ifs.open(configFilePathStr);
	if(ifs.is_open())
	{
		string line;
		vector<string> tokens;
		vector<string> tokens2;
		while(getline(ifs,line))
		{
			lineNumber++;
			tokenize(line,":",tokens);
			tokenize(tokens[1],",",tokens2);

			//remove whitespace
			for(int i=0;i<tokens2.size();i++)
			{
				removeWhiteSpace(tokens2[i]);
			}
			

			if(tokens[0].compare("FP adder")==0)
			{
				this->FPAdderStages = atoi(tokens2[0].c_str());
				if(this->FPAdderStages<=0)
				{
					string errorString = "Error: Invalid config file: invalid cycle number [Line ";
					errorString.append(std::to_string((long double)lineNumber));
					errorString.append("]");
					throw errorString;
				}
				Utility::toUpper(tokens2[1]);
				this->isFPAdderPipe = tokens2[1].compare("YES")==0?true:false;
			}
			else if(tokens[0].compare("FP Multiplier")==0)
			{
				this->FPMultStages = atoi(tokens2[0].c_str());
				if(this->FPMultStages<=0)
				{
					string errorString = "Error: Invalid config file: invalid cycle number [Line ";
					errorString.append(std::to_string((long double)lineNumber));
					errorString.append("]");
					throw errorString;
				}
				Utility::toUpper(tokens2[1]);
				this->isFPMultPipe = tokens2[1].compare("YES")==0?true:false;
			}
			else if(tokens[0].compare("FP divider")==0)
			{
				this->FPDivStages = atoi(tokens2[0].c_str());
				if(this->FPDivStages<=0)
				{
					string errorString = "Error: Invalid config file: invalid cycle number [Line ";
					errorString.append(std::to_string((long double)lineNumber));
					errorString.append("]");
					throw errorString;
				}
				Utility::toUpper(tokens2[1]);
				this->isFPDivPipe = tokens2[1].compare("YES")==0?true:false;
			}
			else if(tokens[0].compare("Main memory")==0)
			{
				this->memAccessCycles = atoi(tokens2[0].c_str());
				if(this->memAccessCycles<=0)
				{
					string errorString = "Error: Invalid config file: invalid cycle number [Line ";
					errorString.append(std::to_string((long double)lineNumber));
					errorString.append("]");
					throw errorString;
				}
			}
			else if(tokens[0].compare("I-Cache")==0)
			{
				this->iCacheAccessCycles = atoi(tokens2[0].c_str());
				if(this->iCacheAccessCycles<=0)
				{
					string errorString = "Error: Invalid config file: invalid cycle number [Line ";
					errorString.append(std::to_string((long double)lineNumber));
					errorString.append("]");
					throw errorString;
				}
			}
			else if(tokens[0].compare("D-Cache")==0)
			{
				this->dCacheAccessCycles = atoi(tokens2[0].c_str());
				if(this->dCacheAccessCycles<=0)
				{
					string errorString = "Error: Invalid config file: invalid cycle number [Line ";
					errorString.append(std::to_string((long double)lineNumber));
					errorString.append("]");
					throw errorString;
				}
			}

			tokens.clear();
			tokens2.clear();

		}//while for file
		ifs.close();

			
		//rest of instructions depending on config file
		this->symbolTable["LW"]= SymbolTableAttr(OPER,1 + this->dCacheAccessCycles,REGEX_R_N_R,DEST_SRC_SRC,FU_INTEGER,true);
		this->symbolTable["SW"]= SymbolTableAttr(OPER,1 + this->dCacheAccessCycles,REGEX_R_N_R,DEST_SRC_SRC,FU_INTEGER,true);

		//load double should require 2 dcacheCycles???????
		this->symbolTable["L.D"]= SymbolTableAttr(OPER,1 + 2* this->dCacheAccessCycles,REGEX_F_N_R,DEST_SRC_SRC,FU_INTEGER,true);
		this->symbolTable["S.D"]= SymbolTableAttr(OPER,1 + 2* this->dCacheAccessCycles,REGEX_F_N_R,DEST_SRC_SRC,FU_INTEGER,true);

		this->symbolTable["ADD.D"]= SymbolTableAttr(OPER,this->FPAdderStages,REGEX_F_F_F,DEST_SRC_SRC,FU_FP_ADDER,false);
		this->symbolTable["SUB.D"]= SymbolTableAttr(OPER,this->FPAdderStages,REGEX_F_F_F,DEST_SRC_SRC,FU_FP_ADDER,false);
		this->symbolTable["MUL.D"]= SymbolTableAttr(OPER,this->FPMultStages,REGEX_F_F_F,DEST_SRC_SRC,FU_FP_MULT,false);
		this->symbolTable["DIV.D"]= SymbolTableAttr(OPER,this->FPDivStages,REGEX_F_F_F,DEST_SRC_SRC,FU_FP_DIV,false);

	}
}

void Simulator::readInstructions(char * instructionFilePath)
{
	int instructionNumber = 0;
	ifstream ifs;
	ifs.open(instructionFilePath);
	int lineNumber = 0;
	if(ifs.is_open())
	{
		string line= "";
		while(getline(ifs,line))
		{
			lineNumber++;
			vector<string> tokenVector;
			//look for labels
			tokenize(line,":",tokenVector);

			//add label to label instruction Map
			if(tokenVector.size()==2)
			{
				labelInstMap[tokenVector[0]] = instructionVector.size();
				line = tokenVector[1];
			}
			tokenVector.clear();
			//parse the instruction
			
			SourceLine * instr = new SourceLine();
			try{
				parseInstruction(instr,line);
			}
			catch(char * exceptStr)
			{
				string errorString(exceptStr);
				errorString.append(" [Line ");
				errorString.append(std::to_string((long double)lineNumber));
				errorString.append("]");
				throw errorString;
			}
			//add actual line as well
			instr->actualSourceLine = line;
			this->instructionVector.push_back(*instr);

		}//while lines in file
	}//if file opened successfully
}
//parse a single line of instruction
void Simulator::parseInstruction(SourceLine * instr, string instrLine)
{
	vector<string> tokenVector;
	instrLine = trim(instrLine);
	tokenizeFirstOcc(instrLine," ",tokenVector);
	if(tokenVector.size()<1)
	{
		throw "Instruction File Parsing Error";
	}
	//convert to upper case
	Utility::toUpper(tokenVector[0]);
	instr->opCode = tokenVector[0];
	//if J or HLT return
	if(tokenVector.size()==1)
	{
		return;
	}

	//trim and remove whitespace from operand
	tokenVector[1] = trim(tokenVector[1]);
	removeWhiteSpace(tokenVector[1]);

	//apply regular expression
	if(this->symbolTable.find(instr->opCode)==symbolTable.end())
	{
		throw "Instruction File Parsing Error";
	}

	regex e(this->symbolTable[instr->opCode].regex,regex_constants::icase);
	smatch matches;
	if(!regex_match(tokenVector[1],matches,e))
	{
		throw "Instruction File Parsing Error";
	}

	for(int i=0;i<matches.size()-1;i++)
	{
		if(symbolTable[instr->opCode].destSrcFormat[i]!=9)
		{
			instr->operands[symbolTable[instr->opCode].destSrcFormat[i]-'0'] = matches[i+1].str();
			cout<<matches[i+1].str()<<endl;
		}
	}

}

void Simulator::readRegisters(char* registerFilePath)
{
	ifstream ifs;
	ifs.open(registerFilePath);
	if(ifs.is_open())
	{
		string line="";
		int registerCount = 0;
		string reg = "r";
		char buffer[33];
		while(getline(ifs,line))
		{
			//trim and remove whitespaces
			line = trim(line);
			removeWhiteSpace(line);
			itoa(registerCount,buffer,10);
			registerMap[reg.append(buffer)] = getIntFromBinaryString(line);
			reg = "r";
			registerCount++;
		}
	}
	
}

SymbolTableAttr::SymbolTableAttr()
{
	this->symbolType = -1;
	this->numberOfExCycles = -1;
}
SymbolTableAttr::SymbolTableAttr(int symbolType,int numberOfCycles,const char *regex, const char * destSrcFormat, const char * fuToBeUsed,bool memoryRequired)
{
		this->symbolType = symbolType;
		this->numberOfExCycles = numberOfCycles;
		this->regex=regex;
		this->destSrcFormat = destSrcFormat;
		this->fuUsed = fuToBeUsed;
		this->memoryRequired = memoryRequired;
}

SourceLine::SourceLine()
{
	this->exStagesLeft=0;
	this->opCode="";
	for(int i=0;i<3;i++)
	{
		operands[i]="";
	}
	for(int i=0;i<3;i++)
	{
		operandValues[i] = 0;
	}
}

//remove whitespace from string
void removeWhiteSpace(string & inputString)
{
	std::string::iterator iter = std::remove(inputString.begin(),inputString.end(),' ');
	inputString.erase(iter,inputString.end());
}
//tokenize input string
void tokenize(string str, string delimiter, vector<string> & instance)
{
	std::size_t pos=0;
	while((pos = str.find(delimiter.c_str())) != string::npos)
	{
		instance.push_back(str.substr(0,pos));
		str.erase(0,pos+1);	//1 for delimiter length
	}
	instance.push_back(str);
}

string trim(string &s)
{
	size_t posBegin = s.find_first_not_of(" \t");
	size_t posEnd = s.find_last_not_of(" \t\n");
	size_t range = posEnd - posBegin +1;
	return s.substr(posBegin,range);
}
//tokenize on first occurrence of delimter
void tokenizeFirstOcc(string str, string delimiter, vector<string> & instance)
{
	std::size_t pos=0;
	if((pos = str.find(delimiter.c_str())) != string::npos)
	{
		instance.push_back(str.substr(0,pos));
		str.erase(0,pos+1);	//1 for delimiter length
	}
	instance.push_back(str);
}
//convert 32 bit binary string to int
int getIntFromBinaryString(string line)
{
	int result =0;
	for(int i = 31;i>=0;i--)
	{
		result = result + (int)(pow((float)2,31-i)) * (line.at(i)-'0');	
	}
	return result;
}