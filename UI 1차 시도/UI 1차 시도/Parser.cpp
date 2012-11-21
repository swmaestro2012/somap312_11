#include "stdafx.h"
#include"Parser.h"


void Parser::LoadFile(char Dir[])
{
	char cmd[MAX_PATH];

	TCHAR lpFileName[MAX_PATH+1];
	GetModuleFileName(NULL, lpFileName, MAX_PATH);
	int i;
	for(i=strlen(lpFileName)-1;i>0;--i)
	{
		if(lpFileName[i]=='\\')
			break;
	}
	if(lpFileName[i]=='\\')
		lpFileName[i]=0;

	sprintf(cmd,"\"%s\\loger.exe\" --enable=all --template=vs --inconclusive \"%s\"",lpFileName,Dir);
//*
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory( &si, sizeof(si) );
	si.cb = sizeof(si);
	ZeroMemory( &pi, sizeof(pi) );
	CreateProcess(NULL,cmd,NULL,NULL, FALSE, 0, NULL, NULL, &si, &pi);
	WaitForSingleObject(pi.hProcess,INFINITE);
//*/
//	WinExec(cmd,SW_SHOW);
//	system(cmd);


	ReadState readState=rNon;

	std::string tableName;
	std::string rowName;
	std::string columnName;
	std::list<std::string>::iterator it;

	char str[MAX_PATH];
	FILE* fp=fopen("log.txt","r");
	while(fscanf(fp,"%[^\n]\n",str)!=-1)
	{
		if(str[0]=='=')
		{
			if(!strcmp(str,"=====ClassVarialbeFunctionsTable====="))
				readState=rClassVarFunc;
			else if(!strcmp(str,"=====ClassFunctionFunctionsTable====="))
				readState=rClassFuncFunc;
			else if(!strcmp(str,"=====FunctionFunctionTable====="))
				readState=rFuncFunc;
			else if(!strcmp(str,"=====VariableFunctionTable====="))
				readState=rVarFunc;
			else
				readState=rNon;
		}
		else
		{
			switch(str[0])
			{
			case 't':
				tableName=std::string(&str[2]);
				switch(readState)
				{
				case rClassVarFunc:
					for(it=classList.begin();it!=classList.end();++it)
						if(it->find(tableName)!=std::string::npos)
							break;
					if(it==classList.end())
						classList.push_back(tableName);
					break;
				case rVarFunc:
					for(it=fileList.begin();it!=fileList.end();++it)
						if(it->find(tableName)!=std::string::npos)
							break;
					if(it==fileList.end())
						fileList.push_back(tableName);
					break;
				case rClassFuncFunc:
					for(it=classList.begin();it!=classList.end();++it)
						if(it->find(tableName)!=std::string::npos)
							break;
					if(it==classList.end())
						classList.push_back(tableName);
					break;
				}
				break;

			case 'r':
				rowName=std::string(&str[2]);
				switch(readState)
				{

				case rClassVarFunc:
					for(it=masterList[tableName].first.begin();it!=masterList[tableName].first.end();++it)
						if(it->find(rowName)!=std::string::npos)
							break;
					if(it==masterList[tableName].first.end())
						masterList[tableName].first.push_back(rowName);
					break;

				case rVarFunc:
					for(it=masterList[tableName].first.begin();it!=masterList[tableName].first.end();++it)
						if(it->find(rowName)!=std::string::npos)
							break;
					if(it==masterList[tableName].first.end())
						masterList[tableName].first.push_back(rowName);
					break;

				case rClassFuncFunc:
					for(it=masterList[tableName].second.begin();it!=masterList[tableName].second.end();++it)
						if(it->find(rowName)!=std::string::npos)
							break;
					if(it==masterList[tableName].second.end())
						masterList[tableName].second.push_back(rowName);
					break;

				case rFuncFunc:
					for(it=masterList[tableName].second.begin();it!=masterList[tableName].second.end();++it)
						if(it->find(rowName)!=std::string::npos)
							break;
					if(it==masterList[tableName].second.end())
						masterList[tableName].second.push_back(rowName);
					break;
				}
				break;
			case 'c':
				columnName=std::string(&str[2]);
				switch(readState)
				{
				case rNon:
					break;

				case rClassVarFunc:
					masterTable[tableName].first[rowName][columnName]=1;
					for(it=masterList[tableName].second.begin();it!=masterList[tableName].second.end();++it)
						if(it->find(rowName)!=std::string::npos)
							break;
					if(it==masterList[tableName].second.end())
						masterList[tableName].second.push_back(columnName);
					break;

				case rVarFunc:
					masterTable[tableName].first[rowName][columnName]=1;
					for(it=masterList[tableName].second.begin();it!=masterList[tableName].second.end();++it)
						if(it->find(rowName)!=std::string::npos)
							break;
					if(it==masterList[tableName].second.end())
						masterList[tableName].second.push_back(columnName);
					break;

				case rClassFuncFunc:
					masterTable[tableName].second[rowName][columnName]=1;
					for(it=masterList[tableName].second.begin();it!=masterList[tableName].second.end();++it)
						if(it->find(rowName)!=std::string::npos)
							break;
					if(it==masterList[tableName].second.end())
						masterList[tableName].second.push_back(columnName);
					break;

				case rFuncFunc:
					masterTable[tableName].second[rowName][columnName]=1;
					for(it=masterList[tableName].second.begin();it!=masterList[tableName].second.end();++it)
						if(it->find(rowName)!=std::string::npos)
							break;
					if(it==masterList[tableName].second.end())
						masterList[tableName].second.push_back(columnName);
					break;
				}
				break;
			}
		}
	}
	fclose(fp);
}