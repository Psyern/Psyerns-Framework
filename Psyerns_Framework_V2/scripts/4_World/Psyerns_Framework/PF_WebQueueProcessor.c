class PF_WebQueueProcessor
{
	protected static ref PF_WebQueueProcessor s_Instance;
	protected ref PF_WebQueue m_Queue;
	protected bool m_Running;
	protected float m_LastSendTime;
	protected float m_SendInterval;
	protected float m_TimeSinceLastSend;

	void PF_WebQueueProcessor()
	{
		m_Queue = new PF_WebQueue();
		m_Running = false;
		m_LastSendTime = 0;
		m_SendInterval = 0.25;
		m_TimeSinceLastSend = 0;
	}

	static PF_WebQueueProcessor GetInstance()
	{
		if (!s_Instance)
			s_Instance = new PF_WebQueueProcessor();

		return s_Instance;
	}

	void SetQueue(PF_WebQueue queue)
	{
		m_Queue = queue;
	}

	PF_WebQueue GetQueue()
	{
		return m_Queue;
	}

	void Start()
	{
		m_Running = true;
		PF_Logger.Log("Queue processor started (interval: " + m_SendInterval.ToString() + "s)");
	}

	void Stop()
	{
		m_Running = false;
		PF_Logger.Log("Queue processor stopped. Remaining items in queue: " + m_Queue.Count().ToString());
	}

	bool IsRunning()
	{
		return m_Running;
	}

	void OnUpdate(float timeslice)
	{
		if (!m_Running)
			return;

		if (m_Queue.IsEmpty())
			return;

		m_TimeSinceLastSend += timeslice;

		if (m_TimeSinceLastSend < m_SendInterval)
			return;

		m_TimeSinceLastSend = 0;
		ProcessNext();
	}

	protected void ProcessNext()
	{
		PF_WebQueueItem item = m_Queue.Dequeue();
		if (!item)
			return;

		PF_Logger.Debug("Processing queue item (remaining: " + m_Queue.Count().ToString() + ", interval: " + m_SendInterval.ToString() + "s)");

		PF_WebClient client = PF_WebClient.GetInstance();
		client.Send(item.m_Request);

		// Reduce interval on successful send (min 0.25s)
		m_SendInterval = Math.Max(0.25, m_SendInterval - 0.05);
	}

	void OnSendFailed(PF_WebQueueItem item)
	{
		if (item.CanRetry())
		{
			item.IncrementRetry();
			m_Queue.EnqueueItem(item);
			// Increase interval on failure (max 2.0s)
			m_SendInterval = Math.Min(2.0, m_SendInterval + 0.25);
			PF_Logger.Error("Request failed, retry " + item.m_RetryCount.ToString() + "/" + item.m_MaxRetries.ToString() + " (next interval: " + m_SendInterval.ToString() + "s)");
		}
		else
		{
			PF_Logger.Error("Request failed, max retries reached. Dropping request.");
		}
	}
}
