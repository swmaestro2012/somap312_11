/*
 */


//---------------------------------------------------------------------------
#include "MakeClassVariableFunctionsTable.h"
#include "tokenize.h"
#include "token.h"
#include <cctype>
//---------------------------------------------------------------------------




//---------------------------------------------------------------------------
// FUNCTION USAGE - Check for unused functions etc
//---------------------------------------------------------------------------

class VariableList {
public:
    enum VariableType { standard, array, pointer, reference, pointerArray, referenceArray, pointerPointer, none };

    /** Store information about variable usage */
    class VariableUsage {
    public:
        VariableUsage(const Token *name = 0,
                      VariableType type = standard,
                      const Scope *scope = NULL,
                      bool read = false,
                      bool write = false,
                      bool modified = false,
                      bool allocateMemory = false) :
            _name(name),
            _scope(scope),
            _type(type),
            _read(read),
            _write(write),
            _modified(modified),
            _allocateMemory(allocateMemory) {
        }

        /** variable is used.. set both read+write */
        void use(unsigned int line) {
            _read = true;
            _write = true;
			std::pair<int,unsigned int> p;
			p.first=2;
			p.second=line;
			useLine.push_back(p);
        }

        /** is variable unused? */
        bool unused() const {
            return (_read == false && _write == false);
        }

		void reset()
		{
            _read=0;
            _write=0;
            _modified=0;
			useLine.clear();
		}

        std::set<unsigned int> _aliases;
        std::set<const Scope*> _assignments;

        const Token *_name;
        const Scope *_scope;
        VariableType _type;
        bool _read;
        bool _write;
        bool _modified; // read/modify/write
		std::list<std::pair<int,			//	usage
							unsigned int	//	line
								>> useLine;

        bool _allocateMemory;
    };

    typedef std::map<unsigned int, VariableUsage> VariableMap;

    void clear() {
        _varUsage.clear();
    }
    const VariableMap &varUsage() const {
        return _varUsage;
    }
    void addVar(const Token *name, VariableType type, const Scope *scope, bool write_);
    void allocateMemory(unsigned int varid);
    void read(unsigned int varid,unsigned int line);
    void readAliases(unsigned int varid,unsigned int line);
    void readAll(unsigned int varid,unsigned int line);
    void write(unsigned int varid,unsigned int line);
    void writeAliases(unsigned int varid,unsigned int line);
    void writeAll(unsigned int varid,unsigned int line);
    void use(unsigned int varid,unsigned int line);
    void modified(unsigned int varid,unsigned int line);
    VariableUsage *find(unsigned int varid);
    void alias(unsigned int varid1, unsigned int varid2, bool replace,unsigned int line);
    void erase(unsigned int varid) {
        _varUsage.erase(varid);
    }
    void eraseAliases(unsigned int varid);
    void eraseAll(unsigned int varid);
    void clearAliases(unsigned int varid);

private:
    VariableMap _varUsage;
};


