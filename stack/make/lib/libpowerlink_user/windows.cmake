################################################################################
#
# Windows CMake configuration for openPOWERLINK user stack library
#
# Copyright (c) 2013, Bernecker+Rainer Industrie-Elektronik Ges.m.b.H. (B&R)
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
#     * Neither the name of the copyright holders nor the
#       names of its contributors may be used to endorse or promote products
#       derived from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDERS BE LIABLE FOR ANY
# DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
# ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
################################################################################

message( FATAL_ERROR "libpowerlink_user currently not supported on windows!")

# additional compiler flags
ADD_DEFINITIONS(-D_CONSOLE -DWPCAP -DHAVE_REMOTE)

SET (LIB_ARCH_SOURCES
     ${COMMON_SOURCE_DIR}/timer/timer-generic.c
     ${LIB_SOURCE_DIR}/sharedbuff/ShbIpc-Win32.c
     ${LIB_SOURCE_DIR}/trace/trace-windows.c
     ${LIB_SOURCE_DIR}/sharedbuff/SharedBuff.c
     ${LIB_SOURCE_DIR}/circbuf/circbuf-win32shm.c
     ${COMMON_SOURCE_DIR}/dll/dllcal-shb.c
     ${USER_SOURCE_DIR}/ctrl/ctrlucal-mem.c
     ${USER_SOURCE_DIR}/event/eventucal-shb.c
     ${USER_SOURCE_DIR}/errhnd/errhnducal-shb.c
     ${COMMON_SOURCE_DIR}/ctrl/ctrlcal-shb.c
     ${COMMON_SOURCE_DIR}/event/eventcal-shb.c
     ${USER_SOURCE_DIR}/pdo/pdoucal-shb.c
     ${USER_SOURCE_DIR}/pdo/pdoucalsync-null.c # replace by correct version
     )

#
# Set type of library
#
IF(CFG_X86_WINDOWS_DLL)
    SET(LIB_TYPE "SHARED")
ELSE(CFG_X86_WINDOWS_DLL)
    SET(LIB_TYPE "STATIC")
ENDIF(CFG_X86_WINDOWS_DLL)

IF (CMAKE_CL_64)
    LINK_DIRECTORIES(${LIB_SOURCE_DIR}/pcap/windows/WpdPack/Lib/x64)
ELSE (CMAKE_CL_64)
    LINK_DIRECTORIES(${LIB_SOURCE_DIR}/pcap/windows/WpdPack/Lib)
ENDIF (CMAKE_CL_64)

#
# Source include directories
#
INCLUDE_DIRECTORIES(${LIB_SOURCE_DIR}/pcap/windows/WpdPack/Include)

IF (CMAKE_CL_64)
    LINK_DIRECTORIES(${LIB_SOURCE_DIR}/pcap/windows/WpdPack/Lib/x64)
ELSE (CMAKE_CL_64)
    LINK_DIRECTORIES(${LIB_SOURCE_DIR}/pcap/windows/WpdPack/Lib)
ENDIF (CMAKE_CL_64)

SET(ARCH_LIBRARIES wpcap iphlpapi)

#
# Installation
#
IF (CFG_X86_WINDOWS_DLL)
    INSTALL(TARGETS powerlink_user RUNTIME DESTINATION bin)
ENDIF (CFG_X86_WINDOWS_DLL)
