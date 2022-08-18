#pragma once
/**
 * @file	util.h
 * @author	radj307
 * @brief	Contains general utility functions for the ckconv application.
 */
#include "conv.hpp"

#include <sysarch.h>
#include <hasPendingDataSTDIN.h>
#include <str.hpp>

#include <algorithm>
#include <vector>
#include <concepts>
#include <cmath>


namespace ckconv {
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

	// defines characters that represent digits
	inline constexpr auto DIGITS{ "0123456789-." };

	// enumerates a given vector of strings and 'expands' any arguments that contain a number AND a unit, i.e. "250m".
	inline std::vector<std::string> expandUnits(std::vector<std::string> const& input)
	{
		std::vector<std::string> vec;
		vec.reserve(input.size());
		for (auto it{ input.begin() }, end{ input.end() }; it != end; ++it) {
			auto s{ str::trim(*it, " \t\v\r\n") };
			s.erase(std::remove(s.begin(), s.end(), ','), s.end()); //< erase all commas

			bool
				digit{ false },			//< has digit chars
				alpha{ false },			//< has alphabetic chars
				invalid{ s.empty() };	//< has invalid chars
			size_t decimalPointCount{ 0ull };

			if (!invalid) {
				for (const auto& c : s) {
					if (str::stdpred::isdigit(c))
						digit = true;
					else if (str::stdpred::isalpha(c))
						alpha = true;
					else if (c == '.') {
						if (++decimalPointCount > 1)
							throw make_exception("Input '", s, "' isn't valid! (Too many decimal places)");
					}
					else if (c == '-') {
						if (digit)
							throw make_exception("Input '", s, "' isn't valid! (Negative sign must precede number)");
					}
					else {
						invalid = true;
						break;
					}
				}
			}

			if (invalid) // check invalid regardless of whether previous if statement triggered or not
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

	// Splits a given vector of strings into a vector of 3-string tuples. Also sorts entries into the correct order, so that input units are defined first, them the input value, then the output unit.
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
			std::tuple<std::string, std::string, std::string> tpl;
			if (i + 2 >= inputSize) {
				bool secondResult{};
				tpl = std::make_tuple(
					input[i],
					((secondResult = (i + 1 < inputSize)) ? input[i + 1] : std::string{}),
					(secondResult && i + 2 < inputSize ? input[i + 2] : std::string{})
				);
			}
			else tpl = std::make_tuple(input[i], input[i + 1], input[i + 2]);

			if (const auto& fst{ std::get<0>(tpl) }; std::all_of(fst.begin(), fst.end(), [](auto&& ch) { return str::stdpred::isdigit(ch) || ch == '-' || ch == '.'; })) {
				// reorder inputs
				std::get<0>(tpl) = std::get<1>(tpl);
				std::get<1>(tpl) = fst;
			}

			vec.emplace_back(tpl);
		}

		//vec.shrink_to_fit(); //< this doesn't have to be called since we precalculated the size
		return vec;
	}

	// Converts from a tuple of 3 strings to a tuple where the first item is the operand's unit, the second item is the operand, and the third item is the output (or 'target') unit.
	template<var::numeric T = long double>
	inline std::tuple<conv::Unit, T, conv::Unit> toConvertible(std::tuple<std::string, std::string, std::string> const& tpl)
	{
		const auto inValue{ str::stold(std::get<1>(tpl)) };
		const auto inUnit{ conv::getUnit(std::get<0>(tpl)) }, outUnit{ conv::getUnit(std::get<2>(tpl)) };
		return{ inUnit, inValue, outUnit };
	}

	template<typename TChar = char, std::derived_from<std::char_traits<TChar>> TCharTraits = std::char_traits<TChar>, std::derived_from<std::allocator<TChar>> TAlloc = std::allocator<TChar>>
	struct stringifier {
		using this_t = stringifier<TChar, TCharTraits, TAlloc>;
		using stream_t = std::basic_stringstream<TChar, TCharTraits, TAlloc>;

		stream_t ss;

		constexpr stringifier() : ss{} {}

		constexpr std::string str() const { return ss.str(); }

		constexpr void setf(const std::ios_base::fmtflags fmtFlags)
		{
			ss.setf(fmtFlags);
		}
		constexpr void setf(const std::ios_base::fmtflags fmtFlags, const std::ios_base::fmtflags mask)
		{
			ss.setf(fmtFlags, mask);
		}
		constexpr void unsetf(const std::ios_base::fmtflags mask)
		{
			ss.unsetf(mask);
		}

		constexpr size_t size() const
		{
		#ifdef OS_WIN
			return ss.view().size();
		#else
			return ss.str().size();
		#endif
		}

		constexpr auto rdbuf() const { return ss.rdbuf(); }

		template<var::streamable<std::basic_stringstream<TChar, TCharTraits, TAlloc>>... Ts>
		constexpr this_t& operator<<(Ts&&... args)
		{
			(ss << ... << std::forward<Ts>(args));
			return *this;
		}
		};
	}