static const Token* doAssignment(VariableList &variableList, const Token *tok, bool dereference, const Scope *scope)
{
    // a = a + b;
    if (Token::Match(tok, "%var% = %var% !!;") && tok->varId() == tok->tokAt(2)->varId()) {
        return tok->tokAt(2);
    }

    const Token* const tokOld = tok;

    // check for aliased variable
    const unsigned int varid1 = tok->varId();
    VariableList::VariableUsage *var1 = variableList.find(varid1);

    if (var1) {
        // jump behind '='
        tok = tok->next();
        while (tok->str() != "=") {
            if (tok->varId())
				variableList.read(tok->varId(),tok->linenr());
            tok = tok->next();
        }
        tok = tok->next();

        if (Token::Match(tok, "&| %var%") ||
            Token::Match(tok, "( const| struct|union| %type% *| ) &| %var%") ||
            Token::Match(tok, "( const| struct|union| %type% *| ) ( &| %var%") ||
            Token::Match(tok->next(), "< const| struct|union| %type% *| > ( &| %var%")) {
            bool addressOf = false;

            if (Token::Match(tok, "%var% ."))
				variableList.use(tok->varId(),tok->linenr());   // use = read + write

            // check for C style cast
            if (tok->str() == "(") {
                tok = tok->next();
                if (tok->str() == "const")
                    tok = tok->next();

                if (Token::Match(tok, "struct|union"))
                    tok = tok->next();

                tok = tok->next();
                if (tok->str() == "*")
                    tok = tok->next();

                tok = tok->next();
                if (tok->str() == "&") {
                    addressOf = true;
                    tok = tok->next();
                } else if (tok->str() == "(") {
                    tok = tok->next();
                    if (tok->str() == "&") {
                        addressOf = true;
                        tok = tok->next();
                    }
                }
            }

            // check for C++ style cast
            else if (tok->str().find("cast") != std::string::npos &&
                     tok->strAt(1) == "<") {
                tok = tok->tokAt(2);
                if (tok->str() == "const")
                    tok = tok->next();

                if (Token::Match(tok, "struct|union"))
                    tok = tok->next();

                tok = tok->next();
                if (tok->str() == "*")
                    tok = tok->next();

                tok = tok->tokAt(2);
                if (tok->str() == "&") {
                    addressOf = true;
                    tok = tok->next();
                }
            }

            // no cast, no ?
            else if (!Token::Match(tok, "%var% ?")) {
                if (tok->str() == "&") {
                    addressOf = true;
                    tok = tok->next();
                } else if (tok->str() == "new")
                    return tokOld;
            }

            // check if variable is local
            unsigned int varid2 = tok->varId();
            VariableList::VariableUsage* var2 = variableList.find(varid2);

            if (var2) { // local variable (alias or read it)
                if (var1->_type == VariableList::pointer || var1->_type == VariableList::pointerArray) {
                    if (dereference)
						variableList.read(varid2,tok->linenr());
                    else {
                        if (addressOf ||
                            var2->_type == VariableList::array ||
                            var2->_type == VariableList::pointer) {
                            bool replace = true;

                            // pointerArray => don't replace
                            if (var1->_type == VariableList::pointerArray)
                                replace = false;

                            // check if variable declared in same scope
                            else if (scope == var1->_scope)
                                replace = true;

                            // not in same scope as declaration
                            else {
                                // no other assignment in this scope
                                if (var1->_assignments.find(scope) == var1->_assignments.end() ||
                                    scope->type == Scope::eSwitch) {
                                    // nothing to replace
                                    if (var1->_assignments.empty())
                                        replace = false;

                                    // this variable has previous assignments
                                    else {
                                        /**
                                         * @todo determine if existing aliases should be replaced or merged
                                         */

                                        replace = false;
                                    }
                                }

                                // assignment in this scope
                                else {
                                    // replace when only one other assignment, merge them otherwise
                                    replace = (var1->_assignments.size() == 1);
                                }
                            }

							variableList.alias(varid1, varid2, replace,tok->linenr());
                        } else if (tok->strAt(1) == "?") {
                            if (var2->_type == VariableList::reference)
								variableList.readAliases(varid2,tok->linenr());
                            else
								variableList.read(varid2,tok->linenr());
                        } else {
                            variableList.read(varid2,tok->linenr());
                        }
                    }
                } else if (var1->_type == VariableList::reference) {
					variableList.alias(varid1, varid2, true,tok->linenr());
                } else {
                    if (var2->_type == VariableList::pointer && tok->strAt(1) == "[")
						variableList.readAliases(varid2,tok->linenr());

                    variableList.read(varid2,tok->linenr());
                }
            } else { // not a local variable (or an unsupported local variable)
                if (var1->_type == VariableList::pointer && !dereference) {
                    // check if variable declaration is in this scope
                    if (var1->_scope == scope)
                        variableList.clearAliases(varid1);
                    else {
                        // no other assignment in this scope
                        if (var1->_assignments.find(scope) == var1->_assignments.end()) {
                            /**
                             * @todo determine if existing aliases should be discarded
                             */
                        }

                        // this assignment replaces the last assignment in this scope
                        else {
                            // aliased VariableList in a larger scope are not supported
                            // remove all aliases
                            variableList.clearAliases(varid1);
                        }
                    }
                }
            }
        } else
            tok = tokOld;

        var1->_assignments.insert(scope);
    }

    // check for alias to struct member
    // char c[10]; a.b = c;
    else if (Token::Match(tok->tokAt(-2), "%var% .")) {
        if (Token::Match(tok->tokAt(2), "%var%")) {
            unsigned int varid2 = tok->tokAt(2)->varId();
            VariableList::VariableUsage *var2 = variableList.find(varid2);

            // struct member aliased to local variable
            if (var2 && (var2->_type == VariableList::array ||
                         var2->_type == VariableList::pointer)) {
                // erase aliased variable and all VariableList that alias it
                // to prevent false positives
                variableList.eraseAll(varid2);
            }
        }
    }

    return tok;
}
static bool isRecordTypeWithoutSideEffects(const Scope* type)
{
    // a type that has no side effects (no constructors and no members with constructors)
    /** @todo false negative: check constructors for side effects */
    if (type && type->numConstructors == 0 &&
        (type->varlist.empty() || type->needInitialization == Scope::True)) {
        bool yes = true;
        for (std::vector<Scope::BaseInfo>::const_iterator i = type->derivedFrom.begin(); yes && i != type->derivedFrom.end(); ++i)
            yes = isRecordTypeWithoutSideEffects(i->scope);
        return yes;
    }
    return false;
}
static bool isPartOfClassStructUnion(const Token* tok)
{
    for (; tok; tok = tok->previous()) {
        if (tok->str() == "}" || tok->str() == ")")
            tok = tok->link();
        else if (tok->str() == "(")
            return(false);
        else if (tok->str() == "{") {
            return(tok->strAt(-1) == "struct" || tok->strAt(-2) == "struct" || tok->strAt(-1) == "class" || tok->strAt(-2) == "class" || tok->strAt(-1) == "union" || tok->strAt(-2) == "union");
        }
    }
    return false;
}
// Skip [ .. ]
static const Token * skipBrackets(const Token *tok)
{
    while (tok && tok->str() == "[")
        tok = tok->link()->next();
    return tok;
}
// Skip [ .. ] . x
static const Token * skipBracketsAndMembers(const Token *tok)
{
    while (tok) {
        if (tok->str() == "[")
            tok = tok->link()->next();
        else if (Token::Match(tok, ". %var%"))
            tok = tok->tokAt(2);
        else
            break;
    }
    return tok;
}

