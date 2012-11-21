#pragma once
#include<map>
#include<string>
#include<list>


class Parser
{
public:

	typedef std::map<std::string,		//row Name
		std::map<std::string,			//column Name
			int							//unused=0, r=1,w=2,m=3
		> 
	> Table;

	typedef std::map<std::string,		//Class Name or File Name
		std::pair<
			Table,						//Variable
			Table						//Function
		>
	>	MasterTable;

	typedef std::map<std::string,	//Class Name or File Name
		std::pair<
			std::list<std::string>,	//Variable List
			std::list<std::string>	//Function List
		>
	>	MasterList;


	MasterTable masterTable;
	MasterList masterList;

	std::list<std::string> fileList;
	std::list<std::string> classList;

	Parser()
	{
	}

	enum ReadState
	{
		rNon,
		rFuncFunc,
		rVarFunc,
		rClassFuncFunc,
		rClassVarFunc
	};

	void LoadFile(char Dir[]);
/*
	const std::list<std::string> fileList()
	{
		return _fileList;
	}

	const std::list<std::string> classList()
	{
		return _classList;
	}
//*/
};