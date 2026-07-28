class PF_JsonBuilder
{
	protected ref array<string> m_Entries;

	void PF_JsonBuilder()
	{
		m_Entries = new array<string>();
	}

	static PF_JsonBuilder Begin()
	{
		return new PF_JsonBuilder();
	}

	PF_JsonBuilder Add(string key, string value)
	{
		m_Entries.Insert("\"" + EscapeString(key) + "\":\"" + EscapeString(value) + "\"");
		return this;
	}

	PF_JsonBuilder AddInt(string key, int value)
	{
		m_Entries.Insert("\"" + EscapeString(key) + "\":" + value.ToString());
		return this;
	}

	PF_JsonBuilder AddFloat(string key, float value)
	{
		m_Entries.Insert("\"" + EscapeString(key) + "\":" + value.ToString());
		return this;
	}

	PF_JsonBuilder AddBool(string key, bool value)
	{
		string val = "false";
		if (value)
			val = "true";

		m_Entries.Insert("\"" + EscapeString(key) + "\":" + val);
		return this;
	}

	PF_JsonBuilder AddArray(string key, array<string> values)
	{
		string arr = "[";
		for (int i = 0; i < values.Count(); i++)
		{
			if (i > 0)
				arr += ",";

			arr += "\"" + EscapeString(values[i]) + "\"";
		}
		arr += "]";

		m_Entries.Insert("\"" + EscapeString(key) + "\":" + arr);
		return this;
	}

	PF_JsonBuilder AddObject(string key, PF_JsonBuilder nested)
	{
		m_Entries.Insert("\"" + EscapeString(key) + "\":" + nested.Build());
		return this;
	}

	PF_JsonBuilder AddRaw(string key, string rawJson)
	{
		m_Entries.Insert("\"" + EscapeString(key) + "\":" + rawJson);
		return this;
	}

	string Build()
	{
		string result = "{";
		for (int i = 0; i < m_Entries.Count(); i++)
		{
			if (i > 0)
				result += ",";

			result += m_Entries[i];
		}
		result += "}";
		return result;
	}

	protected string EscapeString(string input)
	{
		string output = input;
		output.Replace("\\", "\\" + "\\");
		output.Replace("\"", "\\" + "\"");
		output.Replace("\n", "\\n");
		output.Replace("\r", "\\r");
		output.Replace("\t", "\\t");
		return output;
	}
}
