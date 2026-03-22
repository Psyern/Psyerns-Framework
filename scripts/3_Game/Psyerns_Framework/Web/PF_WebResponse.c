class PF_WebResponse
{
	protected bool m_Success;
	protected string m_Data;
	protected int m_DataSize;
	protected int m_ErrorCode;
	protected float m_ElapsedMs;

	bool IsSuccess()
	{
		return m_Success;
	}

	string GetData()
	{
		return m_Data;
	}

	int GetDataSize()
	{
		return m_DataSize;
	}

	int GetErrorCode()
	{
		return m_ErrorCode;
	}

	float GetElapsedMs()
	{
		return m_ElapsedMs;
	}

	void SetSuccess(bool success)
	{
		m_Success = success;
	}

	void SetData(string data, int dataSize)
	{
		m_Data = data;
		m_DataSize = dataSize;
	}

	void SetErrorCode(int errorCode)
	{
		m_ErrorCode = errorCode;
	}

	void SetElapsedMs(float elapsedMs)
	{
		m_ElapsedMs = elapsedMs;
	}
}
