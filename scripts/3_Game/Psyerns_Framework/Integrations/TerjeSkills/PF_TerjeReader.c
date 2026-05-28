/**
 * PF_TerjeReader — Reads TerjeMods .dat profiles and extracts the skill block.
 *
 * Format (from TerjePlayerRecordsBase.OnStoreSave):
 *   int  beginMarker     (literal -3539889261)
 *   int  count
 *   N times:
 *     string id          (record ID, e.g. "ts.exp_athletic")
 *     string typeId      ("str"|"int"|"num"|"bool"|"arrstr"|"arrint"|"arrnum"|"mapstr"|"mapint"|"mapnum")
 *     <value(s)>         (depends on typeId)
 *   int  endMarker       (literal 1719232123)
 *
 * Non-skill records are still consumed (so the stream stays aligned), but
 * their values are discarded. This decouples PF from the TerjeMods PBO —
 * Terje does not need to be loaded for parsing to work.
 */
class PF_TerjeReader
{
	static const int STORE_BEGIN_RW_MARKER = -3539889261;
	static const int STORE_END_RW_MARKER   = 1719232123;

	/**
	 * Read one .dat into a PF_TerjePlayerSkills, keyed by steam ID.
	 * Returns null on missing file or unrecoverable parse error.
	 */
	static PF_TerjePlayerSkills ReadProfile(string fullPath, string steamId)
	{
		if (!FileExist(fullPath))
			return null;

		FileSerializer ctx = new FileSerializer();
		if (!ctx.Open(fullPath, FileMode.READ))
			return null;

		PF_TerjePlayerSkills result = new PF_TerjePlayerSkills();
		result.playerID = steamId;

		int beginMarker;
		if (!ctx.Read(beginMarker) || beginMarker != STORE_BEGIN_RW_MARKER)
		{
			ctx.Close();
			return null;
		}

		int count;
		if (!ctx.Read(count) || count < 0)
		{
			ctx.Close();
			return null;
		}

		for (int i = 0; i < count; i++)
		{
			string recordId;
			if (!ctx.Read(recordId))
			{
				ctx.Close();
				return null;
			}

			string typeId;
			if (!ctx.Read(typeId))
			{
				ctx.Close();
				return null;
			}

			if (!ConsumeRecord(ctx, recordId, typeId, result))
			{
				ctx.Close();
				return null;
			}
		}

		int endMarker;
		if (!ctx.Read(endMarker) || endMarker != STORE_END_RW_MARKER)
		{
			ctx.Close();
			return null;
		}

		ctx.Close();
		return result;
	}

	/**
	 * Read every .dat in a directory. Returns a map of steam_id -> skills.
	 * Files that don't parse are silently skipped (one bad save shouldn't
	 * poison the whole batch).
	 */
	static void ReadAllProfiles(string dirPath, out map<string, ref PF_TerjePlayerSkills> outMap)
	{
		outMap = new map<string, ref PF_TerjePlayerSkills>();

		if (!FileExist(dirPath))
			return;

		string fileName;
		FileAttr fileAttr;
		string searchPattern = dirPath + "/*.dat";

		FindFileHandle handle = FindFile(searchPattern, fileName, fileAttr, FindFileFlags.ALL);
		if (!handle)
			return;

		bool hasFile = true;
		while (hasFile)
		{
			if (fileName != "" && fileName.IndexOf(".dat") > 0)
			{
				string steamId = fileName;
				int dotIdx = steamId.IndexOf(".");
				if (dotIdx > 0)
					steamId = steamId.Substring(0, dotIdx);

				PF_TerjePlayerSkills parsed = ReadProfile(dirPath + "/" + fileName, steamId);
				if (parsed != null)
					outMap.Set(steamId, parsed);
			}

			hasFile = FindNextFile(handle, fileName, fileAttr);
		}

		CloseFindFile(handle);
	}

