#pragma once
#include "version.h"

#include <sysarch.h>
#include <make_exception.hpp>
#include <TermAPI.hpp>
#include <palette.hpp>
#include <ParamsAPI2.hpp>
#include <INI.hpp>
#include <fileutil.hpp>

#include <variant>
#include <filesystem>
#include <cmath>

#ifdef OS_WIN
/// @brief	This is used to keep track of the number output notation.
using FmtFlag = std::_Iosb<int>::_Fmtflags;
#else
/// @brief	This is used to keep track of the number output notation.
using FmtFlag = std::ios_base::fmtflags;
#endif

struct argument_except : public ex::except { argument_except(auto&& message) : ex::except(std::forward<decltype(message)>(message)) {} };
template<typename... Ts>
inline argument_except argument_exception(const std::string& argument, const Ts&... reason)
{
	return ex::make_custom_exception<argument_except>(
		"Invalid Argument:    ", argument, '\n',
		"        Reason:      ", reason..., '\n'
	);
}
template<typename... Ts>
inline argument_except argument_exception(const std::string& argument, const std::string& valid_typename, const Ts&... reason)
{
	return ex::make_custom_exception<argument_except>(
		"Invalid Argument:    ", argument, '\n',
		"        Reason:      ", reason..., '\n',
		"        Valid Type:  ", valid_typename, '\n'
	);
}

INLINE CONSTEXPR const FmtFlag* const FIXED{ &std::ios_base::fixed }, * SCIENTIFIC{ &std::ios_base::scientific };

namespace ckconv {

	/// @brief	Default executable name.
	INLINE CONSTEXPR const auto DEFAULT_NAME{ "ckconv.exe" };

	/// @brief	INI Header that contains output-related settings.
	INLINE CONSTEXPR const auto HEADER_OUTPUT{ "output" }, HEADER_GLOBAL{ "" };

	/**
	 * @enum	OUT
	 * @brief	Corresponds to sections of output that should have the same colors.
	 */
	enum class OUT : char {
		INPUT_UNIT,
		INPUT_VALUE,
		OUTPUT_UNIT,
		OUTPUT_VALUE,
		EQUALS,
		UNITS_SECTION_TEXT,
	};

	static struct {
		///	@brief	Palette instance containing each of the keys from OUT. This is used to allow disabling color sequences program-wide.
		term::palette<OUT> palette{
			std::make_pair(OUT::INPUT_UNIT, term::setcolor(color::white, color::format::BOLD)),
			std::make_pair(OUT::INPUT_VALUE, term::setcolor(color::green)),
			std::make_pair(OUT::OUTPUT_UNIT, term::setcolor(color::white, color::format::BOLD)),
			std::make_pair(OUT::OUTPUT_VALUE, term::setcolor(color::green)),
			std::make_pair(OUT::EQUALS, term::setcolor(color::white)),
			std::make_pair(OUT::UNITS_SECTION_TEXT, term::setcolor(color::intense_white)),
		};


		std::filesystem::path ini_path;

		// CONFIG VALUES:

		std::streamsize precision{ 6LL };
		std::streamsize align_to_column{ 8LL };
		const FmtFlag* notation{ nullptr };
		bool quiet{ false };
		bool use_full_unit_names{ false };
	} Global;

