#include "rc/version.h"
#include "PrintableMeasurementUnits.hpp"
#include "util.h"

#include <opt3.hpp>
#include <TermAPI.hpp>
#include <envpath.hpp>
#include <color-sync.hpp>

#ifdef OS_LINUX
#undef ENABLE_UPDATE_CHECK
#endif

#ifdef ENABLE_UPDATE_CHECK
#include <nlohmann/json.hpp>
#include <cpr/cpr.h>
#include <semver.hpp>
#include <env.hpp>
inline constexpr auto URL_GITHUB_LATEST_RELEASE{ "https://api.github.com/repos/radj307/ckconv/releases/latest" };
inline static const shared::semver CKCONV_SEMVER{ ckconv_VERSION_EXTENDED };
#endif

#ifdef ENABLE_CONFIG_FILE
#include <simpleINI.hpp>
#endif

struct Help {
	std::string programName;
	WINCONSTEXPR Help(std::string const& programName) : programName{ programName } {}
	friend std::ostream& operator<<(std::ostream& os, const Help& h)
	{
		os
			<< "Creation Kit Unit Converter (ckconv) v" << ckconv_VERSION_EXTENDED << '\n'
			<< "  Converts between Metric, Imperial, and the eponymous 'Unit' used by Bethesda's Gamebryo & Creation Kit engines." << '\n'
			<< '\n'
			<< "USAGE:" << '\n'
			<< "  " << h.programName << " [OPTIONS] [<UNIT> <VALUE> <OUTPUT_UNIT> ...]\n"
			<< '\n'
			<< "  The input syntax is flexible and supports multiple forms. For example, these are both valid:" << '\n'
			<< "   '260meters kilounits' or '<VALUE> <UNIT> <OUTPUT_UNIT>'" << '\n'
			<< '\n'
			<< "OPTIONS:\n"
			<< "  -h, --help                Show the help display and exit." << '\n'
			<< "  -v, --version             Show the current version number and exit." << '\n'
			<< "  -f, --full-name           Use the full name instead of the official unit symbols when possible." << '\n'
			<< "  -p, --precision <#>       Force the specified number of digits to appear after the decimal point." << '\n'
			<< "  -a, --align-to <#>        Aligns output to <#> character columns." << '\n'
			<< "                            Does nothing if the quiet option is specified." << '\n'
			<< "  -u, --units [name]        Displays a list of all recognized unit names, symbols, & relative size." << '\n'
			<< "                             Optionally accepts the name of a specific measurement system or unit to" << '\n'
			<< "                             only show units from that system." << '\n'
			<< "  -w, --where               Prints the location of the `ckconv` executable." << '\n'
			<< '\n'
			<< "NOTATION:\n"
			<< "  -F, --standard, --fixed   Force print numbers in fixed-point (standard) notation." << '\n'
			<< "  -S, --scientific, --sci   Force print numbers in scientific notation." << '\n'
			<< "  -H, --hexadecimal, --hex  Prints all numbers in hexadecimal." << '\n'
			<< '\n'
			<< "APPEARANCE:\n"
			<< "  -q, --quiet               Print only output values." << '\n'
			<< "  -n, --no-color            Don't use color escape sequences." << '\n'
			;
	#ifdef ENABLE_CONFIG_FILE
		os
			<< '\n'
			<< "CONFIG:\n"
			<< "      --ini <PATH>          Overrides the default location of the INI config. This will also override-" << '\n'
			<< "                             -the $CKCONV_INI environment variable, if it exists." << '\n'
			<< "      --new-ini [PATH]      Creates (or overwrites) a new INI config file with default values, then exits." << '\n'
			<< "  -g, --get [H:]<K>         Gets the current value of a key in the INI config." << '\n'
			<< "  -s, --set [H:]<K:V>       Changes the value of the specified key in the INI config." << '\n'
			<< "      --dry                 When writing the INI config, write to STDOUT instead of to disk. This allows-" << '\n'
			<< "                             -you to observe the changes made by the set option without actually writing them." << '\n'
			;
	#endif
	#ifdef ENABLE_UPDATE_CHECK
		os
			<< '\n'
			<< "UPDATE:\n"
			<< "  -U, --check-for-updates   Checks for newer releases by querying the Github API. You can optionally set the-" << '\n'
			<< "                             -$GITHUB_TOKEN environment variable to a personal access token to use that for-" << '\n'
			<< "                             -authentication. This isn't required, but it increases the rate limit cap on requests." << '\n'
			;
	#endif
		return os;
	}
};

