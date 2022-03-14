# name of your application
APPLICATION = dsmr_splitter

# If no BOARD is found in the environment, use this default:
BOARD ?= native

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../..

USEMODULE += dsmr
USEMODULE += shell
USEMODULE += shell_commands
USEMODULE += ps

FEATURES_REQUIRED += periph_gpio periph_gpio_irq
FEATURES_REQUIRED += periph_uart

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
DEVELHELP ?= 1

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

include $(RIOTBASE)/Makefile.include
