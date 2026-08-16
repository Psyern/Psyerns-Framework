// Stable Timsort utility for the framework (index-permutation based).
//
// Port of the verified Ninjin-Leaderboard sorter. Algorithm: standard
// Timsort (Peters/CPython, as adopted by java.util.TimSort) - natural run
// detection, binary insertion sort up to minrun, merge stack with the
// CORRECTED collapse invariant (includes the runLen[n-4] term the original
// java.util.TimSort omitted), galloping merges.
//
// Usage: extract an int/float/string key per element into a keys array,
// call BuildOrderBy*(keys, ascending, order), then ApplyOrder*(items, order).
// Only the int permutation array is moved during sorting; ref-counted object
// arrays are touched exactly once at the end.
//
// EnScript port notes:
//  - Same-array copies where dest > src MUST run backwards (Java relies on
//    System.arraycopy overlap handling; a forward loop clobbers unread data).
//  - STABLE: equal keys keep their input order.

const int PF_SORT_MIN_MERGE = 32;
const int PF_SORT_MIN_GALLOP = 7;
const int PF_SORT_KEY_INT = 0;
const int PF_SORT_KEY_FLOAT = 1;
const int PF_SORT_KEY_STRING = 2;

class PF_Sort
{
	ref array<int> m_Order;
	ref array<int> m_Tmp;
	ref array<int> m_RunBase;
	ref array<int> m_RunLen;
	ref array<int> m_KeysInt;
	ref array<float> m_KeysFloat;
	ref array<string> m_KeysStr;
	int m_KeyType;
	bool m_Ascending;
	int m_StackSize;
	int m_MinGallop;

	void PF_Sort()
	{
		m_Tmp = new array<int>();
		m_RunBase = new array<int>();
		m_RunLen = new array<int>();
		m_KeyType = PF_SORT_KEY_INT;
		m_Ascending = true;
		m_StackSize = 0;
		m_MinGallop = PF_SORT_MIN_GALLOP;
	}

	// ---------------------------------------------------------- public entries

	// Fills outOrder with the original indices 0..keys.Count()-1, arranged so
	// that reading keys in that order is sorted. Stable.
	static void BuildOrderByInt(array<int> keys, bool ascending, array<int> outOrder)
	{
		PF_Sort sorter;

		if (!keys)
			return;

		if (!PF_Sort.SeedOrder(outOrder, keys.Count()))
			return;

		sorter = new PF_Sort();
		sorter.m_KeysInt = keys;
		sorter.m_KeyType = PF_SORT_KEY_INT;
		sorter.m_Ascending = ascending;
		sorter.m_Order = outOrder;
		sorter.SortOrder();
	}

	static void BuildOrderByFloat(array<float> keys, bool ascending, array<int> outOrder)
	{
		PF_Sort sorter;

		if (!keys)
			return;

		if (!PF_Sort.SeedOrder(outOrder, keys.Count()))
			return;

		sorter = new PF_Sort();
		sorter.m_KeysFloat = keys;
		sorter.m_KeyType = PF_SORT_KEY_FLOAT;
		sorter.m_Ascending = ascending;
		sorter.m_Order = outOrder;
		sorter.SortOrder();
	}

	static void BuildOrderByString(array<string> keys, bool ascending, array<int> outOrder)
	{
		PF_Sort sorter;

		if (!keys)
			return;

		if (!PF_Sort.SeedOrder(outOrder, keys.Count()))
			return;

		sorter = new PF_Sort();
		sorter.m_KeysStr = keys;
		sorter.m_KeyType = PF_SORT_KEY_STRING;
		sorter.m_Ascending = ascending;
		sorter.m_Order = outOrder;
		sorter.SortOrder();
	}

	// Fills outOrder with 0..count-1. Returns false when there is nothing to sort.
	static bool SeedOrder(array<int> outOrder, int count)
	{
		int i;

		if (!outOrder)
			return false;

		outOrder.Clear();
		for (i = 0; i < count; i++)
		{
			outOrder.Insert(i);
		}

		return count > 1;
	}

	// ------------------------------------------------------- order appliers

