class CfgPatches
{
	class Psyerns_Framework
	{
		units[] = {};
		weapons[] = {};
		requiredVersion = 0.1;
		requiredAddons[] = {"DZ_Data", "JM_CF_Scripts"};
	};
};

class CfgMods
{
	class Psyerns_Framework
	{
		type = "mod";
		name = "Psyerns Framework";
		author = "Psyern";
		credits = "Psyern, Deadmans Echo Community. Incorporates DayZ-UniversalApi by daemonforge (AGPL-3.0). Whole work AGPL-3.0.";
		version = "1.0.0";
		inputs = "Psyerns_Framework/data/modded_inputs.xml";
		dependencies[] = {"Core", "Game", "World", "Mission"};

		class defs
		{
			class engineScriptModule
			{
				value = "";
				files[] = {
					"Psyerns_Framework/scripts/Common",
					"Psyerns_Framework/scripts/1_Core"
				};
			};
			class gameScriptModule
			{
				value = "";
				files[] = {
					"Psyerns_Framework/scripts/Common",
					"Psyerns_Framework/scripts/3_Game"
				};
			};
			class worldScriptModule
			{
				value = "";
				files[] = {
					"Psyerns_Framework/scripts/Common",
					"Psyerns_Framework/scripts/4_World"
				};
			};
			class missionScriptModule
			{
				value = "";
				files[] = {
					"Psyerns_Framework/scripts/Common",
					"Psyerns_Framework/scripts/5_Mission"
				};
			};
		};
	};
};
