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

		string formatted = "[Psyerns Framework] [DEBUG] " + message;
		Print(formatted);
		WriteToFile(formatted);
	}

	protected static void WriteToFile(string message)
	{
		int year, month, day, hour, minute, second;
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