	static void ApplyOrderPlayers(array<ref PF_WP_PlayerData> items, array<int> order)
	{
		array<ref PF_WP_PlayerData> snapshot;
		int i;
		int count;

		if (!items || !order)
			return;

		count = items.Count();
		if (count != order.Count())
			return;

		snapshot = new array<ref PF_WP_PlayerData>();
		for (i = 0; i < count; i++)
		{
			snapshot.Insert(items.Get(i));
		}

		for (i = 0; i < count; i++)
		{
			items.Set(i, snapshot.Get(order.Get(i)));
		}
	}

	// Ordinal string compare, byte for byte. Kept identical to the comparison
	// the leaderboard columns used before, so name sorting does not change.
	static int CompareStrings(string a, string b)
	{
		int lenA;
		int lenB;
		int minLen;
		int i;
		int charA;
		int charB;

		lenA = a.Length();
		lenB = b.Length();
		minLen = lenA;
		if (lenB < minLen)
			minLen = lenB;

		for (i = 0; i < minLen; i++)
		{
			charA = a.Get(i).ToAscii();
			charB = b.Get(i).ToAscii();
			if (charA < charB)
				return -1;
			if (charA > charB)
				return 1;
		}

		if (lenA < lenB)
			return -1;
		if (lenA > lenB)
			return 1;

		return 0;
	}

	// ------------------------------------------------------------- comparison

	// Compares two ORIGINAL INDICES. <0 means a must come before b.
	int Compare(int a, int b)
	{
		int result;
		int intA;
		int intB;
		float floatA;
		float floatB;

		result = 0;

		if (m_KeyType == PF_SORT_KEY_INT)
		{
			intA = m_KeysInt.Get(a);
			intB = m_KeysInt.Get(b);
			if (intA < intB)
				result = -1;
			else if (intA > intB)
				result = 1;
		}
		else if (m_KeyType == PF_SORT_KEY_FLOAT)
		{
			floatA = m_KeysFloat.Get(a);
			floatB = m_KeysFloat.Get(b);
			if (floatA < floatB)
				result = -1;
			else if (floatA > floatB)
				result = 1;
		}
		else
		{
			result = PF_Sort.CompareStrings(m_KeysStr.Get(a), m_KeysStr.Get(b));
		}

		if (!m_Ascending)
			result = -result;

		return result;
	}

	// ------------------------------------------------------------ sort driver

	void SortOrder()
	{
		int lo;
		int hi;
		int nRemaining;
		int initRunLen;
		int runLen;
		int force;
		int minRun;

		if (!m_Order)
			return;

		lo = 0;
		hi = m_Order.Count();
		nRemaining = hi - lo;

		if (nRemaining < 2)
			return;

		if (nRemaining < PF_SORT_MIN_MERGE)
		{
			initRunLen = CountRunAndMakeOrdered(lo, hi);
			BinarySort(lo, hi, lo + initRunLen);
			return;
		}

		minRun = PF_Sort.MinRunLength(nRemaining);
		while (nRemaining != 0)
		{
			runLen = CountRunAndMakeOrdered(lo, hi);
			if (runLen < minRun)
			{
				force = nRemaining;
				if (force > minRun)
					force = minRun;
				BinarySort(lo, lo + force, lo + runLen);
				runLen = force;
			}

			PushRun(lo, runLen);
			MergeCollapse();
			lo = lo + runLen;
			nRemaining = nRemaining - runLen;
		}

		MergeForceCollapse();
	}

	// Minimum run length: keeps the number of runs a power of two (or just below)
	// so the merges stay balanced. Returns n itself for n < MIN_MERGE.
	static int MinRunLength(int n)
	{
		int r;
		int m;

		r = 0;
		m = n;
		while (m >= PF_SORT_MIN_MERGE)
		{
			r = r | (m & 1);
			m = m >> 1;
		}

		return m + r;
	}

	// Length of the natural run at lo. A run that is ordered the wrong way is
	// reversed in place. The reverse scan stops at ties (strict comparison only),
	// which is what preserves stability.
	int CountRunAndMakeOrdered(int lo, int hi)
	{
		int runHi;

		runHi = lo + 1;
		if (runHi == hi)
			return 1;

		if (Compare(m_Order.Get(runHi), m_Order.Get(lo)) < 0)
		{
			runHi = runHi + 1;
			while (runHi < hi && Compare(m_Order.Get(runHi), m_Order.Get(runHi - 1)) < 0)
			{
				runHi = runHi + 1;
			}
			ReverseRange(lo, runHi);
		}
		else
		{
			runHi = runHi + 1;
			while (runHi < hi && Compare(m_Order.Get(runHi), m_Order.Get(runHi - 1)) >= 0)
			{
				runHi = runHi + 1;
			}
		}

		return runHi - lo;
	}

