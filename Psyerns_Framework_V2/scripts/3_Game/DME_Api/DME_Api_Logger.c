/**
 * Psyerns_Framework — DME_Api
 *
 * Original work Copyright (c) daemonforge — DayZ-UniversalApi (AGPL-3.0)
 *   https://github.com/daemonforge/DayZ-UniveralApi
 * Modifications & integration Copyright (c) 2026 Psyern / Deadmans Echo
 * Modified by Deadmans Echo, 2026.   [AGPL §5(a) change notice]
 *
 * This file is part of Psyerns_Framework and is licensed under the
 * GNU Affero General Public License v3.0. See LICENSE in the repo root.
 * SPDX-License-Identifier: AGPL-3.0-only
 *
 * Cross-PBO merge (Fall B): base logger from _UAPIBase Logger.c with the
 * _UniversalApi SendToApi override inlined into DME_Api_LoggerBaseInstance.
 * Log-level constants prefixed (DME_API_LOG_*), IsServer/IsClient replaced with
 * IsDedicatedServer, copy-paste "MapLink" strings corrected, multi-decl split.
 */
static const int DME_API_LOG_ERROR = 0;
static const int DME_API_LOG_VERBOSE = 1;
static const int DME_API_LOG_INFO = 2;
static const int DME_API_LOG_DEBUG = 3;

class DME_Api_Log extends DME_Api_LoggerBase {
	protected static ref DME_Api_LoggerBaseInstance m_ULoggerBaseInstance;
	static void CreateInstance(){
		m_type = "DME_Api";
		m_ULoggerBaseInstance = new DME_Api_LoggerBaseInstance("DME_Api");
	}
	static DME_Api_LoggerBaseInstance GetInstance(){
		if (!m_ULoggerBaseInstance){CreateInstance();}
		return m_ULoggerBaseInstance;
	}
}

class DME_Api_LoggerBase extends Managed {
	protected static string m_type = "";
	protected static ref DME_Api_LoggerBaseInstance m_LoggerBaseInstance;

	static void CreateInstance(){
		m_LoggerBaseInstance = new DME_Api_LoggerBaseInstance(m_type);
	}

	static DME_Api_LoggerBaseInstance GetInstance(){
		if (!m_LoggerBaseInstance){CreateInstance();}
		return m_LoggerBaseInstance;
	}

	static void Log(string text, int level = 1) {
		GetInstance().DoLog(text,level);
	}

	static void Info(string text){
		GetInstance().DoLog(text, DME_API_LOG_INFO);
	}

	static void Debug(string text){
		GetInstance().DoLog(text, DME_API_LOG_DEBUG);
	}

	static void Err(string text){
		Error2("[" + m_type + "] Error", text);
		GetInstance().DoLog(text, DME_API_LOG_ERROR);
	}

	static void SetLogLevels(int level, int apiLevel = -99){
		if (apiLevel == -99){
			apiLevel = level;
		}
		GetInstance().SetLogLevel(level);
		GetInstance().SetApiLogLevel(apiLevel);
	}

};
class DME_Api_LoggerBaseInstance extends Managed {

	protected int				m_LogLevel	= 3;
	protected int				m_LogToApiLevel = 3;
	protected bool 			m_isInit = false;

	protected static string LogDir = "$profile:";
	protected string m_LogType = "";
	protected FileHandle		m_FileHandle;

	void DME_Api_LoggerBaseInstance(string logType, int level = 4) {
		m_LogLevel = level;
		m_LogType = logType;
		if ( !g_Game || !g_Game.IsDedicatedServer() ){
			return;
		}
		m_FileHandle = CreateFile(LogDir + m_LogType + "_" + GetDateStampFile() + ".log");
		if (m_FileHandle != 0){
			m_isInit = true;
		}
	}

	void ~DME_Api_LoggerBaseInstance() {
		if ( m_isInit ) {
			CloseFile(m_FileHandle);
		}
	}

	void SetLogLevel(int level){
		m_LogLevel = level;
	}

	void SetApiLogLevel(int level){
		m_LogToApiLevel = level;
	}

