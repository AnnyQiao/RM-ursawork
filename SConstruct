# Copyright (c) 2020-2021 Advanced Robotics at the University of Washington <robomstr@uw.edu>
#
# This file is part of mcb-project.
#
# mcb-project is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# mcb-project is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with mcb-project.  If not, see <https://www.gnu.org/licenses/>.

import os
import platform
from os.path import join, abspath

from SCons.Script import *

from taproot.build_tools import parse_args

# Define project settings and build paths
PROJECT_NAME         = "mcb-project"
BUILD_PATH           = "build"
HARDWARE_MODM_PATH   = "taproot/modm"
LINKERSCRIPT_FILE    = join(HARDWARE_MODM_PATH, "link/linkerscript.ld")
SIM_MODM_PARENT_PATH = "taproot/sim-modm/"
TAPROOT_SRC_PATH     = "taproot/src"
TAPROOT_PATH         = join(TAPROOT_SRC_PATH, "tap")
TAPROOT_TEST_PATH    = "taproot/test"

# Define libraries
HOSTED_LIBS          = ["pthread"]
GTEST_LIBS           = ["gtest", "gtest_main", "gmock", "gmock_main"]
COVERAGE_LIBS        = ["-lgcov"]

# Don't compile this stuff
ignored_dirs         = [".lbuild_cache", BUILD_PATH, SIM_MODM_PARENT_PATH, HARDWARE_MODM_PATH, TAPROOT_PATH]
ignored_files        = []
# Don't compile this stuff when testing
IGNORED_FILES_WHILE_TESTING = ["src/main.cpp"]


# Parse and validate arguments
args = parse_args.parse_args()

def _get_hosted_target_name_for_current_platform():
    os_type = platform.system()
    try:
        return {
            "Windows": "hosted-windows",
            "Darwin": "hosted-darwin",
            "Linux": "hosted-linux",
        }[os_type]
    except KeyError:
        raise ValueError(f"Unknown host OS {os_type}")

def _get_sim_modm_instance_path():
    return f"taproot/sim-modm/{_get_hosted_target_name_for_current_platform()}/modm"

# Set up target environment-specific paths
if args["TARGET_ENV"] == "tests":
    modm_path = _get_sim_modm_instance_path()
    ignored_files.extend(IGNORED_FILES_WHILE_TESTING)
elif args["TARGET_ENV"] == "sim":
    modm_path = _get_sim_modm_instance_path()
    ignored_dirs.append(TAPROOT_TEST_PATH)
elif args["TARGET_ENV"] == "hardware":
    modm_path = HARDWARE_MODM_PATH
    ignored_dirs.append(TAPROOT_TEST_PATH)


# Set the number of threads unless it has already been set to anything but 1
if GetOption("num_jobs") == 1:
    SetOption("num_jobs", 8)


# Define a new SCons environment and set up basic build variables
env = DefaultEnvironment(ENV=os.environ)
env["CONFIG_BUILD_BASE"] = abspath(join(BUILD_PATH, args["TARGET_ENV"]))
env["CONFIG_PROJECT_NAME"] = PROJECT_NAME
env["CONFIG_ARTIFACT_PATH"] = join(env["CONFIG_BUILD_BASE"], "artifact")
env["CONFIG_PROFILE"] = args["BUILD_PROFILE"]


print("Configured {0} parallel build jobs (-j{0})".format(GetOption("num_jobs")))


# Building all libraries (read from sconscript files located in provided dirs)
# Ensure that modm is first, since taproot depends on modm
external_scons_libraries = [modm_path, TAPROOT_PATH]
env.SConscript(dirs=external_scons_libraries, exports="env")

# References:
#   GCC flags: https://gcc.gnu.org/onlinedocs/gcc/Option-Summary.html
#   Scons API: https://www.scons.org/doc/latest/HTML/scons-user/apa.html
# 
# You can append one or multiple options like this
#     env.Append(CCFLAGS="-pedantic")
#     env.Append(CCFLAGS=["-pedantic", "-pedantic-errors"])
# If you need to remove options, you need to do this:
#     env["CCFLAGS"].remove("-pedantic")
# Note that a lot of options also have a "-no-{option}" option
# that may overwrite previous options.

# Add source directory to build path (for including header files)
env.AppendUnique(CPPPATH=[
    abspath(TAPROOT_SRC_PATH),
    abspath(TAPROOT_TEST_PATH)
])

