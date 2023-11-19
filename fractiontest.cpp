/*
 * fractiontest.cpp
 * 
 * Copyright 2023 2023 Greg Lander<greg.j.lander@gmail.com>
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


#include <iostream>
#include <iomanip>
//#include <format>
#include <vector>
#include <cassert>
#include "fraction.hpp"

std::string check( std::string s, std::string expected ) {
	if ( expected != "" ) {
		std::string m{ "\nERROR: Actual:" + s + " != Expected:" + expected };
		if ( s.compare( expected ) != 0 )
			std::cout << m << '\n';
		//assert( s.compare( expected ) == 0 );
	};
	return s;
};
std::string fmt( std::string str, std::size_t width = 0 ) {
	if ( width == 0 ) {
		return '"' + str + "\",";
	} else {
		//return std::format("{:{}}", str, width);
		std::string s2( str );
		for ( ;; ) {
			if ( s2.length() < width )
				s2 = ' ' + s2;
			else
				break;
		};
		return s2 + "│";
	};
};

template< typename... T >
void fmt( std::string &s1, std::string &s2, T &&...p ) {
	s2 += "{";
	std::initializer_list< std::tuple< std::string, int > > il{ p... };
	for ( auto [result, width] : il ) {
		s1 += fmt( result, width );
		s2 += fmt( result );
	};
	s1 += '\n';
	s2.pop_back();
	s2 += "},\n";
};

using Fraction = mth::fraction<>;

int main() {
	const std::vector< Fraction > f{
		7_f,		Fraction::f_0,		{ 2, 0 },
		{ 1, 4 },	{ 48, 7 },			{ 3, 2 },
		{ 5, 3 },	{ -25, 49 },		{ 4, -10 },
		2_f,		{ 49, 25 },			{ 8, 27 },
		{ 56, 45 }, { 392, 10125 },		Fraction{ 3.141592654 },
		{ 1, 3 }, Fraction{ 0.33333 }
	};
	std::string result{};
	std::string st{};

	std::vector< std::vector< std::string > > expected;
	expected = {
		{ "7", "7.000000", "7", "(1/7)", "true", "false", "(10/3)", "(31/12)" },
		{ "0", "0.000000", "0", "(1/0)", "true", "false", "1", "(1/4)" },
		{ "(1/0)", "inf", "(1/0)", "0", "false", "false", "2", "(1/0)" },
		{ "(1/4)", "0.250000", "(1/4)", "4", "false", "false", "(2/3)", "(1/3)" },
		{ "(48/7)", "6.857143", "(48/7)", "(7/48)", "false", "false", "(17/3)", "(71/28)" },
		{ "(3/2)", "1.500000", "(3/2)", "(2/3)", "false", "false", "(3/2)", "(3/4)" },
		{ "(5/3)", "1.666667", "(5/3)", "(3/5)", "false", "false", "(8/5)",  "(29/36)" },
		{ "(-25/49)", "-0.510204", "(25/49)", "(-49/25)", "false", "true",
		  "(-22/51)", "(47/588)" },
		{ "(-2/5)", "-0.400000", "(2/5)", "(-5/2)", "false", "true", "(1/7)", "(7/60)" },
		{ "2", "2.000000", "2", "(1/2)", "true", "false", "(5/3)", "(11/12)" },
		{ "(49/25)", "1.960000", "(49/25)", "(25/49)", "false", "false",
		  "(52/27)", "(271/300)" },
		{ "(8/27)", "0.296296", "(8/27)", "(27/8)", "false", "false", "(11/29)", "(113/324)" },
		{ "(56/45)", "1.244444", "(56/45)", "(45/56)", "false", "false",
		  "(59/47)", "(359/540)" },
		{ "(392/10125)", "0.038716", "(392/10125)", "(10125/392)", "false",
		  "false", "(395/10127)", "(31943/121500)" },
		{ "(355/113)", "3.141593", "(355/113)", "(113/355)", "false", "false",
		  "(358/115)", "(1759/1356)" },
		{ "(1/3)", "0.333333", "(1/3)", "3", "false", "false", "(4/5)", "(13/36)" },
		{ "(25641/76924)","0.333329","(25641/76924)", "(76924/25641)" ,"false","false","(4274/12821)","(13889/38462)"}
	};
	std::cout << " Init:       │to_double│ abs:        │ inv:        "
				 "│is_int│is_neg│Mediant(f,1.5)│Av(f,0.5,0.25)\n";
	if ( expected.size() != f.size() ) {
		std::cout << "ERROR: Expected results array[" +
						 std::to_string( expected.size() ) +
						 "] is wrong size[" + std::to_string( f.size() ) +
						 "].\n";
	};
	for ( auto i = 0; i != (int)f.size(); ++i ) {
		fmt( st, result, std::tuple{ f[i].to_string(), 13 },
			 std::tuple{ std::to_string( f[i].to_double() ), 9 },
			 std::tuple{ check( f[i].abs().to_string(), expected[i][2] ), 13 },
			 std::tuple{ check( f[i].inv().to_string(), expected[i][3] ), 13 },
			 std::tuple{
				 check( ( f[i].is_int() ? "true" : "false" ), expected[i][4] ),
				 6 },
			 std::tuple{
				 check( ( f[i].is_neg() ? "true" : "false" ), expected[i][5] ),
				 6 },
			 std::tuple{ check( mediant( f[i], Fraction{ 3, 2 } ).to_string(),
								expected[i][6] ), 14 },
			 std::tuple{
				 check( average( Fraction{ 1, 2 }, Fraction{ 1, 4 }, f[i] )
							.to_string(), expected[i][7] ), 14 } );
	};
	std::cout << st << '\n';
	// std::cout << result;

	result = "";
	st = "";
	expected = {
		{"7","2.645752","(2024/765)","{(2024/765),0}","false","(2024/765)","{1,7}","false","(1494/781)","{1,7}"},
		{"0","0.000000","0","{0,0}","true","0","{1,0}","true","0","{1,0}"},
		{"(1/0)","inf","(1/0)","{(1/0),0}","true","(1/0)","{1,(1/0)}","true","(1/0)","{1,(1/0)}"},
		{"(1/4)","0.500000","(1/2)","{(1/2),0}","true","(1/2)","{(1/2),1}","false","(635/1008)","{1,(1/4)}"},
		{"(48/7)","2.618615","(2307/881)","{(2307/881),0}","false","(2307/881)","{4,(3/7)}","false","(2219/1168)","{2,(6/7)}"},
		{"(3/2)","1.224745","(1079/881)","{(1079/881),0}","false","(1079/881)","{1,(3/2)}","false","(1321/1154)","{1,(3/2)}"},
		{"(5/3)","1.290995","(1362/1055)","{(1362/1055),0}","false","(1362/1055)","{1,(5/3)}","false","(2261/1907)","{1,(5/3)}"},
		{"(-25/49)","0.000000","0","{(1/1000000),(5/7)}","true","0","{(5/7),(-1)}","false","(-1364/1707)","{1,(-25/49)}"},
		{"(-2/5)","0.000000","0","{(1/1000000),(456/721)}","false","0","{1,(-2/5)}","false","(-1061/1440)","{1,(-2/5)}"},
		{"2","1.414213","(1393/985)","{(1393/985),0}","false","(1393/985)","{1,2}","false","(635/504)","{1,2}"},
		{"(49/25)","1.400000","(7/5)","{(7/5),0}","true","(7/5)","{(7/5),1}","false","(1921/1535)","{1,(49/25)}"},
		{"(8/27)","0.544331","(749/1376)","{(749/1376),0}","false","(749/1376)","{(2/3),(2/3)}","true","(2/3)","{(2/3),1}"},
		{"(56/45)","1.115546","(531/476)","{(531/476),0}","false","(531/476)","{(2/3),(14/5)}","false","(825/767)","{2,(7/45)}"},
		{"(392/10125)","0.196763","(231/1174)","{(231/1174),0}","false","(231/1174)","{(14/45),(2/5)}","false","(409/1209)","{(2/15),(49/3)}"},
		{"(355/113)","1.772453","(4993/2817)","{(4993/2817),0}","false","(4993/2817)","{1,(355/113)}","false","(1365/932)","{1,(355/113)}"},
		{"(1/3)","0.577351","(571/989)","{(571/989),0}","false","(571/989)","{1,(1/3)}","false","(658/949)","{1,(1/3)}"},
		{"(25641/76924)","0.577346","(683/1183)","{(683/1183),0}","false","(683/1183)","{(3/2),(2849/19231)}","false","(762/1099)","{1,(25641/76924)}"}
	};
	std::cout << " Init:       │ pow0.5:             │ pow_c0.5:             "
				 "│abssq│ sqrt:     │"
			  << " simplify_sqrt:     │is_cb│ cbrt:      │ simplify_cbrt:\n";
	if ( expected.size() != f.size() ) {
		std::cout << "ERROR: Expected results array[" +
						 std::to_string( expected.size() ) +
						 "] is wrong size[" + std::to_string( f.size() ) +
						 "].\n";
	};
	for ( auto i = 0; i != (int)f.size(); ++i ) {
		const std::pair simp_sqrt = f[i].simplify_sqrt();
		const std::pair simp_cbrt = f[i].simplify_cbrt();
		const std::pair pow_c = f[i].pow_c( 0.5 );
		fmt( st, result, std::tuple{ f[i].to_string(), 13 },
			 std::tuple{ check( std::to_string( f[i].pow( 0.5 ).to_double() ),
								expected[i][1] ), 9 },
			 std::tuple{ check( f[i].pow( 0.5 ).to_string(), expected[i][2] ), 11 },
			 std::tuple{ check( "{" + pow_c.first.to_string() + "," +
									pow_c.second.to_string() + "}",
								expected[i][3] ), 23 },
			 std::tuple{
				 check( f[i].is_abs_sq() ? "true" : "false", expected[i][4] ),
				 5 },
			 std::tuple{ check( f[i].sqrt().to_string(), expected[i][5] ), 11 },
			 std::tuple{ check( "{" + simp_sqrt.first.to_string() + "," +
									simp_sqrt.second.to_string() + "}",
								expected[i][6] ), 20 },
			 std::tuple{
				 check( f[i].is_cb() ? "true" : "false", expected[i][7] ), 5 },
			 std::tuple{ check( f[i].cbrt().to_string(), expected[i][8] ), 12 },
			 std::tuple{ check( "{" + simp_cbrt.first.to_string() + "," +
									simp_cbrt.second.to_string() + "}",
								expected[i][9] ), 17 } );
	};
	std::cout << st << '\n';
	// std::cout << result;

	result = "";
	st = "";
	expected = { 
		{"7","{(7/8),3}","(7/16)","49","(1/49)","343","343"},
		{"0","{0,0}","0","0","0","0","0"},
		{"(1/0)","{(1/0),0}","(1/0)","(1/0)","0","(1/0)","(1/0)"},
		{"(1/4)","{(1/2),-1}","(1/64)","(1/16)","16","(1/64)","(1/64)"},
		{"(48/7)","{(6/7),3}","(3/7)","(2304/49)","(45/2116)","(110592/343)","(110592/343)"},
		{"(3/2)","{(3/4),1}","(3/32)","(9/4)","(4/9)","(27/8)","(27/8)"},
		{"(5/3)","{(5/6),1}","(5/48)","(25/9)","(9/25)","(125/27)","(125/27)"},
		{"(-25/49)","{(-25/49),0}","(-25/784)","(625/2401)","(2401/625)","(-15625/117649)","(-338/2545)"},
		{"(-2/5)","{(-4/5),-1}","(-1/40)","(4/25)","(25/4)","(-8/125)","(-8/125)"},
		{"2","{(1/2),2}","(1/8)","4","(1/4)","8","8"},
		{"(49/25)","{(49/50),1}","(49/400)","(2401/625)","(423/1625)","(117649/15625)","(3569/474)"},
		{"(8/27)","{(16/27),-1}","(1/54)","(64/729)","(729/64)","(512/19683)","(9/346)"},
		{"(56/45)","{(28/45),1}","(7/90)","(3136/2025)","(627/971)","(175616/91125)","(1906/989)"},
		{"(392/10125)","{(433/699),-4}","(3/1240)","(153664/102515625)","(613103/919)","(60236288/1037970703125)","(1/16940)"},
		{"(355/113)","{(355/452),2}","(269/1370)","(126025/12769)","(23/227)","(44738875/1442897)","(34541/1114)"},
		{"(1/3)","{(2/3),-1}","(1/48)","(1/9)","(9000001/1000000)","(1/27)","(1/27)"},
		{"(25641/76924)","{(22989/34484),-1}","(1/48)","(657460881/5917301776)","(38305/4256)","(16857954449721/455182521817024)","(562/15175)"}
	};
	std::cout
		<< " Init:       │ frexp:           │ ldexp(-4):│ sq:              │"
		   " pow-2:          │ cb:           │ pow3:      \n";
	if ( expected.size() != f.size() ) {
		std::cout << "ERROR: Expected results array[" +
						 std::to_string( expected.size() ) +
						 "] is wrong size[" + std::to_string( f.size() ) +
						 "].\n";
	};
	for ( auto i = 0; i != (int)f.size(); ++i ) {
		const std::pair frexp = f[i].frexp();
		fmt( st, result, std::tuple{ f[i].to_string(), 13 },
			 std::tuple{ check( "{" + frexp.first.to_string() + ',' +
									std::to_string( frexp.second ) + "}",
								expected[i][1] ), 18 },
			 std::tuple{ check( f[i].ldexp( -4 ).to_string(), expected[i][2] ),
						 11 },
			 std::tuple{ check( f[i].sq().to_string(), expected[i][3] ), 18 },
			 std::tuple{ check( f[i].pow( -2 ).to_string(), expected[i][4] ),
						 17 },
			 std::tuple{ check( f[i].cb().to_string(), expected[i][5] ), 15 },
			 std::tuple{ check( f[i].pow( 3 ).to_string(), expected[i][6] ),
						 12 } );
	};
	std::cout << st << '\n';
	// std::cout << result;

	result = "";
	st = "";
	expected = {
		{"7","8","10","4","7","(17/2)","(11/2)","(17/2)","(11/2)","(17/2)","(25/2)","(11/2)","(-3/2)"},
		{"0","1","3","(-3)","0","(3/2)","(-3/2)","(3/2)","(-3/2)","(3/2)","(11/2)","(-3/2)","(11/2)"},
		{"(1/0)","(1/0)","(1/0)","(1/0)","(1/0)","(1/0)","(1/0)","(1/0)","(1/0)","(1/0)","(1/0)","(1/0)","(-1/0)"},
		{"(1/4)","(5/4)","(13/4)","(-11/4)","(1/4)","(7/4)","(-5/4)","(7/4)","(-5/4)","(7/4)","(23/4)","(-5/4)","(21/4)"},
		{"(48/7)","(55/7)","(69/7)","(27/7)","(48/7)","(117/14)","(75/14)","(117/14)","(75/14)","(117/14)","(173/14)","(75/14)","(-19/14)"},
		{"(3/2)","(5/2)","(9/2)","(-3/2)","(3/2)","3","0","3","0","3","7","0","4"},
		{"(5/3)","(8/3)","(14/3)","(-4/3)","(5/3)","(19/6)","(1/6)","(19/6)","(1/6)","(19/6)","(43/6)","(1/6)","(23/6)"},
		{"(-25/49)","(24/49)","(122/49)","(-172/49)","(-25/49)","(97/98)","(-197/98)","(97/98)","(-197/98)","(97/98)","(489/98)","(-197/98)","(589/98)"},
		{"(-2/5)","(3/5)","(13/5)","(-17/5)","(-2/5)","(11/10)","(-19/10)","(11/10)","(-19/10)","(11/10)","(51/10)","(-19/10)","(59/10)"},
		{"2","3","5","(-1)","2","(7/2)","(1/2)","(7/2)","(1/2)","(7/2)","(15/2)","(1/2)","(7/2)"},
		{"(49/25)","(74/25)","(124/25)","(-26/25)","(49/25)","(173/50)","(23/50)","(173/50)","(23/50)","(173/50)","(373/50)","(23/50)","(177/50)"},
		{"(8/27)","(35/27)","(89/27)","(-73/27)","(8/27)","(97/54)","(-65/54)","(97/54)","(-65/54)","(97/54)","(313/54)","(-65/54)","(281/54)"},
		{"(56/45)","(101/45)","(191/45)","(-79/45)","(56/45)","(247/90)","(-23/90)","(247/90)","(-23/90)","(247/90)","(607/90)","(-23/90)","(383/90)"},
		{"(392/10125)","(10517/10125)","(30767/10125)","(-29983/10125)","(392/10125)","(1391/904)","(-1321/904)","(31159/20250)","(-29591/20250)","(1391/904)","(5007/904)","(-1321/904)","(4937/904)"},
		{"(355/113)","(468/113)","(694/113)","(16/113)","(355/113)","(1049/226)","(371/226)","(1049/226)","(371/226)","(1049/226)","(1953/226)","(371/226)","(533/226)"},
		{"(1/3)","(4/3)","(10/3)","(-8/3)","(1/3)","(11/6)","(-7/6)","(11/6)","(-7/6)","(11/6)","(35/6)","(-7/6)","(31/6)"},
		{"(25641/76924)","(102565/76924)","(256413/76924)","(-205131/76924)","(25641/76924)","(57297/31253)","(-36462/31253)","(141027/76924)","(-89745/76924)","(57297/31253)","(182309/31253)","(-36462/31253)","(161474/31253)"}
	};
	std::cout
		<< " Init:       │ ++          "
		<< "│ +3      │ -3       │ --      │ +1.5     │ -1.5     │ +{3,2}   "
		<< "│ -{3,2}   │ += 1.5   │ 4+f      │ -= 1.5   │ 4-f    \n";
	if ( expected.size() != f.size() ) {
		std::cout << "ERROR: Expected results array[" +
						 std::to_string( expected.size() ) +
						 "] is wrong size[" + std::to_string( f.size() ) +
						 "].\n";
	};
	for ( auto i = 0; i != (int)f.size(); ++i ) {
		Fraction t1 = f[i];
		Fraction t2 = f[i];
		t1 += 1.5;
		t2 -= 1.5;
		Fraction t3 = f[i];
		++t3;
		Fraction t4 = t3;
		--t4;
		fmt(
			st, result, std::tuple{ f[i].to_string(), 13 },
			std::tuple{ check( t3.to_string(), expected[i][1] ), 13 },
			std::tuple{ check( ( f[i] + 3l ).to_string(), expected[i][2] ), 9 },
			std::tuple{ check( ( f[i] - 3l ).to_string(), expected[i][3] ),
						10 },
			std::tuple{ check( t4.to_string(), expected[i][4] ), 9 },
			std::tuple{ check( ( f[i] + 1.5 ).to_string(), expected[i][5] ),
						10 },
			std::tuple{ check( ( f[i] - 1.5 ).to_string(), expected[i][6] ),
						10 },
			std::tuple{ check( ( f[i] + Fraction{ 3, 2 } ).to_string(),
							   expected[i][7] ),
						10 },
			std::tuple{
				check( ( f[i] - 3_f / 2l ).to_string(), expected[i][8] ), 10 },
			std::tuple{ check( ( t1 ).to_string(), expected[i][9] ), 10 },
			std::tuple{ check( ( 4l + t1 ).to_string(), expected[i][10] ), 10 },
			std::tuple{ check( ( t2 ).to_string(), expected[i][11] ), 10 },
			std::tuple{ check( ( 4l - t2 ).to_string(), expected[i][12] ),
						10 } );
	};
	std::cout << st << '\n';
	//std::cout << result;
	
	result = "";
	st = "";
	expected = {
		{"7","35","(7/5)","2","(21/2)","(14/3)","1","(21/2)","(14/3)","1","(21/2)","21","(14/3)","(3/7)","1","5"},
		{"0","0","0","0","0","0","0","0","0","0","0","0","0","(1/0)","0","(1/0)"},
		{"(1/0)","(1/0)","(1/0)","(1/0)","(1/0)","(1/0)","(1/0)","(1/0)","(1/0)","(1/0)","(1/0)","(1/0)","(1/0)","0","(1/0)","(1/0)"},
		{"(1/4)","(5/4)","(1/20)","(1/4)","(3/8)","(1/6)","(1/4)","(3/8)","(1/6)","(1/4)","(3/8)","(3/4)","(1/6)","12","(1/4)","0"},
		{"(48/7)","(240/7)","(48/35)","(13/7)","(72/7)","(32/7)","(6/7)","(72/7)","(32/7)","(6/7)","(72/7)","(144/7)","(32/7)","(7/16)","(6/7)","5"},
		{"(3/2)","(15/2)","(3/10)","(3/2)","(9/4)","1","0","(9/4)","1","0","(9/4)","(9/2)","1","2","0","(1/2)"},
		{"(5/3)","(25/3)","(1/3)","(5/3)","(5/2)","(10/9)","(1/6)","(5/2)","(10/9)","(1/6)","(5/2)","5","(10/9)","(9/5)","(1/6)","0"},
		{"(-25/49)","(-125/49)","(-5/49)","(-25/49)","(-75/98)","(-50/147)","(-25/49)","(-75/98)","(-50/147)","(-25/49)","(-75/98)","(-75/49)","(-50/147)","(-147/25)","(-25/49)","(20/49)"},
		{"(-2/5)","(-2)","(-2/25)","(-2/5)","(-3/5)","(-4/15)","(-2/5)","(-3/5)","(-4/15)","(-2/5)","(-3/5)","(-6/5)","(-4/15)","(-15/2)","(-2/5)","(1/5)"},
		{"2","10","(2/5)","2","3","(4/3)","(1/2)","3","(4/3)","(1/2)","3","6","(4/3)","(3/2)","(1/2)","1"},
		{"(49/25)","(49/5)","(49/125)","(49/25)","(147/50)","(98/75)","(23/50)","(147/50)","(98/75)","(23/50)","(147/50)","(147/25)","(98/75)","(75/49)","(23/50)","(27/25)"},
		{"(8/27)","(40/27)","(8/135)","(8/27)","(4/9)","(16/81)","(8/27)","(4/9)","(16/81)","(8/27)","(4/9)","(8/9)","(16/81)","(81/8)","(8/27)","(7/27)"},
		{"(56/45)","(56/9)","(56/225)","(56/45)","(28/15)","(112/135)","(56/45)","(28/15)","(112/135)","(56/45)","(28/15)","(56/15)","(112/135)","(135/56)","(56/45)","(1/45)"},
		{"(392/10125)","(392/2025)","(392/50625)","(392/10125)","(41/706)","(35/1356)","(35/904)","(196/3375)","(784/30375)","(392/10125)","(41/706)","(41/353)","(35/1356)","(2712/35)","(35/904)","(19/3375)"},
		{"(355/113)","(1775/113)","(71/113)","(355/113)","(1065/226)","(710/339)","(16/113)","(1065/226)","(710/339)","(16/113)","(1065/226)","(1065/113)","(710/339)","(339/355)","(16/113)","(210/113)"},
		{"(1/3)","(5/3)","(1/15)","(1/3)","(1/2)","(2/9)","(1/3)","(1/2)","(2/9)","(1/3)","(1/2)","1","(2/9)","9","(1/3)","0"},
		{"(25641/76924)","(128205/76924)","(25641/384620)","(25641/76924)","(33334/66669)","(6351/28580)","(20834/62503)","(76923/153848)","(8547/38462)","(25641/76924)","(33334/66669)","(66668/66669)","(6351/28580)","(57160/6351)","(20834/62503)","(5/76924)"}
	};
	std::cout << " Init:       │ *5       │ /5      │ %5      │ *1.5     │ /1.5    " 
		<< "│ %1.5   │ *{3,2}   │ /{3,2}  │ %{3,2}  │ *=1.5    │ 2*f1     │ /=1.5   " 
		<< "│ 2/f2    │ %=1.5   │ 5%f0    │\n";
	if ( expected.size() != f.size() ) {
		std::cout << "ERROR: Expected results array[" +
						 std::to_string( expected.size() ) +
						 "] is wrong size[" + std::to_string( f.size() ) +
						 "].\n";
	};
	for ( auto i = 0; i != (int)f.size(); ++i ) {
		Fraction t1 = f[i];
		Fraction t2 = f[i];
		Fraction t3 = f[i];
		t1 *= 1.5;
		t2 /= 1.5;
		t3 %= 1.5;
		fmt(
			st, result, std::tuple{ f[i].to_string(), 13 },
			std::tuple{ check( ( f[i] * 5l ).to_string(), expected[i][1] ),
						10 },
			std::tuple{ check( ( f[i] / 5l ).to_string(), expected[i][2] ), 9 },
			std::tuple{ check( ( f[i] % 5l ).to_string(), expected[i][3] ), 9 },
			std::tuple{ check( ( f[i] * 1.5 ).to_string(), expected[i][4] ),
						10 },
			std::tuple{ check( ( f[i] / 1.5 ).to_string(), expected[i][5] ),
						9 },
			std::tuple{ check( ( f[i] % 1.5 ).to_string(), expected[i][6] ),
						8 },
			std::tuple{
				check( ( f[i] * 3_f / 2l ).to_string(), expected[i][7] ), 10 },
			std::tuple{ check( ( f[i] / Fraction{ 3, 2 } ).to_string(),
							   expected[i][8] ), 9 },
			std::tuple{ check( ( f[i] % Fraction{ 3, 2 } ).to_string(),
							   expected[i][9] ), 9 },
			std::tuple{ check( t1.to_string(), expected[i][10] ), 10 },
			std::tuple{ check( ( 2l * t1 ).to_string(), expected[i][11] ), 10 },
			std::tuple{ check( t2.to_string(), expected[i][12] ), 9 },
			std::tuple{ check( ( 2l / t2 ).to_string(), expected[i][13] ), 9 },
			std::tuple{ check( t3.to_string(), expected[i][14] ), 9 },
			std::tuple{ check( ( 5l % f[i] ).to_string(), expected[i][15] ),
						9 } );
	};
	std::cout << st << '\n';
	// std::cout << result;

	result = "";
	st = "";
	expected = {
		{"7","false","true","false","true","false","true","false","true"},
		{"0","false","true","true","false","true","false","false","true"},
		{"(1/0)","false","true","false","true","false","true","false","true"},
		{"(1/4)","false","true","true","false","true","false","false","true"},
		{"(48/7)","false","true","false","true","false","true","false","true"},
		{"(3/2)","false","true","false","true","false","true","false","true"},
		{"(5/3)","false","true","false","true","false","true","false","true"},
		{"(-25/49)","false","true","true","false","true","false","true","false"},
		{"(-2/5)","false","true","true","false","true","false","true","false"},
		{"2","false","true","false","true","false","true","false","true"},
		{"(49/25)","false","true","false","true","false","true","false","true"},
		{"(8/27)","false","true","true","false","true","false","false","true"},
		{"(56/45)","false","true","false","true","false","true","false","true"},
		{"(392/10125)","false","true","true","false","true","false","false","true"},
		{"(355/113)","false","true","false","true","false","true","false","true"},
		{"(1/3)","true","false","true","false","true","true","false","true"},
		{"(25641/76924)","false","true","true","false","true","false","false","true"}
	};
	std::cout << " Init:       │==1/3│!=1/3│ <2/3│ >2/3│<=1/3"
			  << "│>=1/3│<-1/3│>-1/3│\n";
	if ( expected.size() != f.size() ) {
		std::cout << "ERROR: Expected results array[" +
						 std::to_string( expected.size() ) +
						 "] is wrong size[" + std::to_string( f.size() ) +
						 "].\n";
	};
	for ( auto i = 0; i != (int)f.size(); ++i ) {
		fmt( st, result, std::tuple{ f[i].to_string(), 13 },
			 std::tuple{ check( ( f[i] == ( 1l / 3_f ) ) ? "true" : "false",
								expected[i][1] ), 5 },
			 std::tuple{ check( ( f[i] != ( 1l / 3_f ) ) ? "true" : "false",
								expected[i][2] ), 5 },
			 std::tuple{ check( ( f[i] < ( 2l / 3_f ) ) ? "true" : "false",
								expected[i][3] ), 5 },
			 std::tuple{ check( ( f[i] > ( 2l / 3_f ) ) ? "true" : "false",
								expected[i][4] ), 5 },
			 std::tuple{ check( ( f[i] <= ( 1l / 3_f ) ) ? "true" : "false",
								expected[i][5] ), 5 },
			 std::tuple{ check( ( f[i] >= ( 1l / 3_f ) ) ? "true" : "false",
								expected[i][6] ), 5 },
			 std::tuple{ check( ( f[i] < ( -1l / 3_f ) ) ? "true" : "false",
								expected[i][7] ), 5 },
			 std::tuple{ check( ( f[i] > ( -1l / 3_f ) ) ? "true" : "false",
								expected[i][8] ), 5 } );
	};
	std::cout << st << '\n';
	// std::cout << result;

	result = "";
	st = "";
	expected = {
		{"7","7.000000","7","7","7"},
		{"0","0.000000","0","0","0"},
		{"(1/0)","inf"},
		{"(1/4)","0.250000","0,4","(1/4)","(1/4)"},
		{"(48/7)","6.857143","6,1,5,1","(48/7)","(48/7)"},
		{"(3/2)","1.500000","1,2","(3/2)","(3/2)"},
		{"(5/3)","1.666667","1,1,2","(5/3)","(5/3)"},
		{"(-25/49)","-0.510204","0,-1,-1,-23,-1","(-25/49)","(-25/49)"},
		{"(-2/5)","-0.400000","0,-2,-2","(-2/5)","(-2/5)"},
		{"2","2.000000","2","2","2"},
		{"(49/25)","1.960000","1,1,23,1","(49/25)","(49/25)"},
		{"(8/27)","0.296296","0,3,2,1,1,1","(8/27)","(8/27)"},
		{"(56/45)","1.244444","1,4,11","(56/45)","(56/45)"},
		{"(392/10125)","0.038716","0,25,1,4,1,5,1,2,3","(392/10125)","(35/904)"},
		{"(355/113)","3.141593","3,7,16","(355/113)","(355/113)"},
		{"(1/3)","0.333333","0,3","(1/3)","(1/3)"},
		{"(25641/76924)","0.333329","0,3,25641","(25641/76924)","(20834/62503)"}
	};
	std::cout << " Init:       │ get:    │continued_frac:    │ decode_cf:  "
				 "│stern_brocot: │ \n";
	if ( expected.size() != f.size() ) {
		std::cout << "ERROR: Expected results array[" +
						 std::to_string( expected.size() ) +
						 "] is wrong size[" + std::to_string( f.size() ) +
						 "].\n";
	};
	for ( auto i = 0; i != (int)f.size(); ++i ) {

		if ( ( f[i].den() == 0 ) )
			fmt( st, result, std::tuple{ f[i].to_string(), 13 },
				 std::tuple{ std::to_string( f[i].to_double() ), 9 } );
		else {
			auto cf = mth::to_continued_fraction( f[i].to_double() );
			std::span cfs = std::span{ cf };
			fmt(
				st, result, std::tuple{ f[i].to_string(), 13 },
				std::tuple{ std::to_string( f[i].to_double() ), 9 },
				// to_string(std::span{cf})
				std::tuple{ check( mth::to_string( cf ), expected[i][2] ), 19 },
				std::tuple{ check( mth::to_fraction_using_continued_fractions(
									   f[i].to_double() ).to_string(),
								   expected[i][3] ), 13 },
				std::tuple{
					check( mth::to_fraction_using_stern_brocot_with_mediants(
							   f[i].to_double() ).to_string(), expected[i][4] ),
					14 } );
		};
	};
	std::cout << st << '\n';
	// std::cout << result;
	
	Fraction t1( (long)INFINITY, 1);
	const long t1_gcd =  std::gcd( (long) INFINITY, 1 );
	const long t1_num = (long)INFINITY / (long)std::copysign( t1_gcd, 1 );
	Fraction t2(1, (long)INFINITY);
	const long t2_gcd =  std::gcd( 1, (long) INFINITY );
	const long t2_den = (long)std::labs( (long)INFINITY );
	std::cout << "Test: inf[" << t1.to_string() << "," << t1_gcd << "," << t1_num 
		<< "],1/inf:[" << t2.to_string() << "," << t2_gcd << "," << t2_den << "], std::max=" 
		<< std::numeric_limits<long>::max()<< "\n";
	Fraction t5((long)-INFINITY, 1);
	const long t5_gcd =  std::gcd( (long) -INFINITY, 1 );
	const long t5_num = (long)-INFINITY / (long)std::copysign( t5_gcd, 1 );
	Fraction t6(1, (long)-INFINITY);
	const long t6_gcd =  std::gcd( 1, (long) -INFINITY );
	const long t6_den = std::labs( (long)-INFINITY );
	std::cout << "Test: -inf[" << t5.to_string() << "," << t5_gcd << "," << t5_num 
		<< "],1/-inf:[" << t6.to_string() << "," << t6_gcd << "," << t6_den << "], std::min=" 
		<< std::numeric_limits<long>::min()<< "\n";
	Fraction t3( (long)NAN, 1);
	Fraction t4(1, (long)NAN);
	std::cout << "Test: NaN[" << t3.to_string() << "],1/NaN:[" << t4.to_string() << "]\n";
	Fraction t7((long)-NAN, 1);
	Fraction t8(1, (long)-NAN);
	std::cout << "Test: -NaN[" << t7.to_string() << "],1/-NaN:[" << t8.to_string() << "]\n";
	return 0;
}
