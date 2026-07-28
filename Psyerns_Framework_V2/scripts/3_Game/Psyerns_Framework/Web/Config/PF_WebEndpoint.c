class PF_WebEndpoint
{
	string Name;
	string BaseUrl;
	string ApiKey;
	bool Enabled;
	int RateLimitMs;

	void PF_WebEndpoint()
	{
		Enabled = false;
		RateLimitMs = 1000;
	}
}
