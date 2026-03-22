class PF_WebQueueItem
{
	ref PF_WebRequest m_Request;
	float m_QueuedAt;
	int m_RetryCount;
	int m_MaxRetries;

	void PF_WebQueueItem(PF_WebRequest request, int maxRetries = 3)
	{
		m_Request = request;
		m_QueuedAt = GetGame().GetTickTime();
		m_RetryCount = 0;
		m_MaxRetries = maxRetries;
	}

	bool CanRetry()
	{
		return m_RetryCount < m_MaxRetries;
	}

	void IncrementRetry()
	{
		m_RetryCount++;
	}
}
