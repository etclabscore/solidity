/*
	This file is part of solidity.

	solidity is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	solidity is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with solidity.  If not, see <http://www.gnu.org/licenses/>.
*/
/**
 * Common code generator for translating Yul / inline assembly to LLVMIR.
 */

#pragma once

#include <llvm/IR/Value.h>

#include <libyul/AsmDataForward.h>
#include <libyul/Dialect.h>
#include <libyul/optimiser/NameDispenser.h>

#include <stack>
#include <map>

namespace yul
{
struct AsmAnalysisInfo;

class LLVMIRCodeTransform: public boost::static_visitor<llvm::Value*>
{
public:
	static std::string run(Dialect const& _dialect, yul::Block const& _ast);

public:
	llvm::Value* operator()(yul::Instruction const& _instruction);
	llvm::Value* operator()(yul::Literal const& _literal);
	llvm::Value* operator()(yul::Identifier const& _identifier);
	llvm::Value* operator()(yul::FunctionalInstruction const& _instr);
	llvm::Value* operator()(yul::FunctionCall const&);
	llvm::Value* operator()(yul::ExpressionStatement const& _statement);
	llvm::Value* operator()(yul::Label const& _label);
	llvm::Value* operator()(yul::StackAssignment const& _assignment);
	llvm::Value* operator()(yul::Assignment const& _assignment);
	llvm::Value* operator()(yul::VariableDeclaration const& _varDecl);
	llvm::Value* operator()(yul::If const& _if);
	llvm::Value* operator()(yul::Switch const& _switch);
	llvm::Value* operator()(yul::FunctionDefinition const&);
	llvm::Value* operator()(yul::ForLoop const&);
	llvm::Value* operator()(yul::Break const&);
	llvm::Value* operator()(yul::Continue const&);
	llvm::Value* operator()(yul::Block const& _block);

private:
	LLVMIRCodeTransform(
		Dialect const& _dialect,
		Block const& _ast
	):
		m_dialect(_dialect)
	{}

	std::unique_ptr<llvm::Value*> visit(yul::Expression const& _expression);
	llvm::Value* visitReturnByValue(yul::Expression const& _expression);
	std::vector<llvm::Value*> visit(std::vector<yul::Expression> const& _expressions);
	llvm::Value* visit(yul::Statement const& _statement);
	std::vector<llvm::Value*> visit(std::vector<yul::Statement> const& _statements);

	/// Returns an assignment or a block containing multiple assignments.
	/// @param _variableNames the names of the variables to assign to
	/// @param _firstValue the value to be assigned to the first variable. If there
	///        is more than one variable, the values are taken from m_globalVariables.
	llvm::Value* generateMultiAssignment(
		std::vector<std::string> _variableNames,
		std::unique_ptr<llvm::Value*> _firstValue
	);

	void translateFunction(yul::FunctionDefinition const& _funDef);

	/// Makes sure that there are at least @a _amount global variables.
	void allocateGlobals(size_t _amount);

	Dialect const& m_dialect;

	std::vector<llvm::Value*> m_localVariables;
	std::vector<llvm::Value*> m_globalVariables;
	std::map<YulString, llvm::Value*> m_functionsToImport;
	std::stack<std::pair<std::string, std::string>> m_breakContinueLabelNames;
};

}