	/**
	 * Read a record value off the stream. If it's a skill record (ts.*),
	 * stash it on the player skills aggregate; otherwise discard but still
	 * advance the cursor. Returns false on a stream error.
	 */
	protected static bool ConsumeRecord(FileSerializer ctx, string recordId, string typeId, PF_TerjePlayerSkills target)
	{
		bool isSkillRecord = (recordId.IndexOf("ts.") == 0);

		if (typeId == "int")
		{
			int v;
			if (!ctx.Read(v)) return false;
			if (isSkillRecord) StoreIntSkill(target, recordId, v);
			return true;
		}

		if (typeId == "str")
		{
			string s;
			if (!ctx.Read(s)) return false;
			if (isSkillRecord) StoreStrSkill(target, recordId, s);
			return true;
		}

		if (typeId == "num")
		{
			float f;
			return ctx.Read(f);
		}

		if (typeId == "bool")
		{
			bool b;
			return ctx.Read(b);
		}

		if (typeId == "arrstr")
		{
			int n;
			if (!ctx.Read(n)) return false;
			for (int i = 0; i < n; i++)
			{
				string item;
				if (!ctx.Read(item)) return false;
			}
			return true;
		}

		if (typeId == "arrint")
		{
			int nI;
			if (!ctx.Read(nI)) return false;
			for (int iI = 0; iI < nI; iI++)
			{
				int item;
				if (!ctx.Read(item)) return false;
			}
			return true;
		}

		if (typeId == "arrnum")
		{
			int nF;
			if (!ctx.Read(nF)) return false;
			for (int iF = 0; iF < nF; iF++)
			{
				float item;
				if (!ctx.Read(item)) return false;
			}
			return true;
		}

		if (typeId == "mapstr")
		{
			int nMs;
			if (!ctx.Read(nMs)) return false;
			for (int iMs = 0; iMs < nMs; iMs++)
			{
				string k;
				string v;
				if (!ctx.Read(k)) return false;
				if (!ctx.Read(v)) return false;
			}
			return true;
		}

		if (typeId == "mapint")
		{
			int nMi;
			if (!ctx.Read(nMi)) return false;
			for (int iMi = 0; iMi < nMi; iMi++)
			{
				string k;
				int v;
				if (!ctx.Read(k)) return false;
				if (!ctx.Read(v)) return false;
			}
			return true;
		}

		if (typeId == "mapnum")
		{
			int nMn;
			if (!ctx.Read(nMn)) return false;
			for (int iMn = 0; iMn < nMn; iMn++)
			{
				string k;
				float v;
				if (!ctx.Read(k)) return false;
				if (!ctx.Read(v)) return false;
			}
			return true;
		}

		// Unknown type — cannot continue safely.
		return false;
	}

	/**
	 * Dispatch a ts.* int record to the right slot:
	 *   ts.exp_<skill>            -> experience
	 *   ts.pps_<skill>            -> perkPoints
	 *   ts.max_<skill>            -> highLevel
	 *   ts.perk_<skill>+<perk>    -> perks[perkId]
	 */
	protected static void StoreIntSkill(PF_TerjePlayerSkills target, string recordId, int value)
	{
		if (recordId.IndexOf("ts.exp_") == 0)
		{
			string skillId = recordId.Substring(7, recordId.Length() - 7);
			target.GetOrCreate(skillId).experience = value;
			return;
		}

		if (recordId.IndexOf("ts.pps_") == 0)
		{
			string skillId = recordId.Substring(7, recordId.Length() - 7);
			target.GetOrCreate(skillId).perkPoints = value;
			return;
		}

		if (recordId.IndexOf("ts.max_") == 0)
		{
			string skillId = recordId.Substring(7, recordId.Length() - 7);
			target.GetOrCreate(skillId).highLevel = value;
			return;
		}

		if (recordId.IndexOf("ts.perk_") == 0)
		{
			string suffix = recordId.Substring(8, recordId.Length() - 8);
			int plusIdx = suffix.IndexOf("+");
			if (plusIdx > 0)
			{
				string skillId = suffix.Substring(0, plusIdx);
				string perkId = suffix.Substring(plusIdx + 1, suffix.Length() - plusIdx - 1);
				target.GetOrCreate(skillId).perks.Set(perkId, value);
			}
		}
	}

	/**
	 * Dispatch a ts.* str record. Only known string record is the
	 * "known books" blob, e.g. "<DZ_Book_A><DZ_Book_B>".
	 */
	protected static void StoreStrSkill(PF_TerjePlayerSkills target, string recordId, string value)
	{
		if (recordId.IndexOf("ts.knb_") == 0)
		{
			string skillId = recordId.Substring(7, recordId.Length() - 7);
			target.GetOrCreate(skillId).knownBooksRaw = value;
		}
	}
}
