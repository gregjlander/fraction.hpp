/*
 * fraction.hpp
 *
 * Copyright 2023 Greg Lander<greg.j.lander@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 *
 *
 */
// There are many examples of people attempting to write simple fraction
// classes in C++. This is my attempt to (partially) write one. It is
// an expansion of and based on the fraction example code at:
// https://en.cppreference.com/w/cpp/language/operators
// Outstanding issues:
//   Complete adding noexcept where appropriate.
//   Improve testing.
//   Provide ability to select which method is used to approximate doubles.
//

//#pragma once

#ifndef FRACTION_HPP
#define FRACTION_HPP

#include <array>
#include <cmath>
#include <complex>
#include <numeric>
#include <ranges>
#include <span>
#include <string>

namespace mth {

// Forward declarations.
template< std::integral IT = std::int64_t, int error_exp = -6 > class fraction;
template< std::integral IT = std::int64_t, int error_exp = -6 >
[[nodiscard]] constexpr fraction< IT, error_exp >
to_fraction_using_stern_brocot_with_mediants( const double from );

template< std::integral IT, int error_exp > class fraction {
  public:
	// Constructors:
	constexpr fraction() { set( 0, 1 ); };
	constexpr fraction( const IT num, const IT den ) { set( num, den ); };
	constexpr fraction( const fraction &from ) {
		set( from.num(), from.den() );
	};
	constexpr explicit fraction( const IT num ) { set( num, 1 ); };
	constexpr explicit fraction( const double from ) {
		*this = to_fraction_using_stern_brocot_with_mediants( from );
	};

	// Useful constants:
	constexpr static const fraction f_0{ 0, 1 };
	constexpr static const fraction f_1{ 1, 1 };
	constexpr static const fraction f_inf{ 1, 0 };

	// Numerator and denominator get methods:
	constexpr IT num() const { return numerator; };
	constexpr IT den() const { return denominator; };

	// Conversions:
	// fraction to double.
	constexpr double to_double() const {
		return (double)numerator / (double)denominator;
	};

	// Accuracy to use when approximating a double with a fraction.
	constexpr static const double error = std::pow( 10.0, (double)error_exp );

	// Use stern brocot algorithm to approximate a double with a fraction.
	friend constexpr fraction
	to_fraction_using_stern_brocot_with_mediants< IT, error_exp >(
		const double from );

	// Calculate the continued fraction of a double.
	template< std::size_t continued_fraction_max_iter >
	friend constexpr const std::array< IT, continued_fraction_max_iter >
	to_continued_fraction( const double num );

	// Convert a continued fraction to a fraction.
	friend constexpr fraction
	to_fraction< IT, error_exp >( const std::span< IT > &from );

	// Use continued fraction algorithm to approximate a double with a fraction.
	template< std::size_t continued_fraction_max_iter >
	friend constexpr fraction
	to_fraction_using_continued_fractions( const double num );

	// Boolean operations:
	// Check if fraction has denominator == 1.
	[[nodiscard]] constexpr bool is_int() const noexcept {
		return denominator == 1;
	};
	// Check if negative. Constructors move "-" sign to numerator.
	[[nodiscard]] constexpr bool is_neg() const noexcept {
		return numerator < 0;
	};

	// Maths:
	// As per std::labs.
	[[nodiscard]] constexpr fraction abs() const {
		return { std::labs( numerator ), denominator };
	};
	// 1/fraction.
	[[nodiscard]] constexpr fraction inv() const {
		return { denominator, numerator };
	};
	// Calculate the mediant of two fractions.
	[[nodiscard]] friend constexpr fraction mediant( const fraction &f1,
													 const fraction &f2 ) {
		return fraction{ f1.num() + f2.num(), f1.den() + f2.den() };
	};
	// Calculate the average of some fractions.
	template< typename... T >
	[[nodiscard]] constexpr friend fraction average( const T... fractions ) {
		const std::size_t size{ sizeof...( fractions ) };
		return ( size == 0 ) ? f_0 : ( fractions + ... ) / (IT)size;
	}

	// Exponentiation:
	// Raise to the power of exp and approximate as a complex fraction.
	[[nodiscard]] constexpr std::pair< fraction, fraction >
	pow_c( double exp ) const {
		if ( ( ( exp < 0 ) && ( numerator == 0 ) ) ||
			 ( ( exp >= 0 ) && ( denominator == 0 ) ) )
			return { *this, f_0 };
		const std::complex< double > pow_d =
			std::pow( std::complex< double >{ to_double(), 0.0 }, exp );
		return std::make_pair(
			to_fraction_using_stern_brocot_with_mediants( pow_d.real() ),
			to_fraction_using_stern_brocot_with_mediants( pow_d.imag() ) );
	};
	// Raise to the power of exp and approximate as a fraction.
	// Note: for negatives, pow(-N, 0.5) results in 0, ie the real part.
	[[nodiscard]] constexpr fraction pow( double exp ) const {
		return ( ( ( exp < 0 ) && ( numerator == 0 ) ) ||
				 ( ( exp >= 0 ) && ( denominator == 0 ) ) )
				   ? *this
				   : to_fraction_using_stern_brocot_with_mediants(
						 std::pow( to_double(), exp ) );
	};
	// Square a fraction.
	[[nodiscard]] constexpr fraction sq() const {
		return ( *this ) * ( *this );
	};
	// Determine if abs(fraction) is a perfect square.
	[[nodiscard]] constexpr bool is_abs_sq() const {
		const fraction sqrt{ (IT)std::sqrt( std::labs( numerator ) ),
							 (IT)std::sqrt( denominator ) };
		return abs() == sqrt * sqrt;
	};
	// Cube a fraction.
	[[nodiscard]] constexpr fraction cb() const {
		return ( *this ) * ( *this ) * ( *this );
	};
	// Determine if this fraction is a perfect cube.
	[[nodiscard]] constexpr bool is_cb() const {
		const fraction cbrt{ (IT)std::cbrt( numerator ),
							 (IT)std::cbrt( denominator ) };
		return *this == cbrt * cbrt * cbrt;
	};
	// Normalized fraction (range (-1, -0.5], [0.5, 1) ) and integral power of 2
	// as per std::frexp() with the normalized fraction approximated as a
	// fraction. e.g. (48/7) i.e. 6*(2^3)/7 => {(6/7), 3}
	// 				  (1/4)                 => {(1/2), -1}
	[[nodiscard]] constexpr std::pair< fraction, int > frexp() const {
		int exp = 0;
		fraction num = ( denominator == 0 )
						   ? *this
						   : fraction{ std::frexp( to_double(), &exp ) };
		return std::make_pair( num, exp );
	};
	// Call load exponent as per std::ldexp() approximated as a fraction.
	// e.g (2/5).ldexp(3) => (8/125)
	[[nodiscard]] constexpr fraction ldexp( int exp ) const {
		return ( denominator == 0 )
				   ? *this
				   : fraction{ std::ldexp( to_double(), exp ) };
	};
	// Calculate sqrt of a fraction and approximate it as a complex fraction.
	[[nodiscard]] constexpr std::pair< fraction, fraction > sqrt_c() const {
		if ( denominator == 0 )
			return { *this, f_0 };
		const std::complex< double > sqrt_d{ std::sqrt(
			std::complex< double >{ to_double(), 0.0 } ) };
		return std::make_pair(
			to_fraction_using_stern_brocot_with_mediants( sqrt_d.real() ),
			to_fraction_using_stern_brocot_with_mediants( sqrt_d.imag() ) );
		;
	};
	// Calculate sqrt of a fraction and approximate it as a fraction.
	// Note: for negatives, sqrt(-N) results in 0, ie the real part.
	[[nodiscard]] constexpr fraction sqrt() const {
		return ( denominator == 0 )
				   ? *this
				   : to_fraction_using_stern_brocot_with_mediants(
						 std::sqrt( to_double() ) );
	};
	// Calculate cbrt of a fraction and approximate it as a fraction.
	[[nodiscard]] constexpr fraction cbrt() const {
		return ( denominator == 0 )
				   ? *this
				   : to_fraction_using_stern_brocot_with_mediants(
						 std::cbrt( to_double() ) );
	};
	// Split fraction into two parts by extracting any squares(2), cubes(3) etc.
	// See examples below.
	[[nodiscard]] constexpr std::pair< fraction, fraction >
	simplify_rt( const double rt ) const {
		std::pair result{ f_1, *this };
		fraction num_remain{ numerator };
		IT num_factor =
			(IT)std::floor( std::pow( std::labs( numerator ), 1.0 / rt ) );
		for ( ; num_factor != 0; --num_factor ) {
			num_remain = fraction{ numerator, (IT)std::pow( num_factor, rt ) };
			if ( num_remain.denominator == 1 )
				break;
		};
		if ( num_factor == 0 )
			++num_factor;
		fraction den_remain{ denominator };
		IT den_factor = (IT)std::floor( std::pow( denominator, 1.0 / rt ) );
		for ( ; den_factor != 0; --den_factor ) {
			den_remain = { denominator, (IT)std::pow( den_factor, rt ) };
			if ( den_remain.denominator == 1 )
				break;
		};
		if ( den_factor == 0 )
			++den_factor;
		if ( ( num_factor > 1 ) || ( den_factor > 1 ) ) {
			result = std::make_pair( fraction{ num_factor, den_factor },
									 num_remain / den_remain );
		};
		return result;
	};
	// Split sqrt into two parts by extracting any squares.
	// e.g. (56/45)     i.e. (2*2*2*7/3*3*5)          =>{(2/3),(14/5)}
	//		(392/10125) i.e. (2*2*2*7*7/3*3*3*3*5*5*5)=>{(14/45),(2/5)}
	[[nodiscard]] constexpr std::pair< fraction, fraction >
	simplify_sqrt() const {
		return simplify_rt( 2.0 );
	};
	// split cbrt into two parts by extracting any cubes.
	// e.g. (56/135)      i.e. (2*2*2*7/3*3*3*5)            =>{(2/3),(7/5)}
	//		(19208/10125) i.e. (2*2*2*7*7*7*7/3*3*3*3*5*5*5)=>{(14/15),(49/3)}
	[[nodiscard]] constexpr std::pair< fraction, fraction >
	simplify_cbrt() const {
		return simplify_rt( 3.0 );
	};

	// operators+
	constexpr fraction operator+( const fraction &rhs ) const {
		return { numerator * rhs.den() + denominator * rhs.num(),
				 denominator * rhs.den() };
	};
	constexpr fraction operator+( const IT &rhs ) const {
		return { numerator + denominator * rhs, denominator };
	};
	constexpr fraction operator+( const double &rhs ) const {
		return denominator == 0 ? *this : fraction{ to_double() + rhs };
	};
	constexpr fraction &operator+=( const fraction &rhs ) {
		*this = *this + rhs;
		return *this;
	};
	constexpr fraction &operator+=( const IT &rhs ) {
		*this = *this + rhs;
		return *this;
	};
	constexpr fraction &operator+=( const double &rhs ) {
		*this = *this + rhs;
		return *this;
	};
	constexpr friend fraction operator+( const IT &lhs, const fraction &rhs ) {
		return rhs + lhs;
	};
	constexpr friend fraction operator+( const double &lhs,
										 const fraction &rhs ) {
		return rhs + lhs;
	};
	constexpr fraction &operator++() { //++f
		set( numerator + denominator, denominator );
		return *this;
	};
	constexpr fraction operator++( int ) { // f++
		fraction tmp = *this;
		set( numerator + denominator, denominator );
		return tmp;
	};

	// operators-
	constexpr fraction operator-() const {
		return { -numerator, denominator };
	};
	constexpr fraction operator-( const fraction &rhs ) const {
		return *this + ( -rhs );
	};
	constexpr fraction operator-( const IT &rhs ) const {
		return *this + ( -rhs );
	};
	constexpr fraction operator-( const double &rhs ) const {
		return *this + ( -rhs );
	};
	constexpr fraction &operator-=( const fraction &rhs ) {
		*this += -rhs;
		return *this;
	};
	constexpr fraction &operator-=( const IT &rhs ) {
		*this += -rhs;
		return *this;
	};
	constexpr fraction &operator-=( const double &rhs ) {
		*this += -rhs;
		return *this;
	};
	constexpr friend fraction operator-( const IT &lhs, const fraction &rhs ) {
		return -rhs + lhs;
	};
	constexpr friend fraction operator-( const double &lhs,
										 const fraction &rhs ) {
		return -rhs + lhs;
	};
	constexpr fraction &operator--() { // --f
		set( numerator - denominator, denominator );
		return *this;
	};
	constexpr fraction operator--( int ) { // f--
		fraction tmp = *this;
		set( numerator - denominator, denominator );
		return tmp;
	};

	// operators*
	constexpr fraction operator*( const fraction &rhs ) const {
		return { numerator * rhs.num(), denominator * rhs.den() };
	};
	constexpr fraction operator*( const IT &rhs ) const {
		return { numerator * rhs, denominator };
	};
	constexpr fraction operator*( const double &rhs ) const {
		return denominator == 0 ? *this : fraction{ to_double() * rhs };
	};
	constexpr fraction &operator*=( const fraction &rhs ) {
		*this = ( *this ) * rhs;
		return *this;
	};
	constexpr fraction &operator*=( const IT &rhs ) {
		*this = ( *this ) * rhs;
		return *this;
	};
	constexpr fraction &operator*=( const double &rhs ) {
		*this = ( *this ) * rhs;
		return *this;
	};
	constexpr friend fraction operator*( const IT lhs, const fraction &rhs ) {
		return rhs * lhs;
	};
	constexpr friend fraction operator*( const double lhs,
										 const fraction &rhs ) {
		return rhs * lhs;
	};

	// operators/
	constexpr fraction operator/( const fraction &rhs ) const {
		return { numerator * rhs.den(), denominator * rhs.num() };
	};
	constexpr fraction operator/( const IT &rhs ) const {
		return { numerator, denominator * rhs };
	};
	constexpr fraction operator/( const double &rhs ) const {
		return ( denominator == 0 ) ? *this : fraction{ to_double() / rhs };
	};
	constexpr fraction &operator/=( const fraction &rhs ) {
		*this = *this / rhs;
		return *this;
	};
	constexpr fraction &operator/=( const IT &rhs ) {
		*this = *this / rhs;
		return *this;
	};
	constexpr fraction &operator/=( const double &rhs ) {
		*this = *this / rhs;
		return *this;
	};
	constexpr friend fraction operator/( const IT lhs, const fraction &rhs ) {
		return fraction{ lhs * rhs.den(), rhs.num() };
	};
	constexpr friend fraction operator/( const double lhs,
										 const fraction &rhs ) {
		return rhs.num() == 0 ? rhs.inv()
							  : fraction{ lhs * rhs.inv().to_double() };
	};

	// operators%
	constexpr fraction operator%( const fraction &rhs ) const {
		return ( ( rhs.num() == 0 ) || ( rhs.den() == 0 ) ||
				 ( denominator == 0 ) )
				   ? f_inf
				   : *this -
						 (IT)std::trunc( ( *this / rhs ).to_double() ) * rhs;
	};
	constexpr fraction operator%( const IT &rhs ) const {
		return ( ( denominator == 0 ) || ( rhs == 0 ) )
				   ? f_inf
				   : *this -
						 (IT)std::trunc( ( *this / rhs ).to_double() ) * rhs;
	};
	constexpr fraction operator%( const double &rhs ) const {
		return ( denominator == 0 ) ? *this
									: fraction{ std::fmod( to_double(), rhs ) };
	};
	constexpr fraction &operator%=( const fraction &rhs ) {
		*this = *this % rhs;
		return *this;
	};
	constexpr fraction &operator%=( const IT &rhs ) {
		*this = *this % rhs;
		return *this;
	};
	constexpr fraction &operator%=( const double &rhs ) {
		*this = *this % rhs;
		return *this;
	};
	constexpr friend fraction operator%( const IT lhs, const fraction &rhs ) {
		return fraction{ lhs, 1 } % rhs;
	};
	constexpr friend fraction operator%( const double lhs,
										 const fraction &rhs ) {
		return ( rhs.den() == 0 )
				   ? rhs
				   : fraction{ std::fmod( lhs, rhs.to_double() ) };
	};

	// =
	constexpr void operator=( const fraction &rhs ) {
		set( rhs.num(), rhs.den() );
	};

	// Comparison operators:
	constexpr bool operator==( const fraction &rhs ) const {
		return ( numerator == rhs.num() ) && ( denominator == rhs.den() );
	};
	constexpr const std::strong_ordering
	operator<=>( const fraction &rhs ) const {
		return ( numerator * rhs.den() ) <=> ( rhs.num() * denominator );
	};

	// string manipulation:
	[[nodiscard]] std::string to_string() const noexcept( false ) {
		if ( is_int() ) {
			if ( is_neg() ) {
				return '(' + std::to_string( numerator ) + ')';
			} else {
				return std::to_string( numerator );
			};
		} else {
			return '(' + std::to_string( numerator ) + '/' +
				   std::to_string( denominator ) + ')';
		};
	};
	[[nodiscard]] friend std::string
	to_string( const fraction &f ) noexcept( false ) {
		return f.to_string();
	};
	[[nodiscard]] const std::string operator+( const std::string &rhs ) const
		noexcept( false ) {
		return to_string() + rhs;
	};
	[[nodiscard]] friend std::string
	operator+( const std::string &lhs, const fraction &rhs ) noexcept( false ) {
		return lhs + rhs.to_string();
	};
	friend std::ostream &operator<<( std::ostream &os,
									 const fraction &f ) noexcept( false ) {
		os << f.to_string();
		return os;
	};
	// Convert continued fraction to string.
	friend std::string to_string( const auto &cf ) noexcept( false );

  private:
	// Set method. A negative result is stored with the numerator.
	constexpr void set( const IT num = 1, const IT den = 1 ) noexcept {
		// standard undefined behaviour if denominator is 0
		initial_num = num;
		initial_den = den;
		const IT gcd = std::gcd( num, den );
		numerator = num / (IT)std::copysign( gcd, den );
		denominator = std::labs( den ) / gcd;
	};

	// Store the fraction.
	IT initial_num;
	IT numerator;
	IT initial_den;
	IT denominator;
}; // class fraction

template< std::integral IT, int error_exp >
[[nodiscard]] constexpr fraction< IT, error_exp >
to_fraction_using_stern_brocot_with_mediants( const double from ) {
	fraction< IT, error_exp > med{};
	// save steps by not starting at infinity and 0
	for ( fraction< IT, error_exp > high{ (IT)std::ceil( from ) },
		  low{ (IT)std::floor( from ) };
		  ; ) {
		med = mediant( low, high );
		if ( med.to_double() - from > fraction< IT, error_exp >::error ) {
			high = med;
		} else if ( med.to_double() - from <
					-fraction< IT, error_exp >::error ) {
			low = med;
		} else {
			break;
		};
	};
	return med;
};

template< std::size_t continued_fraction_max_iter = 25,
		  std::integral IT = std::int64_t, int error_exp = -6 >
[[nodiscard]] constexpr std::array< IT, continued_fraction_max_iter >
to_continued_fraction( const double num ) {
	std::array< IT, continued_fraction_max_iter > result{ 0 };
	double remainder = num;
	double iptr;
	for ( auto it = result.begin(); it != result.end();
		  ++it, remainder = 1.0 / remainder ) {
		remainder = std::modf( remainder, &iptr );
		*it = (IT)iptr;
		if ( std::abs( remainder ) < fraction< IT, error_exp >::error ) {
			break;
		};
	};
	return result;
};

template< std::integral IT = std::int64_t, int error_exp = -6 >
[[nodiscard]] constexpr fraction< IT, error_exp >
to_fraction( const std::span< IT > &from ) {
	return std::accumulate(
		std::next( from.rbegin() ), from.rend(), fraction< IT, error_exp >::f_0,
		[]( fraction< IT, error_exp > a, IT b ) {
			fraction< IT, error_exp > am( std::move( a ) );
			return ( am.num() == 0 ) ? fraction{ b } : am.inv() + b;
		} );
};

template< std::size_t continued_fraction_max_iter = 25,
		  std::integral IT = std::int64_t, int error_exp = -6 >
[[nodiscard]] constexpr fraction< IT, error_exp >
to_fraction_using_continued_fractions( const double num ) {
	auto cf{ to_continued_fraction< continued_fraction_max_iter >( num ) };
	return to_fraction( std::span< IT >{ cf } );
};

template< std::integral IT = std::int64_t, int error_exp = -6 >
[[nodiscard]] std::string to_string( const auto &cf ) noexcept( false ) {
	const auto last =
		std::ranges::find_if( cf.rbegin(), std::prev( cf.rend() ),
							  []( auto val ) { return val != 0; } )
			.base();
	return std::accumulate(
		std::next( cf.begin() ), last, std::to_string( cf[0] ),
		[]( std::string result, int val ) {
			return std::move( result ) + ',' + std::to_string( val );
		} );
	//~ return std::string_view{std::next( cf.begin() ),last}|std::views::join_with(' ');
};

}; // namespace mth

// Suffix operator for std::int64_t case.
// Note: GCC fails to compile literal operator friends for template classes
// [PR C++/61648] (gcc.gnu.org/bugzilla/show_bug.cgi?id=61648), but clang does.
constexpr mth::fraction< std::int64_t >
operator""_f( const unsigned long long from ) {
	return mth::fraction{ (std::int64_t)from };
};
constexpr mth::fraction< std::int64_t > operator""_f( const long double from ) {
	return mth::fraction{ (double)from };
};

#endif
