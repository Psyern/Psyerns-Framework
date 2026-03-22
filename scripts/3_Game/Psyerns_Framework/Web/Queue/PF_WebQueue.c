class PF_WebQueue
{
	protected ref array<ref PF_WebQueueItem> m_Items;

	void PF_WebQueue()
	{
		m_Items = new array<ref PF_WebQueueItem>();
	}

	void Enqueue(PF_WebRequest request)
	{
		PF_WebQueueItem item = new PF_WebQueueItem(request);
		m_Items.Insert(item);
	}

	void EnqueueItem(PF_WebQueueItem item)
	{
		m_Items.Insert(item);
	}

	PF_WebQueueItem Dequeue()
	{
		if (m_Items.Count() == 0)
			return null;

		PF_WebQueueItem item = m_Items[0];
		m_Items.RemoveOrdered(0);
		return item;
	}

	int Count()
	{
		return m_Items.Count();
	}

	bool IsEmpty()
	{
		return m_Items.Count() == 0;
	}

	void Clear()
	{
		m_Items.Clear();
	}
}
