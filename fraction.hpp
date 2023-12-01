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
// https://en.cppreference.com/w/cpp/language/operators.
// g++ -Wall -Werror -Wconversion -Wextra -Wpedantic -fsanitize=address
// -std=c++2a -c "%f"
//   -f analyser
// clang -Wall -Wmove -std=c++2a -c "%f"
// g++ -Wall -std=c++2a -o "%e" "%f"
//
// Possible improvements:
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
template< std::integral INT = std::int64_t, int error_exp = -6 > class fraction;
template< std::integral INT = std::int64_t, int error_exp = -6 >
[[nodiscard]] constexpr fraction< INT, error_exp >
to_fraction_using_stern_brocot_with_mediants( const double from ) noexcept;
template< std::integral INT = std::int64_t, int error_exp = -6 >
[[nodiscard]] constexpr fraction< INT, error_exp >
to_fraction( const std::span< INT > &from ) noexcept;

// remove when clang thinks std::pow is constexpr
[[nodiscard]] constexpr double pow10( const int x ) noexcept {
	const double base = 10.0; 
	double result = 1.0;
	if ( x < 0 ) {
		for ( int i = 0; i > x; --i ) {
			result /= base;
		}
	} else {
		for ( int i = 0; i < x; ++i ) {
			result *= base;
		}
	};
	return result;
};