	protected FileHandle CreateFile(string path) {
		if ( !g_Game || !g_Game.IsDedicatedServer() ){
			return null;
		}

		FileHandle fHandle = OpenFile(path, FileMode.WRITE);
		if (fHandle != 0) {
			FPrintln(fHandle, "DME_Api Log Started: " + GetDateStamp() + " " + GetTimeStamp() );
			return fHandle;
		}
		Error2("[DME_Api] Error", "Unable to create" + path + " file in Profile.");
		return fHandle;
	}

	protected static string GetDateStamp() {
		int yr;
		int mth;
		int day;
		GetYearMonthDay(yr, mth, day);
		string sday = day.ToString();
		if (sday.Length() == 1){
			sday = "0" + sday;
		}

		string smth = mth.ToString();
		if (smth.Length() == 1){
			smth = "0" + mth.ToString();
		}

		return yr.ToString() + "-" + smth + "-" + sday;
	}

	protected static string GetDateStampFile() {
		int hr;
		int min;
		int sec;
		GetHourMinuteSecond(hr, min, sec);

		string ssec = sec.ToString();
		if (ssec.Length() == 1){
			ssec = "0" + ssec;
		}
		string smin = min.ToString();
		if (smin.Length() == 1){
			smin = "0" + smin;
		}
		string shr = hr.ToString();
		if (shr.Length() == 1) {
			shr = "0" + shr;
		}

		return  GetDateStamp() + "_" + shr + "-" + smin + "-" + ssec;
	}

	protected static string GetTimeStamp() {
		int hr;
		int min;
		int sec;
		GetHourMinuteSecond(hr, min, sec);

		string ssec = sec.ToString();
		if (ssec.Length() == 1){
			ssec = "0" + ssec;
		}
		string smin = min.ToString();
		if (smin.Length() == 1){
			smin = "0" + smin;
		}
		string shr = hr.ToString();
		if (shr.Length() == 1) {
			shr = "0" + shr;
		}

		return  shr + ":" + smin + ":" + ssec;
	}


	void DoLog(string text, int level = 1)
	{
		if (!g_Game) return;
		if (level == 2 && m_LogLevel >= level) {
			g_Game.AdminLog("[" + m_LogType + "]" + GetTag(level) + text);
		}
		if (m_isInit && m_LogLevel >= level){
			string towrite = GetTag(level)  + GetTimeStamp() + " | " + " " + text;
			FPrintln(m_FileHandle, towrite);
		} else if (m_LogLevel >= level) {
			Print("[" + m_LogType + "]" + GetTag(level) + " " + text);
		}
		if (m_LogToApiLevel >= level){
			SendToApi(GetJsonObject(m_LogType,text, level));
		}
	}

	protected void SendToApi(string jsonString){
		DME_Api().Rest().Log(jsonString);
	}

	protected static string GetTag(int level){
		switch ( level ) {
			case DME_API_LOG_ERROR:
				return "[ERROR] ";
				break;
			case DME_API_LOG_VERBOSE:
				return "[VERBOSE] ";
				break;
			case DME_API_LOG_DEBUG:
				return "[DEBUG] ";
				break;
			case DME_API_LOG_INFO:
				return "[INFO] ";
				break;
			default:
				return "[INFO] ";
				break;
		}
		return "[NULL] ";
	}

	static string GetJsonObject(string type, string text, int level) {
		string sLevel = "INFO";

		switch ( level ) {
			case DME_API_LOG_ERROR:
				sLevel = "ERROR";
				break;
			case DME_API_LOG_VERBOSE:
				sLevel =  "VERBOSE";
				break;
			case DME_API_LOG_DEBUG:
				sLevel =  "DEBUG";
				break;
			case DME_API_LOG_INFO:
				sLevel =  "INFO";
				break;
			default:
				sLevel =  "INFO";
				break;
		}
		DME_Api_LoggerObject obj = new DME_Api_LoggerObject( type, text, sLevel);
		return obj.ToJson();
	}
}

class DME_Api_LoggerObject extends DME_Api_Object_Base {

	string Type;
	string Message;
	string Level;

	void DME_Api_LoggerObject(string type, string text, string level){
		Type = type;
		Message = text;
		Level = level;

	}

	override string ToJson(){
		string jsonString = JsonFileLoader<DME_Api_LoggerObject>.JsonMakeData(this);
		return jsonString;
	}

}
