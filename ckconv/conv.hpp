#pragma once
/**
 * @file	conv.hpp
 * @author	radj307
 * @brief	Contains real-world measurement unit converters, currently supporting metric & imperial.
 * @details	This header was pulled from my other project, conv-utils/conv2 (https://github.com/radj307/conv-utils)-
 *			-and modified to include conversions for Creation Kit 'Units'
 */
#include <sysarch.h>
#include <make_exception.hpp>
#include <str.hpp>
#include <var.hpp>
#include <math.hpp>

#include <optional>
#include <iterator>
#include <algorithm>

namespace conv {
	/**
	 * @enum	SystemID
	 * @brief	Accepted Measurement Systems
	 */
	enum class SystemID : char {
		/// @brief	The Metric System
		METRIC,
		/// @brief	The Imperial System
		IMPERIAL,
		/// @brief	Bethesda's Creation Kit Measurement System
		CREATIONKIT,
		/// @brief	Represents all SystemIDs
		ALL,
	};

	class Unit {
		SystemID _system;
		std::string _sym, _name;

	public:
		long double unitcf; // unit conversion factor

		WINCONSTEXPR Unit(SystemID const& system, long double const& unit_conversion_factor, std::string const& symbol, std::string_view const& full_name = {}) : _system{ system }, unitcf{ unit_conversion_factor }, _sym{ symbol }, _name{ full_name } {}

		/// @brief	Retrieve the given value in it's base form.
		CONSTEXPR long double to_base(const long double& val) const { return val * unitcf; }
		CONSTEXPR SystemID getSystem() const noexcept { return _system; }
		WINCONSTEXPR std::string getName() const noexcept { return (_name.empty() ? _sym : _name); }
		WINCONSTEXPR std::string getSymbol() const noexcept { return _sym; }

		WINCONSTEXPR bool hasName() const noexcept { return !_name.empty(); }

		CONSTEXPR bool operator==(const Unit& o) const { return _system == o._system && unitcf == o.unitcf; }
	};

	enum class Powers : char {
		PICO = -12,
		NANO = -9,
		MICRO = -6,
		MILLI = -3,
		CENTI = -2,
		DECI = -1,
		BASE = 0,
		DECA = 1,
		HECTO = 2,
		KILO = 3,
		MEGA = 6,
		GIGA = 9,
		TERA = 12,
	};

	struct System {
		using T = long double;
	};

	/**
	 * @struct	Metric
	 * @brief	Intra-Metric-System Conversion Factors. (Relative to Meters)
	 */
	static struct : public System { // SystemID::METRIC
		const std::vector<Unit> units{
			{ SystemID::METRIC, std::pow(10.0L, static_cast<T>(Powers::PICO)), "pm", "Picometer" },
			{ SystemID::METRIC, std::pow(10.0L, static_cast<T>(Powers::NANO)), "nm", "Nanometer" },
			{ SystemID::METRIC, std::pow(10.0L, static_cast<T>(Powers::MICRO)), "um", "Micrometer" },
			{ SystemID::METRIC, std::pow(10.0L, static_cast<T>(Powers::MILLI)), "mm", "Millimeter" },
			{ SystemID::METRIC, std::pow(10.0L, static_cast<T>(Powers::CENTI)), "cm", "Centimeter" },
			{ SystemID::METRIC, std::pow(10.0L, static_cast<T>(Powers::DECI)), "dm", "Decimeter" },
			{ SystemID::METRIC, std::pow(10.0L, static_cast<T>(Powers::BASE)), "m", "Meter" },
			{ SystemID::METRIC, std::pow(10.0L, static_cast<T>(Powers::DECA)), "dam", "Decameter" },
			{ SystemID::METRIC, std::pow(10.0L, static_cast<T>(Powers::HECTO)), "hm", "Hectometer" },
			{ SystemID::METRIC, std::pow(10.0L, static_cast<T>(Powers::KILO)), "km", "Kilometer" },
			{ SystemID::METRIC, std::pow(10.0L, static_cast<T>(Powers::MEGA)), "Mm", "Megameter" },
			{ SystemID::METRIC, std::pow(10.0L, static_cast<T>(Powers::GIGA)), "Gm", "Gigameter" },
			{ SystemID::METRIC, std::pow(10.0L, static_cast<T>(Powers::TERA)), "Tm", "Terameter" }
		};