template< std::integral INT, int error_exp > class fraction {
  public:
	// Useful constants:
	static const fraction f_0;	 //{ 0, 1 };
	static const fraction f_1;	 //{ 1, 1 };
	static const fraction f_inf; //{ 1, 0 };

	// Constructors:
	constexpr fraction() noexcept { *this = f_0; }; // set( 0, 1 ); };
	constexpr fraction( const INT num, const INT den ) noexcept {
		set( num, den );
	};
	constexpr fraction( const fraction &from ) = default;
	constexpr fraction( fraction &&from ) = default;

	constexpr explicit fraction( const INT num ) noexcept { set( num, 1 ); };
	constexpr explicit fraction( const double from ) noexcept {
		*this = to_fraction_using_stern_brocot_with_mediants< INT, error_exp >(
			from );
	};

	// Numerator and denominator get methods:
	constexpr INT num() const noexcept { return numerator; };
	constexpr INT den() const noexcept { return denominator; };

	// Conversions:
	// fraction to double.
	constexpr double to_double() const noexcept {
		return (double)numerator / (double)denominator;
	};

	// Accuracy to use when approximating a double with a fraction.
	constexpr static const double error = pow10( error_exp );
	// std::pow( 10.0, (double)error_exp ); // as not constexpr in clang

	// Use stern brocot algorithm to approximate a double with a fraction.
	friend constexpr fraction
	to_fraction_using_stern_brocot_with_mediants< INT, error_exp >(
		const double from ) noexcept;

	// Calculate the continued fraction of a double.
	template< std::size_t continued_fraction_max_iter >
	friend constexpr const std::array< INT, continued_fraction_max_iter >
	to_continued_fraction( const double num );

	// Convert a continued fraction to a fraction.
	friend constexpr fraction
	to_fraction< INT, error_exp >( const std::span< INT > &from ) noexcept;

	// Use continued fraction algorithm to approximate a double with a fraction.
	template< std::size_t continued_fraction_max_iter >
	friend constexpr fraction
	to_fraction_using_continued_fractions( const double num ) noexcept;

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
	[[nodiscard]] constexpr fraction abs() const noexcept {
		return { std::labs( numerator ), denominator };
	};
	// 1/fraction.
	[[nodiscard]] constexpr fraction inv() const noexcept {
		return { denominator, numerator };
	};
	// Calculate the mediant of two fractions.
	[[nodiscard]] friend constexpr fraction
	mediant( const fraction &f1, const fraction &f2 ) noexcept {
		return fraction{ f1.num() + f2.num(), f1.den() + f2.den() };
	};
	// Decomposes fraction into integral and fractional parts like std::modf
	[[nodiscard]] constexpr std::pair< double, fraction > modf() const noexcept {
		std::pair< double, fraction > result;
		if ( denominator == 0 ) {
			result = std::make_pair( 0, *this );
		} else {
			double iptr = 0;
			[[maybe_unused]] double fr = std::modf( to_double(), &iptr );
			result = std::make_pair( (INT)iptr, *this - (INT)iptr );
		};
		return result;
	};
	// Calculate the average of some fractions.
	template< typename... T >
	[[nodiscard]] constexpr friend fraction
	average( const T... fractions ) noexcept {
		const std::size_t size{ sizeof...( fractions ) };
		return ( size == 0 ) ? f_0 : ( fractions + ... ) / (INT)size;
	}

	// Exponentiation:
	// Raise to the power of exp and approximate as a complex fraction.
	[[nodiscard]] constexpr std::pair< fraction, fraction >
	pow_c( double exp ) const noexcept {
		std::pair< fraction, fraction > result;
		if ( ( ( exp < 0 ) && ( numerator == 0 ) ) ||
			 ( ( exp >= 0 ) && ( denominator == 0 ) ) ) {
			result = std::make_pair( f_inf, f_0 );
		} else {
			const std::complex< double > pow_d =
				std::pow( std::complex< double >{ to_double(), 0.0 }, exp );
			result = std::make_pair(
				to_fraction_using_stern_brocot_with_mediants< INT, error_exp >(
					pow_d.real() ),
				to_fraction_using_stern_brocot_with_mediants< INT, error_exp >(
					pow_d.imag() ) );
		};
		return result;
	};
	// Raise to the power of exp and approximate as a fraction.
	// Note: for negatives, pow(-N, 0.5) results in 0, ie the real part.
	[[nodiscard]] constexpr fraction pow( double exp ) const noexcept {
		return ( ( ( exp < 0 ) && ( numerator == 0 ) ) ||
				 ( ( exp >= 0 ) && ( denominator == 0 ) ) )
				   ? *this
				   : to_fraction_using_stern_brocot_with_mediants(
						 std::pow( to_double(), exp ) );
	};
	// Square a fraction.
	[[nodiscard]] constexpr fraction sq() const noexcept {
		return ( *this ) * ( *this );
	};
	// Determine if abs(fraction) is a perfect square.
	[[nodiscard]] constexpr bool is_abs_sq() const noexcept {
		const fraction sqrt_INT{ (INT)std::sqrt( std::labs( numerator ) ),
								 (INT)std::sqrt( denominator ) };
		return abs() == sqrt_INT * sqrt_INT;
	};
	// Cube a fraction.
	[[nodiscard]] constexpr fraction cb() const noexcept {
		return ( *this ) * ( *this ) * ( *this );
	};
	// Determine if this fraction is a perfect cube.
	[[nodiscard]] constexpr bool is_cb() const noexcept {
		const fraction cbrt_INT{ (INT)std::cbrt( numerator ),
								 (INT)std::cbrt( denominator ) };
		return *this == cbrt_INT * cbrt_INT * cbrt_INT;
	};
	// Normalized fraction (range (-1, -0.5], [0.5, 1) ) and integral power of 2
	// as per std::frexp() with the normalized fraction approximated as a
	// fraction. e.g. (48/7) i.e. 6*(2^3)/7 => {(6/7), 3}
	// 				  (1/4)                 => {(1/2), -1}
	[[nodiscard]] constexpr std::pair< fraction, int > frexp() const noexcept {
		int exp = 0;
		fraction fr = ( denominator == 0 )
						   ? *this
						   : fraction{ std::frexp( to_double(), &exp ) };
		return std::make_pair( fr, exp );
	};
	// Call load exponent as per std::ldexp() approximated as a fraction.
	// e.g (2/5).ldexp(3) i.e. (2/5)*2^3 => (16/5)
	[[nodiscard]] constexpr fraction ldexp( int exp ) const noexcept {
		return ( denominator == 0 )
				   ? *this
				   : fraction{ std::ldexp( to_double(), exp ) };
	};
	// Calculate sqrt of a fraction and approximate it as a complex fraction.
	[[nodiscard]] constexpr std::pair< fraction, fraction >
	sqrt_c() const noexcept {
		std::pair< fraction, fraction > result;
		if ( denominator == 0 ) {
			result = std::make_pair( *this, f_0 );
		} else {
			const std::complex< double > sqrt_d{ std::sqrt(
				std::complex< double >{ to_double(), 0.0 } ) };
			result = std::make_pair(
				to_fraction_using_stern_brocot_with_mediants( sqrt_d.real() ),
				to_fraction_using_stern_brocot_with_mediants( sqrt_d.imag() ) );
		};
		return result;
	};
	// Calculate sqrt of a fraction and approximate it as a fraction.
	// Note: for negatives, sqrt(-N) results in 0, ie the real part.
	[[nodiscard]] constexpr fraction sqrt() const noexcept {
		return ( denominator == 0 )
				   ? *this
				   : to_fraction_using_stern_brocot_with_mediants(
						 std::sqrt( to_double() ) );
	};
	// Calculate cbrt of a fraction and approximate it as a fraction.
	[[nodiscard]] constexpr fraction cbrt() const noexcept {
		return ( denominator == 0 )
				   ? *this
				   : to_fraction_using_stern_brocot_with_mediants(
						 std::cbrt( to_double() ) );
	};
	// Split fraction into two parts by extracting any squares(2), cubes(3) etc.
	// See examples below.
	[[nodiscard]] constexpr std::pair< fraction, fraction >
	simplify_rt( const double rt ) const noexcept {
		std::pair result{ f_1, *this };
		auto [np_factor, np_remain] = simplify_root( numerator, rt );
		auto [dp_factor, dp_remain] = simplify_root( denominator, rt );
		if ( ( np_factor > 1 ) || ( dp_factor > 1 ) ) {
			result = std::make_pair( fraction{ np_factor, dp_factor },
									 np_remain / dp_remain );
		};
		return result;
	};
	// Split sqrt into two parts by extracting any squares.
	// e.g. (56/45)     i.e. (2*2*2*7/3*3*5)          =>{(2/3),(14/5)}
	//		(392/10125) i.e. (2*2*2*7*7/3*3*3*3*5*5*5)=>{(14/45),(2/5)}
	[[nodiscard]] constexpr std::pair< fraction, fraction >
	simplify_sqrt() const noexcept {
		return simplify_rt( 2.0 );
	};
	// split cbrt into two parts by extracting any cubes.
	// e.g. (56/135)      i.e. (2*2*2*7/3*3*3*5)            =>{(2/3),(7/5)}
	//		(19208/10125) i.e. (2*2*2*7*7*7*7/3*3*3*3*5*5*5)=>{(14/15),(49/3)}
	[[nodiscard]] constexpr std::pair< fraction, fraction >
	simplify_cbrt() const noexcept {
		return simplify_rt( 3.0 );
	};

	// operators+
	constexpr fraction operator+( const fraction &rhs ) const {
		return { numerator * rhs.den() + denominator * rhs.num(),
				 denominator * rhs.den() };
	};
	constexpr fraction operator+( const INT &rhs ) const {
		return { numerator + denominator * rhs, denominator };
	};
	constexpr fraction operator+( const double &rhs ) const {
		return denominator == 0 ? *this : fraction{ to_double() + rhs };
	};
	constexpr fraction &operator+=( const fraction &rhs ) {
		*this = *this + rhs;
		return *this;
	};
	constexpr fraction &operator+=( const INT &rhs ) {
		*this = *this + rhs;
		return *this;
	};
	constexpr fraction &operator+=( const double &rhs ) {
		*this = *this + rhs;
		return *this;
	};
	constexpr friend fraction operator+( const INT &lhs, const fraction &rhs ) {
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
	constexpr fraction operator-( const INT &rhs ) const {
		return *this + ( -rhs );
	};
	constexpr fraction operator-( const double &rhs ) const {
		return *this + ( -rhs );
	};
	constexpr fraction &operator-=( const fraction &rhs ) {
		*this += -rhs;
		return *this;
	};
	constexpr fraction &operator-=( const INT &rhs ) {
		*this += -rhs;
		return *this;
	};
	constexpr fraction &operator-=( const double &rhs ) {
		*this += -rhs;
		return *this;
	};
	constexpr friend fraction operator-( const INT &lhs, const fraction &rhs ) {
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
	constexpr fraction operator*( const INT &rhs ) const {
		return { numerator * rhs, denominator };
	};
	constexpr fraction operator*( const double &rhs ) const {
		return denominator == 0 ? *this : fraction{ to_double() * rhs };
	};
	constexpr fraction &operator*=( const fraction &rhs ) {
		*this = ( *this ) * rhs;
		return *this;
	};
	constexpr fraction &operator*=( const INT &rhs ) {
		*this = ( *this ) * rhs;
		return *this;
	};
	constexpr fraction &operator*=( const double &rhs ) {
		*this = ( *this ) * rhs;
		return *this;
	};
	constexpr friend fraction operator*( const INT lhs, const fraction &rhs ) {
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
	constexpr fraction operator/( const INT &rhs ) const {
		return { numerator, denominator * rhs };
	};
	constexpr fraction operator/( const double &rhs ) const {
		return ( denominator == 0 ) ? *this : fraction{ to_double() / rhs };
	};
	constexpr fraction &operator/=( const fraction &rhs ) {
		*this = *this / rhs;
		return *this;
	};
	constexpr fraction &operator/=( const INT &rhs ) {
		*this = *this / rhs;
		return *this;
	};
	constexpr fraction &operator/=( const double &rhs ) {
		*this = *this / rhs;
		return *this;
	};
	constexpr friend fraction operator/( const INT lhs, const fraction &rhs ) {
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
						 (INT)std::trunc( ( *this / rhs ).to_double() ) * rhs;
	};
	constexpr fraction operator%( const INT &rhs ) const {
		return ( ( denominator == 0 ) || ( rhs == 0 ) )
				   ? f_inf
				   : *this -
						 (INT)std::trunc( ( *this / rhs ).to_double() ) * rhs;
	};
	constexpr fraction operator%( const double &rhs ) const {
		return ( denominator == 0 ) ? *this
									: fraction{ std::fmod( to_double(), rhs ) };
	};
	constexpr fraction &operator%=( const fraction &rhs ) {
		*this = *this % rhs;
		return *this;
	};
	constexpr fraction &operator%=( const INT &rhs ) {
		*this = *this % rhs;
		return *this;
	};
	constexpr fraction &operator%=( const double &rhs ) {
		*this = *this % rhs;
		return *this;
	};
	constexpr friend fraction operator%( const INT lhs, const fraction &rhs ) {
		return fraction{ lhs, 1 } % rhs;
	};
	constexpr friend fraction operator%( const double lhs,
										 const fraction &rhs ) {
		return ( rhs.den() == 0 )
				   ? rhs
				   : fraction{ std::fmod( lhs, rhs.to_double() ) };
	};

	// =
	constexpr fraction &operator=( const fraction &rhs ) = default;
	constexpr fraction &operator=( fraction &&rhs ) = default;

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
	constexpr void set( const INT num = 1, const INT den = 1 ) noexcept {
		// standard undefined behaviour if denominator is 0
		initial_num = num;
		initial_den = den;
		const INT gcd = std::gcd( num, den );
		numerator = num / (INT)std::copysign( gcd, den );
		denominator = std::labs( den ) / gcd;
	};
	
	// Split INT into two parts by extracting any squares(2), cubes(3) etc.
	// See examples above.
	[[nodiscard]] constexpr std::pair< INT, fraction >
	simplify_root( const INT i, const double root ) const noexcept {
		fraction remain{ i };
		INT factor = (INT)std::floor( std::pow( std::labs( i ), 1.0 / root ) );
		for ( ; factor != 0; --factor ) {
			remain = fraction{ i, (INT)std::pow( factor, root ) };
			if ( remain.denominator == 1 ) {
				break;
			};
		};
		if ( factor == 0 ) {
			++factor;
		};
		return std::make_pair( factor, remain );
	};

	// Store the fraction.
	INT initial_num;
	INT numerator;
	INT initial_den;
	INT denominator;
}; // class fraction

template< std::integral INT, int error_exp >
const fraction< INT, error_exp > fraction< INT, error_exp >::f_0{ 0, 1 };
template< std::integral INT, int error_exp >
const fraction< INT, error_exp > fraction< INT, error_exp >::f_1{ 1, 1 };
template< std::integral INT, int error_exp >
const fraction< INT, error_exp > fraction< INT, error_exp >::f_inf{ 1, 0 };

template< std::integral INT, int error_exp >
[[nodiscard]] constexpr fraction< INT, error_exp >
to_fraction_using_stern_brocot_with_mediants( const double from ) noexcept {
	fraction< INT, error_exp > med{};
	// save steps by not starting at infinity and 0
	for ( fraction< INT, error_exp > high{ (INT)std::ceil( from ) },
		  low{ (INT)std::floor( from ) };
		  ; ) {
		med = mediant( low, high );
		if ( med.to_double() - from > fraction< INT, error_exp >::error ) {
			high = med;
		} else if ( med.to_double() - from <
					-fraction< INT, error_exp >::error ) {
			low = med;
		} else {
			break;
		};
	};
	return med;
};

template< std::size_t continued_fraction_max_iter = 25,
		  std::integral INT = std::int64_t, int error_exp = -6 >
[[nodiscard]] constexpr std::array< INT, continued_fraction_max_iter >
to_continued_fraction( const double num ) noexcept {
	std::array< INT, continued_fraction_max_iter > result{ 0 };
	double remainder = num;
	double iptr;
	for ( auto it = result.begin(); it != result.end();
		  ++it, remainder = 1.0 / remainder ) {
		remainder = std::modf( remainder, &iptr );
		*it = (INT)iptr;
		if ( std::abs( remainder ) < fraction< INT, error_exp >::error ) {
			break;
		};
	};
	return result;
};

template< std::integral INT, int error_exp >
[[nodiscard]] constexpr fraction< INT, error_exp >
to_fraction( const std::span< INT > &from ) noexcept {
	return std::accumulate( std::next( from.rbegin() ), from.rend(),
							fraction< INT, error_exp >::f_0,
							[]( fraction< INT, error_exp > a, INT b ) {
								return ( a.num() == 0 ) ? fraction{ b }
														: a.inv() + b;
							} );
};

template< std::size_t continued_fraction_max_iter = 25,
		  std::integral INT = std::int64_t, int error_exp = -6 >
[[nodiscard]] constexpr fraction< INT, error_exp >
to_fraction_using_continued_fractions( const double num ) noexcept {
	auto cf{ to_continued_fraction< continued_fraction_max_iter >( num ) };
	return to_fraction( std::span< INT >{ cf } );
};

template< std::integral INT = std::int64_t, int error_exp = -6 >
[[nodiscard]] std::string to_string( const auto &cf ) noexcept {
	const auto last =
		std::ranges::find_if( cf.rbegin(), std::prev( cf.rend() ),
							  []( auto val ) { return val != 0; } )
			.base();
	return std::accumulate(
		std::next( cf.begin() ), last, std::to_string( cf[0] ),
		[]( std::string result, int val ) {
			return std::move( result ) + ',' + std::to_string( val );
		} );
	//~ return std::string_view{std::next( cf.begin()
	//),last}|std::views::join_with(' ');
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
