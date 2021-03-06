#pragma once
// posit_16_1.hpp: specialized 16-bit posit using fast compute specialized for posit<16,1>
//
// Copyright (C) 2017-2019 Stillwater Supercomputing, Inc.
//
// This file is part of the universal numbers project, which is released under an MIT Open Source license.

namespace sw {
  namespace unum {

// set the fast specialization variable to indicate that we are running a special template specialization
#if POSIT_FAST_POSIT_16_1
#pragma message("Fast specialization of posit<16,1>")

	// fast specialized posit<16,1>
	template<>
	class posit<NBITS_IS_16, ES_IS_1> {
	public:
		static constexpr size_t nbits = NBITS_IS_16;
		static constexpr size_t es = ES_IS_1;
		static constexpr size_t sbits = 1;
		static constexpr size_t rbits = nbits - sbits;
		static constexpr size_t ebits = es;
		static constexpr size_t fbits = nbits - 3 - es;
		static constexpr size_t fhbits = fbits + 1;
		static constexpr uint16_t sign_mask = 0x8000u;

		posit() { _bits = 0; }
		posit(const posit&) = default;
		posit(posit&&) = default;
		posit& operator=(const posit&) = default;
		posit& operator=(posit&&) = default;

		// initializers for native types
		posit(const signed char initial_value)        { *this = initial_value; }
		posit(const short initial_value)              { *this = initial_value; }
		posit(const int initial_value)                { *this = initial_value; }
		posit(const long initial_value)               { *this = initial_value; }
		posit(const long long initial_value)          { *this = initial_value; }
		posit(const char initial_value)               { *this = initial_value; }
		posit(const unsigned short initial_value)     { *this = initial_value; }
		posit(const unsigned int initial_value)       { *this = initial_value; }
		posit(const unsigned long initial_value)      { *this = initial_value; }
		posit(const unsigned long long initial_value) { *this = initial_value; }
		posit(const float initial_value)              { *this = initial_value; }
		posit(const double initial_value)             { *this = initial_value; }
		posit(const long double initial_value)        { *this = initial_value; }

		// assignment operators for native types
		posit& operator=(const signed char rhs)       { return operator=((long)(rhs)); }
		posit& operator=(const short rhs)             { return operator=((long)(rhs)); }
		posit& operator=(const int rhs)               { return operator=((long)(rhs)); }
		posit& operator=(const long rhs)              { 
			// special case for speed as this is a common initialization
			if (rhs == 0) {
				_bits = 0x0;
				return *this;
			}

			bool sign = (rhs < 0);
			uint32_t v = sign ? -rhs : rhs; // project to positve side of the projective reals
			uint16_t raw = 0;
			if (v == sign_mask) { // +-maxpos, 0x8000 is special in int16 arithmetic as it is its own negation
				_bits = 0x8000;
				return *this;
			}
			else if (v > 0x0800'0000) { // v > 134,217,728
				raw = 0x7FFFu;  // +-maxpos
			}
			else if (v > 0x02FF'FFFF) { // 50,331,647 < v < 134,217,728
				raw = 0x7FFEu;  // 0.5 of maxpos
			}
			else if (v < 2) {  // v == 0 or v == 1
				raw = (v << 14); // generates 0x0000 if v is 0, or 0x4000 if 1
			}
			else {
				uint32_t mask = 0x0200'0000;
				int8_t scale = 25;
				uint32_t fraction_bits = v;
				while (!(fraction_bits & mask)) {
					--scale;
					fraction_bits <<= 1;
				}
				int8_t k = scale >> 1;
				uint16_t exp = (scale & 0x01) << (12 - k); // extract exponent and shift to correct location
				fraction_bits = (fraction_bits ^ mask);
				raw = (0x7FFF ^ (0x3FFF >> k)) | exp |  (fraction_bits >> (k + 13));

				mask = 0x1000 << k; // bitNPlusOne
				if (mask & fraction_bits) {
					if (((mask - 1) & fraction_bits) | ((mask << 1) & fraction_bits)) raw++; // increment by 1
				}
			}
			_bits = sign ? -raw : raw;
			return *this;
		}
		posit& operator=(const long long rhs)         { return operator=((long)(rhs)); }
		posit& operator=(const char rhs)              { return operator=((unsigned long)(rhs)); }
		posit& operator=(const unsigned short rhs)    { return operator=((unsigned long)(rhs)); }
		posit& operator=(const unsigned int rhs)      { return operator=((unsigned long)(rhs)); }
		posit& operator=(const unsigned long rhs)     { 
			// special case for speed as this is a common initialization
			if (rhs == 0) {
				_bits = 0x0;
				return *this;
			}
			uint32_t v = rhs;
			if (v == sign_mask) { // +-maxpos, 0x8000 is special in int16 arithmetic as it is its own negation
				_bits = 0x8000;
				return *this;
			}
			else if (v > 0x0800'0000) { // v > 134,217,728
				_bits = 0x7FFFu;  // +-maxpos
				return *this;
			}
			else if (v > 0x02FF'FFFF) { // 50,331,647 < v < 134,217,728
				_bits = 0x7FFEu;  // 0.5 of maxpos
				return *this;
			}
			else if (v < 2) {  // v == 0 or v == 1
				_bits = (v << 14); // generates 0x0000 if v is 0, or 0x4000 if 1
				return *this;
			}
			else {
				uint32_t mask = 0x0200'0000;
				int8_t scale = 25;
				uint32_t fraction_bits = v;
				while (!(fraction_bits & mask)) {
					--scale;
					fraction_bits <<= 1;
				}
				int8_t k = scale >> 1;
				uint16_t exp = (scale & 0x01) << (12 - k); // extract exponent and shift to correct location
				fraction_bits = (fraction_bits ^ mask);
				_bits = (0x7FFF ^ (0x3FFF >> k)) | exp | (fraction_bits >> (k + 13));

				mask = 0x1000 << k; // bitNPlusOne
				if (mask & fraction_bits) {
					if (((mask - 1) & fraction_bits) | ((mask << 1) & fraction_bits)) _bits++; // increment by 1
				}
			}
			return *this;
		}
		posit& operator=(const unsigned long long rhs){ return operator=((unsigned long)(rhs)); }
		posit& operator=(const float rhs)             { return float_assign(rhs); }
		posit& operator=(const double rhs)            { return float_assign(rhs); }
		posit& operator=(const long double rhs)       { return float_assign(rhs); }

