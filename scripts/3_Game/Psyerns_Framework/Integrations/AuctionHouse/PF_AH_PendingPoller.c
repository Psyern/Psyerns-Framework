/**
 * PF_AH_PendingPoller — GETs /internal/pending, dispatches actions, PATCHes results.
 *
 * Engine limitation: RestContext.SetHeader() only supports Content-Type — custom
 * Authorization headers are NOT supported. The api_key is sent as query parameter
 * (via PF_RestBase), not as a Bearer header. Agent 6 (WP side) MUST accept both.
 *
 * Engine limitation: RestContext exposes GET and POST — no PATCH verb. We POST
 * the result back to /internal/pending/{uuid} with an explicit "_method":"PATCH"
 * marker in the JSON body, and Agent 6 MUST treat this POST as a PATCH.
 */
class PF_AH_PendingPoller : PF_RestBase
{
	protected ref PF_AH_ActionExecutor m_Executor;

	void PF_AH_PendingPoller(string baseUrl, string apiKey, PF_AH_ActionExecutor executor)
	{
		m_Executor = executor;
	}

	void Poll()
	{
		PF_AH_PendingCallback cb = new PF_AH_PendingCallback(this);
		GetWithCallback("/wp-json/psyern-ah/v1/internal/pending", cb);
	}

	void HandleActions(array<ref PF_AH_PendingAction> actions)
	{
		if (!actions || actions.Count() == 0)
			return;

		PF_Logger.Log("AH: Dispatching " + actions.Count().ToString() + " pending action(s)");

		for (int i = 0; i < actions.Count(); i++)
		{
			PF_AH_PendingAction a = actions[i];
			if (!a)
				continue;

			string status = "failed_executor_missing";
			string resultCode = "executor_missing";
			string message = "Action executor not initialized";

			if (m_Executor)
				m_Executor.Execute(a, status, resultCode, message);

			ReportResult(a.action_uuid, status, resultCode, message);
		}
	}

	protected void ReportResult(string uuid, string status, string resultCode, string message)
	{
		if (uuid == "")
			return;

		PF_JsonBuilder b = PF_JsonBuilder.Begin();
		b.Add("_method", "PATCH");
		b.Add("status", status);
		b.Add("result_code", resultCode);
		b.Add("result_message", message);
		string body = b.Build();

		string endpoint = "/wp-json/psyern-ah/v1/internal/pending/" + uuid;
		PostJson(endpoint, body);
		PF_Logger.Debug("AH: Reported result for " + uuid + " → " + status);
	}
}

/**
 * PF_AH_PendingCallback — Parses /internal/pending response and hands actions to the poller.
 */
class PF_AH_PendingCallback : RestCallback
{
	protected PF_AH_PendingPoller m_Owner;

	void PF_AH_PendingCallback(PF_AH_PendingPoller owner)
	{
		m_Owner = owner;
	}

	override void OnSuccess(string data, int dataSize)
	{
		if (!m_Owner)
			return;
		if (dataSize <= 0 || data == "")
			return;

		PF_AH_PendingActionArray parsed = new PF_AH_PendingActionArray();
		string error;
		JsonSerializer js = new JsonSerializer();
		js.ReadFromString(parsed, data, error);

		if (error != "")
		{
			PF_Logger.Error("AH: Failed to parse /internal/pending response: " + error);
			return;
		}

		m_Owner.HandleActions(parsed.actions);
	}

	override void OnError(int errorCode)
	{
		PF_Logger.Error("AH: /internal/pending GET failed, code=" + errorCode.ToString());
	}

	override void OnTimeout()
	{
		PF_Logger.Error("AH: /internal/pending GET timed out");
	}
}
