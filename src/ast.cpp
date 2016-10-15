/*
 * ast.cpp
 *
 *  Created on: 14 okt. 2016
 *      Author: mattias
 */


#include "ast.h"

using namespace std;

typedef AstUnit::Type Type;

//A class for matching to different types of tokens
class PatternUnit {
public:
	Type type = AstUnit::None;
	Type target = AstUnit::None;
	string name;

	PatternUnit() = default;
	PatternUnit(const char *name, AstUnit::Type target = Type::None): name(name), target(target) {}
	PatternUnit(const string name, AstUnit::Type target = Type::None): name(name), target(target) {}
	PatternUnit(AstUnit::Type type): type(type), target(type) {}
	PatternUnit(AstUnit::Type type, AstUnit::Type target): type(type), target(target) {}

	bool operator == (AstUnit& unit) {
		if (type) {
			if (unit.type != type) {
				return false;
			}
		}
		if (!name.empty()) {
			if (unit.token != name) {
				return false;
			}
		}
		return true;
	}
};


//Check this
//https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Operators/Operator_Precedence
vector<pair<vector<PatternUnit>, Type> > AstUnit::patterns = {
	{{FunctionKeyword, {Word, DeclarationName}, {Paranthesis, Arguments}, Braces}, Function},
	{{FunctionKeyword, {Paranthesis, Arguments}, Braces}, Function},
	{{Word, Paranthesis}, FunctionCall},
	{{ForKeyword, Braces}, ForLoop},
	{{{Any}, "=", {Any}}, Assignment}, //Replace this by more generic BinaryOperator type instead of string
};

map<string, Type> AstUnit::keywordMap = {
	{"function", FunctionKeyword},
	{"for", ForKeyword},
};

AstUnit::Type AstUnit::getKeywordType(Token& token) {
	auto f = keywordMap.find(token);
	if (f == keywordMap.end()) {
		return Type::None;
	}
	else {
		return f->second;
	}
}

void AstUnit::groupByPatterns() {
	for (size_t pi = 0; pi < patterns.size(); ++pi) {
		auto &pattern = patterns[pi].first;
		for (size_t offset = 0; offset <= children.size() - pattern.size() && pattern.size() <= children.size(); ++offset) {
			bool match = true;
			for (size_t i = 0; i < pattern.size(); ++i) {
				if (!(pattern[i] == *children[i + offset])) {
					match = false;
					break;
				}
			}

			if (match) {
				if (offset == 0 && pattern.size() == children.size()) {
					type = patterns[pi].second;

					for (auto i = 0; i < pattern.size(); ++i) {
						auto target = pattern[i].target;
						if (target) {
							(*this)[offset + i].type = target;
						}
					}
				}
				else {
					auto unit = group(offset, offset + pattern.size(), patterns[pi].second);

					for (auto i = 0; i < pattern.size(); ++i) {
						auto target = pattern[i].target;
						if (target) {
							unit->children[i]->type = pattern[i].target;
						}
					}
				}
			}
		}
	}
}
