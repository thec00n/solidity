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

#include <test/tools/yulInterpreter/Interpreter.h>

#include <libyul/AsmAnalysisInfo.h>
#include <libyul/AsmParser.h>
#include <libyul/AsmAnalysis.h>
#include <libyul/Dialect.h>
#include <libyul/backends/evm/EVMDialect.h>
#include <libyul/AssemblyStack.h>

#include <liblangutil/Exceptions.h>
#include <liblangutil/ErrorReporter.h>
#include <liblangutil/EVMVersion.h>
#include <liblangutil/SourceReferenceFormatter.h>

#include <libdevcore/CommonIO.h>
#include <libdevcore/CommonData.h>

#include <string>
#include <memory>
#include <iostream>

using namespace yul;
using namespace std;

using namespace langutil;
using namespace dev;
using namespace yul::test;

void printErrors(ErrorList const& _errors)
{
	for (auto const& error: _errors)
		SourceReferenceFormatter(cout).printExceptionInformation(
				*error,
				(error->type() == Error::Type::Warning) ? "Warning" : "Error"
		);
}

void interpret(ostream& os, shared_ptr<Block> ast)
{
	InterpreterState state;
	state.maxTraceSize = 10000;
	Interpreter interpreter(state);
	try
	{
		interpreter(*ast);
	}
	catch (InterpreterTerminated const&)
	{
	}

	os << "Trace:" << endl;
	for (auto const& line: interpreter.trace())
		os << "  " << line << endl;
	os << "Memory dump:" << endl;
	for (size_t i = 0; i < state.memory.size(); i += 0x20)
		os << "  " << std::hex << std::setw(4) << i << ": " << toHex(bytesConstRef(state.memory.data() + i, 0x20).toBytes()) << endl;
	os << "Storage dump:" << endl;
	for (auto const& slot: state.storage)
		os << "  " << slot.first.hex() << ": " << slot.second.hex() << endl;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* _data, size_t _size)
{
	if (_size > 450)
		return 0;

	string input(reinterpret_cast<char const*>(_data), _size);

	AssemblyStack stack(solidity::EVMVersion::constantinople(), AssemblyStack::Language::StrictAssembly);
	if (stack.parseAndAnalyze("source", input))
		yulAssert(stack.errors().empty(), "Parsed successfully but had errors.");
	else
		printErrors(stack.errors());

	try
	{
		if (!stack.parserResult()->code || !stack.parserResult()->analysisInfo)
			return 0;
	}
	catch (InternalCompilerError const&)
	{
		return 0;
	}

	ostringstream os1, os2;
	interpret(os1, stack.parserResult()->code);
	stack.optimize();
	interpret(os2, stack.parserResult()->code);

	bool isTraceEq = (os2.str().compare(os1.str()) == 0);
	yulAssert(isTraceEq, "Interpreted traces for optimized and unoptimized code differ.");
	return 0;
}