	void ReverseRange(int lo, int hi)
	{
		int a;
		int b;
		int t;

		a = lo;
		b = hi - 1;
		while (a < b)
		{
			t = m_Order.Get(a);
			m_Order.Set(a, m_Order.Get(b));
			m_Order.Set(b, t);
			a = a + 1;
			b = b - 1;
		}
	}

	// Binary insertion sort of [lo,hi), given [lo,start) is already sorted.
	void BinarySort(int lo, int hi, int startIn)
	{
		int start;
		int pivot;
		int left;
		int right;
		int mid;
		int n;
		int i;

		start = startIn;
		if (start == lo)
			start = start + 1;

		while (start < hi)
		{
			pivot = m_Order.Get(start);
			left = lo;
			right = start;

			while (left < right)
			{
				mid = (left + right) / 2;
				if (Compare(pivot, m_Order.Get(mid)) < 0)
					right = mid;
				else
					left = mid + 1;
			}

			n = start - left;
			for (i = 0; i < n; i++)
			{
				m_Order.Set(start - i, m_Order.Get(start - i - 1));
			}
			m_Order.Set(left, pivot);

			start = start + 1;
		}
	}

	// --------------------------------------------------------------- run stack

	void PushRun(int runBase, int runLen)
	{
		while (m_RunBase.Count() <= m_StackSize)
		{
			m_RunBase.Insert(0);
			m_RunLen.Insert(0);
		}

		m_RunBase.Set(m_StackSize, runBase);
		m_RunLen.Set(m_StackSize, runLen);
		m_StackSize = m_StackSize + 1;
	}

	// Restores runLen[n-3] > runLen[n-2] + runLen[n-1] and runLen[n-2] > runLen[n-1].
	void MergeCollapse()
	{
		int n;

		while (m_StackSize > 1)
		{
			n = m_StackSize - 2;

			if (n > 0 && m_RunLen.Get(n - 1) <= m_RunLen.Get(n) + m_RunLen.Get(n + 1))
			{
				if (m_RunLen.Get(n - 1) < m_RunLen.Get(n + 1))
					n = n - 1;
				MergeAt(n);
			}
			else if (n > 1 && m_RunLen.Get(n - 2) <= m_RunLen.Get(n - 1) + m_RunLen.Get(n))
			{
				if (m_RunLen.Get(n - 1) < m_RunLen.Get(n + 1))
					n = n - 1;
				MergeAt(n);
			}
			else if (m_RunLen.Get(n) <= m_RunLen.Get(n + 1))
			{
				MergeAt(n);
			}
			else
			{
				break;
			}
		}
	}

	void MergeForceCollapse()
	{
		int n;

		while (m_StackSize > 1)
		{
			n = m_StackSize - 2;
			if (n > 0 && m_RunLen.Get(n - 1) < m_RunLen.Get(n + 1))
				n = n - 1;
			MergeAt(n);
		}
	}

	void MergeAt(int i)
	{
		int base1;
		int len1;
		int base2;
		int len2;
		int k;

		base1 = m_RunBase.Get(i);
		len1 = m_RunLen.Get(i);
		base2 = m_RunBase.Get(i + 1);
		len2 = m_RunLen.Get(i + 1);

		m_RunLen.Set(i, len1 + len2);
		if (i == m_StackSize - 3)
		{
			m_RunBase.Set(i + 1, m_RunBase.Get(i + 2));
			m_RunLen.Set(i + 1, m_RunLen.Get(i + 2));
		}
		m_StackSize = m_StackSize - 1;

		// Head of run1 that already precedes all of run2 needs no merging.
		k = GallopRight(m_Order.Get(base2), m_Order, base1, len1, 0);
		base1 = base1 + k;
		len1 = len1 - k;
		if (len1 == 0)
			return;

		// Tail of run2 that already follows all of run1 needs no merging.
		len2 = GallopLeft(m_Order.Get(base1 + len1 - 1), m_Order, base2, len2, len2 - 1);
		if (len2 == 0)
			return;

		if (len1 <= len2)
			MergeLo(base1, len1, base2, len2);
		else
			MergeHi(base1, len1, base2, len2);
	}

