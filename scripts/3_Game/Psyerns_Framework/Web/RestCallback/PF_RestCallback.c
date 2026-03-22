class PF_RestCallback : RestCallback
{
	protected ref PF_WebResponse m_Response;
	protected float m_StartTime;

	void PF_RestCallback()
	{
		m_Response = new PF_WebResponse();
		m_StartTime = GetGame().GetTickTime();
	}

	override void OnSuccess(string data, int dataSize)
	{
		float elapsed = (GetGame().GetTickTime() - m_StartTime) * 1000.0;
		m_Response.SetSuccess(true);
		m_Response.SetData(data, dataSize);
		m_Response.SetElapsedMs(elapsed);
		Print("[Psyerns Framework] HTTP Success (" + elapsed.ToString() + "ms, " + dataSize.ToString() + " bytes)");
	}

	override void OnError(int errorCode)
	{
		float elapsed = (GetGame().GetTickTime() - m_StartTime) * 1000.0;
		m_Response.SetSuccess(false);
		m_Response.SetErrorCode(errorCode);
		m_Response.SetElapsedMs(elapsed);
		Print("[Psyerns Framework] HTTP Error: " + errorCode.ToString() + " (" + elapsed.ToString() + "ms)");
	}

	override void OnTimeout()
	{
		float elapsed = (GetGame().GetTickTime() - m_StartTime) * 1000.0;
		m_Response.SetSuccess(false);
		m_Response.SetErrorCode(-1);
		m_Response.SetElapsedMs(elapsed);
		Print("[Psyerns Framework] HTTP Timeout (" + elapsed.ToString() + "ms)");
	}

	PF_WebResponse GetResponse()
	{
		return m_Response;
	}
}
