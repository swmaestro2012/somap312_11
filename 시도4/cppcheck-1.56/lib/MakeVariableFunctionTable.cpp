/*
 */


//---------------------------------------------------------------------------
#include "makevariablefunctiontable.h"
#include "symboldatabase.h"

//---------------------------------------------------------------------------

// Register this check class (by creating a static instance of it)
namespace {
    MakeVariableFunctionTable instance;
}

/**
 * @brief This class is used create a list of variables within a function.
 */
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

void VariableList::read(unsigned int varid,unsigned int line)
{
    VariableUsage *usage = find(varid);

    if (usage)
	{
        usage->_read = true;
		std::pair<int,unsigned int> p;
		p.first=1;
		p.second=line;
		((VariableUsage*)usage)->useLine.push_back(p);
	}
}

void VariableList::write(unsigned int varid, unsigned int line)
{
    VariableUsage *usage = find(varid);

    if (usage)
	{
        usage->_write = true;
		std::pair<int,unsigned int> p;
		p.first=2;
		p.second=line;
		((VariableUsage*)usage)->useLine.push_back(p);
	}
}

void VariableList::modified(unsigned int varid, unsigned int line)
{
    VariableUsage *usage = find(varid);

    if (usage)
	{
		usage->_modified = true;
		std::pair<int,unsigned int> p;
		p.first=3;
		p.second=line;
		((VariableUsage*)usage)->useLine.push_back(p);
	}
}

void VariableList::use(unsigned int varid, unsigned int line)
{
    VariableUsage *usage = find(varid);

    if (usage)
	{
		usage->_modified = true;
		std::pair<int,unsigned int> p;
		p.first=3;
		p.second=line;
		usage->useLine.push_back(p);
	}
}

void VariableList::alias(unsigned int varid1, unsigned int varid2, bool replace, unsigned int line)
{
    VariableUsage *var1 = find(varid1);
    VariableUsage *var2 = find(varid2);

    // alias to self
    if (varid1 == varid2) {
        if (var1)
            var1->use(line);
        return;
    }

    if (replace) {
        // remove var1 from all aliases
        for (std::set<unsigned int>::iterator i = var1->_aliases.begin(); i != var1->_aliases.end(); ++i) {
            VariableUsage *temp = find(*i);

            if (temp)
                temp->_aliases.erase(var1->_name->varId());
        }

        // remove all aliases from var1
        var1->_aliases.clear();
    }

    // var1 gets all var2s aliases
    for (std::set<unsigned int>::iterator i = var2->_aliases.begin(); i != var2->_aliases.end(); ++i) {
        if (*i != varid1)
            var1->_aliases.insert(*i);
    }

    // var2 is an alias of var1
    var2->_aliases.insert(varid1);
    var1->_aliases.insert(varid2);

    if (var2->_type == VariableList::pointer)
	{
        var2->_read = true;
		std::pair<int,unsigned int> p;
		p.first=2;
		p.second=line;
		var2->useLine.push_back(p);
	}
}

void VariableList::clearAliases(unsigned int varid)
{
    VariableUsage *usage = find(varid);

    if (usage) {
        // remove usage from all aliases
        std::set<unsigned int>::iterator i;

        for (i = usage->_aliases.begin(); i != usage->_aliases.end(); ++i) {
            VariableUsage *temp = find(*i);

            if (temp)
                temp->_aliases.erase(usage->_name->varId());
        }

        // remove all aliases from usage
        usage->_aliases.clear();
    }
}

void VariableList::eraseAliases(unsigned int varid)
{
    VariableUsage *usage = find(varid);

    if (usage) {
        std::set<unsigned int>::iterator aliases;

        for (aliases = usage->_aliases.begin(); aliases != usage->_aliases.end(); ++aliases)
            erase(*aliases);
    }
}

void VariableList::eraseAll(unsigned int varid)
{
    eraseAliases(varid);
    erase(varid);
}

void VariableList::addVar(const Token *name,
                       VariableType type,
                       const Scope *scope,
                       bool write_)
{
    if (name->varId() > 0)
        _varUsage.insert(std::make_pair(name->varId(), VariableUsage(name, type, scope, false, write_, false)));
}

