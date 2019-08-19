/*
rThis file is part of solidity.

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

#include <libyul/backends/llvmir/LLVMIRCodeTransform.h>

#include <libyul/optimiser/NameCollector.h>

#include <libyul/AsmData.h>
#include <libyul/Dialect.h>
#include <libyul/Utilities.h>
#include <libyul/Exceptions.h>

#include <liblangutil/Exceptions.h>

#include <boost/range/adaptor/reversed.hpp>
#include <boost/range/adaptor/transformed.hpp>

using namespace std;
using namespace dev;
using namespace yul;

string LLVMIRCodeTransform::run(Dialect const& _dialect, yul::Block const& _ast)
{
	LLVMIRCodeTransform transform(_dialect, _ast);

	for (auto const& s: _ast.statements) {
		yulAssert(s.type() == typeid(yul::FunctionDefinition),
		"Expected only function definitions at highest level"
		);
		if (s.type() == typeid(yul::FunctionDefinition)) {
			transform.translateFunction(boost::get<yul::FunctionDefinition>(s));
		}
		// TODO: Insert builtin funcs
	}
	//return &transform.m_module; // FIXME: return properly
}

// Creates a basic block containing a set of allocas + stores to append to another block
llvm::Value* LLVMIRCodeTransform::operator()(VariableDeclaration const& _varDecl)
{
	llvm::BasicBlock* ret = llvm::BasicBlock::Create(m_context);
	m_builder.SetInsertPoint(ret);
	if (_varDecl.value) {
		llvm::Value* init_expr = *LLVMIRCodeTransform::visit(*_varDecl.value);

		for (auto const& var: _varDecl.variables) {
			// NOTE: Right now we do not check that init_expr returns multiple values.
			// TODO: Fix this
			m_builder.CreateLoad(init_expr->getType(), &*init_expr);
		}
	} else {
		for (auto const& var: _varDecl.variables) {
			m_builder.CreateAlloca(m_types.from_yul(var.type));
		}
	}

	return ret;
}

llvm::Value* LLVMIRCodeTransform::operator()(Assignment const& _assignment)
{
	// NOTE: Assumes caller has positioned the builder correctly.
	llvm::Value* init_expr = *LLVMIRCodeTransform::visit(*_assignment.value);
	//m_builder.CreateLoad(
}

llvm::Value* LLVMIRCodeTransform::operator()(StackAssignment const&)
{
	yulAssert(false, "");
	return {};
}

llvm::Value* LLVMIRCodeTransform::operator()(ExpressionStatement const& _statement)
{
	return visitReturnByValue(_statement.expression);
}

llvm::Value* LLVMIRCodeTransform::operator()(Label const&)
{
	yulAssert(false, "");
	return {};
}

llvm::Value* LLVMIRCodeTransform::operator()(FunctionalInstruction const& _f)
{
	yulAssert(false, "EVM instruction in LLVM code: " + eth::instructionInfo(_f.instruction).name);
	return {};
}

llvm::Value* LLVMIRCodeTransform::operator()(FunctionCall const& _call)
{
	return {};
}

llvm::Value* LLVMIRCodeTransform::operator()(Identifier const& _identifier)
{
	return {};
}

llvm::Value* LLVMIRCodeTransform::operator()(Literal const& _literal)
{
	return {};
}

llvm::Value* LLVMIRCodeTransform::operator()(yul::Instruction const&)
{
	yulAssert(false, "EVM instruction used for LLVM codegen"
	return {};
}

llvm::Value* LLVMIRCodeTransform::operator()(If const& _if)
{
	return {};
}

llvm::Value* LLVMIRCodeTransform::operator()(Switch const& _switch)
{
	return {};
}

llvm::Value* LLVMIRCodeTransform::operator()(FunctionDefinition const&)
{
	yulAssert(false, "Should not have visited here.");
	return {};
}

llvm::Value* LLVMIRCodeTransform::operator()(ForLoop const& _for)
{

	return { };
}

llvm::Value* LLVMIRCodeTransform::operator()(Break const&)
{
	return {};
}

llvm::Value* LLVMIRCodeTransform::operator()(Continue const&)
{
	return {}; 
}

llvm::Value* LLVMIRCodeTransform::operator()(Block const& _block)
{
	return {};
}

unique_ptr<llvm::Value*> LLVMIRCodeTransform::visit(yul::Expression const& _expression)
{
	return std::make_unique<llvm::Value*>(boost::apply_visitor(*this, _expression));
}

llvm::Value* LLVMIRCodeTransform::visitReturnByValue(yul::Expression const& _expression)
{
	return boost::apply_visitor(*this, _expression);
}

vector<llvm::Value*> LLVMIRCodeTransform::visit(vector<yul::Expression> const& _expressions)
{
	vector<llvm::Value*> ret;
	for (auto const& e: _expressions)
		ret.emplace_back(visitReturnByValue(e));
	return ret;
}

llvm::Value* LLVMIRCodeTransform::visit(yul::Statement const& _statement)
{
	return boost::apply_visitor(*this, _statement);
}

vector<llvm::Value*> LLVMIRCodeTransform::visit(vector<yul::Statement> const& _statements)
{
	vector<llvm::Value*> ret;
	for (auto const& s: _statements)
		ret.emplace_back(visit(s));
	return ret;
}

void LLVMIRCodeTransform::translateFunction(yul::FunctionDefinition const& _fun)
{

}

void LLVMIRCodeTransform::allocateGlobals(size_t _amount)
{
	// while (m_globalVariables.size() < _amount) {
	// 	m_globalVariables.emplace_back(
	// }
}
