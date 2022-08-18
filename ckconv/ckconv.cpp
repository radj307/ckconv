#include "rc/version.h"
#include "PrintableMeasurementUnits.hpp"
#include "util.h"

#include <TermAPI.hpp>
#include <opt3.hpp>
#include <envpath.hpp>
#include <color-sync.hpp>

struct Help {
	std::string programName;
	WINCONSTEXPR Help(std::string const& programName) : programName{ programName } {}
	friend std::ostream& operator<<(std::ostream& os, const Help& h)
	{
		return os
			<< "Creation Kit Unit Converter (ckconv) v" << ckconv_VERSION_EXTENDED << '\n'
			<< "  " << '\n'
			<< '\n'
			<< "USAGE:" << '\n'
			<< "  " << h.programName << " [OPTIONS] [<UNIT> <VALUE> <OUTPUT_UNIT> ...]\n"
			<< '\n'
			<< "  The input syntax can be in multiple different forms, such as:" << '\n'
			<< "   '<VALUE><UNIT> <OUTPUT_UNIT>' or '<VALUE> <UNIT> <OUTPUT_UNIT>'" << '\n'
			<< '\n'
			<< "  You can optimize the program for usage in shell scripts with `-qn` (quiet & no-color)." << '\n'
			<< '\n'
			<< "  By default, numbers are printed using variable-notation; you can override this behaviour-" << '\n'
			<< "   -with the options listed under '# NOTATION #'.\n"
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
			<< "  # APPEARANCE #\n"
			<< "  -q, --quiet               Print only output values." << '\n'
			<< "  -n, --no-color            Don't use color escape sequences." << '\n'
			<< '\n'
			<< "  # NOTATION #\n"
			<< "  -F, --standard, --fixed   Force print numbers in fixed-point (standard) notation." << '\n'
			<< "  -S, --scientific, --sci   Force print numbers in scientific notation." << '\n'
			<< "  -H, --hexadecimal, --hex  Prints all numbers in hexadecimal." << '\n'
			;
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
			opt3::make_template('p', "precision").captureStyle(opt3::CaptureStyle::Required),
			opt3::make_template('a', "align-to").captureStyle(opt3::CaptureStyle::Required),
			opt3::make_template('u', "units", "list-units").captureStyle(opt3::CaptureStyle::Optional),
			opt3::make_template(opt3::CaptureStyle::Disabled, $argNames_standardNotation).conflictsWith($argNames_scientificNotation, $argNames_hexNotation),
			opt3::make_template(opt3::CaptureStyle::Disabled, $argNames_scientificNotation).conflictsWith($argNames_standardNotation, $argNames_hexNotation),
			opt3::make_template(opt3::CaptureStyle::Disabled, $argNames_hexNotation).conflictsWith($argNames_standardNotation, $argNames_scientificNotation),
		};

		// -q | --quiet
		global.quiet = args.check_any<opt3::Flag, opt3::Option>('q', "quiet");
		// -f | --full-name
		global.useFullNames = args.check_any<opt3::Flag, opt3::Option>('f', "full-name", "full-names");
		// -n | --no-color
		global.csync.setEnabled(!args.check_any<opt3::Flag, opt3::Option>('n', "no-color"));

		// -h | --help
		if (/*args.empty() || */args.check_any<opt3::Flag, opt3::Option>('h', "help")) {
			std::cout << Help(programName.generic_string());
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
			std::cout << PrintMeasurementUnits(unitsArg.value().value_or(""s));
			return 0;
		}
		// -w | --where
		else if (args.check_any<opt3::Flag, opt3::Option>('w', "where")) {
			std::cout << programPath << std::endl;
			return 0;
		}

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
		global.indent = args.castgetv_any<size_t, opt3::Flag, opt3::Option>(str::stoull, 'a', "align", "align-to");
		// -p | --precision
		global.precision = args.castgetv_any<size_t, opt3::Flag, opt3::Option>(str::stoull, 'p', "precision");

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
		else throw make_exception("No valid conversions specified!");

		return 0;
	} catch (const std::exception& ex) {
		std::cerr << global.csync.get_fatal() << ex.what() << std::endl;
		return 1;
	} catch (...) {
		std::cerr << global.csync.get_fatal() << "An undefined exception occurred!" << std::endl;
		return 1;
	}
}
