/*
 * ast.h
 *
 *  Created on: 13 okt. 2016
 *      Author: mattias
 */

#pragma once

#include "lexer.h"
#include <memory>
#include <map>
#include <set>

typedef std::shared_ptr<class AstUnit> AstUnitPtr;


class AstUnit: public std::enable_shared_from_this<AstUnit> {
public:
	enum Type {
		None,
		Any = None,
		UsedKeyword, //A keyword that is used in a rule, and should not be matched again
		Word,
		GenericGroup,
		Number,
		String,
		Boolean,
		Parenthesis,
		Bracket,
		Braces,
		Statement, //Like braces, but you know from the context that is is not a object eg if() {}
		Identifier, //Phony type that means Word, PropertyAccessor or Paranthesis
		Return,
		This,

		Function,
		Arguments,
		Condition,
		FunctionCall,
		MethodCall, //Function call for object members
		NewStatement,
		ForLoop,
		WhileLoop,
		IfStatement,
		DeclarationName,
		Name,
		Assignment,
		Sequence,
		Conditional,
		Array,
		ArrayBrackets,    //These are so that a expression should not be matched both as a
		AccessorBrackets, //array and a accessor
		PropertyAssignment,

		Left,
		Right,

		PropertyAccessor,
		ComputedMemberAccess,
		PostfixStatement, //i++ etc
		PrefixStatement, //--i etc
		BinaryStatement,
		DeleteStatement,
		ExponentiationStatement,
		ObjectMemberDefinition, //with colon, for example {x: 3}
//		CaseLabel, //for use in switch statements

		Period,
		NewKeyword, //18 with function call
		PrefixOrPostfix, //17
		Prefix, //16
		DeleteKeyword, //16
		ExponentiationOperator, //15
		Operator14, //Multiplication division mod
		AddSubtractOperators, //13
		BitwiseShiftOperators, //12
		Operator11, //< in .. etc
		EqualityOperator, //10
		BitwiseAnd, //0
		BitwiseXor, //8
		BitwiseOr, //7
		And, //6
		Or, //5
		QuestionMark, //
		Colon,
		AssignmentOperator, //Precedence 3
		//Yield: 2
		//Spread: Precedence 1
		Coma, //Precedence 0

		Semicolon,


		VariableDeclaration,
		ReturnStatement,

		FunctionKeyword,
		ForKeyword,
		WhileKeyword,
		IfKeyword,
		ElseKeyword,
		LetKeyword,
		VarKeyword,
//		abstract
//		arguments
//		boolean
//		break	byte
//		case	catch	char	class*	const
//		continue	debugger	default	delete	do
//		double	else	enum*	eval	export*
//		extends*	false	final	finally	float
//		goto	if	implements
//		import*	in	instanceof	int	interface
//		let	long	native	new	null
//		package	private	protected	public	return
//		short	static	super*	switch	synchronized
//		this	throw	throws	transient	true
//		try	typeof	var	void	volatile
//		while	with	yield
	};

	AstUnit() {
	}

	AstUnit(Token &token): token(token) {
		switch (token.type) {
		case token.Word:
			type = Word;
		break;
		case token.StringLiteral:
			type = String;
		break;
		case token.Paranthesis:
			type = Parenthesis;
		return;
		case token.Number:
			type = Number;
		return;
		}
		auto t = getKeywordType(token);
		if (t != None) {
			type = t;
		}
	}

	static Type getKeywordType(Token& token);

	AstUnit(std::string text) {
		SimpleLexer lexer;
		auto tokens = lexer.tokenize(text);
		*this = tokens;
	}

	AstUnit(std::istream &stream) {
		SimpleLexer lexer;
		auto tokens = lexer.tokenize(stream);
		*this = tokens;
	}

	void print(std::ostream &out, int intent = 0) {
		for (int i = 0; i < intent; ++i) {
			out << "	";
		}
		out << "'" << token;
		if (!endToken.empty()) {
			out << endToken;
		}
		out << "'";
		if (type != None) {
			out << " - " << type;
		}
		if (!children.empty()) {
			out << ":";
		}
		out << std::endl;
		for (auto &it: children) {
			it->print(out, intent + 1);
		}
	}

	Token createToken() {
		Token t = token;

		for (auto &it: children) {
			t += it->createToken();
		}

		t += endToken;

		return t;
	}

	void groupByPatterns();

	void groupByParenthesis();

	//Get a child unit by type
	AstUnit *getByType(Type type) {
		for (auto &it: children) {
			if (it->type == type) {
				return &*it;
			}
		}
		return nullptr;
	}

	//Returns the first child that is not of sequence type
	Type getFirstSequenceType() {
		if (type != Sequence) {
			return type;
		}
		return front().getFirstSequenceType();
	}

	//Summarizes the sequence to a plain list
	std::vector<AstUnitPtr> getFlatSequence() {
		std::vector<AstUnitPtr> ret;
		if (type == Sequence) {
			auto l1 = front().getFlatSequence();
			ret.insert(ret.end(), l1.begin(), l1.end());
			auto l2 = back().getFlatSequence();
			ret.insert(ret.end(), l2.begin(), l2.end());
		}
		else {
			ret.insert(ret.end(), shared_from_this());
		}
		return ret;
	}

	//Get the statement after a special statement
	//for example the statement after the keyword else
	AstUnit *getAfterToken(Type type) {
		bool selectNext = false;
		for (auto &it: children) {
			if (it->type == type) {
				selectNext = true;
			}
			else if (selectNext) {
				return &*it;
			}
		}
		return nullptr;
	}

	AstUnit *group(size_t begin, size_t end, Type t) {
		auto it1 = children.begin() + begin;
		auto it2 = children.begin() + end;

		auto unit = new AstUnit();
		unit->type = t;
		unit->children.insert(unit->children.begin(), it1, it2);
		children.erase(it1, it2);
		children.insert(it1, AstUnitPtr(unit));
		return unit;
	}

	AstUnit &operator = (std::vector<Token> &tokens) {
		for (auto &it: tokens) {
			children.push_back(std::shared_ptr<AstUnit>(new AstUnit(it)));
		}
		type = GenericGroup;
		groupUnit();
		return *this;
	}

	void groupUnit() {
		if (children.empty()) {
			return;
		}
		if (children.size() == 1) {
			(*this)[0].groupUnit();
		}
		if (type != GenericGroup && type != Parenthesis && type != Statement && type != Braces && type != Condition && type != Arguments) {
			return; //The unit is already grouped
		}
		groupByParenthesis();
		groupByPatterns();
	}

	AstUnit(std::vector<Token> &tokens) {
		*this = tokens;
	}

	AstUnitPtr getPtr() {
		return shared_from_this();
	}

	bool operator == (std::string &text) {
		return text == token;
	}

	AstUnit &operator [] (size_t index) {
		return *children[index];
	}

	AstUnit &get(size_t index) {
		return *children[index];
	}

	AstUnit &at(size_t index) {
		return *children.at(index);
	}

	AstUnit &front() {
		return *children.front();
	}

	AstUnit &back() {
		return *children.back();
	}

	bool empty() {
		return children.empty();
	}

	size_t size() {
		return children.size();
	}

	std::vector<AstUnitPtr>::iterator begin() {
		return children.begin();
	}

	std::vector<AstUnitPtr>::iterator end() {
		return children.end();
	}

	Token token;
	Token endToken;
	Type type = None;
	std::vector<AstUnitPtr> children;

	//Defined here to skip prefix to enum values:
	static std::vector<std::pair<std::set<std::string>, Type>> keywordMap;
	static std::vector<class PatternRule > patterns;
};
