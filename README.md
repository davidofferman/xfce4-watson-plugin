# xfce4-watson-plugin

A simple plugin for the xfce4-panel that displays your current
[Watson](https://github.com/TailorDev/Watson) status.

The plugin will displays either an idle or active icon. More details
about the Watson status can be seen in the tooltip by hovering over the
plugin in the panel.

## Prerequisites

The following libraries and system is required.

- `Linux kernel >= 2.6.13`
- `libxfce4panel-2.0 >= 4.12.0`
- `libxfce4ui-2 >= 4.12.0`
- `scons`
- `build-essential`

### Debian

The below command installs the required dependencies to compile the plugin
on Debian systems.

```
$ apt install \
	libxfce4ui-2-dev \
	libxfce4panel-2.0-dev \
	scons \
	build-essential
```

## Compiling

Below are available commands for the SCons build. The file
`xfce4-watson-plugin.conf` stores build and installation settings.

- `$ scons` - Build the plugin
- `$ scons --clean` - Clean up the build
- `$ scons --help` - Help about allowed variables
- `$ scons install` - Install the plugin to the system
- `$ scons --clean install` - Uninstall the plugin from the system

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE)
file for details
