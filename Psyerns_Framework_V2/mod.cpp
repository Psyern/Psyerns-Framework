name = "Psyerns Framework";
author = "Psyern";
version = "1.0.0";
extra = 0;
type = "mod";

class CfgMods
{
	class Psyerns_Framework
	{
		type = "mod";
		name = "Psyerns Framework";
		author = "Psyern";
		credits = "Psyern, Deadmans Echo Community";
		version = "1.0.0";
		extra = 0;

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
