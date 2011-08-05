#ifdef _MSC_VER
#pragma warning(disable: 4786)
#pragma warning(disable:4103)
#endif

#include "unit_hierarchy.h"
#include "unit_scripts.h"
#include <vector>
#include <map>

using namespace boost;
using namespace std;

namespace frozenbyte {
namespace editor {
namespace {

typedef vector<int> IntList;

struct SecondaryGroup
{
	IntList indices;
};

typedef map<string, SecondaryGroup> SecondaryGroups;

struct PrimaryGroup
{
	SecondaryGroups secondaryGroups;
};

typedef map<string, PrimaryGroup> PrimaryGroups;

} // unnamed

struct UnitHierarchy::Data
{
	UnitScripts &scripts;
	PrimaryGroups primaryGroups;

	int primarySelected;
	int secondarySelected;

	Data(UnitScripts &scripts_)
	:	scripts(scripts_),
		primarySelected(0),
		secondarySelected(0)
	{
	}

	void init()
	{
		for(int i = 0; i < scripts.getUnitCount(); ++i)
		{
			const Unit &unit = scripts.getUnit(i);
			
			string primaryName, secondaryName;
			getGroups(unit, primaryName, secondaryName);

			if(primaryName.empty())
				primaryName = "Legacy";
			if(secondaryName.empty())
				secondaryName = "Legacy";

			if(primaryName == "Mission2WTF")
				int a = 0;

			PrimaryGroup &pg = primaryGroups[primaryName];
			SecondaryGroup &sg = pg.secondaryGroups[secondaryName];

			sg.indices.push_back(i);
		}
	}

	void getGroups(const Unit &unit, string &primary, string &secondary)
	{
		if(!unit.primaryGroup.empty())
		{
			primary = unit.primaryGroup;
			secondary = unit.secondaryGroup;
			return;
		}

		// If has no groups readily defined, parse them from name. Requires two "," as tokens

		const std::string name = unit.name;
		int first = name.find_first_of(",");
		if(first == name.npos)
			return;

		// Hardcode overriding
		primary = name.substr(0, first);
		if(primary == "C")
			primary = "Common";

		int second = name.find_first_of(",", first + 1);
		if(second == name.npos)
			return;

		secondary = name.substr(first + 1, second - first - 1);

		// ToDo:
		//  - Combine some common badly named secondary groups (eg. all doors to single group, etc)
	}

	PrimaryGroups::const_iterator getPrimary(int index) const
	{
		assert(index >= 0 && index < int(primaryGroups.size()));
		
		int i = 0;
		PrimaryGroups::const_iterator it = primaryGroups.begin();
		for(; it != primaryGroups.end(); ++it)
		{
			if(i++ == index)
				return it;
		}

		assert(!"Should never get here");
		return primaryGroups.begin();
	}

	SecondaryGroups::const_iterator getSecondary(int indexP, int indexS) const
	{
		PrimaryGroups::const_iterator ip = getPrimary(indexP);
		const PrimaryGroup &pg = ip->second;

		int secondaryGroups = pg.secondaryGroups.size();
		assert(indexS >= 0 && indexS < secondaryGroups);

		int i = 0;
		SecondaryGroups::const_iterator it = pg.secondaryGroups.begin();
		for(; it != pg.secondaryGroups.end(); ++it)
		{
			if(i++ == indexS)
				return it;
		}

		assert(!"Should never get here");
		return pg.secondaryGroups.begin();
	}

	SecondaryGroups::const_iterator getSecondary(int indexS) const
	{
		int currentPrimary = 0;

		PrimaryGroups::const_iterator it = primaryGroups.begin();
		for(; it != primaryGroups.end(); ++it)
		{
			const PrimaryGroup &pg = it->second;
			if(indexS < int(pg.secondaryGroups.size()))
				return getSecondary(currentPrimary, indexS);

			++currentPrimary;
			indexS -= pg.secondaryGroups.size();
		}

		assert(!"Should never get here, secondary index out of bounds");
		return primaryGroups.begin()->second.secondaryGroups.begin();
	}

	int getPrimaryAmount() const
	{
		return primaryGroups.size();
	}

	int getSecondaryAmount(int primaryIndex) const
	{
		if(--primaryIndex == -1)
		{
			int amount = 0;

			PrimaryGroups::const_iterator it = primaryGroups.begin();
			for(; it != primaryGroups.end(); ++it)
				amount += it->second.secondaryGroups.size();

			return amount;
		}

		PrimaryGroups::const_iterator it = getPrimary(primaryIndex);
		return it->second.secondaryGroups.size();
	}

	const string &getPrimaryName(int primaryIndex) const
	{
		if(primaryIndex-- == 0)
		{
			static string all = "All";
			return all;
		}

		PrimaryGroups::const_iterator it = getPrimary(primaryIndex);
		return it->first;
	}

	const string &getSecondaryName(int primaryIndex, int secondaryIndex) const
	{
		primaryIndex -= 1;
		secondaryIndex -= 1;

		if(secondaryIndex == -1)
		{
			static string all = "All";
			return all;
		}

		if(primaryIndex == -1)
		{
			SecondaryGroups::const_iterator it = getSecondary(secondaryIndex);
			return it->first;
		}

		SecondaryGroups::const_iterator it = getSecondary(primaryIndex, secondaryIndex);
		return it->first;
	}

