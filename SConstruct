#!python
import os
import sys

CFLAGS_WARNING = " ".join([
		"-Wall",
		"-Wextra",
		"-Wfloat-equal",
		"-Wundef",
		"-Wshadow",
		"-Wpointer-arith",
		"-Wcast-align",
		"-Wstrict-prototypes",
		"-Wwrite-strings",
		"-Waggregate-return",
		"-Wcast-qual",
		"-Wswitch-default",
		"-Wswitch-enum",
		"-Wconversion",
		"-Wunreachable-code",
		"-Wformat=2",
		"-Werror-implicit-function-declaration"
	])

# SConstruct Environments
env = Environment()
env.Append(LINKFLAGS = "-std=c99 -ansi")
env.Append(CFLAGS = CFLAGS_WARNING)
env.Append(CFLAGS = "-isystem third-party")
env.ParseConfig("pkg-config --cflags --libs libxfce4panel-2.0 libxfce4ui-2")

# Command Line Variables
opts = Variables("xfce4-watson-plugin.conf", ARGUMENTS)
opts.Add(PathVariable("PREFIX", "Directory to install under", "/usr", PathVariable.PathIsDir))
opts.Update(env)

# Help Text
Help(opts.GenerateHelpText(env))

# Installation Paths
idir_prefix = '${PREFIX}'
idir_data   = "${PREFIX}/share"
idir_lib    = "${PREFIX}/lib"
idir_icon   = "${PREFIX}/local/share"

wdir_data = os.path.join(idir_data, "xfce4", "watson")

# Defines
env.Append(CPPDEFINES = "WATSON_DATADIR=\\\"" + wdir_data + "\\\"")

# Build Plugin
src_files = Glob("src/*.c")
libwatson = env.SharedLibrary("watson", src_files)

# Install
env.Install(
	os.path.join(idir_lib, "xfce4", "panel-plugins"),
	libwatson)

env.Install(
	os.path.join(idir_data, "xfce4", "panel", "plugins"),
	"#resources/watson.desktop")

env.Install(wdir_data, Glob("#resources/data/*"))

for icon in Glob("#resources/icons/*/*", strings=True):
	index_lo = icon.find("/icons/") + len("/icons/")
	index_hi = icon.rfind("/")
	icon_size = icon[index_lo:index_hi]
	icon_name = icon[index_hi+1:]
	dest = os.path.join(idir_icon, "icons", "hicolor", icon_size, "apps", icon_name)

	env.InstallAs(dest, icon)

	env.AddPostAction(
		dest,
		"gtk-update-icon-cache -f -t " + os.path.join(idir_icon, "icons", "hicolor"))

env.Alias("install", idir_prefix)

