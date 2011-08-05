// Copyright 2002-2004 Frozenbyte Ltd.

#ifndef PARTICLE_RANDOM_H
#define PARTICLE_RANDOM_H

#ifndef INCLUDED_BOOST_MERSENNE_TWISTER_HPP
#define INCLUDED_BOOST_MERSENNE_TWISTER_HPP
#include <boost/random/mersenne_twister.hpp>
#endif

namespace frozenbyte
{
namespace particle
{
	
	static boost::mt19937 g_randomNumberGen;
		
	inline void seedRandomGen(int value) {
		//g_randomNumberGen.seed(value);
	}
	
	template<class T>
		randValueMinMax(T min, T max) {
			return min + static_cast<T>(g_randomNumberGen()) / 
				static_cast<T>(g_randomNumberGen.max()) * (max - min);
		}
	
	template<class T>
		randValueMinVar(T min, T var) {
			return min + static_cast<T>(g_randomNumberGen()) / 
				static_cast<T>(g_randomNumberGen.max()) * var;
		}	
	
	template<class T> randUnitValue() {
		return (-1.0 + 2.0 * (static_cast<T>(g_randomNumberGen()) / 
				static_cast<T>(g_randomNumberGen.max())));
	}

	
} // particle
} // frozenbyte



#endif