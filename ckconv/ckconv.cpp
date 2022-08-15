#include "rc/version.h"
#include "conv.hpp"
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
			<< "ckconv v" << ckconv_VERSION_EXTENDED << '\n'
			<< "  " << '\n'
			<< '\n'
			<< "USAGE:" << '\n'
			<< "  " << h.programName << " [OPTIONS] [<INPUT_UNIT> <INPUT_VALUE> <OUTPUT_UNIT>]...\n"
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
};

int main(const int argc, char** argv)
{
	color::sync syncc;
	try {
		using namespace opt3_literals;
		opt3::ArgManager args{ argc, argv,
			'p'_reqcap, "precision"_reqcap,
			'a'_reqcap, "align-to"_reqcap,
			'u', "units"
		};
		env::PATH pathVar;
		const auto& [programPath, programName] { pathVar.resolve_split(argv[0]) };

		// -q | --quiet
		const bool quiet{ args.check_any<opt3::Flag, opt3::Option>('q', "quiet") };
		// -n | --no-color
		syncc.setEnabled(!quiet && !args.check_any<opt3::Flag, opt3::Option>('n', "no-color"));
		// -h | --help
		if (/*args.empty() || */args.check_any<opt3::Flag, opt3::Option>('h', "help")) {
			std::cout << Help(programName.generic_string()) << std::endl;
			return 0;
		}
		// -v | --version
		else if (args.check_any<opt3::Flag, opt3::Option>('v', "version")) {
			if (!quiet) std::cout << "ckconv v";
			std::cout << ckconv_VERSION_EXTENDED << std::endl;
			return 0;
		}

		if (const auto& userInputs{ processInput(expandUnits(cat(getInputsFromSTDIN(), args.getv_all<opt3::Parameter>()))) };
			!userInputs.empty()) {

			for (const auto& it : userInputs) {
				const auto& [inUnit, inValue, outUnit] { toConvertible(it) };
				const auto outValue{ conv::convert(inUnit, inValue, outUnit) };

				std::cout << inValue << ' ' << inUnit.getName() << "\t=> " << outValue << ' ' << outUnit.getName() << '\n';
			}
		}

		return 0;
	} catch (const std::exception& ex) {
		std::cerr << syncc.get_fatal() << ex.what() << std::endl;
		return 1;
	} catch (...) {
		std::cerr << syncc.get_fatal() << "An undefined exception occurred!" << std::endl;
		return 1;
	}
}
