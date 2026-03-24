class CfgPatches
{
	class Psyerns_Framework
	{
		units[] = {};
		weapons[] = {};
		requiredVersion = 0.1;
		requiredAddons[] = {"DZ_Data"};
	};
};

class CfgMods
{
	class Psyerns_Framework
	{
		type = "mod";
		name = "Psyerns Framework";
		author = "Psyern";
		credits = "Psyern, Deadmans Echo Community";
		version = "1.0.0";
		inputs = "Psyerns_Framework/data/modded_inputs.xml";

		class defs
		{
			class gameScriptModule
			{
				value = "";
				files[] = {"Psyerns_Framework/scripts/3_Game"};
			};
			class worldScriptModule
			{
				value = "";
				files[] = {"Psyerns_Framework/scripts/4_World"};
			};
			class missionScriptModule
			{
				value = "";
				files[] = {"Psyerns_Framework/scripts/5_Mission"};
			};
		};
	};
};
