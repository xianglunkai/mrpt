/* +------------------------------------------------------------------------+
   |                     Mobile Robot Programming Toolkit (MRPT)            |
   |                          https://www.mrpt.org/                         |
   |                                                                        |
   | Copyright (c) 2005-2022, Individual contributors, see AUTHORS file     |
   | See: https://www.mrpt.org/Authors - All rights reserved.               |
   | Released under BSD License. See: https://www.mrpt.org/License          |
   +------------------------------------------------------------------------+ */

#include "obs-precomp.h"  // Precompiled headers
//
#include <mrpt/obs/CObservationBatteryState.h>
#include <mrpt/serialization/CArchive.h>
#include <mrpt/system/os.h>

using namespace mrpt::obs;
using namespace mrpt::poses;
using namespace mrpt::math;

// This must be added to any CSerializable class implementation file.
IMPLEMENTS_SERIALIZABLE(CObservationBatteryState, CObservation, mrpt::obs)

/** Constructor
 */
CObservationBatteryState::CObservationBatteryState()
	: voltageOtherBatteries(), voltageOtherBatteriesValid()
{
}

uint8_t CObservationBatteryState::serializeGetVersion() const { return 2; }
void CObservationBatteryState::serializeTo(
	mrpt::serialization::CArchive& out) const
{
	// The data
	out << voltageMainRobotBattery << voltageMainRobotComputer
		<< voltageMainRobotBatteryIsValid << voltageMainRobotComputerIsValid
		<< voltageOtherBatteries << voltageOtherBatteriesValid << sensorLabel
		<< timestamp;
}

void CObservationBatteryState::serializeFrom(
	mrpt::serialization::CArchive& in, uint8_t version)
{
	switch (version)
	{
		case 0:
		case 1:
		case 2:
		{
			in >> voltageMainRobotBattery >> voltageMainRobotComputer >>
				voltageMainRobotBatteryIsValid >>
				voltageMainRobotComputerIsValid >> voltageOtherBatteries >>
				voltageOtherBatteriesValid;
			if (version >= 1) in >> sensorLabel;
			else
				sensorLabel = "";

			if (version >= 2) in >> timestamp;
			else
				timestamp = INVALID_TIMESTAMP;
		}
		break;
		default: MRPT_THROW_UNKNOWN_SERIALIZATION_VERSION(version);
	};
}

// See base class docs
void CObservationBatteryState::getSensorPose(CPose3D& out_sensorPose) const
{
	out_sensorPose = CPose3D(0, 0, 0);
}

// See base class docs
void CObservationBatteryState::setSensorPose([
	[maybe_unused]] const CPose3D& newSensorPose)
{
}

void CObservationBatteryState::getDescriptionAsText(std::ostream& o) const
{
	CObservation::getDescriptionAsText(o);

	o << format(
		"Measured VoltageMainRobotBattery: %.02fV  isValid= %s \n",
		voltageMainRobotBattery,
		(voltageMainRobotBatteryIsValid == true) ? "True" : "False");

	o << format(
		"Measured VoltageMainRobotComputer: %.02fV  isValid= %s \n",
		voltageMainRobotComputer,
		(voltageMainRobotComputerIsValid == true) ? "True" : "False");

	o << "VoltageOtherBatteries: \n";
	for (CVectorDouble::Index i = 0; i < voltageOtherBatteries.size(); i++)
	{
		o << format(
			"Index: %d --> %.02fV  isValid= %s \n", int(i),
			voltageOtherBatteries[i],
			(voltageOtherBatteriesValid[i] == true) ? "True" : "False");
	}
}

std::string CObservationBatteryState::exportTxtHeader() const
{
	return "VoltageMainRobotBattery "
		   "VoltageMainRobotComputer "
		   "[other voltages...]";
}
std::string CObservationBatteryState::exportTxtDataRow() const
{
	std::string s;
	s += mrpt::format("%18.5f ", voltageMainRobotBattery);
	s += mrpt::format("%18.5f ", voltageMainRobotComputer);

	for (CVectorDouble::Index i = 0; i < voltageOtherBatteries.size(); i++)
		s += mrpt::format("%18.5f ", voltageOtherBatteries[i]);

	return s;
}
