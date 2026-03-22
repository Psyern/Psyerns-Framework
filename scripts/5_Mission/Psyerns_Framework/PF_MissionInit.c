modded class MissionServer
{
	protected ref PF_WebQueueProcessor m_PF_QueueProcessor;

	override void OnInit()
	{
		super.OnInit();

		PF_WebConfig config = PF_WebConfig.GetInstance();

		PF_Logger.Init(config.EnableDebugLogging);
		PF_Logger.Log("Psyerns Framework v1.0.0 initializing...");
		PF_Logger.Log("Author: Psyern | Community: Deadmans Echo");

		PF_WebClient.GetInstance();

		m_PF_QueueProcessor = PF_WebQueueProcessor.GetInstance();
		m_PF_QueueProcessor.Start();

		int enabledCount = 0;
		for (int i = 0; i < config.Endpoints.Count(); i++)
		{
			PF_WebEndpoint ep = config.Endpoints[i];
			if (ep.Enabled)
			{
				PF_Logger.Log("Endpoint enabled: " + ep.Name + " (" + ep.BaseUrl + ")");
				enabledCount++;
			}
		}

		if (enabledCount == 0)
			PF_Logger.Log("No endpoints enabled. Configure PsyernsFrameworkConfig.json to activate.");

		PF_Logger.Log("Framework initialized. Queue processor running.");
	}

	override void OnUpdate(float timeslice)
	{
		super.OnUpdate(timeslice);

		if (m_PF_QueueProcessor)
			m_PF_QueueProcessor.OnUpdate(timeslice);
	}

	override void OnMissionFinish()
	{
		if (m_PF_QueueProcessor)
			m_PF_QueueProcessor.Stop();

		PF_Logger.Log("Psyerns Framework shutdown.");
		super.OnMissionFinish();
	}
}
