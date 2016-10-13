/*
 * ast.h
 *
 *  Created on: 13 okt. 2016
 *      Author: mattias
 */

#pragma once

#include "lexer.h"
#include <memory>


typedef std::shared_ptr<class AstUnit> AstUnitPtr;

class AstUnit: public std::enable_shared_from_this<AstUnit> {
public:
	enum Type {
		None,
		Word,
		Digit,
		Paranthesis,
		Bracket,
		Braces,
		GenericGroup,
		FunctionCall,
	};

	AstUnit() {
	}

	AstUnit(Token &token): token(token) {
		switch (token.type) {
		case token.Paranthesis:
			type = Paranthesis;
		break;
		}
	}

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
		if (!children.empty()) {
			out << ":";
		}
		out << std::endl;
		for (auto &it: children) {
			it->print(out, intent + 1);
		}
	}

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

	AstUnit &operator = (std::vector<Token> &tokens) {
		for (auto &it: tokens) {
			children.push_back(std::shared_ptr<AstUnit>(new AstUnit(it)));
		}
		type = GenericGroup;
		groupByParanthesis();
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
};



