class PF_JsonPayload
{
	string Serialize()
	{
		string result;
		JsonSerializer().WriteToString(this, false, result);
		return result;
	}
}
