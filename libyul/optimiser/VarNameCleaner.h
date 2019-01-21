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

#pragma once


#include <libyul/AsmDataForward.h>
#include <libyul/optimiser/ASTWalker.h>
#include <libyul/YulString.h>

#include <boost/optional.hpp>

#include <map>
#include <set>
#include <string>

namespace yul
{

struct Dialect;

/**
 * Pass to normalize identifier suffixes.
 *
 * That is, for each function scope, nested suffixes get flattened and all suffixes
 * renumbered by their base name.
 *
 * Prerequisites: Disambiguator
 */
class VarNameCleaner: public ASTModifier
{
public:
	using BlackList = std::set<YulString>;

	VarNameCleaner(Dialect const& _dialect, BlackList _blacklist = {}):
		m_dialect{_dialect},
		m_blacklist{_blacklist},
		m_usedNames{std::move(_blacklist)},
		m_translatedNames{}
	{
	}

	using ASTModifier::operator();
	void operator()(VariableDeclaration& _varDecl) override;
	void operator()(Identifier& _identifier) override;
	void operator()(FunctionDefinition&) override;

private:
	/// @returns suffix-stripped name, if a suffix was detected, none otherwise.
	boost::optional<YulString> stripSuffix(YulString const& _name) const;

	/// Looks out for a "clean name" the given @p name could be trimmed down to.
	/// @returns a trimmed down and "clean name" in case it found one, none otherwise.
	boost::optional<YulString> findCleanName(YulString const& name) const;

	/// Uses findCleanName to find a clean name, and then remembers it, so future calls
	/// don't pick that name for their use.
	///
	/// @returns a trimmed down and "clean name" in case it found one, none otherwise.
	boost::optional<YulString> makeCleanName(YulString const& name);

	/// Returns the new name, if one was mapped, or none.
	boost::optional<YulString> newlyAssignedName(YulString const& _name) const;

	/// Tests whether or not given name is already in used within this pass.
	bool isUsedName(YulString const& _name) const;

	Dialect const& m_dialect;
	BlackList m_blacklist;

	/// Set of names that are in use.
	std::set<YulString> m_usedNames;

	/// map on old name to new name
	std::map<YulString, YulString> m_translatedNames;
};

}
