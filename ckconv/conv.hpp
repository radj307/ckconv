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

	/// @brief	Changes the *first occurrence* of the word 'metre' in the given string to 'meter'. This function ignores case.
	inline std::string ChangeMetreToMeter(std::string s)
	{
		static const std::string METRE{ "metre" };
		const auto& sLower{ str::tolower(s) };
		if (const auto& beg{ sLower.find(METRE) }, end{ (beg + METRE.size()) }; beg < sLower.size() && end < sLower.size())
			s.replace(s.begin() + beg, s.begin() + end, "meter");
		return s;
	}
	/// @brief	Changes the *first occurrence* of the word 'meter' in the given string to 'metre'. This function ignores case.
	inline std::string ChangeMeterToMetre(std::string s)
	{
		static const std::string METER{ "metre" };
		const auto& sLower{ str::tolower(s) };
		if (const auto& beg{ sLower.find(METER) }, end{ (beg + METER.size()) }; beg < sLower.size() && end < sLower.size())
			s.replace(s.begin() + beg, s.begin() + end, "metre");
		return s;
	}

	/// @brief	Type used for numbers.
	using number_t = long double;

	/**
	 * @struct	Unit
	 * @brief	Represents a length measurement unit. *(Does not contain a value.)*
	 */
	class Unit {
		SystemID _system;
		number_t unitcf;

		std::string symbol;
		std::string fullName;
		std::string fullNamePluralExt;
		bool pluralIsOverrideNotExt{ false };

		std::vector<std::string> extraNames;

	public:
		template<var::same_or_convertible<std::string>... TExtraNames>
		CONSTEXPR Unit(SystemID const& systemID, number_t const& conversionFactor, std::string const& symbol, std::string const& fullName = {}, std::string const& fullNamePluralExtension = "s", TExtraNames&&... extraNames)
			: _system{ systemID }, unitcf{ conversionFactor }, symbol{ symbol }, fullName{ fullName }, fullNamePluralExt{ fullNamePluralExtension }, extraNames{ (std::string{ std::forward<TExtraNames>(extraNames) })... } {}

		template<var::same_or_convertible<std::string>... TExtraNames>
		CONSTEXPR Unit(SystemID const& systemID, number_t const& conversionFactor, std::string const& symbol, std::string const& fullName, std::string const& fullNamePluralExtension, const bool pluralFormIsOverrideNotExtension, TExtraNames&&... extraNames)
			: _system{ systemID }, unitcf{ conversionFactor }, symbol{ symbol }, fullName{ fullName }, fullNamePluralExt{ fullNamePluralExtension }, pluralIsOverrideNotExt{ pluralFormIsOverrideNotExtension }, extraNames{ (std::string{ std::forward<TExtraNames>(extraNames) })... } {}

		CONSTEXPR number_t GetConversionFactor() const noexcept { return unitcf; }
		CONSTEXPR operator number_t() const noexcept { return this->GetConversionFactor(); }

		CONSTEXPR SystemID GetSystemID() const noexcept { return _system; }
		CONSTEXPR explicit operator SystemID() const noexcept { return this->GetSystemID(); }

		CONSTEXPR bool HasUniquePlural() const noexcept { return pluralIsOverrideNotExt; }

		WINCONSTEXPR bool HasSymbol() const noexcept { return !symbol.empty(); }
		WINCONSTEXPR std::string GetSymbol() const noexcept { return symbol; }

		WINCONSTEXPR bool HasFullName() const noexcept { return !fullName.empty(); }
		WINCONSTEXPR std::string GetFullName(const bool plural = true) const noexcept { return (plural ? (pluralIsOverrideNotExt ? fullNamePluralExt : fullName + fullNamePluralExt) : fullName); }

		WINCONSTEXPR bool HasExtraNames() const noexcept { return !extraNames.empty(); }
		WINCONSTEXPR std::vector<std::string> GetExtraNames() const noexcept { return extraNames; }

		WINCONSTEXPR std::string GetPrintableName(const bool preferFullName, const bool plural = true) const noexcept
		{
			return (preferFullName
					? (HasFullName() ? GetFullName(plural) : GetSymbol())
					: (!HasSymbol() ? GetFullName(plural) : GetSymbol()));
		}

		CONSTEXPR number_t ConvertToBase(number_t const& value) const noexcept { return value * unitcf; }
	};

	/**
	 * @enum	Powers
	 * @brief	Defines all SI prefixes and their base-10 exponent.
	 *\n		See https://en.wikipedia.org/wiki/Metric_prefix#List_of_SI_prefixes
	 */
	enum class SIPrefix : int8_t {
		YOCTO = -24,
		ZEPTO = -21,
		ATTO = -18,
		FEMTO = -15,
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
		PETA = 15,
		EXA = 18,
		ZETTA = 21,
		YOTTA = 24,
	};

	struct System {
		using unit_vec_t = std::vector<Unit>;
		using unit_iterator = typename unit_vec_t::iterator;
		using unit_const_iterator = typename unit_vec_t::const_iterator;

		const char* const name;
		const std::vector<Unit> units;
		const Unit* base{ nullptr };

		template<var::same_or_convertible<Unit>... TUnits>
		CONSTEXPR System(const char* const name, TUnits&&... units) : name{ name }, units{ (Unit{ std::forward<TUnits>(units) })... } {}

		virtual bool compare_unit_symbol(std::string const& s, std::string const& symbol) const noexcept
		{
			return s == symbol;
		}
		virtual bool compare_unit_name(std::string const& s, std::string const& name) const noexcept
		{
			std::string sLower{ str::tolower(s) };
			const std::string nameLower{ str::tolower(name) };

			if (sLower == nameLower) return true;

			if (sLower.ends_with('s')) // remove plurals from names
				sLower.erase(sLower.end() - 1, sLower.end());

			return sLower == nameLower;
		}
		virtual bool compare_unit_extraNames(std::string const& s, std::vector<std::string> const& extraNames) const noexcept
		{
			for (const auto& name : extraNames)
				if (this->compare_unit_name(s, name))
					return true;
			return false;
		}

		virtual unit_const_iterator find(std::string const& s) const noexcept
		{
			for (auto it{ units.begin() }, end{ units.end() }; it != end; ++it) {
				if (it->HasUniquePlural() && it->HasFullName() && (this->compare_unit_name(s, it->GetFullName(false)) || this->compare_unit_name(s, it->GetFullName(true))))
					return it;
				if ((it->HasSymbol() && this->compare_unit_symbol(s, it->GetSymbol())) || (it->HasFullName() && this->compare_unit_name(s, it->GetFullName())) || (it->HasExtraNames() && this->compare_unit_extraNames(s, it->GetExtraNames())))
					return it;
			}
			return units.end();
		}

		WINCONSTEXPR auto begin() const noexcept { return units.begin(); }
		WINCONSTEXPR auto end() const noexcept { return units.end(); }

	protected:
		System& SetBaseUnit(const Unit* base)
		{
			this->base = base;
			return *this;
		}
	};

	/**
	 * @struct	Metric
	 * @brief	Intra-Metric-System Conversion Factors. (Relative to Meters)
	 */
	struct MetricSystem : public System { // SystemID::METRIC
		MetricSystem() : System("Metric",
								Unit{ SystemID::METRIC, std::pow(10.0L, static_cast<number_t>(SIPrefix::YOCTO)), "ym", "Yoctometer" },
								Unit{ SystemID::METRIC, std::pow(10.0L, static_cast<number_t>(SIPrefix::ZEPTO)), "zm", "Zeptometer" },
								Unit{ SystemID::METRIC, std::pow(10.0L, static_cast<number_t>(SIPrefix::ATTO)), "am", "Attometer" },
								Unit{ SystemID::METRIC, std::pow(10.0L, static_cast<number_t>(SIPrefix::FEMTO)), "fm", "Femtometer" },
								Unit{ SystemID::METRIC, std::pow(10.0L, static_cast<number_t>(SIPrefix::PICO)), "pm", "Picometer" },
								Unit{ SystemID::METRIC, std::pow(10.0L, static_cast<number_t>(SIPrefix::NANO)), "nm", "Nanometer" },
								Unit{ SystemID::METRIC, std::pow(10.0L, static_cast<number_t>(SIPrefix::MICRO)), "um", "Micrometer" },
								Unit{ SystemID::METRIC, std::pow(10.0L, static_cast<number_t>(SIPrefix::MILLI)), "mm", "Millimeter" },
								Unit{ SystemID::METRIC, std::pow(10.0L, static_cast<number_t>(SIPrefix::CENTI)), "cm", "Centimeter" },
								Unit{ SystemID::METRIC, std::pow(10.0L, static_cast<number_t>(SIPrefix::DECI)), "dm", "Decimeter" },
								Unit{ SystemID::METRIC, std::pow(10.0L, static_cast<number_t>(SIPrefix::BASE)), "m", "Meter" },
								Unit{ SystemID::METRIC, std::pow(10.0L, static_cast<number_t>(SIPrefix::DECA)), "dam", "Decameter" },
								Unit{ SystemID::METRIC, std::pow(10.0L, static_cast<number_t>(SIPrefix::HECTO)), "hm", "Hectometer" },
								Unit{ SystemID::METRIC, std::pow(10.0L, static_cast<number_t>(SIPrefix::KILO)), "km", "Kilometer" },
								Unit{ SystemID::METRIC, std::pow(10.0L, static_cast<number_t>(SIPrefix::MEGA)), "Mm", "Megameter" },
								Unit{ SystemID::METRIC, std::pow(10.0L, static_cast<number_t>(SIPrefix::GIGA)), "Gm", "Gigameter" },
								Unit{ SystemID::METRIC, std::pow(10.0L, static_cast<number_t>(SIPrefix::TERA)), "Tm", "Terameter" },
								Unit{ SystemID::METRIC, std::pow(10.0L, static_cast<number_t>(SIPrefix::PETA)), "Pm", "Petameter" },
								Unit{ SystemID::METRIC, std::pow(10.0L, static_cast<number_t>(SIPrefix::EXA)), "Em", "Exameter" },
								Unit{ SystemID::METRIC, std::pow(10.0L, static_cast<number_t>(SIPrefix::ZETTA)), "Zm", "Zettameter" },
								Unit{ SystemID::METRIC, std::pow(10.0L, static_cast<number_t>(SIPrefix::YOTTA)), "Ym", "Yottameter" })
		{
			SetBaseUnit(&units.at(10));
		}

		const Unit* YOCTOMETER{ &units[0] };
		const Unit* ZEPTOMETER{ &units[1] };
		const Unit* ATTOMETER{ &units[2] };
		const Unit* FEMTOMETER{ &units[3] };
		const Unit* PICOMETER{ &units[4] };
		const Unit* NANOMETER{ &units[5] };
		const Unit* MICROMETER{ &units[6] };
		const Unit* MILLIMETER{ &units[7] };
		const Unit* CENTIMETER{ &units[8] };
		const Unit* DECIMETER{ &units[9] };
		const Unit* METER{ &units[10] };
		const Unit* DECAMETER{ &units[11] };
		const Unit* HECTOMETER{ &units[12] };
		const Unit* KILOMETER{ &units[13] };
		const Unit* MEGAMETER{ &units[14] };
		const Unit* GIGAMETER{ &units[15] };
		const Unit* TERAMETER{ &units[16] };
		const Unit* PETAMETER{ &units[17] };
		const Unit* EXAMETER{ &units[18] };
		const Unit* ZETTAMETER{ &units[19] };
		const Unit* YOTTAMETER{ &units[20] };

		// the base unit of the Metric system (meters)
		const Unit* const base{ METER };
	} Metric;

	/**
	 * @struct	CreationKit
	 * @brief	Intra-CreationKit-System Conversion Factors. (Relative to Units)
	 */
	struct CreationKitSystem : public System { // SystemID::CREATIONKIT
		CreationKitSystem() : System("Creation Kit",
									 Unit{ SystemID::CREATIONKIT, std::pow(10.0L, static_cast<number_t>(SIPrefix::YOCTO)), "yu", "Yoctounit" },
									 Unit{ SystemID::CREATIONKIT, std::pow(10.0L, static_cast<number_t>(SIPrefix::ZEPTO)), "zu", "Zeptounit" },
									 Unit{ SystemID::CREATIONKIT, std::pow(10.0L, static_cast<number_t>(SIPrefix::ATTO)), "au", "Attounit" },
									 Unit{ SystemID::CREATIONKIT, std::pow(10.0L, static_cast<number_t>(SIPrefix::FEMTO)), "fu", "Femtounit" },
									 Unit{ SystemID::CREATIONKIT, std::pow(10.0L, static_cast<number_t>(SIPrefix::PICO)), "pu", "Picounit" },
									 Unit{ SystemID::CREATIONKIT, std::pow(10.0L, static_cast<number_t>(SIPrefix::NANO)), "nu", "Nanounit" },
									 Unit{ SystemID::CREATIONKIT, std::pow(10.0L, static_cast<number_t>(SIPrefix::MICRO)), "uu", "Microunit" },
									 Unit{ SystemID::CREATIONKIT, std::pow(10.0L, static_cast<number_t>(SIPrefix::MILLI)), "mu", "Milliunit" },
									 Unit{ SystemID::CREATIONKIT, std::pow(10.0L, static_cast<number_t>(SIPrefix::CENTI)), "cu", "Centiunit" },
									 Unit{ SystemID::CREATIONKIT, std::pow(10.0L, static_cast<number_t>(SIPrefix::DECI)), "du", "Deciunit" },
									 Unit{ SystemID::CREATIONKIT, std::pow(10.0L, static_cast<number_t>(SIPrefix::BASE)), "u", "Unit" },
									 Unit{ SystemID::CREATIONKIT, std::pow(10.0L, static_cast<number_t>(SIPrefix::DECA)), "dau", "Decaunit" },
									 Unit{ SystemID::CREATIONKIT, std::pow(10.0L, static_cast<number_t>(SIPrefix::HECTO)), "hu", "Hectounit" },
									 Unit{ SystemID::CREATIONKIT, std::pow(10.0L, static_cast<number_t>(SIPrefix::KILO)), "ku", "Kilometer" },
									 Unit{ SystemID::CREATIONKIT, std::pow(10.0L, static_cast<number_t>(SIPrefix::MEGA)), "Mu", "Megaunit" },
									 Unit{ SystemID::CREATIONKIT, std::pow(10.0L, static_cast<number_t>(SIPrefix::GIGA)), "Gu", "Gigaunit" },
									 Unit{ SystemID::CREATIONKIT, std::pow(10.0L, static_cast<number_t>(SIPrefix::TERA)), "Tu", "Teraunit" },
									 Unit{ SystemID::CREATIONKIT, std::pow(10.0L, static_cast<number_t>(SIPrefix::PETA)), "Pu", "Petaunit" },
									 Unit{ SystemID::CREATIONKIT, std::pow(10.0L, static_cast<number_t>(SIPrefix::EXA)), "Eu", "Exaunit" },
									 Unit{ SystemID::CREATIONKIT, std::pow(10.0L, static_cast<number_t>(SIPrefix::ZETTA)), "Zu", "Zettaunit" },
									 Unit{ SystemID::CREATIONKIT, std::pow(10.0L, static_cast<number_t>(SIPrefix::YOTTA)), "Yu", "Yottaunit" })
		{
			SetBaseUnit(&units.at(10));
		}

		const Unit* YOCTOUNIT{ &units[0] };
		const Unit* ZEPTOUNIT{ &units[1] };
		const Unit* ATTOUNIT{ &units[2] };
		const Unit* FEMTOUNIT{ &units[3] };
		const Unit* PICOUNIT{ &units[4] };
		const Unit* NANOUNIT{ &units[5] };
		const Unit* MICROUNIT{ &units[6] };
		const Unit* MILLIUNIT{ &units[7] };
		const Unit* CENTIUNIT{ &units[8] };
		const Unit* DECIUNIT{ &units[9] };
		const Unit* UNIT{ &units[10] };
		const Unit* DECAUNIT{ &units[11] };
		const Unit* HECTOUNIT{ &units[12] };
		const Unit* KILOUNIT{ &units[13] };
		const Unit* MEGAUNIT{ &units[14] };
		const Unit* GIGAUNIT{ &units[15] };
		const Unit* TERAUNIT{ &units[16] };
		const Unit* PETAUNIT{ &units[17] };
		const Unit* EXAUNIT{ &units[18] };
		const Unit* ZETTAUNIT{ &units[19] };
		const Unit* YOTTAUNIT{ &units[20] };

		// the base unit of this system
		const Unit* const base{ UNIT };
	} CreationKit;

	/**
	 * @struct	Imperial
	 * @brief	Intra-Imperial-System Conversion Factors. (Relative to Feet)
	 */
	struct ImperialSystem : public System { // SystemID::IMPERIAL
		ImperialSystem() : System("Imperial",
								  Unit{ SystemID::IMPERIAL, (1.0L / 17280.0L), "", "Twip" },
								  Unit{ SystemID::IMPERIAL, (1.0L / 12000.0L), "th", "Thou" },
								  Unit{ SystemID::IMPERIAL, (1.0L / 36.0L), "Bc", "Barleycorn" },
								  Unit{ SystemID::IMPERIAL, (1.0L / 12.0L), "\"", "Inch", "es", "in" },
								  Unit{ SystemID::IMPERIAL, (1.0L / 3.0L), "h", "Hand" },
								  Unit{ SystemID::IMPERIAL, (1.0L), "\'", "Foot", "Feet", true, "ft" },
								  Unit{ SystemID::IMPERIAL, (3.0L), "yd", "Yard", "s" },
								  Unit{ SystemID::IMPERIAL, (66.0L), "ch", "Chain" },
								  Unit{ SystemID::IMPERIAL, (660.0L), "fur", "Furlong" },
								  Unit{ SystemID::IMPERIAL, (5280.0L), "mi", "Mile" },
								  Unit{ SystemID::IMPERIAL, (15840.0L), "lea", "League" },
								  Unit{ SystemID::IMPERIAL, (6.0761L), "ftm", "Fathom" },
								  Unit{ SystemID::IMPERIAL, (607.61L), "Cable" },
								  Unit{ SystemID::IMPERIAL, (6076.1L), "nmi", "NauticalMile", "nautical mile", "nmile" },
								  Unit{ SystemID::IMPERIAL, (66.0L / 100.0L), "Link" },
								  Unit{ SystemID::IMPERIAL, (66.0L / 4.0L), "rd", "Rod" })
		{
			SetBaseUnit(&units.at(5));
		}

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
	const constexpr auto ONE_UNIT_IN_METERS{ 0.0142875313L };
	/// @brief	Inter-System (CKUnit:Imperial) Conversion Factor
	const constexpr auto ONE_UNIT_IN_FEET{ 0.046875L };

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
		if (in.GetConversionFactor() == 0.0L)
			throw make_exception("Illegal input conversion factor '", in.GetConversionFactor(), "'");
		if (out.GetConversionFactor() == 0.0L)
			throw make_exception("Illegal output conversion factor '", out.GetConversionFactor(), "'");

		if (in.GetSystemID() == out.GetSystemID()) // convert between units only
			return convert_unit(in.GetConversionFactor(), val, out.GetConversionFactor());
		// Convert between systems & units
		return convert_system(in.GetSystemID(), in.ConvertToBase(static_cast<long double>(val)), out.GetSystemID()) / out.GetConversionFactor();
	}

	$DefineExcept(invalid_unit_exception);

	/**
	 * @brief		Retrieve the unit specified by a string containing the unit's official symbol, or name.
	 * @param str	Input String. Must match at least one symbol exactly, or any name using case-insensitive comparison.
	 * @param def	Optional default return value if the string is invalid.
	 * @returns		Unit
	 */
	inline Unit getUnit(std::string const& s, std::optional<Unit> const& def = std::nullopt)
	{
		if (const auto& it{ Imperial.find(s) }; it != Imperial.end())
			return *it;
		if (const auto& it{ Metric.find(ChangeMetreToMeter(s)) }; it != Metric.end())
			return *it;
		if (const auto& it{ CreationKit.find(s) }; it != CreationKit.end())
			return *it;

		if (def.has_value())
			return def.value();
		throw ex::make_custom_exception<invalid_unit_exception>("Couldn't find any measurement units matching '", s, "'");
	}
	//inline Unit getUnit(const std::string& str, const std::optional<Unit>& def = std::nullopt)
	//{
	//	if (str.empty()) {
	//		if (def.has_value())
	//			return def.value();
	//		throw make_exception("No unit specified ; string was empty!");
	//	}
	//	const auto s{ str::tolower(str) };
	//
	//	//#define DISABLE_NUTJOB_UNITS
	//
	//	// BEGIN IMPERIAL //
	//#ifndef DISABLE_NUTJOB_UNITS
	//	if (s.find("twip") < s.size())
	//		return *Imperial.TWIP;
	//	if (str == "th" || s.find("thou") < s.size())
	//		return *Imperial.THOU;
	//	if (str == "Bc" || s.find("barleycorn") < s.size())
	//		return *Imperial.BARLEYCORN;
	//	if (str == "h" || s.find("hand") < s.size())
	//		return *Imperial.HAND;
	//	if (str == "ch" || s.find("chain") < s.size())
	//		return *Imperial.CHAIN;
	//	if (str == "fur" || s.find("furlong") < s.size())
	//		return *Imperial.FURLONG;
	//	if (str == "lea" || s.find("league") < s.size())
	//		return *Imperial.LEAGUE;
	//	if (str == "ftm" || s.find("fathom") < s.size())
	//		return *Imperial.FATHOM;
	//	if (s.find("cable") < s.size())
	//		return *Imperial.CABLE;
	//	if (s.find("link") < s.size())
	//		return *Imperial.LINK;
	//	if (str == "rd" || s.find("rod") < s.size())
	//		return *Imperial.ROD;
	//#endif // DISABLE_NUTJOB_UNITS
	//	if (str == "\"" || str == "in" || s == "i" || s.find("inch") < s.size())
	//		return *Imperial.INCH;
	//	if (str == "\'" || str == "ft" || s == "f" || s.find("foot") < s.size() || s.find("feet") < s.size())
	//		return *Imperial.FOOT;
	//	if (str == "yd" || s.find("yard") < s.size())
	//		return *Imperial.YARD;
	//	if (str == "nmi" || s.find("nauticalmile") < s.size() || s.find("nmile") < s.size())
	//		return *Imperial.CABLE;
	//	if (str == "mi" || s.find("mile") < s.size())
	//		return *Imperial.MILE;
	//	// END IMPERIAL //
	//
	//	// BEGIN METRIC //
	//	// comparisons omit -er|-re to allow both the American and British spelling of "meter|metre".
	//	if (str == "pm" || s.find("picomet") < s.size())
	//		return *Metric.PICOMETER;
	//	if (str == "nm" || s.find("nanomet") < s.size())
	//		return *Metric.NANOMETER;
	//	if (str == "um" || s.find("micromet") < s.size())
	//		return *Metric.MICROMETER;
	//	if (str == "mm" || s.find("millimet") < s.size())
	//		return *Metric.MILLIMETER;
	//	if (str == "cm" || s.find("centimet") < s.size())
	//		return *Metric.CENTIMETER;
	//	if (str == "dm" || s.find("decimet") < s.size())
	//		return *Metric.DECIMETER;
	//	if (str == "dam" || s.find("decamet") < s.size())
	//		return *Metric.DECAMETER;
	//	if (str == "hm" || s.find("hectomet") < s.size())
	//		return *Metric.HECTOMETER;
	//	if (str == "km" || s.find("kilomet") < s.size())
	//		return *Metric.KILOMETER;
	//	if (str == "Mm" || s.find("megamet") < s.size())
	//		return *Metric.MEGAMETER;
	//	if (str == "Gm" || s.find("gigamet") < s.size())
	//		return *Metric.GIGAMETER;
	//	if (str == "Tm" || s.find("teramet") < s.size())
	//		return *Metric.TERAMETER;
	//	// this has to be checked after all prefix types
	//	if (str == "m" || s.find("met") < s.size())
	//		return *Metric.METER;
	//	// END METRIC //
	//
	//	// BEGIN CREATIONKIT //
	//	if (str == "pu" || s.find("picounit") < s.size())
	//		return *CreationKit.PICOUNIT;
	//	if (str == "nu" || s.find("nanounit") < s.size())
	//		return *CreationKit.NANOUNIT;
	//	if (str == "uu" || s.find("microunit") < s.size())
	//		return *CreationKit.MICROUNIT;
	//	if (str == "mu" || s.find("milliunit") < s.size())
	//		return *CreationKit.MILLIUNIT;
	//	if (str == "cu" || s.find("centiunit") < s.size())
	//		return *CreationKit.CENTIUNIT;
	//	if (str == "du" || s.find("deciunit") < s.size())
	//		return *CreationKit.DECIUNIT;
	//	if (str == "dau" || s.find("decaunit") < s.size())
	//		return *CreationKit.DECAUNIT;
	//	if (str == "hu" || s.find("hectounit") < s.size())
	//		return *CreationKit.HECTOUNIT;
	//	if (str == "ku" || s.find("kilounit") < s.size())
	//		return *CreationKit.KILOUNIT;
	//	if (str == "Mu" || s.find("megaunit") < s.size())
	//		return *CreationKit.MEGAUNIT;
	//	if (str == "Gu" || s.find("gigaunit") < s.size())
	//		return *CreationKit.GIGAUNIT;
	//	if (str == "Tu" || s.find("teraunit") < s.size())
	//		return *CreationKit.TERAUNIT;
	//	// this has to be checked after all prefix types
	//	if (str == "u" || s.find("unit") < s.size())
	//		return *CreationKit.UNIT;
	//	// END CREATIONKIT //
	//
	//	if (def.has_value())
	//		return def.value();
	//
	//	throw make_exception("Unrecognized Unit: \"", str, '\"');
	//}
}
