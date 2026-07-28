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

			query += m_Keys[i] + "=" + m_Values[i];
		}

		return query;
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
