/*
 */


//---------------------------------------------------------------------------
#ifndef makeclassvariablefunctionsH
#define makeclassvariablefunctionsH
//---------------------------------------------------------------------------

#include "config.h"
#include "check.h"
#include "tokenize.h"
#include "errorlogger.h"
#include "symboldatabase.h"

class Token;
class Scope;
class VariableList;

/// @addtogroup Checks
/// @{

class CPPCHECKLIB MakeClassVariableFunctionsTable: public Check {
public:
    /** @brief This constructor is used when registering the MakeClassVariableFunctionsTable */
    MakeClassVariableFunctionsTable() : Check(myName())
    { }

    /** @brief This constructor is used when running checks. */
    MakeClassVariableFunctionsTable(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
        : Check(myName(), tokenizer, settings, errorLogger)
    { }

    // Parse current tokens and determine..
    // * Check what functions are used
    // * What functions are declared

	//�Լ� �� �Լ� ���̺�
	std::map<std::string,						//class		name
	std::map<std::string,						//function	name
	std::map<std::string,						//location(function name)
	std::list<
			std::pair<bool,			// read or write
			std::pair<	std::string,	//file path
			unsigned int	//code line
					>>>>>> variableFunctionTable;

//	std::list<std::string> classList;

	//���Ϻ� �۾�
    void parseTokens(const Tokenizer &tokenizer);

	//������ �۾�
    void check(ErrorLogger * const errorLogger);

private:
    const SymbolDatabase *symbolDatabase;


    void getErrorMessages(ErrorLogger *errorLogger, const Settings *settings) const {
        MakeClassVariableFunctionsTable c(0, settings, errorLogger);
        c.unusedFunctionError(errorLogger, "", 0, "funcName");
    }

    /**
     * Dummy implementation, just to provide error for --errorlist
     */
    void unusedFunctionError(ErrorLogger * const errorLogger,
                             const std::string &filename, unsigned int lineNumber,
                             const std::string &funcname);

    /**
     * Dummy implementation, just to provide error for --errorlist
     */
    void runSimplifiedChecks(const Tokenizer *, const Settings *, ErrorLogger *) {

    }

	//����
    void checkFunctionVariableUsage_iterateScopes(const Scope* const scope, VariableList& variables,
		std::map<std::string,std::map<std::string,std::list<std::pair<bool,std::pair<std::string,unsigned int>>>>>
			&variableFunctionTable);

    void checkFunctionVariableUsage();

	void getClassVariableList(const Tokenizer &tokenizer,const Scope* const scope, VariableList& variables);
	void checkFunctionVariableUsage_iterateScopes(const Tokenizer &tokenizer,const Scope* const scope, VariableList& variables);

    static std::string myName() {
        return "Unused functions";
    }

    std::string classInfo() const {
        return "Check for functions that are never called\n";
    }

    class CPPCHECKLIB FunctionDeclaration {
    public:
        FunctionDeclaration() : lineNumber(0)
        { }

        std::string filename;
		std::string argment;
		std::string type;
		std::string scopeName;
		Scope::ScopeType scopeType;

        unsigned int lineNumber;
    };

    std::map<std::string, FunctionDeclaration> _functions;
};
/// @}
//---------------------------------------------------------------------------
#endif