	/**
	 * @brief				Write the help documentation to the given output stream.
	 * @param os			Output stream ref to write to.
	 * @param program_name	The current name of the executable.
	 */
	inline void write_help(std::ostream& os, const std::string& program_name) noexcept
	{
		os << program_name << ' '; // print current program name
		if (program_name != DEFAULT_NAME) // print original program name if modified
			os << "(" << DEFAULT_NAME << ") ";
		os
			<< 'v' << ckconv_VERSION << '\n' // print current program version number
			<< "  Converts between Metric, Imperial, and CreationKit length units.\n"
			<< '\n'
			<< "USAGE:\n"
			<< "  " << program_name << " [OPTIONS] [<INPUT_UNIT> <INPUT_VALUE> <OUTPUT_UNIT>]...\n"
			<< '\n'
			<< "OPTIONS:\n"
			<< "  -h            --help            Show the help display and exit." << '\n'
			<< "  -v            --version         Show the current version number and exit." << '\n'
			<< "  -u [system]   --units [system]  Displays a list of all of the recognized units, optionally from a single" << '\n'
			<< "                                  measurement system, then exit. By default, all systems are shown." << '\n'
			<< "  -f            --full-name       Use the full name instead of the official unit symbols when possible." << '\n'
			<< "  --standard    --fixed           Force standard notation." << '\n'
			<< "  --scientific  --sci             Force scientific notation." << '\n'
			<< "  -p <#>        --precision <#>   Force show at least <INT> number of digits after the decimal point." << '\n'
			<< "  -a <#>        --align-to <#>    Aligns output to <#> character columns." << '\n'
			<< "                                  Does nothing if the quiet option is specified." << '\n'
			<< "  -q            --quiet           Print only output values." << '\n'
			<< "  -n            --no-color        Don't use color escape sequences." << '\n'
			<< "                --set-ini         Create or overwrite the config with the current configuration, including options." << '\n'
			<< "                                  This is affected by other options like precision & no-color." << '\n'
			;
	}
	/**
	 * @brief		Handle all arguments except for help/version.
	 * @param args	The argument container instance.
	 */
	inline void handle_args(const opt::ParamsAPI2& args) noexcept(false)
	{
		// notation
		if (args.check_any<opt::Option>("fixed", "standard"))
			Global.notation = FIXED;
		else if (args.check_any<opt::Option>("sci", "scientific"))
			Global.notation = SCIENTIFIC;

		// precision
		if (const auto precision{ args.typegetv_any<opt::Flag, opt::Option>('p', "precision") }; precision.has_value()) {
			const auto val{ precision.value() };
			if (std::all_of(val.begin(), val.end(), isdigit))
				Global.precision = str::stoll(val);
			else throw argument_exception("precision", "Integer", val, " is not a valid precision value!");
		}

		// align-to
		if (const auto alignment{ args.typegetv_any<opt::Flag, opt::Option>('a', "align-to") }; alignment.has_value()) {
			const auto val{ alignment.value() };
			if (std::all_of(val.begin(), val.end(), isdigit))
				Global.precision = str::stoll(val);
			else throw argument_exception("align-to", "Integer", val, " is not a valid column!");
		}

		Global.use_full_unit_names = args.check_any<opt::Flag, opt::Option>('f', "full-name");

		// quiet
		Global.quiet = args.check_any<opt::Flag, opt::Option>('q', "quiet");

		// no-color
		Global.palette.setActive(!args.check_any<opt::Flag, opt::Option>('n', "no-color"));
	}

	/**
	 * @brief	Write the current configuration to the file specified by (Global.ini_path).
	 */
	inline void write_settings_to_config()
	{
		const bool is_new{ !file::exists(Global.ini_path) };
		using section = file::ini::INIContainer::SectionContent;
		using var = file::ini::VariableT;
		if (file::INI(std::move(file::ini::INIContainer::Map{ {
			{
				std::string{ HEADER_GLOBAL }, section{
					{ "version", file::ini::VariableT{ static_cast<file::ini::String>(ckconv_VERSION) } },
			}
			},
				{
					std::string{ HEADER_OUTPUT }, section{
						{ "precision", var{ static_cast<file::ini::Integer>(Global.precision) } },
			{ "notation", var{ []() -> std::string {
				if (Global.notation != nullptr) {
					if (Global.notation == FIXED)
						return "fixed";
					else if (Global.notation == SCIENTIFIC)
						return "scientific";
				}
				return{};
			}()
			} },
			{ "quiet", var{ Global.quiet } },
			{ "no-color", var{ !Global.palette.isActive() } },
			}
				},
			},
			})).write(Global.ini_path)) {
			std::cout << Global.palette.get_msg() << "Successfully " << (is_new ? "created" : "reset") << " config: " << Global.ini_path << std::endl;
		}
		else throw make_exception("Failed to ", (is_new ? "create" : "reset"), " config: ", Global.ini_path);
	}

	/**
	 * @brief			Handle the config file's version number, and print a warning.
	 * @param strver
	 */
	inline void handle_config_version(const std::string& strver) noexcept(false)
	{
		if (!strver.empty())
			if (const auto pos{ strver.find('.') }; pos < strver.size() && strver.substr(0ull, pos) == STRINGIZE(ckconv_VERSION_MAJOR))
				return;
		throw make_exception("Configuration file created with an incompatible build of ckconv:  ", Global.ini_path, "!  (Current version: ", ckconv_VERSION, ')');
	}