	// --------------------------------------------------------------- galloping

	// Leftmost index in src[base, base+len) where `key` could be inserted.
	int GallopLeft(int key, array<int> src, int base, int len, int hint)
	{
		int lastOfs;
		int ofs;
		int maxOfs;
		int tmp;
		int m;

		lastOfs = 0;
		ofs = 1;

		if (Compare(key, src.Get(base + hint)) > 0)
		{
			maxOfs = len - hint;
			while (ofs < maxOfs && Compare(key, src.Get(base + hint + ofs)) > 0)
			{
				lastOfs = ofs;
				ofs = (ofs << 1) + 1;
				if (ofs <= 0)
					ofs = maxOfs;
			}
			if (ofs > maxOfs)
				ofs = maxOfs;
			lastOfs = lastOfs + hint;
			ofs = ofs + hint;
		}
		else
		{
			maxOfs = hint + 1;
			while (ofs < maxOfs && Compare(key, src.Get(base + hint - ofs)) <= 0)
			{
				lastOfs = ofs;
				ofs = (ofs << 1) + 1;
				if (ofs <= 0)
					ofs = maxOfs;
			}
			if (ofs > maxOfs)
				ofs = maxOfs;
			tmp = lastOfs;
			lastOfs = hint - ofs;
			ofs = hint - tmp;
		}

		lastOfs = lastOfs + 1;
		while (lastOfs < ofs)
		{
			m = lastOfs + ((ofs - lastOfs) / 2);
			if (Compare(key, src.Get(base + m)) > 0)
				lastOfs = m + 1;
			else
				ofs = m;
		}

		return ofs;
	}

	// Rightmost index in src[base, base+len) where `key` could be inserted.
	int GallopRight(int key, array<int> src, int base, int len, int hint)
	{
		int ofs;
		int lastOfs;
		int maxOfs;
		int tmp;
		int m;

		ofs = 1;
		lastOfs = 0;

		if (Compare(key, src.Get(base + hint)) < 0)
		{
			maxOfs = hint + 1;
			while (ofs < maxOfs && Compare(key, src.Get(base + hint - ofs)) < 0)
			{
				lastOfs = ofs;
				ofs = (ofs << 1) + 1;
				if (ofs <= 0)
					ofs = maxOfs;
			}
			if (ofs > maxOfs)
				ofs = maxOfs;
			tmp = lastOfs;
			lastOfs = hint - ofs;
			ofs = hint - tmp;
		}
		else
		{
			maxOfs = len - hint;
			while (ofs < maxOfs && Compare(key, src.Get(base + hint + ofs)) >= 0)
			{
				lastOfs = ofs;
				ofs = (ofs << 1) + 1;
				if (ofs <= 0)
					ofs = maxOfs;
			}
			if (ofs > maxOfs)
				ofs = maxOfs;
			lastOfs = lastOfs + hint;
			ofs = ofs + hint;
		}

		lastOfs = lastOfs + 1;
		while (lastOfs < ofs)
		{
			m = lastOfs + ((ofs - lastOfs) / 2);
			if (Compare(key, src.Get(base + m)) < 0)
				ofs = m;
			else
				lastOfs = m + 1;
		}

		return ofs;
	}

	// ------------------------------------------------------------------ merges

	void CopyToTmp(int base, int len)
	{
		int i;

		while (m_Tmp.Count() < len)
		{
			m_Tmp.Insert(0);
		}

		for (i = 0; i < len; i++)
		{
			m_Tmp.Set(i, m_Order.Get(base + i));
		}
	}

