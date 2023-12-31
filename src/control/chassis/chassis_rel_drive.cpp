/*
 * Copyright (c) 2020-2021 Advanced Robotics at the University of Washington <robomstr@uw.edu>
 *
 * This file is part of aruw-mcb.
 *
 * aruw-mcb is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * aruw-mcb is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with aruw-mcb.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "chassis_rel_drive.hpp"

#include "tap/drivers.hpp"

#include "src/robot/control_operator_interface.hpp"
#include "tap/algorithms/math_user_utils.hpp"

#include "chassis_subsystem.hpp"

using namespace tap::algorithms;

namespace xcysrc::chassis
{
void ChassisRelDrive::computeDesiredUserTranslation(
    xcysrc::control::ControlOperatorInterface *operatorInterface,
    tap::Drivers *drivers,
    MecanumChassisSubsystem *chassis,
    float chassisRotation,
    float *chassisXDesiredWheelspeed,
    float *chassisYDesiredWheelspeed)
{
    if (drivers == nullptr || operatorInterface == nullptr || chassis == nullptr ||
        chassisXDesiredWheelspeed == nullptr || chassisYDesiredWheelspeed == nullptr)
    {
        return;
    }

    const float maxWheelSpeed = MecanumChassisSubsystem::getMaxWheelSpeed(
        drivers->refSerial.getRefSerialReceivingData(),
        drivers->refSerial.getRobotData().chassis.powerConsumptionLimit);

    // what we will multiply x and y speed by to take into account rotation
    float rotationLimitedMaxTranslationalSpeed =
        chassis->calculateRotationTranslationalGain(chassisRotation) * maxWheelSpeed;

    *chassisXDesiredWheelspeed = limitVal(
        operatorInterface->getChassisXInput(),
        -rotationLimitedMaxTranslationalSpeed,
        rotationLimitedMaxTranslationalSpeed);

    *chassisYDesiredWheelspeed = limitVal(
        operatorInterface->getChassisYInput(),
        -rotationLimitedMaxTranslationalSpeed,
        rotationLimitedMaxTranslationalSpeed);
}

void ChassisRelDrive::onExecute(
    xcysrc::control::ControlOperatorInterface *operatorInterface,
    tap::Drivers *drivers,
    MecanumChassisSubsystem *chassis)
{
    float chassisRotationDesiredWheelspeed = operatorInterface->getChassisRInput();

    float chassisXDesiredWheelspeed = 0.0f;
    float chassisYDesiredWheelspeed = 0.0f;

    computeDesiredUserTranslation(
        operatorInterface,
        drivers,
        chassis,
        chassisRotationDesiredWheelspeed,
        &chassisXDesiredWheelspeed,
        &chassisYDesiredWheelspeed);

    chassis->setDesiredOutput(
        chassisXDesiredWheelspeed,
        chassisYDesiredWheelspeed,
        chassisRotationDesiredWheelspeed);
}
}  // namespace xcysrc::chassis
