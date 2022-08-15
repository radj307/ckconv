#pragma once
#include "conv.hpp"

#include <sysarch.h>
#include <hasPendingDataSTDIN.h>
#include <str.hpp>

#include <algorithm>
#include <vector>
#include <concepts>
#include <cmath>


// gets piped input from STDIN as a vector of strings, where each element was received as a space-delimited string.
inline std::vector<std::string> getInputsFromSTDIN()
{
	std::vector<std::string> vec;
	if (hasPendingDataSTDIN())
		for (std::string buf; std::getline(std::cin, buf, ' '); std::cin.clear()) { vec.emplace_back(buf); }
	return vec;
}


// Concatenates two given vectors
template<typename TElem, std::derived_from<std::allocator<TElem>> TAlloc = std::allocator<TElem>>
std::vector<TElem> cat(std::vector<TElem> const& l, std::vector<TElem> const& r)
{
	std::vector<TElem> lr;
	lr.reserve(l.size() + r.size());
	lr.insert(lr.end(), l.begin(), l.end());
	lr.insert(lr.end(), r.begin(), r.end());
	return lr;
}

inline constexpr auto DIGITS{ "0123456789" };
inline std::vector<std::string> expandUnits(std::vector<std::string> const& input)
{
	std::vector<std::string> vec;
	vec.reserve(input.size());
	for (auto it{ input.begin() }, end{ input.end() }; it != end; ++it) {
		const auto& s{ str::trim(*it, " \t\v\r\n") };
		bool digit{ false }, alpha{ false }, invalid{ s.empty() };
		if (!invalid) {
			for (const auto& c : s) {
				if (str::stdpred::isdigit(c))
					digit = true;
				else if (str::stdpred::isalpha(c))
					alpha = true;
				else {
					invalid = true;
					break;
				}
			}
		}

		if (invalid)
			throw make_exception("Malformed input '", s, "' contains unexpected characters!");

		if (digit && alpha) {
			const size_t alphaPos{ s.find_first_not_of(DIGITS) };

			if (s.find_first_of(DIGITS) > alphaPos || s.find_last_of(DIGITS) > alphaPos)
				throw make_exception("Malformed input '", s, "' is invalid!");

			vec.reserve(vec.capacity() + 1ull);
			vec.emplace_back(s.substr(alphaPos));
			vec.emplace_back(s.substr(0ull, alphaPos));
		}
		else vec.emplace_back(s);
	}
	vec.shrink_to_fit();
	return vec;
}

// Splits a given vector of strings into a vector of 3-string tuples
inline WINCONSTEXPR std::vector<std::tuple<std::string, std::string, std::string>> processInput(std::vector<std::string> const& input)
{
	std::vector<std::tuple<std::string, std::string, std::string>> vec;

	const size_t inputSize{ input.size() };
	if (inputSize == 0ull) return vec;

	// calculate the amount of space required & reserve it
	size_t size{ $c(size_t, ceil($c(double, input.size()) / 3.0)) };
	vec.reserve(size);

	// insert each pair of 3 into the new vector
	for (size_t i{ 0ull }; i < inputSize; i += 3) {
		if (i + 2 >= inputSize) {
			bool secondResult;
			vec.emplace_back(std::make_tuple(
				input[i],
				((secondResult = (i + 1 < inputSize)) ? input[i + 1] : std::string{}),
				(secondResult && i + 2 < inputSize ? input[i + 2] : std::string{})
			));
		}
		else vec.emplace_back(std::make_tuple(input[i], input[i + 1], input[i + 2]));
	}

	//vec.shrink_to_fit(); //< this doesn't have to be called since we precalculated the size
	return vec;
}

inline std::tuple<conv::Unit, long double, conv::Unit> toConvertible(std::tuple<std::string, std::string, std::string> const& tpl)
{
	const auto inValue{ str::stold(std::get<1>(tpl)) };
	const auto inUnit{ conv::getUnit(std::get<0>(tpl)) }, outUnit{ conv::getUnit(std::get<2>(tpl)) };
	return{ inUnit, inValue, outUnit };
}

//inline WINCONSTEXPR std:":"
