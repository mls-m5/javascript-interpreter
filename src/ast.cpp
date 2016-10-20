/*
 * ast.cpp
 *
 *  Created on: 14 okt. 2016
 *      Author: mattias
 */


#include "ast.h"

using namespace std;

typedef AstUnit::Type Type;

enum GroupingAction {
	GroupStandard,
	GroupExtra,
};

//A class for matching to different types of tokens
class PatternUnit {
public:
	Type type = AstUnit::None;
	Type target = AstUnit::None;
	string name;
	GroupingAction group = GroupStandard;

	PatternUnit() = default;
	PatternUnit(const char *name, AstUnit::Type target = Type::None, GroupingAction groupingAction = GroupStandard):
		name(name),
		target(target),
		group(groupingAction){}
	PatternUnit(const string name, AstUnit::Type target = Type::None, GroupingAction groupingAction = GroupStandard):
		name(name),
		target(target),
		group(groupingAction){}
	PatternUnit(AstUnit::Type type): type(type), target(type) {}
	PatternUnit(AstUnit::Type type, AstUnit::Type target, GroupingAction groupingAction = GroupStandard):
		type(type),
		target(target),
		group(groupingAction){}

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

class PatternRule: public pair<vector<PatternUnit>, Type> {
public:
	enum Associativity {
		LeftToRight,
		RightToLeft,
		None,
	};

	PatternRule(vector<PatternUnit> pattern, Type type, Associativity associativity = LeftToRight, GroupingAction groupAction = GroupStandard):
		associativity(associativity),
		group(groupAction){
		first = pattern;
		second = type;
	}

	Associativity associativity;
	GroupingAction group = GroupStandard;
};


vector<pair<set<string>, Type>> AstUnit::keywordMap {
	{{"for"}, ForKeyword},
	{{"function"}, FunctionKeyword},
	{{"new"}, NewKeyword},
	{{"let"}, LetKeyword},
	{{"var"}, VarKeyword},
	{{"++", "--"}, Postfix}, //How to separate prefix from the postfix
	{{"delete", "typeof", "void", "!"}, Prefix},
	{{"**"}, ExponentiationOperator},
	{{"*", "/", "%"}, ExponentiationOperator},
	{{"+", "-"}, AddSubtractOperators},
	{{"<<", ">>"}, BitwiseShiftOperators},
	{{"<", "<=", ">", ">=", "in", "instanceof"}, Operator11},
	{{"==", "!=", "===", "!=="}, EqualityOperator},
	{{"&"}, BitwiseAnd},
	{{"^"}, BitwiseXor},
	{{"|"}, BitwiseOr},
	{{"&&"}, And},
	{{"||"}, Or},
	{{"?"}, QuestionMark},
	{{":"}, Colon},
	{{"=", "+=", "-=", "**=", "*=", "/=", "%=", "<<=", ">>=", ">>>=", "&=", "^=", "!="}, AssignmentOperator},
	{{","}, Coma},
};


//Rules is taken from the description at:
//https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Operators/Operator_Precedence

//Defines the way lots of the expressions is grouped
vector<PatternRule> AstUnit::patterns = {
	{{FunctionKeyword, {Word, DeclarationName}, {Paranthesis, Arguments}, Braces}, Function},
	{{FunctionKeyword, {Paranthesis, Arguments}, Braces}, Function},
	{{ForKeyword, Paranthesis, Braces}, ForLoop},

	{{Any, Period, Any}, MemberAccess}, //19
	{{Any, Bracket}, MemberAccess}, //19
	{{NewKeyword, Any, {Paranthesis, Arguments}}, NewStatement}, //19: new with arguments
	{{Any, {Paranthesis, Arguments}}, FunctionCall}, //Precence 18
	{{NewKeyword, Any}, NewStatement}, //Precence also 18
	{{Any, Postfix}, PostfixStatement}, //Precedence 17
	{{Prefix, Any}, PrefixStatement, PatternRule::RightToLeft}, //Precence 16
	{{Any, ExponentiationOperator, Any}, BinaryStatement}, //15
	{{Any, Operator14, Any}, BinaryStatement}, //14
	{{Any, AddSubtractOperators, Any}, BinaryStatement}, //13
	{{Any, BitwiseShiftOperators, Any}, BinaryStatement}, //12
	{{Any, Operator11, Any}, BinaryStatement}, //11
	{{Any, EqualityOperator, Any}, BinaryStatement}, //10
	{{Any, BitwiseAnd, Any}, BinaryStatement}, //9
	{{Any, BitwiseXor, Any}, BinaryStatement}, //8
	{{Any, BitwiseOr, Any}, BinaryStatement}, //7
	{{Any, And, Any}, BinaryStatement}, //6
	{{Any, Or, Any}, BinaryStatement}, //5
	{{Any, QuestionMark, Any, Colon, Any}, Conditional, PatternRule::RightToLeft}, //4
	{{Any, AssignmentOperator, Any}, BinaryStatement, PatternRule::RightToLeft}, //3
	{{Any, Coma, Any}, Sequence, PatternRule::LeftToRight, GroupExtra}, //3

	//Variable declarations
	{{LetKeyword, {Word, Name}}, VariableDeclaration},
	{{LetKeyword, {Word, Name}, "=", Any}, VariableDeclaration},
	{{VarKeyword, {Word, Name}}, VariableDeclaration},
	{{VarKeyword, {Word, Name}, "=", Any}, VariableDeclaration},
};

AstUnit::Type AstUnit::getKeywordType(Token& token) {
	for (auto &it: keywordMap) {
		if (it.first.find(token) != it.first.end()) {
			return it.second;
		}
	}
	return Type::None;
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
				if ((offset == 0 && pattern.size() == children.size()) && patterns[pi].group == GroupStandard) {
					auto newPattern = patterns[pi].second;

					if (newPattern == type) {
						//Prevent the algorithm from doing the same thing twice
						continue;
					}
					type = newPattern;

					for (auto i = 0; i < pattern.size(); ++i) {
						auto target = pattern[i].target;
						if (target) {
							auto &unit = (*this)[offset + i];
							unit.type = target;
							unit.groupUnit();
						}
					}
				}
				else {
					auto unit = group(offset, offset + pattern.size(), patterns[pi].second);

					for (auto i = 0; i < pattern.size(); ++i) {
						auto target = pattern[i].target;
						auto groupAction = pattern[i].group;
						if (target) {
							if (groupAction == GroupExtra) {
								//Encapsulate the unit in a new Ast unit to save their types
								auto tmpChild = unit->children[i];
								auto newChild = AstUnitPtr(new AstUnit());
								newChild->type = pattern[i].target;
								newChild->children.push_back(tmpChild);
								unit->children[i] = newChild;
							}
							else {
								unit->children[i]->type = pattern[i].target;
								unit->children[i]->groupUnit();
							}
						}
					}
				}
			offset -= 1;
			}
		}
	}
}