		explicit operator long double() const { return to_long_double(); }
		explicit operator double() const { return to_double(); }
		explicit operator float() const { return to_float(); }
		explicit operator long long() const { return to_long_long(); }
		explicit operator long() const { return to_long(); }
		explicit operator int() const { return to_int(); }
		explicit operator unsigned long long() const { return to_long_long(); }
		explicit operator unsigned long() const { return to_long(); }
		explicit operator unsigned int() const { return to_int(); }

		posit& set(sw::unum::bitblock<NBITS_IS_16>& raw) {
			_bits = uint16_t(raw.to_ulong());
			return *this;
		}
		posit& set_raw_bits(uint64_t value) {
			_bits = uint16_t(value & 0xffff);
			return *this;
		}
		posit operator-() const {
			if (iszero()) {
				return *this;
			}
			if (isnar()) {
				return *this;
			}
			posit p;
			return p.set_raw_bits((~_bits) + 1);
		}
		posit& operator+=(const posit& b) { // derived from SoftPosit
			uint16_t lhs = _bits;
			uint16_t rhs = b._bits;
			// process special cases
			if (isnar() || b.isnar()) {  // NaR
				_bits = 0x8000;
				return *this;
			}
			if (iszero() || b.iszero()) { // zero
				_bits = lhs | rhs;
				return *this;
			}
			bool sign = bool(_bits & sign_mask);
			if (sign) {
				lhs = -lhs & 0xFFFF;
				rhs = -rhs & 0xFFFF;
			}
			if (lhs < rhs) std::swap(lhs, rhs);
			
			// decode the regime of lhs
			int8_t m = 0; // pattern length
			uint16_t remaining = 0;
			decode_regime(lhs, m, remaining);

			// extract the exponent
			uint16_t exp = remaining >> 14;

			// extract remaining fraction bits
			uint32_t frac32A = (0x4000 | remaining) << 16;
			int8_t shiftRight = m;

			// adjust shift and extract fraction bits of rhs
			extractAddand(rhs, shiftRight, remaining);
			uint32_t frac32B = (0x4000 | remaining) << 16;

			//This is 2kZ + expZ; (where kZ=kA-kB and expZ=expA-expB)
			shiftRight = (shiftRight << 1) + exp - (remaining >> 14);

			if (shiftRight == 0) {
				frac32A += frac32B;  // this will always product a carry
				if (exp) ++m;
				exp ^= 1;
				frac32A >>= 1;
			}
			else {
				//Manage CLANG (LLVM) compiler when shifting right more than number of bits
				(shiftRight>31) ? (frac32B = 0) : (frac32B >>= shiftRight); //frac32B >>= shiftRight
				frac32A += frac32B;

				bool rcarry = 0x8000'0000 & frac32A; // first left bit
				if (rcarry) {
					if (exp) ++m;
					exp ^= 1;
					frac32A >>= 1;
				}
			}

			_bits = round(m, exp, frac32A);
			if (sign) _bits = -_bits & 0xFFFF;
			return *this;
		}
		posit& operator-=(const posit& b) {  // derived from SoftPosit
			uint16_t lhs = _bits;
			uint16_t rhs = b._bits;
			// process special cases
			if (isnar() || b.isnar()) {
				_bits = 0x8000;
				return *this;
			}
			if (iszero() || b.iszero()) {
				_bits = lhs | rhs;
				return *this;
			}
			// Both operands are actually the same sign if rhs inherits sign of sub: Make both positive
			bool sign = bool(lhs & sign_mask);
			(sign) ? (lhs = (-lhs & 0xFFFF)) : (rhs = (-rhs & 0xFFFF));

			if (lhs == rhs) {
				_bits = 0x0;
				return *this;
			}
			if (lhs < rhs) {
				std::swap(lhs, rhs);
				sign = !sign;
			}

			// decode the regime of lhs
			int8_t m = 0; // pattern length
			uint16_t remaining = 0;
			decode_regime(lhs, m, remaining);

			// extract the exponent
			uint16_t exp = remaining >> 14;

			uint32_t frac32A = (0x4000 | remaining) << 16;
			int8_t shiftRight = m;

			// adjust shift and extract fraction bits of rhs
			extractAddand(rhs, shiftRight, remaining);
			uint32_t frac32B = (0x4000 | remaining) << 16;

			// align the fractions for subtraction
			shiftRight = (shiftRight << 1) + exp - (remaining >> 14);
			if (shiftRight != 0) {
				if (shiftRight >= 29) {
					_bits = lhs;
					if (sign) _bits = -_bits & 0xFFFF;
					return *this;
				}
				else {
					frac32B >>= shiftRight;
				}
			}
			else {
				frac32B >>= shiftRight;
			}
			frac32A -= frac32B;

			while ((frac32A >> 29) == 0) {
				--m;
				frac32A <<= 2;
			}
			bool ecarry = bool (0x4000'0000 & frac32A);
			if (!ecarry) {
				if (exp == 0) --m;
				exp ^= 1;
				frac32A <<= 1;
			}

			_bits = round(m, exp, frac32A);
			if (sign) _bits = -_bits & 0xFFFF;
			return *this;
		}
		posit& operator*=(const posit& b) {
			uint16_t lhs = _bits;
			uint16_t rhs = b._bits;
			// process special cases
			if (isnar() || b.isnar()) {
				_bits = 0x8000;
				return *this;
			}
			if (iszero() || b.iszero()) {
				_bits = 0x0000;
				return *this;
			}

			// calculate the sign of the result
			bool sign = bool(lhs & sign_mask) ^ bool(rhs & sign_mask);
			lhs = lhs & sign_mask ? -lhs : lhs;
			rhs = rhs & sign_mask ? -rhs : rhs;

			// decode the regime of lhs
			int8_t m = 0; // pattern length
			uint16_t remaining = 0;
			decode_regime(lhs, m, remaining);

			// extract the exponent
			int32_t exp = remaining >> 14;

			// add the hidden bit
			uint32_t lhs_fraction = (0x4000 | remaining);
			// adjust shift and extract fraction bits of rhs
			extractMultiplicand(rhs, m, remaining);
			exp += (remaining >> 14);
			uint32_t rhs_fraction = (0x4000 | remaining);
			uint32_t result_fraction = lhs_fraction * rhs_fraction;
			//std::cout << "fbits 0x" << std::hex << result_fraction << std::dec << std::endl;

			if (exp > 1) {
				++m;
				exp ^= 0x2;
			}
			bool rcarry = bool(result_fraction & 0x2000'0000);
			if (rcarry) {
				if (exp) m++;
				exp ^= 0x1;
				result_fraction >>= 1;
			}

			// round
			_bits = adjustAndRound(m, exp, result_fraction);
			if (sign) _bits = -_bits & 0xFFFF;
			return *this;
		}
		posit& operator/=(const posit& b) {
			uint16_t lhs = _bits;
			uint16_t rhs = b._bits;
			// process special cases
			if (isnar() || b.isnar() || b.iszero()) {
				_bits = 0x8000;
				return *this;
			}
			if (iszero()) {
				_bits = 0x0000;
				return *this;
			}

			// calculate the sign of the result
			bool sign = bool(lhs & sign_mask) ^ bool(rhs & sign_mask);
			lhs = lhs & sign_mask ? -lhs : lhs;
			rhs = rhs & sign_mask ? -rhs : rhs;

			// decode the regime of lhs
			int8_t m = 0; // pattern length
			uint16_t remaining = 0;
			decode_regime(lhs, m, remaining);

			// extract the exponent
			int32_t exp = remaining >> 14;

			// extract the fraction
			uint16_t lhs_fraction = (0x4000 | remaining);
			uint32_t fraction = lhs_fraction << 14;

			// adjust shift and extract fraction bits of rhs
			extractDividand(rhs, m, remaining);
			exp -= remaining >> 14;
			uint16_t rhs_fraction = (0x4000 | remaining);

			div_t result = div(fraction, rhs_fraction);
			uint32_t result_fraction = result.quot;
			uint32_t remainder = result.rem;

			// adjust the exponent if needed
			if (exp < 0) {
				exp = 0x01;
				--m;
			}
			if (result_fraction != 0) {
				bool rcarry = result_fraction >> 14; // this is the hidden bit (14th bit), extreme right bit is bit 0
				if (!rcarry) {
					if (exp == 0) --m;
					exp ^= 0x01;
					result_fraction <<= 1;
				}
			}

			// round
			_bits = divRound(m, exp, result_fraction, remainder != 0);
			if (sign) _bits = -_bits & 0xFFFF;

			return *this;
		}
		posit& operator++() {
			++_bits;
			return *this;
		}
		posit operator++(int) {
			posit tmp(*this);
			operator++();
			return tmp;
		}
		posit& operator--() {
			--_bits;
			return *this;
		}
		posit operator--(int) {
			posit tmp(*this);
			operator--();
			return tmp;
		}
		posit reciprocate() const {
			posit p = 1.0 / *this;
			return p;
		}
		// SELECTORS
		inline bool isnar() const      { return (_bits == sign_mask); }
		inline bool iszero() const     { return (_bits == 0x0); }
		inline bool isone() const      { return (_bits == 0x4000); } // pattern 010000...
		inline bool isminusone() const { return (_bits == 0xC000); } // pattern 110000...
		inline bool isneg() const      { return (_bits & sign_mask); }
		inline bool ispos() const      { return !isneg(); }
		inline bool ispowerof2() const { return !(_bits & 0x1); }

		inline int sign_value() const  { return (_bits & 0x8 ? -1 : 1); }

		bitblock<NBITS_IS_16> get() const { bitblock<NBITS_IS_16> bb; bb = int(_bits); return bb; }
		unsigned long long encoding() const { return (unsigned long long)(_bits); }

		inline void clear() { _bits = 0; }
		inline void setzero() { clear(); }
		inline void setnar() { _bits = sign_mask; }
		inline posit twosComplement() const {
			posit<NBITS_IS_16, ES_IS_1> p;
			int16_t v = -*(int16_t*)&_bits;
			p.set_raw_bits(v);
			return p;
		}

	private:
		uint16_t _bits;

		// Conversion functions
#if POSIT_THROW_ARITHMETIC_EXCEPTION
		int         to_int() const {
			if (iszero()) return 0;
			if (isnar()) throw not_a_real{};
			return int(to_float());
		}
		long        to_long() const {
			if (iszero()) return 0;
			if (isnar()) throw not_a_real{};
			return long(to_double());
		}
		long long   to_long_long() const {
			if (iszero()) return 0;
			if (isnar()) throw not_a_real{};
			return long(to_long_double());
		}
#else
		int         to_int() const {
			if (iszero()) return 0;
			if (isnar())  return int(INFINITY);
			return int(to_float());
		}
		long        to_long() const {
			if (iszero()) return 0;
			if (isnar())  return long(INFINITY);
			return long(to_double());
		}
		long long   to_long_long() const {
			if (iszero()) return 0;
			if (isnar())  return (long long)(INFINITY);
			return long(to_long_double());
		}
#endif
		float       to_float() const {
			return (float)to_double();
		}
		double      to_double() const {
			if (iszero())	return 0.0;
			if (isnar())	return NAN;
			bool		     	 _sign;
			regime<nbits, es>    _regime;
			exponent<nbits, es>  _exponent;
			fraction<fbits>      _fraction;
			bitblock<nbits>		 _raw_bits;
			_raw_bits.reset();
			uint64_t mask = 1;
			for (size_t i = 0; i < nbits; i++) {
				_raw_bits.set(i, (_bits & mask));
				mask <<= 1;
			}
			decode(_raw_bits, _sign, _regime, _exponent, _fraction);
			double s = (_sign ? -1.0 : 1.0);
			double r = _regime.value();
			double e = _exponent.value();
			double f = (1.0 + _fraction.value());
			return s * r * e * f;
		}
		long double to_long_double() const {
			if (iszero())  return 0.0;
			if (isnar())   return NAN;
			bool		     	 _sign;
			regime<nbits, es>    _regime;
			exponent<nbits, es>  _exponent;
			fraction<fbits>      _fraction;
			bitblock<nbits>		 _raw_bits;
			_raw_bits.reset();
			uint64_t mask = 1;
			for (size_t i = 0; i < nbits; i++) {
				_raw_bits.set(i, (_bits & mask));
				mask <<= 1;
			}
			decode(_raw_bits, _sign, _regime, _exponent, _fraction);
			long double s = (_sign ? -1.0 : 1.0);
			long double r = _regime.value();
			long double e = _exponent.value();
			long double f = (1.0 + _fraction.value());
			return s * r * e * f;
		}

		template <typename T>
		posit& float_assign(const T& rhs) {
			constexpr int dfbits = std::numeric_limits<T>::digits - 1;
			value<dfbits> v((T)rhs);

			// special case processing
			if (v.iszero()) {
				setzero();
				return *this;
			}
			if (v.isinf() || v.isnan()) {  // posit encode for FP_INFINITE and NaN as NaR (Not a Real)
				setnar();
				return *this;
			}

			bitblock<NBITS_IS_16> ptt;
			convert_to_bb<NBITS_IS_16, ES_IS_1, dfbits>(v.sign(), v.scale(), v.fraction(), ptt); // TODO: needs to be faster
			_bits = uint16_t(ptt.to_ulong());
			return *this;
		}

		// helper method


		// decode_regime takes the raw bits of the posit, and returns the regime run-length, m, and the remaining fraction bits in remainder
		inline void decode_regime(const uint16_t bits, int8_t& m, uint16_t& remaining) const {
			remaining = (bits << 2) & 0xFFFF;
			if (bits & 0x4000) {  // positive regimes
				while (remaining >> 15) {
					++m;
					remaining = (remaining << 1) & 0xFFFF;
				}
			}
			else {              // negative regimes
				m = -1;
				while (!(remaining >> 15)) {
					--m;
					remaining = (remaining << 1) & 0xFFFF;
				}
				remaining &= 0x7FFF;
			}
		}
		inline void extractAddand(const uint16_t bits, int8_t& m, uint16_t& remaining) const {
			remaining = (bits << 2) & 0xFFFF;
			if (bits & 0x4000) {  // positive regimes
				while (remaining >> 15) {
					--m;
					remaining = (remaining << 1) & 0xFFFF;
				}
			}
			else {              // negative regimes
				++m;
				while (!(remaining >> 15)) {
					++m;
					remaining = (remaining << 1) & 0xFFFF;
				}
				remaining &= 0x7FFF;
			}
		}
		inline void extractMultiplicand(const uint16_t bits, int8_t& m, uint16_t& remaining) const {
			remaining = (bits << 2) & 0xFFFF;
			if (bits & 0x4000) {  // positive regimes
				while (remaining >> 15) {
					++m;
					remaining = (remaining << 1) & 0xFFFF;
				}
			}
			else {              // negative regimes
				--m;
				while (!(remaining >> 15)) {
					--m;
					remaining = (remaining << 1) & 0xFFFF;
				}
				remaining &= 0x7FFF;
			}
		}
		inline void extractDividand(const uint16_t bits, int8_t& m, uint16_t& remaining) const {
			remaining = (bits << 2) & 0xFFFF;
			if (bits & 0x4000) {  // positive regimes
				while (remaining >> 15) {
					--m;
					remaining = (remaining << 1) & 0xFFFF;
				}
			}
			else {              // negative regimes
				++m;
				while (!(remaining >> 15)) {
					++m;
					remaining = (remaining << 1) & 0xFFFF;
				}
				remaining &= 0x7FFF;
			}
		}
		inline uint16_t round(const int8_t m, uint16_t exp, uint32_t fraction) const {
			uint16_t scale, regime, bits;
			if (m < 0) {
				scale = (-m & 0xFFFF);
				regime = 0x4000 >> scale;
			}
			else {
				scale = m + 1;
				regime = 0x7FFF - (0x7FFF >> scale);
			}

			if (scale > 14) {
				bits = m<0 ? 0x0001 : 0x7FFF;  // minpos and maxpos
			}
			else {
				fraction = (fraction & 0x3FFF'FFFF) >> (scale + 1); // remove both carry bits
				uint16_t final_fbits = uint16_t(fraction >> 16);
				bool bitNPlusOne = false;
				if (scale != 14) { 
					bitNPlusOne = bool(0x8000 & fraction);	
				}
				else if (final_fbits > 0) {
					final_fbits = 0;
				}
				if (scale == 14 && exp != 0) bitNPlusOne = true;
				exp <<= (13 - scale);
				bits = uint16_t(regime) + uint16_t(exp) + uint16_t(final_fbits);
				// n+1 frac bit is 1. Need to check if another bit is 1 too if not round to even
				if (bitNPlusOne) {
					uint16_t moreBits = (0x7FFF & fraction) ? 0x0001 : 0x0000;
					bits += (bits & 0x0001) | moreBits;
				}
			}
			return bits;
		}
		inline uint16_t divRound(const int8_t m, uint16_t exp, uint32_t fraction, bool nonZeroRemainder) const {
			uint16_t scale, regime, bits;
			if (m < 0) {
				scale = (-m & 0xFFFF);
				regime = 0x4000 >> scale;
			}
			else {
				scale = m + 1;
				regime = 0x7FFF - (0x7FFF >> scale);
			}

			if (scale > 14) {
				bits = m<0 ? 0x0001 : 0x7FFF;  // minpos and maxpos
			}
			else {
				fraction &= 0x3FFF; // remove both carry bits
				uint16_t final_fbits = uint16_t(fraction >> (scale + 1));
				bool bitNPlusOne = false;
				if (scale != 14) {
					bitNPlusOne = bool((fraction >> scale) & 0x1);
				}
				else if (final_fbits > 0) {
					final_fbits = 0;
				}
				if (scale == 14 && exp != 0) bitNPlusOne = true;
				exp <<= (13 - scale);
				bits = uint16_t(regime) + uint16_t(exp) + uint16_t(final_fbits);
				
				if (bitNPlusOne) {
					uint16_t moreBits = (fraction & ((1 << scale) -1)) ? 0x0001 : 0x0000;
					if (nonZeroRemainder) moreBits = 0x0001;
						// n+1 frac bit is 1. Need to check if another bit is 1 too if not round to even
					bits += (bits & 0x0001) | moreBits;
				}
			}
			return bits;
		}
		inline uint16_t adjustAndRound(const int8_t m, uint16_t exp, uint32_t fraction) const {
			uint16_t scale, regime, bits;
			if (m < 0) {
				scale = (-m & 0xFFFF);
				regime = 0x4000 >> scale;
			}
			else {
				scale = m + 1;
				regime = 0x7FFF - (0x7FFF >> scale);
			}

			if (scale > 14) {
				bits = m<0 ? 0x0001 : 0x7FFF;  // minpos and maxpos
			}
			else {
				fraction = (fraction & 0x0FFF'FFFF) >> (scale - 1); // remove both carry bits
				uint16_t final_fbits = uint16_t(fraction >> 16);
				bool bitNPlusOne = false;
				if (scale != 14) {
					bitNPlusOne = bool(0x8000 & fraction);
				}
				else if (final_fbits > 0) {
					final_fbits = 0;
				}
				if (scale == 14 && exp != 0) bitNPlusOne = true;
				exp <<= (13 - scale);
				bits = uint16_t(regime) + uint16_t(exp) + uint16_t(final_fbits);
				// n+1 frac bit is 1. Need to check if another bit is 1 too if not round to even
				if (bitNPlusOne) {
					uint16_t moreBits = (0x7FFF & fraction) ? 0x0001 : 0x0000;
					bits += (bits & 0x0001) | moreBits;
				}
			}
			return bits;
		}
		// I/O operators
		friend std::ostream& operator<< (std::ostream& ostr, const posit<NBITS_IS_16, ES_IS_1>& p);
		friend std::istream& operator>> (std::istream& istr, posit<NBITS_IS_16, ES_IS_1>& p);

		// posit - posit logic functions
		friend bool operator==(const posit<NBITS_IS_16, ES_IS_1>& lhs, const posit<NBITS_IS_16, ES_IS_1>& rhs);
		friend bool operator!=(const posit<NBITS_IS_16, ES_IS_1>& lhs, const posit<NBITS_IS_16, ES_IS_1>& rhs);
		friend bool operator< (const posit<NBITS_IS_16, ES_IS_1>& lhs, const posit<NBITS_IS_16, ES_IS_1>& rhs);
		friend bool operator> (const posit<NBITS_IS_16, ES_IS_1>& lhs, const posit<NBITS_IS_16, ES_IS_1>& rhs);
		friend bool operator<=(const posit<NBITS_IS_16, ES_IS_1>& lhs, const posit<NBITS_IS_16, ES_IS_1>& rhs);
		friend bool operator>=(const posit<NBITS_IS_16, ES_IS_1>& lhs, const posit<NBITS_IS_16, ES_IS_1>& rhs);

	};

	// posit I/O operators
	// generate a posit format ASCII format nbits.esxNN...NNp
	inline std::ostream& operator<<(std::ostream& ostr, const posit<NBITS_IS_16, ES_IS_1>& p) {
		// to make certain that setw and left/right operators work properly
		// we need to transform the posit into a string
		std::stringstream ss;
#if POSIT_ROUNDING_ERROR_FREE_IO_FORMAT
		ss << NBITS_IS_16 << '.' << ES_IS_1 << 'x' << to_hex(p.get()) << 'p';
#else
		std::streamsize prec = ostr.precision();
		std::streamsize width = ostr.width();
		std::ios_base::fmtflags ff;
		ff = ostr.flags();
		ss.flags(ff);
		ss << std::showpos << std::setw(width) << std::setprecision(prec) << (long double)p;
#endif
		return ostr << ss.str();
	}

	// read an ASCII float or posit format: nbits.esxNN...NNp, for example: 32.2x80000000p
	inline std::istream& operator>> (std::istream& istr, posit<NBITS_IS_16, ES_IS_1>& p) {
		std::string txt;
		istr >> txt;
		if (!parse(txt, p)) {
			std::cerr << "unable to parse -" << txt << "- into a posit value\n";
		}
		return istr;
	}

	// convert a posit value to a string using "nar" as designation of NaR
	std::string to_string(const posit<NBITS_IS_16, ES_IS_1>& p, std::streamsize precision) {
		if (p.isnar()) {
			return std::string("nar");
		}
		std::stringstream ss;
		ss << std::setprecision(precision) << float(p);
		return ss.str();
	}

	// posit - posit binary logic operators
	inline bool operator==(const posit<NBITS_IS_16, ES_IS_1>& lhs, const posit<NBITS_IS_16, ES_IS_1>& rhs) {
		return lhs._bits == rhs._bits;
	}
	inline bool operator!=(const posit<NBITS_IS_16, ES_IS_1>& lhs, const posit<NBITS_IS_16, ES_IS_1>& rhs) {
		return !operator==(lhs, rhs);
	}
	inline bool operator< (const posit<NBITS_IS_16, ES_IS_1>& lhs, const posit<NBITS_IS_16, ES_IS_1>& rhs) {
		return (signed short)(lhs._bits) < (signed short)(rhs._bits);
	}
	inline bool operator> (const posit<NBITS_IS_16, ES_IS_1>& lhs, const posit<NBITS_IS_16, ES_IS_1>& rhs) {
		return operator< (rhs, lhs);
	}
	inline bool operator<=(const posit<NBITS_IS_16, ES_IS_1>& lhs, const posit<NBITS_IS_16, ES_IS_1>& rhs) {
		return operator< (lhs, rhs) || operator==(lhs, rhs);
	}
	inline bool operator>=(const posit<NBITS_IS_16, ES_IS_1>& lhs, const posit<NBITS_IS_16, ES_IS_1>& rhs) {
		return !operator< (lhs, rhs);
	}

	inline posit<NBITS_IS_16, ES_IS_1> operator+(const posit<NBITS_IS_16, ES_IS_1>& lhs, const posit<NBITS_IS_16, ES_IS_1>& rhs) {
		posit<NBITS_IS_16, ES_IS_1> result = lhs;
		if (lhs.isneg() == rhs.isneg()) {  // are the posits the same sign?
			result += rhs;
		} 
		else {
			result -= rhs;
		}
		return result;
	}
	inline posit<NBITS_IS_16, ES_IS_1> operator-(const posit<NBITS_IS_16, ES_IS_1>& lhs, const posit<NBITS_IS_16, ES_IS_1>& rhs) {
		posit<NBITS_IS_16, ES_IS_1> result = lhs;
		if (lhs.isneg() == rhs.isneg()) {  // are the posits the same sign?
			result -= rhs.twosComplement();
		}
		else {
			result += rhs.twosComplement();
		}
		return result;

	}
	// binary operator*() is provided by generic class
	// binary operator/() is provided by generic class

#if POSIT_ENABLE_LITERALS
	// posit - literal logic functions

	// posit - int logic operators
	inline bool operator==(const posit<NBITS_IS_16, ES_IS_1>& lhs, int rhs) {
		return operator==(lhs, posit<NBITS_IS_16, ES_IS_1>(rhs));
	}
	inline bool operator!=(const posit<NBITS_IS_16, ES_IS_1>& lhs, int rhs) {
		return !operator==(lhs, posit<NBITS_IS_16, ES_IS_1>(rhs));
	}
	inline bool operator< (const posit<NBITS_IS_16, ES_IS_1>& lhs, int rhs) {
		return operator<(lhs, posit<NBITS_IS_16, ES_IS_1>(rhs));
	}
	inline bool operator> (const posit<NBITS_IS_16, ES_IS_1>& lhs, int rhs) {
		return operator< (posit<NBITS_IS_16, ES_IS_1>(rhs), lhs);
	}
	inline bool operator<=(const posit<NBITS_IS_16, ES_IS_1>& lhs, int rhs) {
		return operator< (lhs, posit<NBITS_IS_16, ES_IS_1>(rhs)) || operator==(lhs, posit<NBITS_IS_16, ES_IS_1>(rhs));
	}
	inline bool operator>=(const posit<NBITS_IS_16, ES_IS_1>& lhs, int rhs) {
		return !operator<(lhs, posit<NBITS_IS_16, ES_IS_1>(rhs));
	}

	// int - posit logic operators
	inline bool operator==(int lhs, const posit<NBITS_IS_16, ES_IS_1>& rhs) {
		return posit<NBITS_IS_16, ES_IS_1>(lhs) == rhs;
	}
	inline bool operator!=(int lhs, const posit<NBITS_IS_16, ES_IS_1>& rhs) {
		return !operator==(posit<NBITS_IS_16, ES_IS_1>(lhs), rhs);
	}
	inline bool operator< (int lhs, const posit<NBITS_IS_16, ES_IS_1>& rhs) {
		return operator<(posit<NBITS_IS_16, ES_IS_1>(lhs), rhs);
	}
	inline bool operator> (int lhs, const posit<NBITS_IS_16, ES_IS_1>& rhs) {
		return operator< (posit<NBITS_IS_16, ES_IS_1>(rhs), lhs);
	}
	inline bool operator<=(int lhs, const posit<NBITS_IS_16, ES_IS_1>& rhs) {
		return operator< (posit<NBITS_IS_16, ES_IS_1>(lhs), rhs) || operator==(posit<NBITS_IS_16, ES_IS_1>(lhs), rhs);
	}
	inline bool operator>=(int lhs, const posit<NBITS_IS_16, ES_IS_1>& rhs) {
		return !operator<(posit<NBITS_IS_16, ES_IS_1>(lhs), rhs);
	}

#endif // POSIT_ENABLE_LITERALS

#else  // POSIT_FAST_POSIT_16_1
// too verbose #pragma message("Standard posit<16,1>")
#	define POSIT_FAST_POSIT_16_1 0
#endif // POSIT_FAST_POSIT_16_1

  }
}
