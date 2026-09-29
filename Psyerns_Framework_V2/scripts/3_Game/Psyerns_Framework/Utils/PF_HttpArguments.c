class PF_HttpArguments
{
	protected ref array<string> m_Keys;
	protected ref array<string> m_Values;

	void PF_HttpArguments()
	{
		m_Keys = new array<string>();
		m_Values = new array<string>();
	}

	void Add(string key, string value)
	{
		if (key == "" || value == "")
			return;

		m_Keys.Insert(key);
		m_Values.Insert(value);
	}

	string ToQuery(string basePath)
	{
		if (m_Keys.Count() == 0)
			return basePath;

		string query = basePath + "?";
		for (int i = 0; i < m_Keys.Count(); i++)
		{
			if (i > 0)
				query += "&";

			query += m_Keys[i] + "=" + UrlEncode(m_Values[i]);
		}

		return query;
	}

	// Percent-encodes everything except RFC 3986 unreserved characters
	static string UrlEncode(string input)
	{
		string unreserved = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_.~";
		string hex = "0123456789ABCDEF";
		string result = "";
		int len = input.Length();
		for (int i = 0; i < len; i++)
		{
			string ch = input.Substring(i, 1);
			if (unreserved.Contains(ch))
			{
				result += ch;
				continue;
			}

			int code = ch.ToAscii();
			if (code < 0)
				code = code + 256;

			result += "%" + hex.Substring(code / 16, 1) + hex.Substring(code % 16, 1);
		}

		return result;
	}

	int Count()
	{
		return m_Keys.Count();
	}

	void Clear()
	{
		m_Keys.Clear();
		m_Values.Clear();
	}
}
