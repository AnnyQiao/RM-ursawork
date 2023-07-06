/*
 * Copyright (c) 2020-2021 Advanced Robotics at the University of Washington <robomstr@uw.edu>
 *
 * This file is part of taproot-examples.
 *
 * taproot-examples is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * taproot-examples is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with taproot-examples.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef EXAMPLE_COMPRISED_COMMAND_HPP_
#define EXAMPLE_COMPRISED_COMMAND_HPP_

#include "tap/control/comprised_command.hpp"

#include "example_command.hpp"
#include "example_subsystem.hpp"

class ExampleComprisedCommand : public tap::control::ComprisedCommand
{
public:
    explicit ExampleComprisedCommand(Drivers* drivers, ExampleSubsystem* subsystem);

    void initialize() override;

    void execute() override;

    void end(bool interrupted) override;

    bool isFinished() const override { return false; }

    const char* getName() const override { return "example comprised"; }

private:
    ExampleCommand exampleCommand;

    ExampleCommand otherExampleCommand;

    tap::arch::MilliTimeout switchTimer;

    bool switchCommand;
};

#endif  // EXAMPLE_COMPRISED_COMMAND_HPP_