		const Unit* PICOMETER{ &units[0] };
		const Unit* NANOMETER{ &units[1] };
		const Unit* MICROMETER{ &units[2] };
		const Unit* MILLIMETER{ &units[3] };
		const Unit* CENTIMETER{ &units[4] };
		const Unit* DECIMETER{ &units[5] };
		const Unit* METER{ &units[6] };
		const Unit* DECAMETER{ &units[7] };
		const Unit* HECTOMETER{ &units[8] };
		const Unit* KILOMETER{ &units[9] };
		const Unit* MEGAMETER{ &units[10] };
		const Unit* GIGAMETER{ &units[11] };
		const Unit* TERAMETER{ &units[12] };

		// the base unit of the Metric system (meters)
		const Unit* const base{ METER };
	} Metric;

	/**
	 * @struct	CreationKit
	 * @brief	Intra-CreationKit-System Conversion Factors. (Relative to Units)
	 */
	static struct : public System { // SystemID::CREATIONKIT
		const std::vector<Unit> units{
			{ SystemID::CREATIONKIT, std::pow(10.0L, static_cast<T>(Powers::PICO)), "pu", "Picounit" },
			{ SystemID::CREATIONKIT, std::pow(10.0L, static_cast<T>(Powers::NANO)), "nu", "Nanounit" },
			{ SystemID::CREATIONKIT, std::pow(10.0L, static_cast<T>(Powers::MICRO)), "uu", "Microunit" },
			{ SystemID::CREATIONKIT, std::pow(10.0L, static_cast<T>(Powers::MILLI)), "mu", "Milliunit" },
			{ SystemID::CREATIONKIT, std::pow(10.0L, static_cast<T>(Powers::CENTI)), "cu", "Centiunit" },
			{ SystemID::CREATIONKIT, std::pow(10.0L, static_cast<T>(Powers::DECI)), "du", "Deciunit" },
			{ SystemID::CREATIONKIT, std::pow(10.0L, static_cast<T>(Powers::BASE)), "u", "Unit" },
			{ SystemID::CREATIONKIT, std::pow(10.0L, static_cast<T>(Powers::DECA)), "dau", "Decaunit" },
			{ SystemID::CREATIONKIT, std::pow(10.0L, static_cast<T>(Powers::HECTO)), "hu", "Hectounit" },
			{ SystemID::CREATIONKIT, std::pow(10.0L, static_cast<T>(Powers::KILO)), "ku", "Kilometer" },
			{ SystemID::CREATIONKIT, std::pow(10.0L, static_cast<T>(Powers::MEGA)), "Mu", "Megaunit" },
			{ SystemID::CREATIONKIT, std::pow(10.0L, static_cast<T>(Powers::GIGA)), "Gu", "Gigaunit" },
			{ SystemID::CREATIONKIT, std::pow(10.0L, static_cast<T>(Powers::TERA)), "Tu", "Teraunit" }
		};

		const Unit* PICOUNIT{ &units[0] };
		const Unit* NANOUNIT{ &units[1] };
		const Unit* MICROUNIT{ &units[2] };
		const Unit* MILLIUNIT{ &units[3] };
		const Unit* CENTIUNIT{ &units[4] };
		const Unit* DECIUNIT{ &units[5] };
		const Unit* UNIT{ &units[6] };
		const Unit* DECAUNIT{ &units[7] };
		const Unit* HECTOUNIT{ &units[8] };
		const Unit* KILOUNIT{ &units[9] };
		const Unit* MEGAUNIT{ &units[10] };
		const Unit* GIGAUNIT{ &units[11] };
		const Unit* TERAUNIT{ &units[12] };

