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

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/Twine.h>

#include <libyul/AsmDataForward.h>
#include <libyul/Dialect.h>
#include <libyul/optimiser/NameDispenser.h>
#include <libyul/Exceptions.h>

#include <stack>
#include <map>

namespace yul
{
struct AsmAnalysisInfo;

/*
 * Simple provider for YUL types.
 */
class YulTypeProvider {
public:
	YulTypeProvider(llvm::LLVMContext & context):
		m_bool(llvm::IntegerType::get(context, 1)),
		m_u8(llvm::IntegerType::get(context, 8)),
		m_s8(llvm::IntegerType::get(context, 8)),
		m_u32(llvm::IntegerType::get(context, 32)),
		m_s32(llvm::IntegerType::get(context, 32)),
		m_u64(llvm::IntegerType::get(context, 64)),
		m_s64(llvm::IntegerType::get(context, 64)),
		m_u128(llvm::IntegerType::get(context, 128)),
		m_s128(llvm::IntegerType::get(context, 128)),
		m_u256(llvm::IntegerType::get(context, 256)),
		m_s256(llvm::IntegerType::get(context, 256))
	{
	}

	llvm::IntegerType* from_yul(YulString const& type) {
		if (type.str() == std::string("bool"))
			return bool_type();
		if (type.str() == std::string("u8")) 
			return u8_type();
		if (type.str() == std::string("s8"))
			return s8_type();
		if (type.str() == std::string("u32"))
			return u32_type();
		if (type.str() == std::string("s32"))
			return s32_type();
		if (type.str() == std::string("u64"))
			return u64_type();
		if (type.str() == std::string("s64"))
			return s64_type();
		if (type.str() == std::string("u128"))
			return u128_type();
		if (type.str() == std::string("s128"))
			return s128_type();
		if (type.str() == std::string("u256"))
			return u256_type();
		if (type.str() == std::string("s256"))
			return s256_type();
		yulAssert(false, "Invalid type");
	}

	llvm::IntegerType* bool_type() {
		return m_bool;
	}

	llvm::IntegerType* u8_type() {
		return m_u8;
	}

	llvm::IntegerType* s8_type() {
		return m_s8;
	}

	llvm::IntegerType* u32_type() {
		return m_u32;
	}

	llvm::IntegerType* s32_type() {
		return m_s32;
	}

	llvm::IntegerType* u64_type() {
		return m_u64;
	}

	llvm::IntegerType* s64_type() {
		return m_s64;
	}

	llvm::IntegerType* u128_type() {
		return m_u128;
	}

	llvm::IntegerType* s128_type() {
		return m_s128;
	}

	llvm::IntegerType* u256_type() {
		return m_u256;
	}

	llvm::IntegerType* s256_type() {
		return m_s256;
	}
private:
	// NOTE: Because LLVM does not distinguish between signed and unsigned integers, there is no difference here.
	// These are only distinguished for readability. If an integer type of some width was already instantiated, then it will
	// return the same pointer. 
	llvm::IntegerType* m_bool;
	llvm::IntegerType* m_u8;
	llvm::IntegerType* m_s8;
	llvm::IntegerType* m_u32;
	llvm::IntegerType* m_s32;
	llvm::IntegerType* m_u64;
	llvm::IntegerType* m_s64;
	llvm::IntegerType* m_u128;
	llvm::IntegerType* m_s128;
	llvm::IntegerType* m_u256;
	llvm::IntegerType* m_s256;
};

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
		m_dialect(_dialect),
		m_context(),
		m_module(llvm::StringRef(std::string("yul")), m_context),
		m_builder(m_context),
		m_types(m_context)
	{
	}

	std::unique_ptr<llvm::Value*> visit(yul::Expression const& _expression);
	llvm::Value* visitReturnByValue(yul::Expression const& _expression);
	std::vector<llvm::Value*> visit(std::vector<yul::Expression> const& _expressions);
	llvm::Value* visit(yul::Statement const& _statement);
	std::vector<llvm::Value*> visit(std::vector<yul::Statement> const& _statements);

	/// Appends assignments to the current block.
	/// @param _firstValue the value to be assigned to the first variable. If there
	///        is more than one variable, the values are taken from m_globalVariables.
	void generateMultiAssignment(
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
	
	llvm::LLVMContext m_context;
	llvm::Module m_module;
	llvm::IRBuilder<> m_builder;
	YulTypeProvider m_types;
};

}
