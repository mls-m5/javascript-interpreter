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



vector<pair<vector<PatternUnit>, Type> > AstUnit::patterns = {
	{{FunctionKeyword, {Word, DeclarationName}, Paranthesis, Braces}, Function},
	{{FunctionKeyword, Paranthesis, Braces}, Function},
	{{Word, Paranthesis}, FunctionCall},
	{{ForKeyword, Braces}, ForLoop},
	{{{Any}, "=", {Any}}, Assignment},
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
		//Om size_t blir mindre än 0 så blir det talet i andra änden av skalan
		//Se till att det inte blir mindre än noll (ta kanske bort -1
		if (pattern.size() > children.size()) {
			continue;
		}
		for (size_t offset = 0; offset <= children.size() - pattern.size(); ++offset) {
			if (pattern.size() > children.size()) {
				break;
			}
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