		// the base unit of this system
		const Unit* const base{ UNIT };
	} CreationKit;

	/**
	 * @struct	Imperial
	 * @brief	Intra-Imperial-System Conversion Factors. (Relative to Feet)
	 */
	static struct : public System { // SystemID::IMPERIAL
		const std::vector<Unit> units{
			{ SystemID::IMPERIAL, (1.0L / 17280.0L), "Twip" },
			{ SystemID::IMPERIAL, (1.0L / 12000.0L), "th", "Thou" },
			{ SystemID::IMPERIAL, (1.0L / 36.0L), "Bc", "Barleycorn" },
			{ SystemID::IMPERIAL, (1.0L / 12.0L), "\"", "Inch" },
			{ SystemID::IMPERIAL, (1.0L / 3.0L), "h", "Hand" },
			{ SystemID::IMPERIAL, (1.0L), "\'", "Feet" },
			{ SystemID::IMPERIAL, (3.0L), "yd", "Yard" },
			{ SystemID::IMPERIAL, (66.0L), "ch", "Chain" },
			{ SystemID::IMPERIAL, (660.0L), "fur", "Furlong" },
			{ SystemID::IMPERIAL, (5280.0L), "mi", "Mile" },
			{ SystemID::IMPERIAL, (15840.0L), "lea", "League" },
			{ SystemID::IMPERIAL, (6.0761L), "ftm", "Fathom" },
			{ SystemID::IMPERIAL, (607.61L), "Cable" },
			{ SystemID::IMPERIAL, (6076.1L), "nmi", "Nautical Mile" },
			{ SystemID::IMPERIAL, (66.0L / 100.0L), "Link" },
			{ SystemID::IMPERIAL, (66.0L / 4.0L), "rd", "Rod" }
		};

		const Unit* TWIP{ &units[0] };
		const Unit* THOU{ &units[1] };
		const Unit* BARLEYCORN{ &units[2] };
		const Unit* INCH{ &units[3] };
		const Unit* HAND{ &units[4] };
		const Unit* FOOT{ &units[5] };
		const Unit* YARD{ &units[6] };
		const Unit* CHAIN{ &units[7] };
		const Unit* FURLONG{ &units[8] };
		const Unit* MILE{ &units[9] };
		const Unit* LEAGUE{ &units[10] };
		// maritime units
		const Unit* FATHOM{ &units[11] };
		const Unit* CABLE{ &units[12] };
		const Unit* NAUTICAL_MILE{ &units[13] };
		// 17th century onwards
		const Unit* LINK{ &units[14] };
		const Unit* ROD{ &units[15] };
		// the base unit of this system
		const Unit* const base{ FOOT };
	} Imperial;

	/// @brief	Inter-System (Metric:Imperial) Conversion Factor
	const constexpr auto ONE_FOOT_IN_METERS{ 0.3048L };
	/// @brief	Inter-System (CKUnit:Metric) Conversion Factor
	const constexpr auto ONE_UNIT_IN_METERS{ 0.0142875313L }, ONE_UNIT_IN_FEET{ 0.046875L };

