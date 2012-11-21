/*
 */


//---------------------------------------------------------------------------
#ifndef MakeVariableFunctionTableH
#define MakeVariableFunctionTableH
//---------------------------------------------------------------------------

#include "config.h"
#include "check.h"
#include "settings.h"

class Token;
class Scope;
class VariableList;

/// @addtogroup Checks
/// @{


/** @brief Various small checks */

class CPPCHECKLIB MakeVariableFunctionTable : public Check {
public:
    /** @brief This constructor is used when registering the CheckClass */
    MakeVariableFunctionTable() : Check(myName())
    { }

    /** @brief This constructor is used when running checks. */
    MakeVariableFunctionTable(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
        : Check(myName(), tokenizer, settings, errorLogger)
    { }

    /** @brief Run checks against the normal token list */
    void runChecks(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger) {
        MakeVariableFunctionTable makeVariableFunctionTable(tokenizer, settings, errorLogger);

        // Coding style checks
        makeVariableFunctionTable.checkStructMemberUsage();
        makeVariableFunctionTable.checkFunctionVariableUsage();
    }

    /** @brief Run checks against the simplified token list */
    void runSimplifiedChecks(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger) {
        (void)tokenizer;
        (void)settings;
        (void)errorLogger;
    }

    /** @brief %Check for unused function variables */
	//편집
    void checkFunctionVariableUsage_iterateScopes(const Scope* const scope, VariableList& variables);

    void checkFunctionVariableUsage();
	void getGlobalVariableList(const SymbolDatabase* const symbolDatabase, VariableList& variables);

	
	/** @brief %Check that all struct members are used */
    void checkStructMemberUsage();

private:
	
	//자료 구조 정의
	std::map<std::string,						//variable name
	std::map<std::string,						//function name
	std::list<
						std::pair<bool,			// read or write
						std::pair<	std::string,	//file path
									unsigned int	//code line
									>>>>> variableFunctionTable;



    // Error messages..

    void getErrorMessages(ErrorLogger *errorLogger, const Settings *settings) const {
        MakeVariableFunctionTable c(0, settings, errorLogger);
    }

    static std::string myName() {
        return "UnusedVar";
    }

    std::string classInfo() const {
        return "UnusedVar checks\n"

               // style
               "* unused variable\n"
               "* allocated but unused variable\n"
               "* unred variable\n"
               "* unassigned variable\n"
               "* unused struct member\n";
    }
};
/// @}
//---------------------------------------------------------------------------
#endif