void MakeClassVariableFunctionsTable::getClassVariableList(const Tokenizer &tokenizer,const Scope* const scope, VariableList& variables)
{
    if (scope->type == Scope::eClass) 
	{
        // Find declarations
        for (std::list<Variable>::const_iterator i = scope->varlist.begin(); i != scope->varlist.end(); ++i) {
			//����
            if (i->isThrow()) //|| i->isExtern())
                continue;
            VariableList::VariableType type = VariableList::none;
            if (i->isArray() && (i->nameToken()->previous()->str() == "*" || i->nameToken()->strAt(-2) == "*"))
                type = VariableList::pointerArray;
            else if (i->isArray() && i->nameToken()->previous()->str() == "&")
                type = VariableList::referenceArray;
            else if (i->isArray())
                type = VariableList::array;
            else if (i->isReference())
                type = VariableList::reference;
            else if (i->nameToken()->previous()->str() == "*" && i->nameToken()->strAt(-2) == "*")
                type = VariableList::pointerPointer;
            else if (i->isPointer())
                type = VariableList::pointer;
            else if (tokenizer.isC() || i->typeEndToken()->isStandardType() || isRecordTypeWithoutSideEffects(i->type()) || Token::simpleMatch(i->typeStartToken(), "std ::"))
                type = VariableList::standard;
//            if (type == VariableList::none || isPartOfClassStructUnion(i->typeStartToken()))
//                continue;
            const Token* defValTok = i->nameToken()->next();
            for (; defValTok; defValTok = defValTok->next()) {
                if (defValTok->str() == "[")
                    defValTok = defValTok->link();
                else if (defValTok->str() == "(" || defValTok->str() == "=") {
                    variables.addVar(i->nameToken(), type, scope, true);
                    break;
                } else if (defValTok->str() == ";" || defValTok->str() == "," || defValTok->str() == ")") {
                    variables.addVar(i->nameToken(), type, scope, i->isStatic());
                    break;
                }
            }
            if (i->isArray() && i->isClass()) // Array of class/struct members. Initialized by ctor.
				variables.write(i->varId(),i->nameToken()->linenr());
            if (i->isArray() && Token::Match(i->nameToken(), "%var% [ %var% ]")) // Array index variable read.
                variables.read(i->nameToken()->tokAt(2)->varId(),i->nameToken()->tokAt(2)->linenr());

            if (defValTok && defValTok->str() == "=") {
                if (defValTok->next() && defValTok->next()->str() == "{") {
                    for (const Token* tok = defValTok; tok && tok != defValTok->linkAt(1); tok = tok->next())
                        if (Token::Match(tok, "%var%")) // VariableList used to initialize the array read.
                            variables.read(tok->varId(),tok->linenr());
                } else
                    doAssignment(variables, i->nameToken(), false, scope);
            } else if (Token::Match(defValTok, "( %var% )")) // VariableList used to initialize the variable read.
				variables.readAll(defValTok->next()->varId(),defValTok->next()->linenr()); // ReadAll?
        }
    }
}

