/**
 * Psyerns_Framework — DME_Api
 *
 * Original work Copyright (c) daemonforge — DayZ-UniversalApi (AGPL-3.0)
 *   https://github.com/daemonforge/DayZ-UniveralApi
 * Modifications & integration Copyright (c) 2026 Psyern / Deadmans Echo
 * Modified by Deadmans Echo, 2026.   [AGPL §5(a) change notice]
 *
 * This file is part of Psyerns_Framework and is licensed under the
 * GNU Affero General Public License v3.0. See LICENSE in the repo root.
 * SPDX-License-Identifier: AGPL-3.0-only
 */
class DME_Api_AmmoData extends Managed{
	protected int m_cartIndex;
	protected float m_dmg;
	protected string m_cartTypeName;
	
	void DME_Api_AmmoData(int cartIndex, float dmg, string cartTypeName){
		m_cartIndex = cartIndex;
		m_dmg = dmg;
		m_cartTypeName = cartTypeName;
	}
	
	int cartIndex(){return m_cartIndex;}	
	float dmg(){return m_dmg;}
	string cartTypeName(){return m_cartTypeName;}
}

class DME_Api_MetaData extends Managed{
	protected string Var;
	protected string Data;
	
	void DME_Api_MetaData(string var, string data){
		Var = var;
		Data = data;
	}
	
	bool Is(string var){ return Var == var;}
	string ReadString(){return Data;}
	int ReadInt(){return Data.ToInt();	}
	float ReadFloat(){return Data.ToFloat();}
	vector ReadVector(){ return Data.ToVector(); }
}


class DME_Api_ZoneHealthData extends Managed{
	string m_Zone;
	float m_Health;
	
	void DME_Api_ZoneHealthData(string zone, float health){
		m_Zone = zone;
		m_Health = health;
	}
	
	bool Is(string zone){ return (m_Zone == zone); }
	string Zone(){ return m_Zone; }
	float Health(){ return m_Health; }
}
