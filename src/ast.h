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


typedef std::shared_ptr<class AstUnit> AstUnitPtr;


class AstUnit: public std::enable_shared_from_this<AstUnit> {
public:
	enum Type {
		None,
		Any = None,
		Word,
		Digit,
		Paranthesis,
		Bracket,
		Braces,
		GenericGroup,
		Function,
		FunctionCall,
		ForLoop,
		DeclarationName,
		Assignment,

		Operator,
		Left,
		Right,

		FunctionKeyword,
		ForKeyword,
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
		case token.Paranthesis:
			type = Paranthesis;
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

	void groupByPatterns();

	void groupByParanthesis() {
		//Todo implement
		const std::string beginStrings[] = {"{", "(", "["};
		const std::string endStrings[] = {"}", ")", "]"};
		const Type paranthesisTypes[] = {Braces, Paranthesis, Bracket};

		for (auto bracketIndex = 0; bracketIndex < 3; ++bracketIndex) {
			const std::string &beginString = beginStrings[bracketIndex];
			const std::string &endString = endStrings[bracketIndex];
			Type paranthesisType = paranthesisTypes[bracketIndex];

			for (auto i = children.size() - 1; i > 0; --i) {
				AstUnit &c = *children[i];
				if (c.type == Paranthesis && c.token == beginString) {
					for (auto j = 0; j < children.size(); ++j) {
						AstUnit &c2 = *children[j];
						if (c2.type == Paranthesis && c2.token == endString) {
							auto ptr = new AstUnit();
							ptr->token = children[i]->token;
							ptr->endToken = children[j]->token;
							ptr->type = paranthesisType;
							auto it1 = children.begin() + i;
							auto it2 = children.begin() + j;
							ptr->children.insert(ptr->children.begin(), it1 + 1, it2);
							children.erase(it1, it2 + 1);
							children.insert(it1, AstUnitPtr(ptr));
						}
					}
				}
			}
		}
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
		groupByParanthesis();
		groupByPatterns();
		return *this;
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
	static std::vector<std::pair<std::vector<class PatternUnit>, Type> > patterns;
	static std::map<std::string, Type> keywordMap;
};