	/**
	 * @brief			Converts between units in one measurement system.
	 * @param in_unit	Input Conversion Factor
	 * @param v			Input Value
	 * @param out_unit	Output Conversion Factor
	 * @returns			long double
	 */
	inline constexpr long double convert_unit(const long double& in_unitcf, const long double& v, const long double& out_unitcf)
	{
		if (math::equal(out_unitcf, 0.0L))
			throw make_exception("convert_unit() failed:  Cannot divide by zero!");
		return ((v * in_unitcf) / out_unitcf);
	}
	/**
	 * @brief				Convert between measurement systems.
	 * @param in_system		Input Measurement SystemID
	 * @param v_base		Input Value, in the input system's base unit. (Metric = Meters, Imperial = Feet)
	 * @param out_system	Output Measurement SystemID
	 * @returns				long double
	 */
	inline constexpr long double convert_system(const SystemID& in_system, const long double& v_base, const SystemID& out_system)
	{
		if (in_system == out_system) // same system
			return v_base;
		// convert system
		switch (in_system) {
		case SystemID::METRIC:
			switch (out_system) { // METRIC ->
			case SystemID::IMPERIAL:
				return v_base / ONE_FOOT_IN_METERS;
			case SystemID::CREATIONKIT:
				return v_base / ONE_UNIT_IN_METERS;
			}
			break;
		case SystemID::IMPERIAL:
			switch (out_system) { // IMPERIAL ->
			case SystemID::METRIC:
				return v_base * ONE_FOOT_IN_METERS;
			case SystemID::CREATIONKIT:
				return v_base / ONE_UNIT_IN_FEET;
			}
			break;
		case SystemID::CREATIONKIT:
			switch (out_system) { // CREATIONKIT ->
			case SystemID::METRIC:
				return v_base * ONE_UNIT_IN_METERS;
			case SystemID::IMPERIAL:
				return v_base * ONE_UNIT_IN_FEET;
			}
			break;
		default:break;
		}
		throw make_exception("convert_system() failed:  No handler exists for the given input type!");
	}

	/**
	 * @brief		Convert a number in a given unit to another unit and/or system.
	 * @param in	Input Unit.
	 * @param val	Input Value.
	 * @param out	Output Unit.
	 * @returns		long double
	 */
	inline static constexpr long double convert(const Unit& in, const long double& val, const Unit& out)
	{
		if (in.unitcf == 0.0L)
			throw make_exception("Illegal input conversion factor '", in.unitcf, "'");
		if (out.unitcf == 0.0L)
			throw make_exception("Illegal output conversion factor '", out.unitcf, "'");

		if (in.getSystem() == out.getSystem()) // convert between units only
			return convert_unit(in.unitcf, val, out.unitcf);
		// Convert between systems & units
		return convert_system(in.getSystem(), in.to_base(static_cast<long double>(val)), out.getSystem()) / out.unitcf;
	}

