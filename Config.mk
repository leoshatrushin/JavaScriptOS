# --- User Defined Configurations ----

# Kernel Configuration

USE_FRAMEBUFFER ?= 1
SMALL_PAGES ?= 0

# PIN_LOGO set to 0 if we want the logo to be scrolled.
PIN_LOGO ?= 1

# Build Configuration

TOOLPREFIX ?= x86_64-elf-
BUILD_FOLDER ?= build
FONT_FOLDER ?= fonts

# Image Base Name

IMAGE_BASE_NAME ?= JavaScriptOS