void MakeClassVariableFunctionsTable::checkFunctionVariableUsage_iterateScopes(const Tokenizer &tokenizer,const Scope* scope, VariableList& variableList)
{
    // Check variable usage
    for (const Token *tok = scope->classDef->next(); tok && tok != scope->classEnd; tok = tok->next()) {
        if (tok->str() == "for") {
            for (std::list<Scope*>::const_iterator i = scope->nestedList.begin(); i != scope->nestedList.end(); ++i) {
                if ((*i)->classDef == tok) { // Find associated scope
                    checkFunctionVariableUsage_iterateScopes(tokenizer,*i, variableList); // Scan child scope
                    tok = (*i)->classStart->link();
                    break;
                }
            }
            if (!tok)
                break;
        }
        if (tok->str() == "{") {
            for (std::list<Scope*>::const_iterator i = scope->nestedList.begin(); i != scope->nestedList.end(); ++i) {
                if ((*i)->classStart == tok) { // Find associated scope
					checkFunctionVariableUsage_iterateScopes(tokenizer,*i, variableList); // Scan child scope
                    tok = tok->link();
                    break;
                }
            }
            if (!tok)
                break;
        }

        if (Token::Match(tok, "asm ( %str% )")) {
            variableList.clear();
            break;
        }

        if (Token::Match(tok->previous(), "[;{}]")) {
            for (const Token* tok2 = tok->next(); tok2; tok2 = tok2->next()) {
                if (tok2->varId()) {
                    const Variable* var = tokenizer.getSymbolDatabase()->getVariableFromVarId(tok2->varId());
                    if (var && var->nameToken() == tok2) { // Declaration: Skip
                        tok = tok2->next();
                        if (Token::Match(tok, "( %var% )")) // Simple initialization through copy ctor
                            tok = tok->next();
                        else if (Token::Match(tok, "= %var% ;")) // Simple initialization
                            tok = tok->next();
                        else if (var->typeEndToken()->str() == ">") // Be careful with types like std::vector
                            tok = tok->previous();
                        break;
                    }
                } else if (Token::Match(tok2, "[;({=]"))
                    break;
            }
        }
        // Freeing memory (not considered "using" the pointer if it was also allocated in this function)
        if (Token::Match(tok, "free|g_free|kfree|vfree ( %var% )") ||
            Token::Match(tok, "delete %var% ;") ||
            Token::Match(tok, "delete [ ] %var% ;")) {
            unsigned int varid = 0;
            if (tok->str() != "delete") {
                varid = tok->tokAt(2)->varId();
                tok = tok->tokAt(3);
            } else if (tok->strAt(1) == "[") {
                varid = tok->tokAt(3)->varId();
                tok = tok->tokAt(3);
            } else {
                varid = tok->next()->varId();
                tok = tok->next();
            }

            VariableList::VariableUsage *var = variableList.find(varid);
            if (var && !var->_allocateMemory) {
				variableList.readAll(varid,var->_name->linenr());
            }
        }

        else if (Token::Match(tok, "return|throw")) {
            for (const Token *tok2 = tok->next(); tok2; tok2 = tok2->next()) {
                if (tok2->varId())
					variableList.readAll(tok2->varId(),tok2->linenr());
                else if (tok2->str() == ";")
                    break;
            }
        }

        // assignment
        else if (!Token::Match(tok->tokAt(-2), "[;{}.] %var% (") &&
                 (Token::Match(tok, "*| (| ++|--| %var% ++|--| )| =") ||
                  Token::Match(tok, "*| ( const| %type% *| ) %var% ="))) {
            bool dereference = false;
            bool pre = false;
            bool post = false;

            if (tok->str() == "*") {
                dereference = true;
                tok = tok->next();
            }

            if (Token::Match(tok, "( const| %type% *| ) %var% ="))
                tok = tok->link()->next();

            else if (tok->str() == "(")
                tok = tok->next();

            if (tok->type() == Token::eIncDecOp) {
                pre = true;
                tok = tok->next();
            }

            if (tok->next()->type() == Token::eIncDecOp)
                post = true;

            const unsigned int varid1 = tok->varId();
            const Token *start = tok;

            tok = doAssignment(variableList, tok, dereference, scope);

            if (pre || post)
                variableList.use(varid1,start->linenr());

            if (dereference) {
                VariableList::VariableUsage *var = variableList.find(varid1);
                if (var && var->_type == VariableList::array)
					variableList.write(varid1,start->linenr());
				variableList.writeAliases(varid1,tok->linenr());
                variableList.read(varid1,start->linenr());
            } else {
                VariableList::VariableUsage *var = variableList.find(varid1);
                if (var && var->_type == VariableList::reference) {
					variableList.writeAliases(varid1,tok->linenr());
                    variableList.read(varid1,start->linenr());
                }
                // Consider allocating memory separately because allocating/freeing alone does not constitute using the variable
                else if (var && var->_type == VariableList::pointer &&
                         Token::Match(start, "%var% = new|malloc|calloc|kmalloc|kzalloc|kcalloc|strdup|strndup|vmalloc|g_new0|g_try_new|g_new|g_malloc|g_malloc0|g_try_malloc|g_try_malloc0|g_strdup|g_strndup|g_strdup_printf")) {
                    bool allocate = true;

                    if (start->strAt(2) == "new") {
                        const Token *type = start->tokAt(3);

                        // skip nothrow
                        if (Token::simpleMatch(type, "( nothrow )") ||
                            Token::simpleMatch(type, "( std :: nothrow )"))
                            type = type->link()->next();

                        // is it a user defined type?
                        if (!type->isStandardType()) {
                            const Variable* variable = tokenizer.getSymbolDatabase()->getVariableFromVarId(start->varId());
                            if (!variable || !isRecordTypeWithoutSideEffects(variable->type()))
                                allocate = false;
                        }
                    }

                    if (allocate)
                        variableList.allocateMemory(varid1);
                    else
                        variableList.write(varid1,start->linenr());
                } else if (varid1 && Token::Match(tok, "%varid% .", varid1)) {
                    variableList.use(varid1,start->linenr());
                } else {
                    variableList.write(varid1,start->linenr());
                }

                VariableList::VariableUsage *var2 = variableList.find(tok->varId());
                if (var2) {
                    if (var2->_type == VariableList::reference) {
						variableList.writeAliases(tok->varId(),tok->linenr());
						variableList.read(tok->varId(),tok->linenr());
                    } else if (tok->varId() != varid1 && Token::Match(tok, "%var% ."))
                        variableList.read(tok->varId(),tok->linenr());
                    else if (tok->varId() != varid1 &&
                             var2->_type == VariableList::standard &&
                             tok->strAt(-1) != "&")
                        variableList.use(tok->varId(),tok->linenr());
                }
            }

            const Token * const equal = skipBracketsAndMembers(tok->next());

            // checked for chained assignments
            if (tok != start && equal && equal->str() == "=") {
                VariableList::VariableUsage *var = variableList.find(tok->varId());

                if (var && var->_type != VariableList::reference)
				{
                    var->_read = true;
					std::pair<int,unsigned int> p;
					p.first=1;
					p.second=tok->linenr();
					var->useLine.push_back(p);
				}

                tok = tok->previous();
            }
        }

        // assignment
        else if ((Token::Match(tok, "%var% [") && Token::simpleMatch(skipBracketsAndMembers(tok->next()), "=")) ||
                 (Token::simpleMatch(tok, "* (") && Token::simpleMatch(tok->next()->link(), ") ="))) {
            if (tok->str() == "*") {
                tok = tok->tokAt(2);
                if (tok->str() == "(")
                    tok = tok->link()->next();
            }

            unsigned int varid = tok->varId();
            const VariableList::VariableUsage *var = variableList.find(varid);

            if (var) {
                // Consider allocating memory separately because allocating/freeing alone does not constitute using the variable
                if (var->_type == VariableList::pointer &&
                    Token::Match(skipBrackets(tok->next()), "= new|malloc|calloc|kmalloc|kzalloc|kcalloc|strdup|strndup|vmalloc|g_new0|g_try_new|g_new|g_malloc|g_malloc0|g_try_malloc|g_try_malloc0|g_strdup|g_strndup|g_strdup_printf")) {
                    variableList.allocateMemory(varid);
                } else if (var->_type == VariableList::pointer || var->_type == VariableList::reference) {
                    variableList.read(varid,tok->linenr());
                    variableList.writeAliases(varid,tok->linenr());
                } else if (var->_type == VariableList::pointerArray) {
                    tok = doAssignment(variableList, tok, false, scope);
                } else
					variableList.writeAll(varid,tok->linenr());
            }
        }

        else if (Token::Match(tok, "& %var%")) {
            if (tok->previous()->isName() || tok->previous()->isNumber()) { // bitop
                variableList.read(tok->next()->varId(),tok->next()->linenr());
            } else // addressof
                variableList.use(tok->next()->varId(),tok->next()->linenr()); // use = read + write
        } else if (Token::Match(tok, ">> %var%"))
            variableList.use(tok->next()->varId(),tok->next()->linenr()); // use = read + write
        else if (Token::Match(tok, "%var% >>|&") && Token::Match(tok->previous(), "[{};:]"))
            variableList.read(tok->varId(),tok->linenr());

        // function parameter
        else if (Token::Match(tok, "[(,] %var% ["))
            variableList.use(tok->next()->varId(),tok->next()->linenr());   // use = read + write
        else if (Token::Match(tok, "[(,] %var% [,)]") && tok->previous()->str() != "*") {
            variableList.use(tok->next()->varId(),tok->next()->linenr());   // use = read + write
        } else if (Token::Match(tok, "[(,] (") &&
                   Token::Match(tok->next()->link(), ") %var% [,)]"))
				   variableList.use(tok->next()->link()->next()->varId(),tok->next()->link()->next()->linenr());   // use = read + write

        // function
        else if (Token::Match(tok, "%var% (")) {
            variableList.read(tok->varId(),tok->linenr());
            if (Token::Match(tok->tokAt(2), "%var% ="))
                variableList.read(tok->tokAt(2)->varId(),tok->tokAt(2)->linenr());
        }

        else if (Token::Match(tok, "[{,] %var% [,}]"))
            variableList.read(tok->next()->varId(),tok->next()->linenr());

        else if (Token::Match(tok, "%var% ."))
            variableList.use(tok->varId(),tok->linenr());   // use = read + write

        else if (tok->isExtendedOp() &&
                 Token::Match(tok->next(), "%var%") && !Token::Match(tok->next(), "true|false|new") && tok->strAt(2) != "=")
				 variableList.readAll(tok->next()->varId(),tok->next()->linenr());

        else if (Token::Match(tok, "%var%") && tok->next() && (tok->next()->str() == ")" || tok->next()->isExtendedOp()))
			variableList.readAll(tok->varId(),tok->linenr());

        else if (Token::Match(tok, "%var% ;") && Token::Match(tok->previous(), "[;{}:]"))
			variableList.readAll(tok->varId(),tok->linenr());

        else if (Token::Match(tok, "++|-- %var%")) {
            if (!Token::Match(tok->previous(), "[;{}:]"))
                variableList.use(tok->next()->varId(),tok->next()->linenr());
            else
                variableList.modified(tok->next()->varId(),tok->next()->linenr());
        }

        else if (Token::Match(tok, "%var% ++|--")) {
            if (!Token::Match(tok->previous(), "[;{}:]"))
                variableList.use(tok->varId(),tok->linenr());
            else
                variableList.modified(tok->varId(),tok->linenr());
        }

        else if (tok->isAssignmentOp()) {
            for (const Token *tok2 = tok->next(); tok2 && tok2->str() != ";"; tok2 = tok2->next()) {
                if (tok2->varId()) {
                    if (tok2->next()->isAssignmentOp())
                        variableList.write(tok2->varId(),tok2->linenr());
                    else
                        variableList.read(tok2->varId(),tok2->linenr());
                }
            }
        }
    }
}