void VariableList::allocateMemory(unsigned int varid)
{
    VariableUsage *usage = find(varid);

    if (usage)
        usage->_allocateMemory = true;
}


void VariableList::readAliases(unsigned int varid,unsigned int line)
{
    VariableUsage *usage = find(varid);

    if (usage) {
        std::set<unsigned int>::iterator aliases;

        for (aliases = usage->_aliases.begin(); aliases != usage->_aliases.end(); ++aliases) {
            VariableUsage *aliased = find(*aliases);

            if (aliased)
			{
                aliased->_read = true;
				std::pair<int,unsigned int> p;
				p.first=1;
				p.second=line;
				aliased->useLine.push_back(p);
			}
        }
    }
}

void VariableList::readAll(unsigned int varid, unsigned int line)
{
    VariableUsage *usage = find(varid);

    if (usage) {
        usage->_read = true;

        std::set<unsigned int>::iterator aliases;

        for (aliases = usage->_aliases.begin(); aliases != usage->_aliases.end(); ++aliases) {
            VariableUsage *aliased = find(*aliases);

            if (aliased)
			{
                aliased->_read = true;
				std::pair<int,unsigned int> p;
				p.first=1;
				p.second=line;
				aliased->useLine.push_back(p);
			}
        }
    }
}


void VariableList::writeAliases(unsigned int varid,unsigned int line)
{
    VariableUsage *usage = find(varid);

    if (usage) {
        std::set<unsigned int>::iterator aliases;

        for (aliases = usage->_aliases.begin(); aliases != usage->_aliases.end(); ++aliases) {
            VariableUsage *aliased = find(*aliases);

            if (aliased)
			{
                aliased->_write = true;
				std::pair<int,unsigned int> p;
				p.first=2;
				p.second=line;
				aliased->useLine.push_back(p);
			}
        }
    }
}

void VariableList::writeAll(unsigned int varid, unsigned int line)
{
    VariableUsage *usage = find(varid);

    if (usage) {
        usage->_write = true;
		std::pair<int,unsigned int> p;
		p.first=2;
		p.second=line;
		usage->useLine.push_back(p);

        std::set<unsigned int>::iterator aliases;

        for (aliases = usage->_aliases.begin(); aliases != usage->_aliases.end(); ++aliases) {
            VariableUsage *aliased = find(*aliases);

            if (aliased)
			{
                aliased->_write = true;
				std::pair<int,unsigned int> p;
				p.first=2;
				p.second=line;
				aliased->useLine.push_back(p);
			}
        }
    }
}


VariableList::VariableUsage *VariableList::find(unsigned int varid)
{
    if (varid) {
        VariableMap::iterator i = _varUsage.find(varid);
        if (i != _varUsage.end())
            return &i->second;
    }
    return 0;
}


