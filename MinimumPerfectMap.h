// Map-Like implementation using the BooPHF (minimal perfect hash function) library

#include "BooPHF.h"

namespace BBHash {

	template <typename _KeyType, typename _ValueType>
	class MinimumPerfectMap {

		class GenericHasher {
		public:
			uint64_t operator () (_KeyType key, uint64_t seed=0) const {
				return hashFn(key);
			}
		     std::hash<_KeyType> hashFn;
		};

	private:

		// Boo's minimum perfect hash structure
		boomphf::mphf<_KeyType, GenericHasher>* bphf;

		// keys
		const _KeyType* keysArray;
		unsigned  keysArrayLength;

		// values
		_ValueType* valuesArray;
		unsigned    valuesArrayLength;

	public:

		MinimumPerfectMap(const _KeyType* keysArray, unsigned keysArrayLength, unsigned nThreads)
				: keysArray(keysArray)
				, keysArrayLength(keysArrayLength) {

			cerr << "Instantiating MinimumPerfectMap with "<<keysArrayLength<<" keys..." << endl << flush;

			// mphf takes as input a c++ range. A simple array of keys can be wrapped with boomphf::range
			// but could be from a user defined iterator (enabling keys to be read from a file or from some complex non-contiguous structure)
			boomphf::iter_range dataIterator = boomphf::range<const _KeyType*>(keysArray, keysArray+keysArrayLength);

			double gammaFactor = 1.0; // lowest bit/elem is achieved with gamma=1, higher values lead to larger mphf but faster construction/query

			//build the mphf
			bphf = new boomphf::mphf<_KeyType, GenericHasher>(keysArrayLength, dataIterator, nThreads, gammaFactor, false, 0.0);

			cerr << "\tInstantiated. Now determining minimum and maximum keys... " << flush;
			unsigned min = -1;
			unsigned max = 0;
			for (unsigned i=0; i<keysArrayLength; i++) {
				_KeyType key = keysArray[i];
				unsigned index = bphf->lookup(key);
				if (index < min) min = index;
				if (index > max) max = index;
			}
			cerr << "min=" << min << "; max=" << max << endl << flush;

			valuesArrayLength = max;
			valuesArray = new _ValueType[valuesArrayLength];

			clear();
		}

		~MinimumPerfectMap() {
			delete[] valuesArray;
			delete bphf;
		}

		// map methods
		//////////////

		_ValueType& operator [] (_KeyType key) {
			uint64_t index = bphf->lookup(key);
	        return valuesArray[index];
	    }

		/** fill with zeroes all values */
		void clear() {
			memset(valuesArray, valuesArrayLength, sizeof(valuesArray[0]));
		}

		void erase(_KeyType key) {
			uint64_t index = bphf->lookup(key);
			memset(&valuesArray[index], 1, sizeof(valuesArray[0]));
		}

	};
}
