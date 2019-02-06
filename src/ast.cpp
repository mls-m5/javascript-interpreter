/*
 * ast.cpp
 *
 *  Created on: 14 okt. 2016
 *      Author: mattias
 */


#include "ast.h"
#include <iostream>

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
	PatternUnit(AstUnit::Type type): type(type), target(Type::None) {}
	PatternUnit(AstUnit::Type type, AstUnit::Type target, GroupingAction groupingAction = GroupStandard):
		type(type),
		target(target),
		group(groupingAction){}

	bool operator == (AstUnit& unit) {
		if (type) {
			if (type == unit.Identifier) {
				if (unit.type != unit.Word && unit.type != unit.Parenthesis && unit.type != unit.PropertyAccessor) {
					return false;
				}
			}
			else if (unit.type != type) {
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
	{{"while"}, WhileKeyword},
	{{"if"}, IfKeyword},
	{{"else"}, ElseKeyword},
	{{"function"}, FunctionKeyword},
	{{"true"}, Boolean},
	{{"false"}, Boolean},
	{{"new"}, NewKeyword},
	{{"let"}, LetKeyword},
	{{"var"}, VarKeyword},
	{{"return"}, Return},
	{{"this"}, This},
	{{"++", "--"}, PrefixOrPostfix}, //How to separate prefix from the postfix?
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
	{{";"}, Semicolon},
	{{"."}, Period},
	{{"NaN"}, Number},
	{{"Infinity"}, Number},
};


//Rules is taken from the description at:
//https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Operators/Operator_Precedence

//Defines the way lots of the expressions is grouped
vector<PatternRule> AstUnit::patterns = {
	{{FunctionKeyword, {Word, Name}, {Parenthesis, Arguments}, {Braces, Statement}}, Function},
	//{{SwitchKeyword, {Parenthesis, Arguments}, Braces}, Function}, //Uncomment when implementing
	{{FunctionKeyword, {Parenthesis, Arguments}, {Braces, Statement}}, Function},
	{{ForKeyword, {Parenthesis, Arguments}, {Braces, Statement}}, ForLoop},
	{{ForKeyword, {Parenthesis, Arguments}, Any}, ForLoop},
	{{WhileKeyword, {Parenthesis, Condition}, Any}, WhileLoop},
	{{IfKeyword, {Parenthesis, Condition}, Any}, IfStatement},
//	{{IfStatement, ElseKeyword, IfKeyword, {Parenthesis, Condition}, Any}, IfStatement}, //Is grouped as a else and a if statement
	{{IfStatement, ElseKeyword, Any}, IfStatement}, //Append else statement

	{{Any, Period, {Word, String}}, PropertyAccessor}, //19
	{{Any, {Bracket, AccessorBrackets}}, PropertyAccessor}, //19
	{{{Bracket, ArrayBrackets}}, Array},
	{{NewKeyword, Any, {Parenthesis, Arguments}}, NewStatement}, //19: new with arguments
	{{PropertyAccessor, {Parenthesis, Arguments}}, MethodCall}, //Precedence 19
	{{Identifier, {Parenthesis, Arguments}}, FunctionCall}, //Precence 19
	{{Function, {Parenthesis, Arguments}}, FunctionCall}, //Precence 19
	{{NewKeyword, Any}, NewStatement}, //Precence also 18
	{{Identifier, PrefixOrPostfix}, PostfixStatement}, //Precedence 17

	{{PrefixOrPostfix, Word}, PrefixStatement, PatternRule::RightToLeft}, //Precence 16
	{{PrefixOrPostfix, Parenthesis}, PrefixStatement, PatternRule::RightToLeft}, //Precence 16
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
	{{Any, AssignmentOperator, Any}, Assignment, PatternRule::RightToLeft}, //3

	//Object member
	{{Any, Colon, Any}, ObjectMemberDefinition},
//	{{Any, Any, Colon}, CaseLabel}, //Uncomment when implementing


	{{Any, Coma, Any}, Sequence}, //3

	//Variable declarations
	{{LetKeyword, {Word, Name}}, VariableDeclaration},
	{{LetKeyword, {Assignment}}, VariableDeclaration},
	{{VarKeyword, {Word, Name}}, VariableDeclaration},
	{{VarKeyword, {Assignment}}, VariableDeclaration},
	{{Return, Any}, ReturnStatement},
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
	int offset;

	enum class FunctionAction{
		None,
		Continue,
		BreakLoop,
		ChangeOffset,
	};

	auto f = [&offset, this] (vector<PatternUnit> &pattern, int pi) {
		bool match = true;
		for (size_t i = 0; i < pattern.size(); ++i) {
			if (children[i + offset]->type == Semicolon) {
				match = false;
				break;
			}
			else if (!(pattern[i] == *children[i + offset])) {
				match = false;
				break;
			}
		}

		if (match) {
			if ((offset == 0 && pattern.size() == children.size()) && patterns[pi].group == GroupStandard && type == GenericGroup) {
				auto newPattern = patterns[pi].second;

				if (newPattern == type) {
					//Prevent the algorithm from doing the same thing twice
//					continue;
					return FunctionAction::Continue;
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
				return FunctionAction::BreakLoop; //Nothing more to group
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
			return FunctionAction::ChangeOffset;
//			offset -= 1;
		}
		return FunctionAction::None;
	};

	for (size_t pi = 0; pi < patterns.size(); ++pi) {
		auto &rule = patterns[pi];
		auto &pattern = rule.first;
		//Do the pattern recognition in different directions dependent on associativity
		if (rule.associativity == rule.LeftToRight) {
			for (offset = 0; offset <= children.size() - pattern.size() && pattern.size() <= children.size(); ++offset) {
				auto action = f(pattern, pi);
				if (action == FunctionAction::Continue) {
					continue;
				}
				else if (action == FunctionAction::BreakLoop) {
					break;
				}
				else if (action == FunctionAction::ChangeOffset) {
					offset -= 1;
				}
			}
		}
		else {
			for (offset = children.size() - pattern.size(); offset >= 0 && pattern.size() <= children.size(); --offset) {
				auto action = f(pattern, pi);
				if (action == FunctionAction::Continue) {
					continue;
				}
				else if (action == FunctionAction::BreakLoop) {
					break;
				}
				else if (action == FunctionAction::ChangeOffset) {
					offset -= pattern.size();
					offset += 1;
				}
			}
		}
	}
}

void AstUnit::groupByParenthesis() {
	//Todo implement
	const std::string beginStrings[] = { "{", "(", "[" };
	const std::string endStrings[] = { "}", ")", "]" };
	const Type parenthesisTypes[] = { Braces, Parenthesis, Bracket };
	for (auto bracketIndex = 0; bracketIndex < 3; ++bracketIndex) {
		const std::string& beginString = beginStrings[bracketIndex];
		const std::string& endString = endStrings[bracketIndex];
		Type parenthesisType = parenthesisTypes[bracketIndex];
		for (int i = children.size() - 1; i >= 0; --i) {
			AstUnit& c = *children[i];
			if (c.type == Parenthesis && c.token == beginString) {
				for (size_t j = i + 1; j < children.size(); ++j) {
					AstUnit& c2 = *children[j];
					if (c2.type == Parenthesis && c2.token == endString) {
						auto ptr = new AstUnit();
						ptr->token = children[i]->token;
						ptr->endToken = children[j]->token;
						ptr->type = parenthesisType;
						auto it1 = children.begin() + i;
						auto it2 = children.begin() + j;
						ptr->children.insert(ptr->children.begin(), it1 + 1,
								it2);
						ptr->groupByParenthesis();
						children.erase(it1, it2 + 1);
						children.insert(it1, AstUnitPtr(ptr));
						break;
					}
				}
			}
		}
	}
}