	/**
	 * @brief		Handle setting values from the configuration file.
	 * @param ini	The INI config instance.
	 */
	inline void handle_config(const file::INI& ini) noexcept(false)
	{
		// version
		handle_config_version(ini.getvs(HEADER_GLOBAL, "version").value_or(ckconv_VERSION));

		// precision
		if (const auto precision{ ini.getv(HEADER_OUTPUT, "precision") }; precision.has_value()) {
			Global.precision = std::visit([](auto&& val) -> std::streamsize {
				using T = std::decay_t<decltype(val)>;
				if constexpr (std::same_as<T, file::ini::Integer>)
					return val;
				else if constexpr (std::same_as<T, file::ini::Float>)
					return static_cast<std::streamsize>(val);
				else if constexpr (std::same_as<T, file::ini::String>)
					return str::stoll(val);
				else if constexpr (std::same_as<T, file::ini::Boolean> || std::same_as<T, std::monostate>)
					return Global.precision;
				else static_assert(var::false_v<T>, "Visitor cannot handle all potential type cases!");
				}, precision.value());
		}

		// notation
		if (const auto notation{ ini.getvs(HEADER_OUTPUT, "notation") }; notation.has_value()) {
			if (const auto lc{ str::tolower(notation.value()) }; lc == "fixed")
				Global.notation = FIXED;
			else if (lc == "scientific")
				Global.notation = SCIENTIFIC;
		}

		// quiet
		Global.quiet = ini.checkv(HEADER_OUTPUT, "quiet", true);

		// no-color
		Global.palette.setActive(!ini.checkv(HEADER_OUTPUT, "no-color", true));
	}

	inline std::ostream& configure_ostream(std::ostream& os)
	{
		// override floatfield notation
		if (Global.notation != nullptr)
			os.setf(*Global.notation);

		// modify precision
		if (Global.precision != 6ll)
			os.precision(Global.precision);

		return os;
	}

	/**
	 * @brief		Stream insertion operator for the Unit struct that uses full names or symbols depending on Global.use_full_unit_names.
	 *\n			This is implicitly called whenever inserting a Unit into an output stream.
	 * @param os	Output Stream
	 * @param u		Length Unit Instance
	 * @returns		std::ostream&
	 */
	inline std::ostream& operator<<(std::ostream& os, const Unit& u)
	{
		if (Global.use_full_unit_names)
			return os << u.getName();
		return os << u.getSymbol();
	}

	template<SystemID System>
	struct PrintableMeasurementUnits {
		friend std::ostream& operator<<(std::ostream& os, const PrintableMeasurementUnits<System>& u)
		{
			bool fallthrough{ System == SystemID::ALL }; // fallthrough all cases and print everything
			constexpr const std::streamsize symbol_indent_postfix{ 8ll }, name_indent_postfix{ 16ll };
			std::stringstream ss;
			switch (System) {
			case SystemID::ALL:
				if (!fallthrough) break;
				[[fallthrough]];
			case SystemID::CREATIONKIT:
			{
				ss
					<< Global.palette.set(OUT::UNITS_SECTION_TEXT) << "Creation Kit Units:" << Global.palette.reset() << "\n"
					<< "  Symbol  Name            1 in Base Unit\n"
					<< "  --------------------------------------\n"
					;
				int power{ -12 };
				for (const auto& unit : CreationKit.units) {
					const auto& symbol{ (unit.hasName() ? unit.getSymbol() : ""s) }, name{ unit.getName() };
					ss
						<< "  " << symbol << str::VIndent(symbol_indent_postfix, symbol.size())
						<< name << str::VIndent(name_indent_postfix, name.size())
						<< unit.unitcf << ' ' << *CreationKit.base
						<< '\n';
				}
				if (!fallthrough) break;
				ss << '\n';
				[[fallthrough]];
			}
			case SystemID::METRIC:
			{
				ss
					<< Global.palette.set(OUT::UNITS_SECTION_TEXT) << "Metric Units:" << Global.palette.reset() << "\n"
					<< "  Symbol  Name            1 in Base Unit\n"
					<< "  --------------------------------------\n"
					;
				int power{ -12 };
				for (const auto& unit : Metric.units) {
					const auto& symbol{ (unit.hasName() ? unit.getSymbol() : ""s) }, name{ unit.getName() };
					ss
						<< "  " << symbol << str::VIndent(symbol_indent_postfix, symbol.size())
						<< name << str::VIndent(name_indent_postfix, name.size())
						<< unit.unitcf << ' ' << *Metric.base
						<< '\n';
					power += (power < 3 || power > 3 ? 3 : 1);
				}
				if (!fallthrough) break;
				ss << '\n';
				[[fallthrough]];
			}
			case SystemID::IMPERIAL:
			{
				ss
					<< Global.palette.set(OUT::UNITS_SECTION_TEXT) << "Imperial Units:" << Global.palette.reset() << "\n"
					<< "  Symbol  Name            1 in Base Unit\n"
					<< "  --------------------------------------\n"
					;
				for (const auto& unit : Imperial.units) {
					const auto& symbol{ (unit.hasName() ? unit.getSymbol() : ""s) }, name{ unit.getName() };
					ss
						<< "  " << symbol << str::VIndent(symbol_indent_postfix, symbol.size())
						<< name << str::VIndent(name_indent_postfix, name.size())
						<< unit.unitcf << ' ' << *Imperial.base
						<< '\n';
				}
				if (!fallthrough) break;
				[[fallthrough]];
			}
			default:break;
			}
			return os << ss.rdbuf();
		}
	};
}
