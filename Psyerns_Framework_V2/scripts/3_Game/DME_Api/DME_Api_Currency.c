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
class DME_Api_CurrencyValue extends Managed {
	protected string m_TypeClass = "";
	protected int m_Value = 0;
	
	void DME_Api_CurrencyValue(string itemType, int value){
		m_TypeClass = itemType;
		m_Value = value;
	}

	string TypeClass(){
		return m_TypeClass;
	}
	
	int Value(){
		return m_Value;
	}
}



typedef array<ref DME_Api_CurrencyValue> DME_Api_CurrencyBase;

class DME_Api_Currency extends DME_Api_CurrencyBase{
	protected static ref map<string, ref DME_Api_Currency> m_UCurrencysMap = new map<string, ref DME_Api_Currency>;
	
	static DME_Api_Currency GetCurrency(string key){
		return m_UCurrencysMap.Get(key);
	}
	
	static int GetLastIndex(string key){
		return m_UCurrencysMap.Get(key).Count() - 1;
	}
	
	static int GetLowestDenominationValue(string key){
		return m_UCurrencysMap.Get(key).LowestDenominationValue();
	}
	
	static DME_Api_Currency Register(string key, TStringIntMap currency){
		DME_Api_Currency uCurrency = new DME_Api_Currency;
		uCurrency.Create(currency);
		m_UCurrencysMap.Set(key, uCurrency);
		return uCurrency;
	}
	
	static DME_Api_Currency InsertNew(string key, array<ref DME_Api_CurrencyValue> values){
		DME_Api_Currency uCurrency = new DME_Api_Currency;
		for (int i = 0; i < values.Count(); i++){
			DME_Api_CurrencyValue value = DME_Api_CurrencyValue.Cast(values.Get(i));
			DME_Api_CurrencyValue v = new DME_Api_CurrencyValue(value.TypeClass(), value.Value());
			uCurrency.Insert( v );
		}
		m_UCurrencysMap.Set(key, uCurrency);
		return uCurrency;
	}
	
	static int GetAmount(DME_Api_CurrencyValue MoneyObj, int amount){
		if (MoneyObj){
			return Math.Floor(amount / MoneyObj.Value());
		} 
		return 0;
	}
	
	static void UDebug(){
		Print("[DME_Api] DME_Api_Currency Debug Start ---------");
		for (int i = 0; i < m_UCurrencysMap.Count(); i++){
			Print("- - - - - - - - - - -");
			Print(m_UCurrencysMap.GetKey(i));
			Print(m_UCurrencysMap.GetElement(i));
			m_UCurrencysMap.GetElement(i).Debug();
			Print("- - - - - - - - - - -");
		}
		Print("[DME_Api] DME_Api_Currency Debug End  ---------");
	}
	
	protected bool m_IsSorted = false;
	protected bool m_CanUseRuined = true;
	
	void SetCanUseRuined(bool state = true){
		m_CanUseRuined = state;
	}
	
	bool CanUseRuined(){
		return m_CanUseRuined;
	}
	
	void SortCurrency(){
		if (m_IsSorted) return;
		array<ref DME_Api_CurrencyValue> StartingValues =  new array<ref DME_Api_CurrencyValue>;
		array<ref DME_Api_CurrencyValue> SortedMoneyValues = new array<ref DME_Api_CurrencyValue>;
		
		for (int h = 0; h < Count(); h++){
			StartingValues.Insert(Get(h));
		}
		while (StartingValues.Count() > 0){
			DME_Api_CurrencyValue HighestValue = StartingValues.Get(0);
			for (int i = 1; i < StartingValues.Count(); i++){
				if (StartingValues.Get(i).Value() > HighestValue.Value()){
					HighestValue = StartingValues.Get(i);
				}
			}
			SortedMoneyValues.Insert(HighestValue);
			StartingValues.RemoveItem(HighestValue);
		}
		if (StartingValues.Count() == 1){
			SortedMoneyValues.Insert(StartingValues.Get(0));
		}
		Clear();
		for (int j = 0; j < SortedMoneyValues.Count(); j++){
			Insert(SortedMoneyValues.Get(j));
		}
		m_IsSorted = true;
	}
	
	
	DME_Api_CurrencyValue GetHighestDenomination(int amount){
		SortCurrency();
		int LastIndex = Count() - 1;
		for (int i = 0; i < Count(); i++){
			if (GetAmount(Get(i), amount) > 0){
				return Get(i);
			}
		}
		return NULL;
	}
	
	string GetTypeClass(int idx){
		return Get(idx).TypeClass();
	}
	
	int GetValue(int idx){
		return Get(idx).Value();
	}
	
	
	void Add(string typeName, int value){
		Insert(new DME_Api_CurrencyValue(typeName, value));
		m_IsSorted = false;
	}
	
	void Create(TStringIntMap values){
		if (!values) return;
		for (int i = 0; i < values.Count(); i++){
			Add( values.GetKey(i), values.GetElement(i));
		}
		SortCurrency();
	}
	int LastIndex(){
		return Count() - 1;
	}
	int LowestDenominationValue(){
		SortCurrency();
		return Get(LastIndex()).Value();
	}
	DME_Api_CurrencyValue LowestDenomination(){
		SortCurrency();
		return Get(LastIndex());
	}
}