	// Merges adjacent runs where run1 is the shorter one. run1 is buffered into
	// m_Tmp and the merge walks forward from the low end.
	void MergeLo(int base1, int len1In, int base2, int len2In)
	{
		int len1;
		int len2;
		int cursor1;
		int cursor2;
		int dest;
		int i;
		int count1;
		int count2;
		int minGallop;
		bool done;

		len1 = len1In;
		len2 = len2In;
		cursor1 = 0;
		cursor2 = base2;
		dest = base1;
		minGallop = m_MinGallop;
		done = false;

		CopyToTmp(base1, len1);

		m_Order.Set(dest, m_Order.Get(cursor2));
		dest = dest + 1;
		cursor2 = cursor2 + 1;
		len2 = len2 - 1;

		if (len2 == 0)
		{
			for (i = 0; i < len1; i++)
			{
				m_Order.Set(dest + i, m_Tmp.Get(cursor1 + i));
			}
			return;
		}

		if (len1 == 1)
		{
			for (i = 0; i < len2; i++)
			{
				m_Order.Set(dest + i, m_Order.Get(cursor2 + i));
			}
			m_Order.Set(dest + len2, m_Tmp.Get(cursor1));
			return;
		}

		while (!done)
		{
			count1 = 0;
			count2 = 0;

			// one element at a time, until one run wins consistently
			while (true)
			{
				if (Compare(m_Order.Get(cursor2), m_Tmp.Get(cursor1)) < 0)
				{
					m_Order.Set(dest, m_Order.Get(cursor2));
					dest = dest + 1;
					cursor2 = cursor2 + 1;
					count2 = count2 + 1;
					count1 = 0;
					len2 = len2 - 1;
					if (len2 == 0)
					{
						done = true;
						break;
					}
				}
				else
				{
					m_Order.Set(dest, m_Tmp.Get(cursor1));
					dest = dest + 1;
					cursor1 = cursor1 + 1;
					count1 = count1 + 1;
					count2 = 0;
					len1 = len1 - 1;
					if (len1 == 1)
					{
						done = true;
						break;
					}
				}

				if ((count1 | count2) >= minGallop)
					break;
			}

			if (done)
				break;

			// galloping mode: copy whole blocks at once
			while (true)
			{
				count1 = GallopRight(m_Order.Get(cursor2), m_Tmp, cursor1, len1, 0);
				if (count1 != 0)
				{
					for (i = 0; i < count1; i++)
					{
						m_Order.Set(dest + i, m_Tmp.Get(cursor1 + i));
					}
					dest = dest + count1;
					cursor1 = cursor1 + count1;
					len1 = len1 - count1;
					if (len1 <= 1)
					{
						done = true;
						break;
					}
				}

				m_Order.Set(dest, m_Order.Get(cursor2));
				dest = dest + 1;
				cursor2 = cursor2 + 1;
				len2 = len2 - 1;
				if (len2 == 0)
				{
					done = true;
					break;
				}

				count2 = GallopLeft(m_Tmp.Get(cursor1), m_Order, cursor2, len2, 0);
				if (count2 != 0)
				{
					// m_Order -> m_Order with dest < cursor2: forward copy is safe
					for (i = 0; i < count2; i++)
					{
						m_Order.Set(dest + i, m_Order.Get(cursor2 + i));
					}
					dest = dest + count2;
					cursor2 = cursor2 + count2;
					len2 = len2 - count2;
					if (len2 == 0)
					{
						done = true;
						break;
					}
				}

				m_Order.Set(dest, m_Tmp.Get(cursor1));
				dest = dest + 1;
				cursor1 = cursor1 + 1;
				len1 = len1 - 1;
				if (len1 == 1)
				{
					done = true;
					break;
				}

				minGallop = minGallop - 1;
				if (count1 < PF_SORT_MIN_GALLOP && count2 < PF_SORT_MIN_GALLOP)
					break;
			}

			if (done)
				break;

			if (minGallop < 0)
				minGallop = 0;
			minGallop = minGallop + 2;
		}

		if (minGallop < 1)
			minGallop = 1;
		m_MinGallop = minGallop;

		if (len1 == 1)
		{
			for (i = 0; i < len2; i++)
			{
				m_Order.Set(dest + i, m_Order.Get(cursor2 + i));
			}
			m_Order.Set(dest + len2, m_Tmp.Get(cursor1));
		}
		else
		{
			for (i = 0; i < len1; i++)
			{
				m_Order.Set(dest + i, m_Tmp.Get(cursor1 + i));
			}
		}
	}

