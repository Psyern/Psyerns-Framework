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
modded class Math
{

	protected static ref TIntArray s_DME_Api_QRandomNumbers = new TIntArray;

	//Adds a new array shouldn't be called manually Use DME_Api().CheckAndRenewQRandom();
	static void AddQRandomNumber(TIntArray numbers){
		if (!s_DME_Api_QRandomNumbers){
			s_DME_Api_QRandomNumbers = new TIntArray;
		}
		s_DME_Api_QRandomNumbers.InsertAll(numbers);
	}

	//returns the remaining Random numbers to choose from
	static int QRandomRemaining(){
		if (!s_DME_Api_QRandomNumbers){
			return 0;
		}
		return s_DME_Api_QRandomNumbers.Count();
	}

	//Gets the number and removes it from the array
	protected static int GetAndRemoveNumber(){
		int idx = s_DME_Api_QRandomNumbers.GetRandomIndex();
		int number = s_DME_Api_QRandomNumbers.Get(idx);
		s_DME_Api_QRandomNumbers.Remove(idx);
		return number;
	}

	//returns a random number across the full int range
	static int QRandom(){
		if (QRandomRemaining() <= 0){
			Error2("[DME_Api] QRandom", "Q RANDOM OUT OF NUMBERS USING VANILLA RANDOM");
			//Avoid RandomInt(int.MIN, int.MAX): the (max-min) span overflows and int.MIN boundary comparisons are unreliable in EnScript
			int fallback = RandomInt(0, int.MAX);
			bool negate = (RandomInt(0, 2) == 1);
			if (negate){
				fallback = -fallback;
			}
			return fallback;
		}
		int number = GetAndRemoveNumber();
		return number;
	}

	//returns a random integer max difference between numbers is int.MAX(2147483647)
	//Unless returning a number between 0 and int.MAX exactly I would recomend not doing more than a difference of 10,000(ish) use random float instead
	static int QRandomInt(int min = 0, int max = int.MAX){
		if (QRandomRemaining() <= 0){
			Error2("[DME_Api] QRandomInt", "Q RANDOM OUT OF NUMBERS USING VANILLA RANDOM");
			return RandomInt(min, max);
		}
		if (min == max){
			return min;
		}
		if (max < min){
			int tmp = max;
			max = min;
			min = tmp;
		}
		int number = Math.AbsInt(GetAndRemoveNumber());
		if (min == 0 && max == int.MAX){
			return number;
		}

		int diff = max - min;
		int randomNum = number % (diff + 1);

		return min + randomNum;
	}

	//returns a random float
	static float QRandomFloat(float min = 0, float max = 1){
		if (QRandomRemaining() <= 0){
			Error2("[DME_Api] QRandomFloat", "Q RANDOM OUT OF NUMBERS USING VANILLA RANDOM");
			return RandomFloat(min, max);
		}
		if (min == max){
			return min;
		}
		if (max < min){
			float tmp = max;
			max = min;
			min = tmp;
		}
		int number = Math.AbsInt(GetAndRemoveNumber());
		//Promote to float BEFORE dividing to avoid integer division (which yielded 0)
		float num = number;
		num = num / int.MAX;
		float diff = max - min;
		float dnum = diff * num;
		return  dnum + min;
	}

	//returns a random true or false value
	static bool QRandomFlip(){
		if (QRandomRemaining() <= 0){
			Error2("[DME_Api] QRandomFlip", "Q RANDOM OUT OF NUMBERS USING VANILLA RANDOM");
			int retval = RandomInt(1, int.MAX) % 2;
			return ( retval != 0);
		}
		int number = Math.AbsInt(GetAndRemoveNumber());
		int reval = number % 2;
		return (reval != 0);

	}
}
