class PF_JsonPayload
{
	string Serialize()
	{
		string result;
		JsonSerializer serializer = new JsonSerializer();
		serializer.WriteToString(this, false, result);
		return result;
	}
}