	// Mirror of MergeLo for when run2 is the shorter one: run2 is buffered into
	// m_Tmp and the merge walks backward from the high end.
	void MergeHi(int base1, int len1In, int base2, int len2In)
	{
		int len1;
		int len2;
		int cursor1;
		int cursor2;
		int dest;
		int i;
		int count1;
		int count2;
		int minGallop;
		bool done;

		len1 = len1In;
		len2 = len2In;
		cursor1 = base1 + len1 - 1;
		dest = base2 + len2 - 1;
		minGallop = m_MinGallop;
		done = false;

		CopyToTmp(base2, len2);
		cursor2 = len2 - 1;

		m_Order.Set(dest, m_Order.Get(cursor1));
		dest = dest - 1;
		cursor1 = cursor1 - 1;
		len1 = len1 - 1;

		if (len1 == 0)
		{
			for (i = 0; i < len2; i++)
			{
				m_Order.Set(dest - i, m_Tmp.Get(cursor2 - i));
			}
			return;
		}

		if (len2 == 1)
		{
			dest = dest - len1;
			cursor1 = cursor1 - len1;
			// m_Order -> m_Order with dest > cursor1: MUST copy backwards
			for (i = len1; i >= 1; i--)
			{
				m_Order.Set(dest + i, m_Order.Get(cursor1 + i));
			}
			m_Order.Set(dest, m_Tmp.Get(cursor2));
			return;
		}

		while (!done)
		{
			count1 = 0;
			count2 = 0;

			while (true)
			{
				if (Compare(m_Tmp.Get(cursor2), m_Order.Get(cursor1)) < 0)
				{
					m_Order.Set(dest, m_Order.Get(cursor1));
					dest = dest - 1;
					cursor1 = cursor1 - 1;
					count1 = count1 + 1;
					count2 = 0;
					len1 = len1 - 1;
					if (len1 == 0)
					{
						done = true;
						break;
					}
				}
				else
				{
					m_Order.Set(dest, m_Tmp.Get(cursor2));
					dest = dest - 1;
					cursor2 = cursor2 - 1;
					count2 = count2 + 1;
					count1 = 0;
					len2 = len2 - 1;
					if (len2 == 1)
					{
						done = true;
						break;
					}
				}

				if ((count1 | count2) >= minGallop)
					break;
			}

			if (done)
				break;

			while (true)
			{
				count1 = len1 - GallopRight(m_Tmp.Get(cursor2), m_Order, base1, len1, len1 - 1);
				if (count1 != 0)
				{
					dest = dest - count1;
					cursor1 = cursor1 - count1;
					len1 = len1 - count1;
					// m_Order -> m_Order with dest > cursor1: MUST copy backwards
					for (i = count1; i >= 1; i--)
					{
						m_Order.Set(dest + i, m_Order.Get(cursor1 + i));
					}
					if (len1 == 0)
					{
						done = true;
						break;
					}
				}

				m_Order.Set(dest, m_Tmp.Get(cursor2));
				dest = dest - 1;
				cursor2 = cursor2 - 1;
				len2 = len2 - 1;
				if (len2 == 1)
				{
					done = true;
					break;
				}

				count2 = len2 - GallopLeft(m_Order.Get(cursor1), m_Tmp, 0, len2, len2 - 1);
				if (count2 != 0)
				{
					dest = dest - count2;
					cursor2 = cursor2 - count2;
					len2 = len2 - count2;
					for (i = 1; i <= count2; i++)
					{
						m_Order.Set(dest + i, m_Tmp.Get(cursor2 + i));
					}
					if (len2 <= 1)
					{
						done = true;
						break;
					}
				}

				m_Order.Set(dest, m_Order.Get(cursor1));
				dest = dest - 1;
				cursor1 = cursor1 - 1;
				len1 = len1 - 1;
				if (len1 == 0)
				{
					done = true;
					break;
				}

				minGallop = minGallop - 1;
				if (count1 < PF_SORT_MIN_GALLOP && count2 < PF_SORT_MIN_GALLOP)
					break;
			}

			if (done)
				break;

			if (minGallop < 0)
				minGallop = 0;
			minGallop = minGallop + 2;
		}

		if (minGallop < 1)
			minGallop = 1;
		m_MinGallop = minGallop;

		if (len2 == 1)
		{
			dest = dest - len1;
			cursor1 = cursor1 - len1;
			// m_Order -> m_Order with dest > cursor1: MUST copy backwards
			for (i = len1; i >= 1; i--)
			{
				m_Order.Set(dest + i, m_Order.Get(cursor1 + i));
			}
			m_Order.Set(dest, m_Tmp.Get(cursor2));
		}
		else
		{
			for (i = 0; i < len2; i++)
			{
				m_Order.Set(dest - i, m_Tmp.Get(cursor2 - i));
			}
		}
	}
}
