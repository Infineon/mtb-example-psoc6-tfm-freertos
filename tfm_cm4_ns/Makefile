################################################################################
# \file Makefile
# \version 1.0
#
# \brief
# Makefile for configurating the Cortex-M4 core project
#
################################################################################
# \copyright
# Copyright 2023, Cypress Semiconductor Corporation (an Infineon company)
# SPDX-License-Identifier: Apache-2.0
# 
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
# 
#     http://www.apache.org/licenses/LICENSE-2.0
# 
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
################################################################################

include ../common.mk

################################################################################
# Basic Configuration
################################################################################

# Type of ModusToolbox Makefile Options include:
#
# COMBINED    -- Top Level Makefile usually for single standalone application
# APPLICATION -- Top Level Makefile usually for multi project application
# PROJECT     -- Project Makefile under Application
#
MTB_TYPE=PROJECT

# Name of application (used to derive name of final linked file).
#
# If APPNAME is edited, ensure to update or regenerate launch
# configurations for your IDE.
APPNAME=tfm_cm4_ns

# If set to "true" or "1", display full command-lines when building.
VERBOSE=


################################################################################
# Advanced Configuration
################################################################################

# Specifies which RTOS to use (FREERTOS, RTX, THREADX)
RTOS?=FREERTOS

# Enable optional code that is ordinarily disabled by default.
#
# Available components depend on the specific targeted hardware and firmware
# in use. In general, if you have
#
#    COMPONENTS=foo bar
#
# ... then code in directories named COMPONENT_foo and COMPONENT_bar will be
# added to the build
#
ifeq ($(RTOS),FREERTOS)
COMPONENTS+= FREERTOS RTOS_AWARE
else ifeq ($(RTOS),RTX)
COMPONENTS+= RTX RTOS_AWARE
else ifeq ($(RTOS),THREADX)
COMPONENTS+= THREADX RTOS_AWARE
endif

# Like COMPONENTS, but disable optional code that was enabled by default.
DISABLE_COMPONENTS=

# By default the build system automatically looks in the Makefile's directory
# tree for source code and builds it. The SOURCES variable can be used to
# manually add source code to the build process from a location not searched
# by default, or otherwise not found by the build system.
SOURCES=

# Like SOURCES, but for include directories. Value should be paths to
# directories (without a leading -I).
INCLUDES=

# Add additional defines to the build process (without a leading -D).
DEFINES=

# Select softfp or hardfp floating point. Default is softfp.
VFP_SELECT=

# Additional / custom C compiler flags.
#
# NOTE: Includes and defines should use the INCLUDES and DEFINES variable
# above.
CFLAGS=

# Additional / custom C++ compiler flags.
#
# NOTE: Includes and defines should use the INCLUDES and DEFINES variable
# above.
CXXFLAGS=

# Additional / custom assembler flags.
#
# NOTE: Includes and defines should use the INCLUDES and DEFINES variable
# above.
ASFLAGS=

# Additional / custom linker flags.
LDFLAGS=

# Additional / custom libraries to link in to the application.
LDLIBS=

# Path to the linker script to use (if empty, use the default linker script).
LINKER_SCRIPT=

# Custom pre-build commands to run.
PREBUILD=

# Custom post-build commands to run.
POSTBUILD=

################################################################################
# Paths
################################################################################

# Relative path to the project directory (default is the Makefile's directory).
#
# This controls where automatic source code discovery looks for code.
CY_APP_PATH=

# Relative path to the shared repo location.
#
# All .mtb files have the format, <URI>#<COMMIT>#<LOCATION>. If the <LOCATION> field 
# begins with $$ASSET_REPO$$, then the repo is deposited in the path specified by 
# the CY_GETLIBS_SHARED_PATH variable. The default location is one directory level 
# above the current app directory.
# This is used with CY_GETLIBS_SHARED_NAME variable, which specifies the directory name.
CY_GETLIBS_SHARED_PATH=../..

# Directory name of the shared repo location.
#
CY_GETLIBS_SHARED_NAME=mtb_shared

################################################################################
# TF-M non-secure image
################################################################################
# Please comment the line "COMPONENTS+= TFM_S_SRC" if you want to rollback to the TF-M binary
COMPONENTS+= TFM_S_SRC

CORE=CM4
CORE_NAME=CM4_1

ifeq ($(findstring TFM_S_SRC,$(COMPONENTS)),TFM_S_SRC)

# Path to TF-M Secure Application
TFM_S_APP_PATH=../tfm_cm0_s

# Path to PSoC64 policy which contains encryption keys
# Default: $(SEARCH_trusted-firmware-m)/security/policy/policy_multi_CM0_CM4_tfm.json
# POST_BUILD_POLICY_PATH=$(SEARCH_trusted-firmware-m)/security/policy/policy_multi_CM0_CM4_tfm.json

ifneq ($(wildcard libs/mtb.mk),)
# Load location of trusted-firmware-m library
OLD_SEARCH:=$(SEARCH)
include libs/mtb.mk
SEARCH=$(OLD_SEARCH)

ifneq ($(wildcard $(SEARCH_trusted-firmware-m)/COMPONENT_TFM_S_SRC/make/tfm_ns.mk),)
# Include TF-M Makefile for non-secure image
include $(SEARCH_trusted-firmware-m)/COMPONENT_TFM_S_SRC/make/tfm_ns.mk

build_proj: $(LINKER_SCRIPT_PARSED)

endif # tfm_ns.mk
endif # mtb.mk

################################################################################
# TF-M configuration
################################################################################

ifeq ($(RTOS),)
# Use TF-M mailbox without RTOS
DEFINES+= NUM_MAILBOX_QUEUE_SLOT=1
else
# Use TF-M mailbox with RTOS
DEFINES+= TFM_MULTI_CORE_NS_OS
endif

# Defines stack size
DEFINES+= NS_MSP_STACK_SIZE=0x1000

endif

################################################################################
# Start MTB build
################################################################################

include $(CY_TOOLS_DIR)/make/start.mk
