/**
 * PF_TerjeSkillData — One skill snapshot for one player.
 *
 * Matches the TerjeSkills record layout (see TerjePlayerProfile in TerjeMods):
 *   ts.exp_<skillId>           int   — experience points
 *   ts.pps_<skillId>           int   — unspent perk points
 *   ts.max_<skillId>           int   — high-water level (kept across deaths)
 *   ts.knb_<skillId>           str   — known books, encoded "<bookId1><bookId2>..."
 *   ts.perk_<skillId>+<perkId> int   — value of a specific bought perk
 *
 * This class owns the per-skill aggregate; PF_TerjePlayerSkills owns the
 * per-player aggregate (one PF_TerjeSkillData per known skillId).
 */
class PF_TerjeSkillData
{
	string skillId;
	int experience;
	int perkPoints;
	int highLevel;
	string knownBooksRaw;
	ref map<string, int> perks;

	void PF_TerjeSkillData()
	{
		skillId = "";
		experience = 0;
		perkPoints = 0;
		highLevel = 0;
		knownBooksRaw = "";
		perks = new map<string, int>();
	}
}

/**
 * PF_TerjePlayerSkills — All skills for one player keyed by skillId.
 */
class PF_TerjePlayerSkills
{
	string playerID;
	ref map<string, ref PF_TerjeSkillData> skills;

	void PF_TerjePlayerSkills()
	{
		playerID = "";
		skills = new map<string, ref PF_TerjeSkillData>();
	}

	PF_TerjeSkillData GetOrCreate(string id)
	{
		PF_TerjeSkillData entry;
		if (!skills.Find(id, entry))
		{
			entry = new PF_TerjeSkillData();
			entry.skillId = id;
			skills.Set(id, entry);
		}
		return entry;
	}
}