# Add build-specific flags
if args["BUILD_PROFILE"] == "debug":
    env.AppendUnique(CCFLAGS=["-fno-inline"])
elif args["BUILD_PROFILE"] == "fast":
    if "-g3" in env["CCFLAGS"]:
        env["CCFLAGS"].remove("-g3")
    env.AppendUnique(CCFLAGS=["-O0"])
elif args["BUILD_PROFILE"] == "release":
    # Replace -Os with -O3
    if "-Os" in env["CCFLAGS"]:
        env["CCFLAGS"].remove("-Os")
    env.AppendUnique(CCFLAGS=["-O2"])

# Add profiling-specific flags
if args["PROFILING"] == "true":
    env.AppendUnique(CPPFLAGS=["-DRUN_WITH_PROFILING"])

# Add target-specific flags
if args["TARGET_ENV"] == "sim":
    env.AppendUnique(CPPFLAGS=["-DPLATFORM_HOSTED"])
    env.AppendUnique(LIBS=HOSTED_LIBS)
elif args["TARGET_ENV"] == "tests":
    env.AppendUnique(CPPFLAGS=[
        "-DPLATFORM_HOSTED",
        "-DENV_UNIT_TESTS"])
    env.AppendUnique(CPPPATH=[abspath(TAPROOT_TEST_PATH)])
    env.AppendUnique(LIBS=GTEST_LIBS)
    env.AppendUnique(LIBS=HOSTED_LIBS)

    # Add flags necessary for coverage
    if COMMAND_LINE_TARGETS[0] == "run-tests-gcov":
        env.AppendUnique(CPPFLAGS=[
            "-fprofile-arcs",
            "-ftest-coverage",
            "-fno-inline",
            "-fno-inline-small-functions",
            "-fno-default-inline"])
        env.Append(CCFLAGS='--coverage')
        env.Append(LINKFLAGS='--coverage')
        env.AppendUnique(LIBS=COVERAGE_LIBS)
else:  # hardware target env
    # Update linkerscript file
    env["LINKFLAGS"].remove("-Tmodm/link/linkerscript.ld")
    env.AppendUnique(LINKFLAGS=["-T" + LINKERSCRIPT_FILE])


# Append little endian flag to all
env.AppendUnique(CPPFLAGS=["-DLITTLE_ENDIAN"])


# Add any additional flags the user chooses specify
if "additional-ccflags" in ARGUMENTS:
    env.AppendUnique(CCFLAGS=[*ARGUMENTS["additional-ccflags"].split(",")])


# Find source files to compile
sources = env.FindSourceFiles(".", ignorePaths=ignored_dirs, ignoreFiles=ignored_files)


if args["TARGET_ENV"] == "hardware":
    program = env.Program(target=env["CONFIG_PROJECT_NAME"]+".elf", source=sources)

    # The executable depends on the linkerscript
    env.Depends(target=program, dependency=abspath(LINKERSCRIPT_FILE))

    # Add target environment-specific SCons aliases
    # WARNING: all aliases must be checked during argument validation
    env.Alias("build", program)
    env.Alias("size", env.Size(program))
    env.Alias("gdb", env.DebugGdbRemote(program))
    env.Alias("run", [env.ProgramOpenOcd(program)])
    env.Alias("all", ["build", "size"])
    env.Default("all")  # For the hardware target env, "all" runs if you don't
                        # specify anything (i.e. just type "scons")
elif args["TARGET_ENV"] == "tests":
    env.Append(toolpath=[abspath(r"taproot/build_tools")])
    env.Tool("run_gcov")

    program = env.Program(target=env["CONFIG_PROJECT_NAME"]+"-tests.elf", source=sources)

    # Add target environment-specific SCons aliases
    # WARNING: all aliases must be checked during argument validation
    env.Alias("build-tests", program)
    env.Alias("run-tests", env.Run(program))
    env.Alias("run-tests-gcov", [env.RunGCOV(program)])

else:
    program = env.Program(target=env["CONFIG_PROJECT_NAME"]+".elf", source=sources)

    # Add target environment-specific SCons aliases
    # WARNING: all aliases must be checked during argument validation
    env.Alias("build-sim", program)
    env.Alias("run-sim", env.Run(program))
