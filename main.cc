//===================================================================
// File:        main.cc
// Author:      Drahoslav Zan
// Email:       izan@fit.vutbr.cz
// Affiliation: Brno University of Technology,
//              Faculty of Information Technology
// Date:        Tue Mar 20 18:01:23 CET 2012
// Project:     Password cracker for Vigenere cipher (PCVC)
//-------------------------------------------------------------------
// Copyright (C) 2012 Drahoslav Zan
//
// This file is part of PCVC.
//
// PCVC is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// PCVC is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with PCVC. If not, see <http://www.gnu.org/licenses/>.
//===================================================================
// vim: set nowrap sw=2 ts=2


#include <iostream>
#include <string>
#include <cassert>
#include <cstdio>
#include <vector>
#include <map>


using namespace std;


/****************************** CONSTANTS ******************************/

static const unsigned LETTERS_ENG =  26;
static const float    IOC_ENG     = .067;

static const unsigned MIN_GRAM    =  3;
static const unsigned KL_INTERVAL =  3;

// [a-z]
static const float LF_ENG[LETTERS_ENG] = {
	0.08167, 0.01492, 0.02782, 0.04253,
	0.12702, 0.02228, 0.02015, 0.06094,
	0.06966, 0.00153, 0.00772, 0.04025,
	0.02406, 0.06749, 0.07507, 0.01929,
	0.00095, 0.05987, 0.06327, 0.09056,
	0.02758, 0.00978, 0.02360, 0.00150,
	0.01974, 0.00074 };


/****************************** INLINES ******************************/

static unsigned gcd(unsigned a, unsigned b)
{
	return (!b) ? a : gcd(b, a % b);
}

static float abs(float x)
{
	return (x < 0) ? -x : x;
}


/***************************** VIGENERE ******************************/

unsigned kasiski(const string &str)
{
	size_t len = str.length();

	size_t lsl = 1;
	size_t kg  = 1;
	map<unsigned, unsigned> km;

	for(size_t i = 0; i < len; ++i)
	{
		size_t n = str.find_first_of(str[i], i + 3);

		size_t sl = 0;
		size_t kt = 0;
		size_t ng = 1e6;

		for(; n < len; n = str.find_first_of(str[i], n + 1))
		{
			size_t nng = 1;

			for(; n + nng < len; ++nng)
				if(str[i + nng] != str[n + nng]) break;

			if(nng < 3) continue;

			if(nng < ng) ng = nng;

			++sl;
			kt = gcd(n - i, kt);

			if(kt <= 1) break;
		}

		if(kt > 1 && (sl > lsl || ng > kg))
		{
			km[kt]++;
			kg = ng;
			lsl = sl;
		}
	}

	unsigned max = 0;
	unsigned k = 0;

	for(map<unsigned, unsigned>::iterator it = km.begin(); it != km.end(); ++it)
		if((*it).second > max)
		{
			max = (*it).second;
			k = (*it).first;
		}

	return k;
}

float indexOfCoincidence(const string &str, size_t from, size_t step)
{
	assert(step != 0);

	size_t freq[LETTERS_ENG] = { 0 };
	size_t len = str.length();

	for(size_t i = from; i < len; i += step)
	{
		int s = str[i] - 'a';

		assert(s >= 0 && s < (int)LETTERS_ENG);

		++freq[s];
	}

	double l = (len / step) * ((len / step) - 1);
	double ic = 0;

	for(size_t i = 0; i < LETTERS_ENG; ++i)
		ic += ((double)freq[i] * (freq[i] - 1)) / l;

	return (float)ic;
}

float friedman(const string &str)
{
	float ic = indexOfCoincidence(str, 0, 1);
	float kr = 1.0 / LETTERS_ENG;

	return (IOC_ENG - kr) / (ic - kr);
}

unsigned keyLength(const string &str, float fr, unsigned ks)
{
	float diff = 1.0;
	size_t kl = 0;

	size_t left = fr / KL_INTERVAL - .5;

	for(size_t i = (!left) ? 1 : left; i <= ks && i <= KL_INTERVAL * fr + .5; ++i)
	{
		if(ks % i) continue;

		float ic = 0;

		for(size_t j = 0; j < i; ++j)
			ic += abs(indexOfCoincidence(str, j, i) - IOC_ENG);

		ic /= i;

		if(ic < diff)
		{
			diff = ic;
			kl = i;
		}
	}

	return kl;
}

string key(const string &str, unsigned klen)
{
	if(!klen) return "";

	size_t len = str.length();
	string k("");

	for(size_t i = 0; i < klen; ++i)
	{
		unsigned clen = 0;
		unsigned freq[LETTERS_ENG] = { 0 };

		for(size_t j = i; j < len; j += klen, ++clen)
		{
			int s = str[j] - 'a';

			assert(s >= 0 && s < int(LETTERS_ENG));

			++freq[s];
		}

		float diff = 1.0;
		size_t shift = 0;

		for(size_t j = 0; j < LETTERS_ENG; ++j)
		{
			float mg = 0;

			for(size_t k = 0; k < LETTERS_ENG; ++k)
			{
				int s = (k + j) % LETTERS_ENG;

				mg += (LF_ENG[k] * freq[s]) / clen;
			}

			mg = abs(mg - IOC_ENG);

			if(mg < diff)
			{
				shift = j;
				diff = mg;
			}
		}

		k.push_back('a' + shift);
	}

	return k;
}


/****************************** MAIN ******************************/

int main(void)
{
	string ct;

	for(int i = getc(stdin); i != EOF; i = getc(stdin))
		if(isalpha(i)) ct.push_back(tolower(i));
	
	float    ft = friedman(ct);
	unsigned kt = kasiski(ct);
	unsigned kl = keyLength(ct, ft, kt);
	string   k  = key(ct, kl);

	// printf("%.4f;%u;%u;%s\n", ft, kt, kl, k.c_str());

	cout << k << endl;

	return 0;
}

