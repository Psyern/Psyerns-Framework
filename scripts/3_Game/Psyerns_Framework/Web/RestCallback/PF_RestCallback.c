class PF_RestCallback : RestCallback
{
	// Haelt alle laufenden Callbacks am Leben, bis die Engine geantwortet hat.
	// RestContext.GET/POST/FILE sind asynchron und merken sich nur einen rohen
	// Pointer. Ohne diese Referenz faellt der Refcount beim Verlassen der
	// Sende-Funktion auf 0, waehrend der HTTP-Request noch laeuft - die Engine
	// schreibt die Antwort dann in freigegebenen Heap (Heap-Korruption).
	// Element-Typ ist RestCallback (nicht PF_RestCallback), damit auch die
	// DME_Api_*-Callbacks ueber denselben Mechanismus gehalten werden.
	protected static ref array<ref RestCallback> s_PF_InFlight = new array<ref RestCallback>();

	// Ablaufzeitpunkt je Eintrag, index-parallel zu s_PF_InFlight.
	protected static ref array<float> s_PF_RetainUntil = new array<float>();

	// Haltezeit in Sekunden. Vanilla begrenzt REST-Timeouts auf 120s (ERestOption).
	protected static const float PF_RETAIN_SECONDS = 180.0;

	protected ref PF_WebResponse m_Response;
	protected float m_StartTime;

	// Muss VOR jedem GET/POST/FILE mit dem Callback aufgerufen werden.
	static void PF_Retain(RestCallback cb)
	{
		PF_SweepExpired();

		if (!cb)
			return;

		float retainUntil = 0;
		if (g_Game)
			retainUntil = g_Game.GetTickTime() + PF_RETAIN_SECONDS;

		s_PF_InFlight.Insert(cb);
		s_PF_RetainUntil.Insert(retainUntil);
	}

	// Laeuft nur aus PF_Retain heraus, nie aus einem Callback - so gibt sich
	// kein Objekt waehrend seiner eigenen Methode selbst frei. Ein Abmelden in
	// OnSuccess/OnError waere ebenfalls falsch: OnError kann bei RetryCount > 1
	// mehrfach kommen.
	protected static void PF_SweepExpired()
	{
		if (!g_Game)
			return;

		float now = g_Game.GetTickTime();
		int i = s_PF_InFlight.Count() - 1;
		while (i >= 0)
		{
			bool isExpired = s_PF_RetainUntil[i] < now;
			if (isExpired)
			{
				s_PF_InFlight.Remove(i);
				s_PF_RetainUntil.Remove(i);
			}

			i = i - 1;
		}
	}

	void PF_RestCallback()
	{
		m_Response = new PF_WebResponse();
		if (g_Game)
			m_StartTime = g_Game.GetTickTime();
	}

	override void OnSuccess(string data, int dataSize)
	{
		float elapsed = 0;
		if (g_Game)
			elapsed = (g_Game.GetTickTime() - m_StartTime) * 1000.0;
		m_Response.SetSuccess(true);
		m_Response.SetData(data, dataSize);
		m_Response.SetElapsedMs(elapsed);
		PF_Logger.Log("HTTP Success (" + elapsed.ToString() + "ms, " + dataSize.ToString() + " bytes)");
		PF_Logger.Debug("Response data: " + data);
	}

	override void OnError(int errorCode)
	{
		float elapsed = 0;
		if (g_Game)
			elapsed = (g_Game.GetTickTime() - m_StartTime) * 1000.0;
		m_Response.SetSuccess(false);
		m_Response.SetErrorCode(errorCode);
		m_Response.SetElapsedMs(elapsed);
		PF_Logger.Error("HTTP Error: " + errorCode.ToString() + " (" + elapsed.ToString() + "ms)");
	}

	override void OnTimeout()
	{
		float elapsed = 0;
		if (g_Game)
			elapsed = (g_Game.GetTickTime() - m_StartTime) * 1000.0;
		m_Response.SetSuccess(false);
		m_Response.SetErrorCode(-1);
		m_Response.SetElapsedMs(elapsed);
		PF_Logger.Error("HTTP Timeout after " + elapsed.ToString() + "ms");
	}

	PF_WebResponse GetResponse()
	{
		return m_Response;
	}
}