	/**
	 * @brief		Retrieve the unit specified by a string containing the unit's official symbol, or name.
	 * @param str	Input String. (This is not processed beyond case-conversion)
	 * @param def	Optional default return value if the string is invalid.
	 * @returns		Unit
	 */
	inline Unit getUnit(const std::string& str, const std::optional<Unit>& def = std::nullopt)
	{
		if (str.empty()) {
			if (def.has_value())
				return def.value();
			throw make_exception("No unit specified ; string was empty!");
		}
		const auto s{ str::tolower(str) };

		//#define DISABLE_NUTJOB_UNITS

		// BEGIN IMPERIAL //
		#ifndef DISABLE_NUTJOB_UNITS
		if (s.find("twip") < s.size())
			return *Imperial.TWIP;
		if (str == "th" || s.find("thou") < s.size())
			return *Imperial.THOU;
		if (str == "Bc" || s.find("barleycorn") < s.size())
			return *Imperial.BARLEYCORN;
		if (str == "h" || s.find("hand") < s.size())
			return *Imperial.HAND;
		if (str == "ch" || s.find("chain") < s.size())
			return *Imperial.CHAIN;
		if (str == "fur" || s.find("furlong") < s.size())
			return *Imperial.FURLONG;
		if (str == "lea" || s.find("league") < s.size())
			return *Imperial.LEAGUE;
		if (str == "ftm" || s.find("fathom") < s.size())
			return *Imperial.FATHOM;
		if (s.find("cable") < s.size())
			return *Imperial.CABLE;
		if (s.find("link") < s.size())
			return *Imperial.LINK;
		if (str == "rd" || s.find("rod") < s.size())
			return *Imperial.ROD;
		#endif // DISABLE_NUTJOB_UNITS
		if (str == "\"" || str == "in" || s == "i" || s.find("inch") < s.size())
			return *Imperial.INCH;
		if (str == "\'" || str == "ft" || s == "f" || s.find("foot") < s.size() || s.find("feet") < s.size())
			return *Imperial.FOOT;
		if (str == "yd" || s.find("yard") < s.size())
			return *Imperial.YARD;
		if (str == "nmi" || s.find("nauticalmile") < s.size() || s.find("nmile") < s.size())
			return *Imperial.CABLE;
		if (str == "mi" || s.find("mile") < s.size())
			return *Imperial.MILE;
		// END IMPERIAL //

		// BEGIN METRIC //
		// comparisons omit -er|-re to allow both the American and British spelling of "meter|metre".
		if (str == "pm" || s.find("picomet") < s.size())
			return *Metric.PICOMETER;
		if (str == "nm" || s.find("nanomet") < s.size())
			return *Metric.NANOMETER;
		if (str == "um" || s.find("micromet") < s.size())
			return *Metric.MICROMETER;
		if (str == "mm" || s.find("millimet") < s.size())
			return *Metric.MILLIMETER;
		if (str == "cm" || s.find("centimet") < s.size())
			return *Metric.CENTIMETER;
		if (str == "dm" || s.find("decimet") < s.size())
			return *Metric.DECIMETER;
		if (str == "dam" || s.find("decamet") < s.size())
			return *Metric.DECAMETER;
		if (str == "hm" || s.find("hectomet") < s.size())
			return *Metric.HECTOMETER;
		if (str == "km" || s.find("kilomet") < s.size())
			return *Metric.KILOMETER;
		if (str == "Mm" || s.find("megamet") < s.size())
			return *Metric.MEGAMETER;
		if (str == "Gm" || s.find("gigamet") < s.size())
			return *Metric.GIGAMETER;
		if (str == "Tm" || s.find("teramet") < s.size())
			return *Metric.TERAMETER;
		// this has to be checked after all prefix types
		if (str == "m" || s.find("met") < s.size())
			return *Metric.METER;
		// END METRIC //

		// BEGIN CREATIONKIT //
		if (str == "pu" || s.find("picounit") < s.size())
			return *CreationKit.PICOUNIT;
		if (str == "nu" || s.find("nanounit") < s.size())
			return *CreationKit.NANOUNIT;
		if (str == "uu" || s.find("microunit") < s.size())
			return *CreationKit.MICROUNIT;
		if (str == "mu" || s.find("milliunit") < s.size())
			return *CreationKit.MILLIUNIT;
		if (str == "cu" || s.find("centiunit") < s.size())
			return *CreationKit.CENTIUNIT;
		if (str == "du" || s.find("deciunit") < s.size())
			return *CreationKit.DECIUNIT;
		if (str == "dau" || s.find("decaunit") < s.size())
			return *CreationKit.DECAUNIT;
		if (str == "hu" || s.find("hectounit") < s.size())
			return *CreationKit.HECTOUNIT;
		if (str == "ku" || s.find("kilounit") < s.size())
			return *CreationKit.KILOUNIT;
		if (str == "Mu" || s.find("megaunit") < s.size())
			return *CreationKit.MEGAUNIT;
		if (str == "Gu" || s.find("gigaunit") < s.size())
			return *CreationKit.GIGAUNIT;
		if (str == "Tu" || s.find("teraunit") < s.size())
			return *CreationKit.TERAUNIT;
		// this has to be checked after all prefix types
		if (str == "u" || s.find("unit") < s.size())
			return *CreationKit.UNIT;
		// END CREATIONKIT //

		if (def.has_value())
			return def.value();

		throw make_exception("Unrecognized Unit: \"", str, '\"');
	}
}