static const Token* doAssignment(VariableList &variables, const Token *tok, bool dereference, const Scope *scope)
{
    // a = a + b;
    if (Token::Match(tok, "%var% = %var% !!;") && tok->varId() == tok->tokAt(2)->varId()) {
        return tok->tokAt(2);
    }

    const Token* const tokOld = tok;

    // check for aliased variable
    const unsigned int varid1 = tok->varId();
    VariableList::VariableUsage *var1 = variables.find(varid1);

    if (var1) {
        // jump behind '='
        tok = tok->next();
        while (tok->str() != "=") {
            if (tok->varId())
				variables.read(tok->varId(),tok->linenr());
            tok = tok->next();
        }
        tok = tok->next();

        if (Token::Match(tok, "&| %var%") ||
            Token::Match(tok, "( const| struct|union| %type% *| ) &| %var%") ||
            Token::Match(tok, "( const| struct|union| %type% *| ) ( &| %var%") ||
            Token::Match(tok->next(), "< const| struct|union| %type% *| > ( &| %var%")) {
            bool addressOf = false;

            if (Token::Match(tok, "%var% ."))
				variables.use(tok->varId(),tok->linenr());   // use = read + write

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
            VariableList::VariableUsage* var2 = variables.find(varid2);

            if (var2) { // local variable (alias or read it)
                if (var1->_type == VariableList::pointer || var1->_type == VariableList::pointerArray) {
                    if (dereference)
						variables.read(varid2,tok->linenr());
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

							variables.alias(varid1, varid2, replace,tok->linenr());
                        } else if (tok->strAt(1) == "?") {
                            if (var2->_type == VariableList::reference)
								variables.readAliases(varid2,tok->linenr());
                            else
								variables.read(varid2,tok->linenr());
                        } else {
                            variables.read(varid2,tok->linenr());
                        }
                    }
                } else if (var1->_type == VariableList::reference) {
                    variables.alias(varid1, varid2, true, tok->linenr());
                } else {
                    if (var2->_type == VariableList::pointer && tok->strAt(1) == "[")
						variables.readAliases(varid2,tok->linenr());

                    variables.read(varid2,tok->linenr());
                }
            } else { // not a local variable (or an unsupported local variable)
                if (var1->_type == VariableList::pointer && !dereference) {
                    // check if variable declaration is in this scope
                    if (var1->_scope == scope)
                        variables.clearAliases(varid1);
                    else {
                        // no other assignment in this scope
                        if (var1->_assignments.find(scope) == var1->_assignments.end()) {
                            /**
                             * @todo determine if existing aliases should be discarded
                             */
                        }

                        // this assignment replaces the last assignment in this scope
                        else {
                            // aliased variables in a larger scope are not supported
                            // remove all aliases
                            variables.clearAliases(varid1);
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
            VariableList::VariableUsage *var2 = variables.find(varid2);

            // struct member aliased to local variable
            if (var2 && (var2->_type == VariableList::array ||
                         var2->_type == VariableList::pointer)) {
                // erase aliased variable and all variables that alias it
                // to prevent false positives
                variables.eraseAll(varid2);
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
//---------------------------------------------------------------------------
// Usage of function variables
//---------------------------------------------------------------------------
//편집
void MakeVariableFunctionTable::checkFunctionVariableUsage_iterateScopes(const Scope* const scope, 
	VariableList& variables)
{
    // Check variable usage
    for (const Token *tok = scope->classDef->next(); tok && tok != scope->classEnd; tok = tok->next()) {
        if (tok->str() == "for") {
            for (std::list<Scope*>::const_iterator i = scope->nestedList.begin(); i != scope->nestedList.end(); ++i) {
                if ((*i)->classDef == tok) { // Find associated scope
                    checkFunctionVariableUsage_iterateScopes(*i, variables); // Scan child scope
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
                    checkFunctionVariableUsage_iterateScopes(*i, variables); // Scan child scope
                    tok = tok->link();
                    break;
                }
            }
            if (!tok)
                break;
        }

        if (Token::Match(tok, "asm ( %str% )")) {
            variables.clear();
            break;
        }

        if (Token::Match(tok->previous(), "[;{}]")) {
            for (const Token* tok2 = tok->next(); tok2; tok2 = tok2->next()) {
                if (tok2->varId()) {
                    const Variable* var = _tokenizer->getSymbolDatabase()->getVariableFromVarId(tok2->varId());
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

            VariableList::VariableUsage *var = variables.find(varid);
            if (var && !var->_allocateMemory) {
				variables.readAll(varid,var->_name->linenr());
            }
        }

        else if (Token::Match(tok, "return|throw")) {
            for (const Token *tok2 = tok->next(); tok2; tok2 = tok2->next()) {
                if (tok2->varId())
					variables.readAll(tok2->varId(),tok2->linenr());
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

            tok = doAssignment(variables, tok, dereference, scope);

            if (pre || post)
                variables.use(varid1,start->linenr());

            if (dereference) {
                VariableList::VariableUsage *var = variables.find(varid1);
                if (var && var->_type == VariableList::array)
					variables.write(varid1,start->linenr());
				variables.writeAliases(varid1,var->_name->linenr());
                variables.read(varid1,start->linenr());
            } else {
                VariableList::VariableUsage *var = variables.find(varid1);
                if (var && var->_type == VariableList::reference) {
					variables.writeAliases(varid1,var->_name->linenr());
                    variables.read(varid1,start->linenr());
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
                            const Variable* variable = _tokenizer->getSymbolDatabase()->getVariableFromVarId(start->varId());
                            if (!variable || !isRecordTypeWithoutSideEffects(variable->type()))
                                allocate = false;
                        }
                    }

                    if (allocate)
                        variables.allocateMemory(varid1);
                    else
                        variables.write(varid1,start->linenr());
                } else if (varid1 && Token::Match(tok, "%varid% .", varid1)) {
                    variables.use(varid1,start->linenr());
                } else {
                    variables.write(varid1,start->linenr());
                }

                VariableList::VariableUsage *var2 = variables.find(tok->varId());
                if (var2) {
                    if (var2->_type == VariableList::reference) {
						variables.writeAliases(tok->varId(),tok->linenr());
						variables.read(tok->varId(),tok->linenr());
                    } else if (tok->varId() != varid1 && Token::Match(tok, "%var% ."))
                        variables.read(tok->varId(),tok->linenr());
                    else if (tok->varId() != varid1 &&
                             var2->_type == VariableList::standard &&
                             tok->strAt(-1) != "&")
                        variables.use(tok->varId(),tok->linenr());
                }
            }

            const Token * const equal = skipBracketsAndMembers(tok->next());

            // checked for chained assignments
            if (tok != start && equal && equal->str() == "=") {
                VariableList::VariableUsage *var = variables.find(tok->varId());

                if (var && var->_type != VariableList::reference)
				{
                    var->_read = true;
					std::pair<int,unsigned int> p;
					p.first=2;
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
            const VariableList::VariableUsage *var = variables.find(varid);

            if (var) {
                // Consider allocating memory separately because allocating/freeing alone does not constitute using the variable
                if (var->_type == VariableList::pointer &&
                    Token::Match(skipBrackets(tok->next()), "= new|malloc|calloc|kmalloc|kzalloc|kcalloc|strdup|strndup|vmalloc|g_new0|g_try_new|g_new|g_malloc|g_malloc0|g_try_malloc|g_try_malloc0|g_strdup|g_strndup|g_strdup_printf")) {
                    variables.allocateMemory(varid);
                } else if (var->_type == VariableList::pointer || var->_type == VariableList::reference) {
                    variables.read(varid,tok->linenr());
					variables.writeAliases(varid,tok->linenr());
                } else if (var->_type == VariableList::pointerArray) {
                    tok = doAssignment(variables, tok, false, scope);
                } else
					variables.writeAll(varid,tok->linenr());
            }
        }

        else if (Token::Match(tok, "& %var%")) {
            if (tok->previous()->isName() || tok->previous()->isNumber()) { // bitop
                variables.read(tok->next()->varId(),tok->next()->linenr());
            } else // addressof
                variables.use(tok->next()->varId(),tok->next()->linenr()); // use = read + write
        } else if (Token::Match(tok, ">> %var%"))
            variables.use(tok->next()->varId(),tok->next()->linenr()); // use = read + write
        else if (Token::Match(tok, "%var% >>|&") && Token::Match(tok->previous(), "[{};:]"))
            variables.read(tok->varId(),tok->linenr());

        // function parameter
        else if (Token::Match(tok, "[(,] %var% ["))
            variables.use(tok->next()->varId(),tok->next()->linenr());   // use = read + write
        else if (Token::Match(tok, "[(,] %var% [,)]") && tok->previous()->str() != "*") {
            variables.use(tok->next()->varId(),tok->next()->linenr());   // use = read + write
        } else if (Token::Match(tok, "[(,] (") &&
                   Token::Match(tok->next()->link(), ") %var% [,)]"))
				   variables.use(tok->next()->link()->next()->varId(),tok->next()->link()->next()->linenr());   // use = read + write

        // function
        else if (Token::Match(tok, "%var% (")) {
            variables.read(tok->varId(),tok->linenr());
            if (Token::Match(tok->tokAt(2), "%var% ="))
                variables.read(tok->tokAt(2)->varId(),tok->tokAt(2)->linenr());
        }

        else if (Token::Match(tok, "[{,] %var% [,}]"))
            variables.read(tok->next()->varId(),tok->next()->linenr());

        else if (Token::Match(tok, "%var% ."))
            variables.use(tok->varId(),tok->linenr());   // use = read + write

        else if (tok->isExtendedOp() &&
                 Token::Match(tok->next(), "%var%") && !Token::Match(tok->next(), "true|false|new") && tok->strAt(2) != "=")
				 variables.readAll(tok->next()->varId(),tok->next()->linenr());

        else if (Token::Match(tok, "%var%") && tok->next() && (tok->next()->str() == ")" || tok->next()->isExtendedOp()))
			variables.readAll(tok->varId(),tok->linenr());

        else if (Token::Match(tok, "%var% ;") && Token::Match(tok->previous(), "[;{}:]"))
			variables.readAll(tok->varId(),tok->linenr());

        else if (Token::Match(tok, "++|-- %var%")) {
            if (!Token::Match(tok->previous(), "[;{}:]"))
                variables.use(tok->next()->varId(),tok->next()->linenr());
            else
                variables.modified(tok->next()->varId(),tok->next()->linenr());
        }

        else if (Token::Match(tok, "%var% ++|--")) {
            if (!Token::Match(tok->previous(), "[;{}:]"))
                variables.use(tok->varId(),tok->linenr());
            else
                variables.modified(tok->varId(),tok->linenr());
        }

        else if (tok->isAssignmentOp()) {
            for (const Token *tok2 = tok->next(); tok2 && tok2->str() != ";"; tok2 = tok2->next()) {
                if (tok2->varId()) {
                    if (tok2->next()->isAssignmentOp())
                        variables.write(tok2->varId(),tok2->linenr());
                    else
                        variables.read(tok2->varId(),tok2->linenr());
                }
            }
        }
    }
}

void MakeVariableFunctionTable::checkFunctionVariableUsage()
{
    if (!_settings->isEnabled("style"))
        return;

    // Parse all executing scopes..
    const SymbolDatabase *symbolDatabase = _tokenizer->getSymbolDatabase();


    for (std::list<Scope>::const_iterator scope = symbolDatabase->scopeList.begin(); scope != symbolDatabase->scopeList.end(); ++scope) {
        // only check functions
        if (scope->type != Scope::eFunction)
            continue;

		//전역 변수 목록을 받음 (extrun으로 선언된 변수 포함)
		VariableList gVariableList;
		getGlobalVariableList(symbolDatabase, gVariableList);

		//VariableList로 받은 변수가 사용되었는지 검사
        checkFunctionVariableUsage_iterateScopes(&*scope, gVariableList);


		//VariableList의 사용 여부 체크
        for (VariableList::VariableMap::const_iterator it = gVariableList.varUsage().begin(); it != gVariableList.varUsage().end(); ++it) {
            const VariableList::VariableUsage &usage = it->second;
            const std::string &varname = usage._name->str();
            const Variable* var = symbolDatabase->getVariableFromVarId(it->first);

			// variable has been marked as unused so ignore it
			if (usage.unused())
                continue;

			//사용기록 저장
			std::pair<bool,			// read or write
						std::pair<	std::string,	//file path
									unsigned int	//code line
									>> a;
			a.first=false;
			a.second.first=_tokenizer->getSourceFilePath();
			for(std::list<std::pair<int,unsigned int>>::const_iterator it2=usage.useLine.begin();it2!=usage.useLine.end();++it2)
			{
				a.second.second=it2._Ptr->_Myval.second;
				variableFunctionTable[usage._name->str()][scope->className].push_back(a);
			}
        }
	}
	int check;
	FILE *fp=fopen("log.txt","a");
	//결과 확인용 코드
	fprintf(fp,"=====VariableFunctionTable=====\n");
	fprintf(fp,"t\t%s\n",_tokenizer->getSourceFilePath().begin()._Ptr);
	for(std::map<std::string,std::map<std::string,std::list<std::pair<bool,std::pair<std::string,unsigned int>>>>>
		::iterator it0=variableFunctionTable.begin();	it0!=variableFunctionTable.end()	;++it0)
	{
		fprintf(fp,"r\t%s\n",(it0->first.begin()._Ptr));
		for(std::map<std::string,std::list<std::pair<bool,std::pair<std::string,unsigned int>>>>
			::iterator it1=it0->second.begin();	it1!=it0->second.end()	;++it1)
		{
			fprintf(fp,"c\t%s\n",it1->first.begin()._Ptr);
/*
			for(std::list<std::pair<bool,std::pair<std::string,unsigned int>>>::iterator it2=it1->second.begin();
				it2!=it1->second.end();++it2)
			{
				printf("\t\tline : %d\n",it2->second.second);
			}
*/
		}
	}
	fprintf(fp,"=====VariableFunctionTableEnd=====\n");
	fclose(fp);
}



//---------------------------------------------------------------------------
// Check that all struct members are used
//---------------------------------------------------------------------------
void MakeVariableFunctionTable::checkStructMemberUsage()
{
    if (!_settings->isEnabled("style"))
        return;

    std::string structname;
    for (const Token *tok = _tokenizer->tokens(); tok; tok = tok->next()) {
        if (tok->fileIndex() != 0)
            continue;

        if (Token::Match(tok, "struct|union %type% {")) {
            structname.clear();
            if (tok->strAt(-1) == "extern")
                continue;
            if ((!tok->previous() || tok->previous()->str() == ";") && Token::Match(tok->linkAt(2), ("} ; " + tok->strAt(1) + " %var% ;").c_str()))
                continue;

            structname = tok->strAt(1);

            // Bail out if struct/union contain any functions
            for (const Token *tok2 = tok->tokAt(2); tok2; tok2 = tok2->next()) {
                if (tok2->str() == "(") {
                    structname.clear();
                    break;
                }

                if (tok2->str() == "}")
                    break;
            }

            // bail out if struct is inherited
            if (!structname.empty() && Token::findmatch(tok, (",|private|protected|public " + structname).c_str()))
                structname.clear();

            // Bail out if some data is casted to struct..
            const std::string s("( struct| " + tok->next()->str() + " * ) & %var% [");
            if (Token::findmatch(tok, s.c_str()))
                structname.clear();

            // Try to prevent false positives when struct members are not used directly.
            if (Token::findmatch(tok, (structname + " *").c_str()))
                structname.clear();
            else if (Token::findmatch(tok, (structname + " %type% *").c_str()))
                structname = "";
        }

        if (tok->str() == "}")
            structname.clear();

        if (!structname.empty() && Token::Match(tok, "[{;]")) {
            // Declaring struct variable..
            std::string varname;

            // declaring a POD variable?
            if (!tok->next()->isStandardType())
                continue;

            if (Token::Match(tok->next(), "%type% %var% [;[]"))
                varname = tok->strAt(2);
            else if (Token::Match(tok->next(), "%type% %type% %var% [;[]"))
                varname = tok->strAt(3);
            else if (Token::Match(tok->next(), "%type% * %var% [;[]"))
                varname = tok->strAt(3);
            else if (Token::Match(tok->next(), "%type% %type% * %var% [;[]"))
                varname = tok->strAt(4);
            else
                continue;

            // Check if the struct variable is used anywhere in the file
            const std::string usagePattern(". " + varname);
            bool used = false;
            for (const Token *tok2 = _tokenizer->tokens(); tok2; tok2 = tok2->next()) {
                if (Token::simpleMatch(tok2, usagePattern.c_str())) {
                    used = true;
                    break;
                }
            }

        }
    }
}



void MakeVariableFunctionTable::getGlobalVariableList(const SymbolDatabase* const symbolDatabase, VariableList& variables)
{
	//편집
	const Scope* scope=&*symbolDatabase->scopeList.begin();

    if (scope->type != Scope::eClass && scope->type != Scope::eUnion && scope->type != Scope::eStruct) 
	{
        // Find declarations
        for (std::list<Variable>::const_iterator i = scope->varlist.begin(); i != scope->varlist.end(); ++i) {
			//편집
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
            else if (_tokenizer->isC() || i->typeEndToken()->isStandardType() || isRecordTypeWithoutSideEffects(i->type()) || Token::simpleMatch(i->typeStartToken(), "std ::"))
                type = VariableList::standard;
            if (type == VariableList::none || isPartOfClassStructUnion(i->typeStartToken()))
                continue;
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