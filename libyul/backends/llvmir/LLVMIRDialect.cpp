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
 * Dialects for LLVMIR.
 */

#include <libyul/backends/llvmir/LLVMIRDialect.h>

using namespace std;
using namespace yul;

LLVMIRDialect::LLVMIRDialect():
	Dialect{AsmFlavour::Strict}
{

	addEthereumExternals();
}

BuiltinFunction const* LLVMIRDialect::builtin(YulString _name) const
{
	auto it = m_functions.find(_name);
	if (it != m_functions.end())
		return &it->second;
	else
		return nullptr;
}

LLVMIRDialect const& LLVMIRDialect::instance()
{
	static std::unique_ptr<LLVMIRDialect> dialect;
	static YulStringRepository::ResetCallback callback{[&] { dialect.reset(); }};
	if (!dialect)
		dialect = make_unique<LLVMIRDialect>();
	return *dialect;
}

void LLVMIRDialect::addEthereumExternals()
{
	// These are not YulStrings because that would be too complicated with regards
	// to the YulStringRepository reset.
	static string const i64{"i64"};
	static string const i32{"i32"};
	static string const i32ptr{"i32"}; // Uses "i32" on purpose.
	struct External { string name; vector<string> parameters; vector<string> returns; };
	static vector<External> externals{
		{"getAddress", {i32ptr}, {}},
		{"getExternalBalance", {i32ptr, i32ptr}, {}},
		{"getBlockHash", {i64, i32ptr}, {i32}},
		{"call", {i64, i32ptr, i32ptr, i32ptr, i32}, {i32}},
		{"callDataCopy", {i32ptr, i32, i32}, {}},
		{"getCallDataSize", {}, {i32}},
		{"callCode", {i64, i32ptr, i32ptr, i32ptr, i32}, {i32}},
		{"callDelegate", {i64, i32ptr, i32ptr, i32}, {i32}},
		{"callStatic", {i64, i32ptr, i32ptr, i32}, {i32}},
		{"storageStore", {i32ptr, i32ptr}, {}},
		{"storageLoad", {i32ptr, i32ptr}, {}},
		{"getCaller", {i32ptr}, {}},
		{"getCallValue", {i32ptr}, {}},
		{"codeCopy", {i32ptr, i32, i32}, {}},
		{"getCodeSize", {i32ptr}, {}},
		{"getBlockCoinbase", {i32ptr}, {}},
		{"create", {i32ptr, i32ptr, i32, i32ptr}, {i32}},
		{"getBlockDifficulty", {i32ptr}, {}},
		{"externalCodeCopy", {i32ptr, i32ptr, i32, i32}, {}},
		{"getExternalCodeSize", {i32ptr}, {i32}},
		{"getGasLeft", {}, {i64}},
		{"getBlockGasLimit", {}, {i64}},
		{"getTxGasPrice", {i32ptr}, {}},
		{"log", {i32ptr, i32, i32, i32ptr, i32ptr, i32ptr, i32ptr}, {}},
		{"getBlockNumber", {}, {i64}},
		{"getTxOrigin", {i32ptr}, {}},
		{"finish", {i32ptr, i32}, {}},
		{"revert", {i32ptr, i32}, {}},
		{"getReturnDataSize", {}, {i32}},
		{"returnDataCopy", {i32ptr, i32, i32}, {}},
		{"selfDestruct", {i32ptr}, {}},
		{"getBlockTimestamp", {}, {i64}}
	};
	for (External const& ext: externals)
	{
		YulString name{"eth." + ext.name};
		BuiltinFunction& f = m_functions[name];
		f.name = name;
		for (string const& p: ext.parameters)
			f.parameters.emplace_back(YulString(p));
		for (string const& p: ext.returns)
			f.returns.emplace_back(YulString(p));
		f.movable = false;
		// TODO some of them are side effect free.
		f.sideEffectFree = false;
		f.sideEffectFreeIfNoMSize = false;
		f.isMSize = false;
		f.invalidatesStorage = (ext.name == "storageStore");
		// TODO some of them do not invalidate memory
		f.invalidatesMemory = true;
		f.literalArguments = false;
	}
}

void LLVMIRDialect::addFunction(
	string _name,
	size_t _params,
	size_t _returns,
	bool _movable,
	bool _literalArguments
)
{
	YulString name{move(_name)};
	BuiltinFunction& f = m_functions[name];
	f.name = name;
	f.parameters.resize(_params);
	f.returns.resize(_returns);
	f.movable = _movable;
	f.sideEffectFree = _movable;
	f.sideEffectFreeIfNoMSize = _movable;
	f.isMSize = false;
	f.invalidatesStorage = !_movable;
	f.invalidatesMemory = !_movable;
	f.literalArguments = _literalArguments;
}
