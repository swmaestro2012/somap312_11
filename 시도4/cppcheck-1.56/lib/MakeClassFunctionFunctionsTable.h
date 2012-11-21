/*
 */


//---------------------------------------------------------------------------
#ifndef makeclassfunctionfunctionsH
#define makeclassfunctionfunctionsH
//---------------------------------------------------------------------------

#include "config.h"
#include "check.h"
#include "tokenize.h"
#include "errorlogger.h"
#include "symboldatabase.h"

/// @addtogroup Checks
/// @{

class CPPCHECKLIB MakeClassFunctionFunctionsTable: public Check {
public:
    /** @brief This constructor is used when registering the MakeClassFunctionFunctionsTable */
    MakeClassFunctionFunctionsTable() : Check(myName())
    { }

    /** @brief This constructor is used when running checks. */
    MakeClassFunctionFunctionsTable(const Tokenizer *tokenizer, const Settings *settings, ErrorLogger *errorLogger)
        : Check(myName(), tokenizer, settings, errorLogger)
    { }

    // Parse current tokens and determine..
    // * Check what functions are used
    // * What functions are declared

	//함수 대 함수 테이블
	std::map<std::string,						//class		name
	std::map<std::string,						//function	name
	std::map<std::string,						//location(function name)
	std::list<
			std::pair<bool,			// read or write
			std::pair<	std::string,	//file path
			unsigned int	//code line
					>>>>>> functionFunctionTable;

//	std::list<std::string> classList;

	//파일별 작업
    void parseTokens(const Tokenizer &tokenizer);

	//총정리 작업
    void check(ErrorLogger * const errorLogger);

private:
    const SymbolDatabase *symbolDatabase;


    void getErrorMessages(ErrorLogger *errorLogger, const Settings *settings) const {
        MakeClassFunctionFunctionsTable c(0, settings, errorLogger);
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

