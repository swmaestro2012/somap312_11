﻿/*
 */


//---------------------------------------------------------------------------
#include "makefunctionfunctiontable.h"
#include "tokenize.h"
#include "token.h"
#include <cctype>
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
// FUNCTION USAGE - Check for unused functions etc
//---------------------------------------------------------------------------

void MakeFunctionFunctionTable::parseTokens(const Tokenizer &tokenizer)
{
	symbolDatabase=tokenizer.getSymbolDatabase();

	// Function declarations..
    for (std::list<Scope>::const_iterator scope = symbolDatabase->scopeList.begin(); scope != symbolDatabase->scopeList.end(); ++scope) {
 
		if (scope->type != Scope::eClass)
			continue;	

	    for (const Token *tok = scope->classDef->next(); tok && tok != scope->classEnd; tok = tok->next()) {

			const Token *funcname = 0;

			if (Token::Match(tok, "%type% %var% ("))
				funcname = tok->next();
			else if (Token::Match(tok, "%type% *|& %var% ("))
				funcname = tok->tokAt(2);

			// Don't assume throw as a function name: void foo() throw () {}
			if (Token::Match(tok->previous(), ")|const") || funcname == 0)
				continue;

			tok = funcname->linkAt(1);

			if (funcname) {
				FunctionDeclaration &func = _functions[ funcname->str()];

				if (!func.lineNumber)
					func.lineNumber = funcname->linenr();

				// No filename set yet..
				if (func.filename.empty()) {
					func.filename = tokenizer.getSourceFilePath();
				}

				if(func.scopeName.empty())
				{
					func.scopeName = tok->scope()->className;
					func.scopeType = tok->scope()->type;
				}
			}
		}
    }

	// Function usage..
    for (std::list<Scope>::const_iterator scope = symbolDatabase->scopeList.begin(); scope != symbolDatabase->scopeList.end(); ++scope) {
        // only check functions
        if (scope->type != Scope::eFunction)
            continue;
	
	    for (const Token *tok = scope->classDef->next(); tok && tok != scope->classEnd; tok = tok->next()) {
			const Token *funcname = 0;

			if (Token::Match(tok->next(), "%var% (")) {
				funcname = tok->next();
			}

			else if (Token::Match(tok, "[;{}.,()[=+-/&|!?:] %var% [(),;:}]"))
				funcname = tok->next();

			else if (Token::Match(tok, "[=(,] &| %var% :: %var%")) {
				funcname = tok->next();
				if (funcname->str() == "&")
					funcname = funcname->next();
				while (Token::Match(funcname,"%var% :: %var%"))
					funcname = funcname->tokAt(2);
				if (!Token::Match(funcname, "%var% [,);]"))
					continue;
			}

			else
				continue;

			// funcname ( => Assert that the end parenthesis isn't followed by {
			if (Token::Match(funcname, "%var% (")) {
				if (Token::Match(funcname->linkAt(1), ") const|{"))
					funcname = NULL;
			}
			else
				funcname=NULL;

			if (funcname) {
				if(!_functions[funcname->str()].scopeName.empty() && _functions[funcname->str()].scopeType==Scope::eClass)
					continue;

				//사용기록 저장
				std::pair<bool,			// read or write
							std::pair<	std::string,	//file path
										unsigned int	//code line
										>> a;
				a.first=false;
				a.second.first=tokenizer.getSourceFilePath();
				a.second.second=funcname->linenr();
				functionFunctionTable[funcname->str()][scope->className].push_back(a);
			}
		}
	}
}




void MakeFunctionFunctionTable::check(ErrorLogger * const errorLogger)
{
	FILE *fp=fopen("log.txt","a");
	//결과 확인용 코드
	fprintf(fp,"=====FunctionFunctionTable=====\n");
	fprintf(fp,"t\tproject\n");
	for(std::map<std::string,std::map<std::string,std::list<std::pair<bool,std::pair<std::string,unsigned int>>>>>
		::iterator it0=functionFunctionTable.begin();	it0!=functionFunctionTable.end()	;++it0)
	{
		fprintf(fp,"r\t%s\n",(it0->first.begin()._Ptr));
		for(std::map<std::string,std::list<std::pair<bool,std::pair<std::string,unsigned int>>>>
			::iterator it1=it0->second.begin();	it1!=it0->second.end()	;++it1)
		{
			fprintf(fp,"c\t%s\n",it1->first.begin()._Ptr);
/*
			for(std::list<std::pair<bool,std::pair<std::string,unsigned int>>>
				::iterator it2=it1->second.begin();it2!=it1->second.end();++it2)
				printf("\t\t%s(%d)\n",it2._Ptr->_Myval.second.first.begin()._Ptr,it2._Ptr->_Myval.second.second);
*/
		}
		fprintf(fp,"\n");
	}
	fprintf(fp,"=====FunctionFunctionTableEnd=====\n");
	fclose(fp);
}

void MakeFunctionFunctionTable::unusedFunctionError(ErrorLogger * const errorLogger,
        const std::string &filename, unsigned int lineNumber,
        const std::string &funcname)
{
    std::list<ErrorLogger::ErrorMessage::FileLocation> locationList;
    if (!filename.empty()) {
        ErrorLogger::ErrorMessage::FileLocation fileLoc;
        fileLoc.setfile(filename);
        fileLoc.line = lineNumber;
        locationList.push_back(fileLoc);
    }

    const ErrorLogger::ErrorMessage errmsg(locationList, Severity::style, "The function '" + funcname + "' is never used", "unusedFunction", false);
    if (errorLogger)
        errorLogger->reportErr(errmsg);
    else
        reportError(errmsg);
}
