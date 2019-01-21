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

#include <libyul/optimiser/VarNameCleaner.h>
#include <libyul/AsmData.h>
#include <libyul/Dialect.h>
#include <algorithm>
#include <cctype>
#include <climits>
#include <iterator>
#include <string>
#include <regex>

using namespace yul;
using namespace std;

void VarNameCleaner::operator()(FunctionDefinition& _funDef)
{
	m_usedNames = m_blacklist;
	m_translatedNames.clear();

	ASTModifier::operator()(_funDef);
}

void VarNameCleaner::operator()(VariableDeclaration& _varDecl)
{
	for (TypedName& typedName: _varDecl.variables)
		if (auto newName = makeCleanName(typedName.name))
			typedName.name = *newName;

	ASTModifier::operator()(_varDecl);
}

void VarNameCleaner::operator()(Identifier& _identifier)
{
	if (auto newName = newlyAssignedName(_identifier.name))
		_identifier.name = *newName;
}

boost::optional<YulString> VarNameCleaner::makeCleanName(YulString const& _name)
{
	if (auto newName = findCleanName(_name))
	{
		m_usedNames.insert(*newName);
		m_translatedNames[_name] = *newName;

		return newName;
	}

	// Name isn't used yet, but we need to make sure nobody else does.
	m_usedNames.insert(_name);
	return boost::none;
}

boost::optional<YulString> VarNameCleaner::findCleanName(YulString const& _name) const
{
	if (auto newName = stripSuffix(_name))
	{
		if (newName->str().length() != 0 && !m_dialect.builtin(*newName) && !m_usedNames.count(*newName))
			return newName;

		// create new name with suffix (by finding a free identifier)
		for (size_t i = 1; i < numeric_limits<decltype(i)>::max(); ++i)
		{
			YulString newNameSuffixed = YulString{newName->str() + "_" + to_string(i)};
			if (!m_usedNames.count(newNameSuffixed))
				return newNameSuffixed;
		}
		yulAssert(false, "Exhausted by attempting to find an available suffix.");
	}

	return boost::none;
}

boost::optional<YulString> VarNameCleaner::stripSuffix(YulString const& _name) const
{
	static regex const suffixRegex("(_+[0-9]+)+$");

	smatch suffixMatch;
	if (regex_search(_name.str(), suffixMatch, suffixRegex))
		return {YulString{suffixMatch.prefix().str()}};

	return boost::none;
}

boost::optional<YulString> VarNameCleaner::newlyAssignedName(YulString const& _name) const
{
	auto n = m_translatedNames.find(_name);
	if (n != m_translatedNames.end() && n->second != _name)
		return {n->second};
	else
		return boost::none;
}
