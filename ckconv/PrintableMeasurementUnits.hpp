#pragma once
/**
 * @file	PrintableMeasurementUnits.hpp
 * @author	radj307
 * @brief	This header was repurposed from build 5 of ckconv
 */
#include "conv.hpp"
#include "global.h"

#include <TermAPI.hpp>
#include <indentor.hpp>

namespace ckconv {
	inline std::ostream& operator<<(std::ostream& os, const conv::Unit& unit)
	{
		return os << (global.useFullNames ? unit.getName() : unit.getSymbol());
	}

	template<conv::SystemID System>
	struct PrintableMeasurementUnits {
		friend std::ostream& operator<<(std::ostream& os, const PrintableMeasurementUnits<System>& u)
		{
			bool fallthrough{ System == conv::SystemID::ALL }; // fallthrough all cases and print everything
			constexpr const std::streamsize
				symbol_indent_postfix{ 8ll },
				name_indent_postfix{ 16ll };
			std::stringstream ss;

			switch (System) {
			case conv::SystemID::ALL:
				if (!fallthrough) break;
				[[fallthrough]];
			case conv::SystemID::CREATIONKIT:
			{
				ss
					<< global.csync(global.HeaderColor) << "Creation Kit Units:" << global.csync() << "\n"
					<< "  Symbol  Name            1 in Base Unit\n"
					<< "  --------------------------------------\n"
					;
				int power{ -12 };
				for (const auto& unit : conv::CreationKit.units) {
					const auto& symbol{ (unit.hasName() ? unit.getSymbol() : ""s) }, name{ unit.getName() };
					ss
						<< "  " << symbol << indent(symbol_indent_postfix, symbol.size())
						<< name << indent(name_indent_postfix, name.size())
						<< unit.unitcf << ' ' << *conv::CreationKit.base
						<< '\n';
				}
				if (!fallthrough) break;
				ss << '\n';
				[[fallthrough]];
			}
			case conv::SystemID::METRIC:
			{
				ss
					<< global.csync(global.HeaderColor) << "Metric Units:" << global.csync() << "\n"
					<< "  Symbol  Name            1 in Base Unit\n"
					<< "  --------------------------------------\n"
					;
				int power{ -12 };
				for (const auto& unit : conv::Metric.units) {
					const auto& symbol{ (unit.hasName() ? unit.getSymbol() : ""s) }, name{ unit.getName() };
					ss
						<< "  " << symbol << indent(symbol_indent_postfix, symbol.size())
						<< name << indent(name_indent_postfix, name.size())
						<< unit.unitcf << ' ' << *conv::Metric.base
						<< '\n';
					power += (power < 3 || power > 3 ? 3 : 1);
				}
				if (!fallthrough) break;
				ss << '\n';
				[[fallthrough]];
			}
			case conv::SystemID::IMPERIAL:
			{
				ss
					<< global.csync(global.HeaderColor) << "Imperial Units:" << global.csync() << "\n"
					<< "  Symbol  Name            1 in Base Unit\n"
					<< "  --------------------------------------\n"
					;
				for (const auto& unit : conv::Imperial.units) {
					const auto& symbol{ (unit.hasName() ? unit.getSymbol() : ""s) }, name{ unit.getName() };
					ss
						<< "  " << symbol << indent(symbol_indent_postfix, symbol.size())
						<< name << indent(name_indent_postfix, name.size())
						<< unit.unitcf << ' ' << *conv::Imperial.base
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

	/// @brief	The default unit, which is used for SystemID::ALL
	inline const conv::Unit DEFAULT_UNIT{ conv::SystemID::ALL, 0.0, "(all)", "(all)" };

	struct PrintMeasurementUnits {
		static conv::SystemID StringToSystemID(std::string const& systemName)
		{
			if (systemName.empty())
				return conv::SystemID::ALL;

			if (str::equalsAny<true>(systemName, "metric", "mt", "standard", "std"))
				return conv::SystemID::METRIC;
			if (str::equalsAny<true>(systemName, "imperial", "imp"))
				return conv::SystemID::IMPERIAL;
			if (str::equalsAny<true>(systemName, "creationkit", "ck", "creation-kit", "creation_kit", "gamebryo", "engine", "bethesda"))
				return conv::SystemID::CREATIONKIT;

			return conv::getUnit(systemName, DEFAULT_UNIT).getSystem();
		}

		conv::SystemID system;
		PrintMeasurementUnits(std::string const& systemName) : system{ StringToSystemID(systemName) } {}
		PrintMeasurementUnits(conv::SystemID const& systemID) : system{ systemID } {}
		friend std::ostream& operator<<(std::ostream& os, const PrintMeasurementUnits& pm)
		{
			switch (pm.system) {
			case conv::SystemID::ALL:
				return os << PrintableMeasurementUnits<conv::SystemID::ALL>();
			case conv::SystemID::METRIC:
				return os << PrintableMeasurementUnits<conv::SystemID::METRIC>();
			case conv::SystemID::IMPERIAL:
				return os << PrintableMeasurementUnits<conv::SystemID::IMPERIAL>();
			case conv::SystemID::CREATIONKIT:
				return os << PrintableMeasurementUnits<conv::SystemID::CREATIONKIT>();
			default:
				return os;
			}
		}
	};
}