void MakeClassVariableFunctionsTable::parseTokens(const Tokenizer &tokenizer)
{
	symbolDatabase=tokenizer.getSymbolDatabase();


	std::map<std::string,VariableList> classVariableList;

	// Function and Variable declarations..
	for (std::list<Scope>::const_iterator scope = symbolDatabase->scopeList.begin(); scope != symbolDatabase->scopeList.end(); ++scope) { 

		if (scope->type != Scope::eClass)
			continue;

		getClassVariableList(tokenizer, &*scope,classVariableList[scope->className]);

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

	// Variable usage..
    for (std::list<Scope>::const_iterator scope = symbolDatabase->scopeList.begin(); scope != symbolDatabase->scopeList.end(); ++scope) {
        // only check functions
        if (scope->type != Scope::eFunction)
            continue;
	
		if(_functions[scope->className].scopeName.empty() || _functions[scope->className].scopeType!=Scope::eClass)
			continue;

		//reset variableList
		for(std::map<std::string,VariableList>::iterator it0 = classVariableList.begin();it0!=classVariableList.end();++it0) {
			for (VariableList::VariableMap::const_iterator it1 = it0->second.varUsage().begin(); it1 != it0->second.varUsage().end(); ++it1) {
				it1._Ptr->_Myval.second.reset();
			}
		}

		checkFunctionVariableUsage_iterateScopes(tokenizer,&*scope, classVariableList[_functions[scope->className].scopeName]);


		//VariableList�� ��� ���� üũ
		for(std::map<std::string,VariableList>::iterator it0 = classVariableList.begin();it0!=classVariableList.end();++it0) {
			for (VariableList::VariableMap::const_iterator it1 = it0->second.varUsage().begin(); it1 != it0->second.varUsage().end(); ++it1) {

				const VariableList::VariableUsage &usage = it1->second;
				const std::string &varname = usage._name->str();
				const Variable* var = symbolDatabase->getVariableFromVarId(it1->first);

				// variable has been marked as unused so ignore it
				if (usage.unused())
					continue;

				//����� ����
				std::pair<bool,			// read or write
							std::pair<	std::string,	//file path
										unsigned int	//code line
										>> a;
				a.first=false;
				a.second.first=tokenizer.getSourceFilePath();
				for(std::list<std::pair<int,unsigned int>>::const_iterator it2=usage.useLine.begin();it2!=usage.useLine.end();++it2)
				{
					std::list<std::pair<bool,std::pair<	std::string,unsigned int>>>::iterator it;
					for(it=variableFunctionTable[_functions[scope->className].scopeName][usage._name->str()][scope->className].begin();
						it!=variableFunctionTable[_functions[scope->className].scopeName][usage._name->str()][scope->className].end();
						++it)
					{
						if(it->second.first.find(a.first) && it->second.second==a.second.second)
							break;
					}
					if(it==variableFunctionTable[_functions[scope->className].scopeName][usage._name->str()][scope->className].end())
						variableFunctionTable[_functions[scope->className].scopeName][usage._name->str()][scope->className].push_back(a);
				}
			}
		}
	}
}


void MakeClassVariableFunctionsTable::check(ErrorLogger * const errorLogger)
{
//*
	FILE *fp=fopen("log.txt","a");
	//��� Ȯ�ο� �ڵ�
	fprintf(fp,"=====ClassVarialbeFunctionsTable=====\n");
	for(std::map<std::string,std::map<std::string,std::map<std::string,std::list<std::pair<bool,std::pair<std::string,unsigned int>>>>>>
		::iterator it0=variableFunctionTable.begin();	it0!=variableFunctionTable.end()	;++it0)
	{
		fprintf(fp,"t\t%s\n",(it0->first.begin()._Ptr));
		for(std::map<std::string,std::map<std::string,std::list<std::pair<bool,std::pair<std::string,unsigned int>>>>>
			::iterator it1=it0->second.begin();	it1!=it0->second.end()	;++it1)
		{
			fprintf(fp,"r\t%s\n",(it1->first.begin()._Ptr));
			for(std::map<std::string,std::list<std::pair<bool,std::pair<std::string,unsigned int>>>>
				::iterator it2=it1->second.begin();	it2!=it1->second.end()	;++it2)
			{
					fprintf(fp,"c\t%s\n",it2->first.begin()._Ptr);
/*
				for(std::list<std::pair<bool,std::pair<std::string,unsigned int>>>
					::iterator it3=it2->second.begin();it3!=it2->second.end();++it3)
					printf("\t\t\t%s(%d)\n",it3._Ptr->_Myval.second.first.begin()._Ptr,it3._Ptr->_Myval.second.second);
*/
			}
		}
	}
	fprintf(fp,"=====ClassVariableFunctionsTableEnd=====\n");
	fclose(fp);
//*/
}

void MakeClassVariableFunctionsTable::unusedFunctionError(ErrorLogger * const errorLogger,
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