#define $argNames_standardNotation 'F', "standard", "fix", "fixed", "fixed-point"
#define $argNames_scientificNotation 'S', "scientific", "sci"
#define $argNames_hexNotation 'H', 'X', "hexadecimal", "hex"


int main(const int argc, char** argv)
{
	using namespace ckconv;

	env::PATH pathVar;
	const auto& [programPath, programName] { pathVar.resolve_split(argv[0]) };

	try {
		opt3::ArgManager args{ argc, argv,
			opt3::make_template(opt3::CaptureStyle::Required, 'p', "precision"),
			opt3::make_template(opt3::CaptureStyle::Required, 'a', "align-to"),
			opt3::make_template(opt3::CaptureStyle::Optional, 'u', "units", "list-units"),
			opt3::make_template(opt3::CaptureStyle::Disabled, $argNames_standardNotation).SetConflicts($argNames_scientificNotation, $argNames_hexNotation),
			opt3::make_template(opt3::CaptureStyle::Disabled, $argNames_scientificNotation).SetConflicts($argNames_standardNotation, $argNames_hexNotation),
			opt3::make_template(opt3::CaptureStyle::Disabled, $argNames_hexNotation).SetConflicts($argNames_standardNotation, $argNames_scientificNotation),
			opt3::make_template(opt3::CaptureStyle::Required, 's', "set").SetConflicts('g', "get"),
			opt3::make_template(opt3::CaptureStyle::Required, 'g', "get").SetConflicts('s', "set"),
			opt3::make_template(opt3::CaptureStyle::Required, "ini").SetMax(1),
			opt3::make_template(opt3::CaptureStyle::Optional, "new-ini").SetMax(1),
		};

	#ifdef ENABLE_CONFIG_FILE
		// --ini | $CKCONV_INI
		const pathstring cfgPath{ args.castgetv<pathstring, opt3::Option>("ini").value_or(env::getvar("CKCONV_INI").value_or(pathstring{ programPath / std::filesystem::path{ programName }.replace_extension(".ini") })) };

		ini::INI::ParserConfig parserCfg{
			{ { ini::GLOBAL, { { "no-color", false }, { "quiet", false } } }, },
		};

		ini::INI cfg;

		// read the ini
		const bool cfgExists{ file::exists(cfgPath) };
		if (cfgExists) cfg.read(cfgPath, parserCfg);
		else cfg.mask(parserCfg);
	#endif // ENABLE_CONFIG_FILE

		// -n | --no-color
		global.csync.setEnabled(!(args.check_any<opt3::Flag, opt3::Option>('n', "no-color")
							#ifdef ENABLE_CONFIG_FILE
								|| cfg(ini::GLOBAL, "no-color", true)
							#endif
		));
		// -q | --quiet
		global.quiet = (args.check_any<opt3::Flag, opt3::Option>('q', "quiet")
		#ifdef ENABLE_CONFIG_FILE
			|| cfg(ini::GLOBAL, "quiet", true)
		#endif
			);


	#ifdef ENABLE_CONFIG_FILE
		// -s | --set
		if (const auto& setterArgs{ args.getv_all<opt3::Flag, opt3::Option>('s', "set") }; !setterArgs.empty()) {
			bool changed{ false };
			for (const auto& it : setterArgs) {
				const auto& segments{ str::split_all(str::trim(it, ':'), ":") };
				if (segments.empty()) throw make_exception("Invalid INI key specifier '", it, "'!");
				else if (segments.size() > 3) throw make_exception("Invalid INI key specifier '", it, "' contains too many delimiters; it cannot possibly point to a valid key!");
				else if (segments.size() == 1) throw make_exception("Invalid INI key specifier '", it, "' doesn't specify a value for key '", segments.at(0), "'!");

				std::string header, key, value;

				if (segments.size() == 3) {
					header = segments.at(0);
					key = segments.at(1);
					value = segments.at(2);
				}
				else if (segments.size() == 2) {
					key = segments.at(0);
					value = segments.at(1);
				}

				if (auto& k{ cfg(header, key) }; k != value) {
					k = value;
					changed = true;
					if (!global.quiet)
						std::cout << global.csync(global.INI_HeaderColor) << header << global.csync() << (header.empty() ? "" : "::") << global.csync(global.INI_KeyColor) << key << global.csync() << " = " << value << '\n';
				}
				else if (!global.quiet)
					std::cout << global.csync(global.INI_HeaderColor) << header << global.csync() << (header.empty() ? "" : "::") << global.csync(global.INI_KeyColor) << key << global.csync() << " was already set to '" << value << "'\n";
			}
			if (changed) {
				cfg.write(cfgPath);
				if (!global.quiet) std::cout << "Saved changes to " << cfgPath << '\n';
			}
			else if (!global.quiet) std::cout << "No changes were made to the INI.\n";
			return 0;
		}
		// -g | --get
		else if (const auto& getterArgs{ args.getv_all<opt3::Flag, opt3::Option>('g', "get") }; !getterArgs.empty()) {
			for (const auto& it : getterArgs) {
				const auto& segments{ str::split_all(str::trim(it, ':'), ":") };
				if (segments.empty()) throw make_exception("Invalid INI key specifier '", it, "'!");
				else if (segments.size() > 3) throw make_exception("Invalid INI key specifier '", it, "' contains too many delimiters; it cannot possibly point to a valid key!");
				else if (segments.size() > 2) throw make_exception("Invalid INI key specifier '", it, "' contains too many delimiters; you can't use the get option to set values!");

				std::string header, key;

				if (segments.size() > 1) { // header was specified:
					header = segments.at(0);
					key = segments.at(1);
				}
				else key = segments.at(0);

				if (!global.quiet)
					std::cout << global.csync(global.INI_HeaderColor) << (header.empty() ? header : "") << global.csync() << (header.empty() ? "" : "::") << global.csync(global.INI_KeyColor) << key << global.csync() << " == ";
				std::cout << cfg(header, key) << '\n';
			}
			return 0;
		}
	#endif // ENABLE_CONFIG_FILE

		// -f | --full-name
		global.useFullNames = args.check_any<opt3::Flag, opt3::Option>('f', "full-name", "full-names");

		// -h | --help
		if (const auto& noArgsProvided{ args.empty() }; noArgsProvided || args.check_any<opt3::Flag, opt3::Option>('h', "help")) {
			std::cout << Help(programName.generic_string());
			if (noArgsProvided)
				std::cerr << term::get_fatal(false) << "No arguments provided!" << std::endl;
			return 0;
		}
		// -v | --version
		else if (args.check_any<opt3::Flag, opt3::Option>('v', "version")) {
			if (!global.quiet) std::cout << "ckconv v";
			std::cout << ckconv_VERSION_EXTENDED << std::endl;
			return 0;
		}
		// -u | --units | --list-units
		else if (const auto& unitsArg{ args.get_any<opt3::Flag, opt3::Option>('u', "units", "list-units") }; unitsArg.has_value()) {
			std::cout << PrintMeasurementUnits(unitsArg.value().capture_or(""s));
			return 0;
		}
		// -w | --where
		else if (args.check_any<opt3::Flag, opt3::Option>('w', "where")) {
			if (!global.quiet) std::cout << programName.generic_string() << ":  ";
			std::cout << (programPath / programName).generic_string() << std::endl;
		#ifdef ENABLE_CONFIG_FILE
			if (!global.quiet) std::cout << cfgPath.filename().generic_string() << ":  ";
			std::cout << cfgPath.generic_string() << std::endl;
		#endif
			return 0;
		}
	#ifdef ENABLE_UPDATE_CHECK
		// --update | $GITHUB_TOKEN
		else if (args.check_any<opt3::Flag, opt3::Option>('U', "update", "check-for-updates")) {
			cpr::Session session{};
			session.SetUrl(URL_GITHUB_LATEST_RELEASE);
			session.SetUserAgent("curl/7.64.1");

			cpr::Header header{ { "Accept", "application/vnd.github+json" } };
			if (const auto& tkn{ env::getvar("GITHUB_TOKEN") }; tkn.has_value())
				header.insert(std::make_pair("Authorization"s, "token "s + tkn.value()));
			session.SetHeader(header);

			const auto& r{ session.Get() };

			switch (r.status_code) {
			case 200: { // Success
				if (r.text.empty())
					throw make_exception("The response status code indicates success (", r.status_code, "), but the response body was empty!");
				try {
					const auto& latest_tag{ $c(std::string, nlohmann::json::parse(r.text)["tag_name"]) };
					const shared::semver latest{ latest_tag };

					if (latest > CKCONV_SEMVER) {
						if (!global.quiet) std::cout << global.csync.get_msg() << "Update Available:  " << global.csync(global.NewTagColor) << 'v';
						else std::cout << global.csync(global.NewTagColor);
						std::cout << latest_tag << global.csync() << '\n';
					}
					else if (!global.quiet)
						std::cout << global.csync.get_msg() << "You're using the latest version of ckconv. (" << ckconv_VERSION_EXTENDED << ").\n";
					// else there are no updates available & quiet is true; don't print anything!
				} catch (const std::exception& ex) {
					throw make_exception("An exception was thrown while parsing the response data!\nException: '", ex.what(), "'");
				}
				break;
			}
			default:
				throw make_exception("The API request failed with status code ", r.status_code, "!\n");
			}
			return 0;
		}
	#endif // ENABLE_UPDATE_CHECK
	#ifdef ENABLE_CONFIG_FILE
		// --new-ini
		else if (const auto& newIniArg{ args.get_any<opt3::Option>("new-ini", "ini-new") }; newIniArg.has_value()) {
			const auto& path{ newIniArg.value().capture_or(cfgPath) };
			const bool alreadyExists{ file::exists(path) };
			if (file::write(path, typename ini::INI::Printer(&parserCfg.mask))) {
				if (!global.quiet)
					std::cout << "Successfully " << (alreadyExists ? "overwrote existing" : "created new") << " INI config at '" << path << "'\n";
			}
			else throw make_exception("Failed to write INI config file to '", path, "'!");
		}
	#endif

		// -F | --standard | --fixed
		if (args.check_any<opt3::Flag, opt3::Option>('F', "standard", "fixed")) {
			if (global.floatfield.has_value())
				global.floatfield = global.floatfield.value() | std::ios_base::fixed;
			else global.floatfield = std::ios_base::fixed;
		}
		// -S | --scientific | --sci
		if (args.check_any<opt3::Flag, opt3::Option>('S', "scientific", "sci")) {
			if (global.floatfield.has_value())
				global.floatfield = global.floatfield.value() | std::ios_base::scientific;
			else global.floatfield = std::ios_base::scientific;
		}
		// -H | -x | --hexadecimal | --hex
		if (args.check_any<opt3::Flag, opt3::Option>('H', 'x', "hexadecimal", "hex")) {
		#ifdef OS_WIN
			global.floatfield = std::ios_base::hexfloat;
		#else
			global.floatfield = (std::ios_base::fixed | std::ios_base::scientific);
		#endif
		}

		// -a | --align | --align-to
		global.indent = args.castgetv_any<size_t, opt3::Flag, opt3::Option>('a', "align", "align-to");
		// -p | --precision
		global.precision = args.castgetv_any<size_t, opt3::Flag, opt3::Option>('p', "precision");

		/// MAIN:

		// process all parameters (trailing) & piped input (preceding) into a vector of string tuples that each represent an operation
		if (const auto& userInputs{ processInput(expandUnits(cat(getInputsFromSTDIN(), args.getv_all<opt3::Parameter>()))) };
			!userInputs.empty()) {

			for (const auto& it : userInputs) {
				stringifier s;
				try {
					const auto& [inUnit, inValue, outUnit] { toConvertible(it) };
					const auto outValue{ conv::convert(inUnit, inValue, outUnit) };

					std::cout << converted{ inUnit, inValue, outUnit, outValue } << '\n';

				} catch (const std::exception& ex) {
					std::cerr << global.csync.get_error() << ex.what() << std::endl;
				}

				std::cout << s.rdbuf() << '\n';
			}
		}
		// 
		else throw make_exception("No valid conversions specified!");

		return 0;
	} catch (const std::exception& ex) {
		std::cerr << term::get_fatal(false) << ex.what() << std::endl;
		return 1;
	} catch (...) {
		std::cerr << term::get_fatal(false) << "An undefined exception occurred!" << std::endl;
		return 1;
	}
}