	int getUnitScriptAmount() const
	{
		PrimaryGroups::const_iterator primary = primaryGroups.begin();
		PrimaryGroups::const_iterator primaryEnd = primaryGroups.end();

		if(primarySelected >= 0)
		{
			primary = getPrimary(primarySelected);
			primaryEnd = primary;
			++primaryEnd;
		}

		int amount = 0;
		for(; primary != primaryEnd; ++primary)
		{
			const PrimaryGroup &pg = primary->second;
			SecondaryGroups::const_iterator secondary = pg.secondaryGroups.begin();
			SecondaryGroups::const_iterator secondaryEnd = pg.secondaryGroups.end();

			if(secondarySelected >= 0)
			{
				for(int i = 0; i < secondarySelected; ++i)
					++secondary;

				secondaryEnd = secondary;
				++secondaryEnd;
			}

			for(; secondary != secondaryEnd; ++secondary)
				amount += secondary->second.indices.size();
		}

		return amount;
	}

	int getUnitScriptIndex(int listIndex)
	{
		PrimaryGroups::const_iterator primary = primaryGroups.begin();
		PrimaryGroups::const_iterator primaryEnd = primaryGroups.end();

		if(primarySelected >= 0)
		{
			primary = getPrimary(primarySelected);
			primaryEnd = primary;
			++primaryEnd;
		}

		for(; primary != primaryEnd; ++primary)
		{
			const PrimaryGroup &pg = primary->second;
			SecondaryGroups::const_iterator secondary = pg.secondaryGroups.begin();
			SecondaryGroups::const_iterator secondaryEnd = pg.secondaryGroups.end();

			if(secondarySelected >= 0)
			{
				for(int i = 0; i < secondarySelected; ++i)
					++secondary;

				secondaryEnd = secondary;
				++secondaryEnd;
			}

			for(; secondary != secondaryEnd; ++secondary)
			{
				const SecondaryGroup &sg = secondary->second;
				if(listIndex < int(sg.indices.size()))
					return sg.indices[listIndex];

				listIndex -= sg.indices.size();
			}
		}

		assert(!"Invalid list index");
		return -1;
	}

	int getUnitListIndex(int scriptIndex) const
	{
		PrimaryGroups::const_iterator primary = primaryGroups.begin();
		PrimaryGroups::const_iterator primaryEnd = primaryGroups.end();

		if(primarySelected >= 0)
		{
			primary = getPrimary(primarySelected);
			primaryEnd = primary;
			++primaryEnd;
		}

		int listIndex = 0;
		for(; primary != primaryEnd; ++primary)
		{
			const PrimaryGroup &pg = primary->second;
			SecondaryGroups::const_iterator secondary = pg.secondaryGroups.begin();
			SecondaryGroups::const_iterator secondaryEnd = pg.secondaryGroups.end();

			if(secondarySelected >= 0)
			{
				secondary = getSecondary(secondarySelected);
				secondaryEnd = secondary;
				++secondaryEnd;
			}

			for(; secondary != secondaryEnd; ++secondary)
			{
				const SecondaryGroup &sg = secondary->second;
				for(unsigned int i = 0; i < sg.indices.size(); ++i)
				{
					if(sg.indices[i] == scriptIndex)
						return listIndex;
					//if(listIndex == scriptIndex)
					//	return sg.indices[i];

					++listIndex;
				}
			}
		}

		//assert(!"Invalid script index");
		return -1;
	}
};

UnitHierarchy::UnitHierarchy(UnitScripts &scripts)
{
	scoped_ptr<Data> tempData(new Data(scripts));
	data.swap(tempData);
}

UnitHierarchy::~UnitHierarchy()
{
}

void UnitHierarchy::init()
{
	data->init();
}

int UnitHierarchy::getPrimaryAmount() const
{
	return data->getPrimaryAmount() + 1;
}

int UnitHierarchy::getSecondaryAmount(int primaryIndex) const
{
	if(primaryIndex == 0)
		return 1;

	return data->getSecondaryAmount(primaryIndex) + 1;
}

const string &UnitHierarchy::getPrimaryName(int primaryIndex) const
{
	return data->getPrimaryName(primaryIndex);
}

const string &UnitHierarchy::getSecondaryName(int primaryIndex, int secondaryIndex) const
{
	return data->getSecondaryName(primaryIndex, secondaryIndex);
}

bool UnitHierarchy::setGroups(int primaryIndex, int secondaryIndex)
{
	int newP = primaryIndex - 1;
	int newS = secondaryIndex - 1;

	if(data->primarySelected == newP && data->secondarySelected == newS)
		return false;

	data->primarySelected = newP;
	data->secondarySelected = newS;
	return true;
}

int UnitHierarchy::getUnitScriptAmount() const
{
	return data->getUnitScriptAmount();
}

int UnitHierarchy::getUnitScriptIndex(int listIndex) const
{
	return data->getUnitScriptIndex(listIndex);
}

int UnitHierarchy::getUnitListIndex(int scriptIndex) const
{
	return data->getUnitListIndex(scriptIndex);
}

} // editor
} // frozenbyte
