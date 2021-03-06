/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 1999-2000 Harri Porten (porten@kde.org)
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 *  Boston, MA 02111-1307, USA.
 */

#ifndef _INTERNAL_H_
#define _INTERNAL_H_

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "kjs.h"
#include "object.h"
#include "function.h"

namespace KJS {

  class Boolean;
  class Number;
  class String;
  class Object;
  class RegExp;
  class Node;
  class ProgramNode;
#ifdef KJS_DEBUGGER
  class Debugger;
#endif

  class UndefinedImp : public Imp {
  public:
    UndefinedImp();
    virtual ~UndefinedImp() { }
    virtual KJSO toPrimitive(Type preferred = UndefinedType) const;
    virtual Boolean toBoolean() const;
    virtual Number toNumber() const;
    virtual String toString() const;
    virtual Object toObject() const;

    virtual const TypeInfo* typeInfo() const { return &info; }
    static const TypeInfo info;
  };

  class NullImp : public Imp {
  public:
    NullImp();
    virtual ~NullImp() { }
    virtual KJSO toPrimitive(Type preferred = UndefinedType) const;
    virtual Boolean toBoolean() const;
    virtual Number toNumber() const;
    virtual String toString() const;
    virtual Object toObject() const;

    virtual const TypeInfo* typeInfo() const { return &info; }
    static const TypeInfo info;
  };

  class BooleanImp : public Imp {
  public:
    virtual ~BooleanImp() { }
    BooleanImp(bool v = false) : val(v) { }
    bool value() const { return val; }
    virtual KJSO toPrimitive(Type preferred = UndefinedType) const;
    virtual Boolean toBoolean() const;
    virtual Number toNumber() const;
    virtual String toString() const;
    virtual Object toObject() const;

    virtual const TypeInfo* typeInfo() const { return &info; }
    static const TypeInfo info;
  private:
    bool val;
  };

  class NumberImp : public Imp {
  public:
    NumberImp(double v);
    virtual ~NumberImp() { }
    double value() const { return val; }
    virtual KJSO toPrimitive(Type preferred = UndefinedType) const;
    virtual Boolean toBoolean() const;
    virtual Number toNumber() const;
    virtual String toString() const;
    virtual Object toObject() const;

    virtual const TypeInfo* typeInfo() const { return &info; }
    static const TypeInfo info;
  private:
    double val;
  };

  class StringImp : public Imp {
  public:
    StringImp(const UString& v);
    virtual ~StringImp() { }
    UString value() const { return val; }
    virtual KJSO toPrimitive(Type preferred = UndefinedType) const;
    virtual Boolean toBoolean() const;
    virtual Number toNumber() const;
    virtual String toString() const;
    virtual Object toObject() const;

    virtual const TypeInfo* typeInfo() const { return &info; }
    static const TypeInfo info;
  private:
    UString val;
  };

  class ReferenceImp : public Imp {
  public:
    ReferenceImp(const KJSO& b, const UString& p);
    virtual ~ReferenceImp() { }
    virtual void mark(Imp*);
    KJSO getBase() const { return base; }
    UString getPropertyName() const { return prop; }

    virtual const TypeInfo* typeInfo() const { return &info; }
    static const TypeInfo info;
  private:
    KJSO base;
    UString prop;
  };

  class CompletionImp : public Imp {
  public:
    CompletionImp(Compl c, const KJSO& v, const UString& t);
    virtual ~CompletionImp() { }
    virtual void mark(Imp*);
    Compl completion() const { return comp; }
    KJSO value() const { return val; }
    UString target() const { return tar; }

    virtual const TypeInfo* typeInfo() const { return &info; }
    static const TypeInfo info;
  private:
    Compl comp;
    KJSO val;
    UString tar;
  };

  class RegExpImp : public ObjectImp {
  public:
    RegExpImp();
    ~RegExpImp();
    void setRegExp(RegExp *r) { reg = r; }
    RegExp* regExp() const { return reg; }
  private:
    RegExp *reg;
  };

  class StatementNode;
  class UString;

  class Reference : public KJSO {
  public:
    Reference(const KJSO& b, const UString &p);
    virtual ~Reference();
  };

