#pragma once
#include <color-sync.hpp>
#include <simpleINI.hpp>

#include <sstream>

namespace ckconv {
	static struct {
		/// @brief	Color synchronization object
		color::sync csync{};

		/// @brief	Color used for input numbers
		color::setcolor InputColor{ color::cyan };
		/// @brief	Color used for output numbers
		color::setcolor ResultColor{ color::green };
		/// @brief	Color used for units
		color::setcolor UnitColor{};
		/// @brief	Color used for headers in the units list
		color::setcolor HeaderColor{ color::intense_white };
		/// @brief	Color used for generic accents used in various places
		color::setcolor AccentColor{ color::intense_yellow };

		bool quiet{ false };
		bool useFullNames{ false };
		std::optional<size_t> precision{};
		std::optional<std::ios_base::fmtflags> floatfield{};
		std::optional<size_t> indent{};
	} global;

	inline std::string format_fp(const long double value)
	{
		std::stringstream ss;

		const bool
			hasFloatfield{ global.floatfield.has_value() },
			hasPrecision{ global.precision.has_value() };

		if (hasFloatfield)
			ss.setf(global.floatfield.value());

		if (hasPrecision)
			ss << std::setprecision(global.precision.value());

		ss << value;

		if (!hasPrecision && hasFloatfield && global.floatfield.value() == std::ios_base::fixed) {
			// remove trailing zeros for brevity if the numeric precision wasn't explicitly specified
			std::string s{ str::trim_trailing(ss.str(), '0') };
			if (s.ends_with('.'))
				return s.substr(0ull, s.size() - 1);
			else return s;
		}
		return ss.str();
	}

	inline std::string format_unit(conv::Unit const& unit)
	{
		return  (global.useFullNames && unit.hasName() ? unit.getName() : unit.getSymbol());
	}

	struct converted {
		conv::Unit inUnit, outUnit;
		long double inValue, outValue;
		std::string inUnit_s, outUnit_s, inValue_s, outValue_s;

		converted(conv::Unit inUnit, long double inValue, conv::Unit outUnit, long double outValue) :
			inUnit{ inUnit },
			outUnit{ outUnit },
			inValue{ inValue },
			outValue{ outValue },
			inUnit_s{ format_unit(inUnit) },
			outUnit_s{ format_unit(outUnit) },
			inValue_s{ format_fp(inValue) },
			outValue_s{ format_fp(outValue) }
		{
		}

		std::string getExpression() const
		{
			std::stringstream ss;

			if (!global.quiet) {
				//                                 account for space before equals sign  ▼▼▼▼
				const size_t margin{ global.indent.has_value() ? global.indent.value() - 1ull : 0ull };
				const size_t used{ inValue_s.size() + 1ull + inUnit_s.size() };
				//                                    ▲▲▲▲    account for this space      |
				ss//                                                                      ▼
					<< global.csync(global.InputColor) << inValue_s << global.csync() << ' '
					<< global.csync(global.UnitColor) << inUnit_s << global.csync()
					<< indent(margin, used) << " = "
					;//                         ▲ (space before equals sign)
			}

			ss << global.csync(global.ResultColor) << outValue_s << global.csync();

			if (!global.quiet)
				ss << ' ' << global.csync(global.UnitColor) << outUnit_s << global.csync();

			return ss.str();
		}

		friend std::ostream& operator<<(std::ostream& os, const converted& c)
		{
			return os << c.getExpression();
		}
	};
}
