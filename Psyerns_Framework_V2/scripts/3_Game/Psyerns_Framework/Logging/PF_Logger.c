// Thin wrapper around PsyCore_Log (tag "Psyerns Framework").
// Output: RPT/script log plus a daily file in $profile:Psyerns_Framework/Logs (same folder as before;
// file name is now "<tag>_<YYYY-MM-DD>.log" as written by PsyCore_Log).
// The public API (Init/Log/Error/Debug/MaskSecrets) is unchanged for all callers.
class PF_Logger
{
	static const string TAG = "Psyerns Framework";
	static const string LOG_DIR = "$profile:Psyerns_Framework/Logs";

	protected static bool s_DebugEnabled;
	protected static bool s_PF_FileEnabled;

	protected static PsyCore_Log GetLog()
	{
		PsyCore_Log log = PsyCore_Log.Get(TAG);
		if (!s_PF_FileEnabled)
		{
			log.EnableFile(LOG_DIR);
			s_PF_FileEnabled = true;
		}
		return log;
	}

	static void Init(bool debugEnabled)
	{
		s_DebugEnabled = debugEnabled;
		GetLog().SetDebug(debugEnabled);
	}

	static bool IsDebugEnabled()
	{
		return s_DebugEnabled;
	}

	static void Log(string message)
	{
		GetLog().Info(message);
	}

	static void Error(string message)
	{
		GetLog().Error(message);
	}

	static void Debug(string message)
	{
		if (!s_DebugEnabled)
			return;
		GetLog().Debug(MaskSecrets(message));
	}

	// Framework-specific masks (server_token=, TopGames /servers/<token>/, Discord /webhooks/<id>/<token>)
	// first, then the generic core masks (api_key=, token=, key=, password=, bearer ...).
	static string MaskSecrets(string input)
	{
		string result = input;
		result = MaskQueryValue(result, "api_key=");
		result = MaskQueryValue(result, "server_token=");
		// Discord: /webhooks/<id>/<token>, TopGames: /servers/<token>/...
		result = MaskPathSegment(result, "/webhooks/", 1);
		result = MaskPathSegment(result, "/servers/", 0);
		return PsyCore_Log.MaskSecrets(result);
	}

	protected static string MaskQueryValue(string input, string marker)
	{
		int keyPos = input.IndexOf(marker);
		if (keyPos < 0)
			return input;

		int valueStart = keyPos + marker.Length();
		int valueEnd = input.IndexOfFrom(valueStart, "&");
		if (valueEnd < valueStart)
			valueEnd = input.Length();

		return MaskRange(input, valueStart, valueEnd);
	}

	// Masks the path segment that follows marker after skipping skipSegments segments
	protected static string MaskPathSegment(string input, string marker, int skipSegments)
	{
		int markerPos = input.IndexOf(marker);
		if (markerPos < 0)
			return input;

		int segStart = markerPos + marker.Length();
		for (int i = 0; i < skipSegments; i++)
		{
			int slash = input.IndexOfFrom(segStart, "/");
			if (slash < 0)
				return input;
			segStart = slash + 1;
		}

		int segEnd = input.Length();
		int nextSlash = input.IndexOfFrom(segStart, "/");
		if (nextSlash >= segStart && nextSlash < segEnd)
			segEnd = nextSlash;
		int query = input.IndexOfFrom(segStart, "?");
		if (query >= segStart && query < segEnd)
			segEnd = query;

		return MaskRange(input, segStart, segEnd);
	}

	protected static string MaskRange(string input, int valueStart, int valueEnd)
	{
		int len = valueEnd - valueStart;
		if (len <= 0)
			return input;

		string visible = "";
		if (len > 6)
			visible = input.Substring(valueStart, 3);

		return input.Substring(0, valueStart) + visible + "***" + input.Substring(valueEnd, input.Length() - valueEnd);
	}
}