  /**
   * @short The "label set" in Ecma-262 spec
   */
  class LabelStack {
  public:
    LabelStack(): tos(0L) {}
    ~LabelStack();

    /**
     * If id is not empty and is not in the stack already, puts it on top of
     * the stack and returns true, otherwise returns false
     */
    bool push(const UString &id);
    /**
     * Is the id in the stack?
     */
    bool contains(const UString &id) const;
    /**
     * Removes from the stack the last pushed id (what else?)
     */
    void pop();
  private:
    struct StackElm {
      UString id;
      StackElm *prev;
    };

    StackElm *tos;
  };

  /**
   * @short Execution context.
   */
  class Context {
  public:
    Context(CodeType type = GlobalCode, Context *callingContext = 0L,
	       FunctionImp *func = 0L, const List *args = 0L, Imp *thisV = 0L);
    virtual ~Context();
    void mark();
    static Context *current();
    static void setCurrent(Context *c);
    const List *pScopeChain() const { return scopeChain; }
    void pushScope(const KJSO &s);
    void popScope();
    List *copyOfChain();
    KJSO variableObject() const { return variable; }
    Imp* thisValue() const { return thisVal; }
    void setThisValue(Imp *t) { thisVal = t; }
    LabelStack *seenLabels() { return &ls; }
  private:
    LabelStack ls;
    Imp *thisVal;
    KJSO activation;
    KJSO variable;
    List *scopeChain;
  };

  class DeclaredFunctionImp : public ConstructorImp {
  public:
    DeclaredFunctionImp(const UString &n, StatementNode *b);
    Completion execute(const List &);
    Object construct(const List &);
    CodeType codeType() const { return FunctionCode; }
  private:
    StatementNode *block;
  };

  class AnonymousFunction : public Function {
  public:
    AnonymousFunction();
    Completion execute(const List &);
    CodeType codeType() const { return AnonymousCode; }
  };

  class ActivationImp : public Imp {
  public:
    ActivationImp(FunctionImp *f, const List *args);
    virtual ~ActivationImp();
    virtual const TypeInfo* typeInfo() const { return &info; }
    static const TypeInfo info;
  private:
    FunctionImp *func;
  };

  class KJScriptImp {
    friend class ::KJScript;
    friend class Lexer;
    friend class Context;
    friend class Global;
    friend class Collector;
  public:
    KJScriptImp();
    ~KJScriptImp();
    void mark();
    static KJScriptImp *current() { return curr; }
    static void setException(Imp *e);
    static void setException(const char *msg);      
    static bool hadException();
    static KJSO exception();
    static void clearException();

#ifdef KJS_DEBUGGER
    /**
     * Attach debugger d to this engine. If there already was another instance
     * attached it will be detached.
     */
    void attachDebugger(Debugger *d);
    Debugger *debugger() const { return dbg; }
    int sourceId() const { return sid; }
    bool setBreakpoint(int id, int line, bool set);
#endif
  private:
    /**
     * Initialize global object and context. For internal use only.
     */
    void init();
    void clear();
    bool evaluate(const UChar *code, unsigned int length, Imp *thisV = 0,
		  bool onlyCheckSyntax = false);
    bool call(const UString &func, const List &args);
  public:
    ProgramNode *progNode;
    Node *firstNode;
    KJScriptImp *next, *prev;
  private:
    static KJScriptImp *curr, *hook;
    bool initialized;
    Lexer *lex;
    Context *con;
    Global glob;
    int errType, errLine;
    UString errMsg;
#ifdef KJS_DEBUGGER
    Debugger *dbg;
    int sid;
#endif
    const char *exMsg;
    Imp *exVal;
    Imp *retVal;
    int recursion;
  };

  inline bool KJScriptImp::hadException()
  {
    assert(curr);
    return curr->exMsg;
  }
  
  /**
   * @short Struct used to return the property names of an object
   */
  class PropList {
  public:
    PropList(UString nm = UString::null, PropList *nx = 0) :
			  name(nm), next(nx) {};
    ~PropList() {
      if(next) delete next;
    }
    /**
     * The property name
     */
    UString name;
    /**
     * The next property
     */
    PropList *next;
    bool contains(const UString &name);
  };

}; // namespace


#endif
