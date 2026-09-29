class PF_Logger
{
	protected static bool s_DebugEnabled;

	static void Init(bool debugEnabled)
	{
		s_DebugEnabled = debugEnabled;
	}

	static void Log(string message)
	{
		string formatted = "[Psyerns Framework] " + message;
		Print(formatted);
		WriteToFile(formatted);
	}

	static void Error(string message)
	{
		string formatted = "[Psyerns Framework] [ERROR] " + message;
		Print(formatted);
		WriteToFile(formatted);
	}

	static void Debug(string message)
	{
		if (!s_DebugEnabled)
			return;

		string formatted = "[Psyerns Framework] [DEBUG] " + MaskSecrets(message);
		Print(formatted);
		WriteToFile(formatted);
	}

	static string MaskSecrets(string input)
	{
		string result = input;
		result = MaskQueryValue(result, "api_key=");
		result = MaskQueryValue(result, "server_token=");
		// Discord: /webhooks/<id>/<token>, TopGames: /servers/<token>/...
		result = MaskPathSegment(result, "/webhooks/", 1);
		result = MaskPathSegment(result, "/servers/", 0);
		return result;
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

	protected static void WriteToFile(string message)
	{
		int year;
		int month;
		int day;
		int hour;
		int minute;
		int second;
		GetYearMonthDay(year, month, day);
		GetHourMinuteSecond(hour, minute, second);

		string dateStr = year.ToStringLen(4) + "-" + month.ToStringLen(2) + "-" + day.ToStringLen(2);
		string timeStr = hour.ToStringLen(2) + ":" + minute.ToStringLen(2) + ":" + second.ToStringLen(2);
		string logDir = "$profile:Psyerns_Framework\\Logs";
		string logPath = logDir + "\\PF_Log_" + dateStr + ".log";

		if (!FileExist(logDir))
			MakeDirectory(logDir);

		FileHandle file = OpenFile(logPath, FileMode.APPEND);
		if (file)
		{
			FPrintln(file, "[" + timeStr + "] " + message);
			CloseFile(file);
		}
	}
}
