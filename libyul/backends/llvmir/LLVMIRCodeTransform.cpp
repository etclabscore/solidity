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

	return "";
}

llvm::Value* LLVMIRCodeTransform::generateMultiAssignment(
	vector<string> _variableNames,
	unique_ptr<llvm::Value*> _firstValue
)
{
}

llvm::Value* LLVMIRCodeTransform::operator()(VariableDeclaration const& _varDecl)
{
}

llvm::Value* LLVMIRCodeTransform::operator()(Assignment const& _assignment)
{
}

llvm::Value* LLVMIRCodeTransform::operator()(StackAssignment const&)
{
}

llvm::Value* LLVMIRCodeTransform::operator()(ExpressionStatement const& _statement)
{
}

llvm::Value* LLVMIRCodeTransform::operator()(Label const&)
{
	return {};
}

llvm::Value* LLVMIRCodeTransform::operator()(FunctionalInstruction const& _f)
{
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
	return make_unique<llvm::Value*>(boost::apply_visitor(*this, _expression));
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
}
