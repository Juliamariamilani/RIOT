#!/bin/sh

#
# Unified Renode script for RIOT
#
# This script is supposed to be called from RIOT's make system, as it depends
# on certain environment variables.
#
# By default, it expects to find "dist/board.resc" in the board folder. This
# file is the entry point for emulation.
#
# It will start the Renode emulator, providing it with several environment
# variables:
# $BOARD               Selected board name
# $CPU                 Selected CPU name
# $ELFFILE             Full path to the image file
# $RIOTBASE            Full path to RIOT-OS directory
# $RIOTBOARD           Full path to board directory
# $RIOTCPU             Full path to CPU directory
#
# Global environment variables used:
# RENODE:              Renode command name, default: "renode"
# RENODE_BOARD_CONFIG: Renode configuration file name,
#                      default: "${BOARDSDIR}/${BOARD}/dist/board.resc"
# RENODE_BIN_CONFIG:   Renode intermediate configuration file name,
#                      default: "${BINDIR}/board.resc"
#
# @author       Bas Stottelaar <basstottelaar@gmail.com>

# Default path to Renode configuration file
: ${RENODE_BOARD_CONFIG:=${RIOTBOARD}/${BOARD}/dist/renode/board.resc}
# Default path to Renode intermediate configuration file
: ${RENODE_BIN_CONFIG:=${BINDIR}/renode.resc}
# Default Renode command
: ${RENODE:=renode}

#
# config test section.
#
test_config() {
    if [ ! -f "${RENODE_BOARD_CONFIG}" ]; then
        echo "Error: Unable to locate Renode board file"
        echo "       (${RENODE_BOARD_CONFIG})"
        exit 1
    fi
}

#
# helper section.
#
write_config() {
    echo "# RIOT-OS variables for Renode emulation" > "${RENODE_BIN_CONFIG}"
    echo "\$BOARD = '${BOARD}'" >> "${RENODE_BIN_CONFIG}"
    echo "\$CPU = '${CPU}'" >> "${RENODE_BIN_CONFIG}"
    echo "\$ELFFILE = @${ELFFILE}" >> "${RENODE_BIN_CONFIG}"
    echo "\$RIOTBASE = @${RIOTBASE}" >> "${RENODE_BIN_CONFIG}"
    echo "\$RIOTBOARD = @${RIOTBOARD}" >> "${RENODE_BIN_CONFIG}"
    echo "\$RIOTCPU = @${RIOTCPU}" >> "${RENODE_BIN_CONFIG}"
    echo "include @${RENODE_BOARD_CONFIG}" >> "${RENODE_BIN_CONFIG}"
}

#
# now comes the actual actions
#
do_write() {
    test_config
    write_config
    echo "Script written to '${RENODE_BIN_CONFIG}'"
}

do_start() {
    test_config
    write_config
    sh -c "${RENODE} '${RENODE_BIN_CONFIG}'"
}

#
# parameter dispatching
#
ACTION="$1"

case "${ACTION}" in
  write)
    echo "### Writing emulation script ###"
    do_write
    ;;
  start)
    echo "### Starting Renode ###"
    do_start
    ;;
  *)
    echo "Usage: $0 {write|start}"
    exit 2
    ;;
esac
