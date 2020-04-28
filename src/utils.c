#include <stdlib.h>
#include <libxfce4panel/libxfce4panel.h>

#include "utils.h"

/**
 * plugin_get_icon_size
 * --------------------
 * Returns the preferred size for icons for the current panel.
 *
 * @param XfcePanelPlugin *plugin
 * @return int
 */
int
plugin_get_icon_size(XfcePanelPlugin *plugin)
{
#if LIBXFCE4PANEL_CHECK_VERSION(4,14,0)
	return xfce_panel_plugin_get_icon_size(plugin);
#else /* LIBXFCE4PANEL_CHECK_VERSION(4,14,0) */
	int width, size, snrows;
	unsigned int nrows;

	size = xfce_panel_plugin_get_size(plugin);
	nrows = xfce_panel_plugin_get_nrows(plugin);
	snrows = (nrows > INT_MAX ? INT_MAX : (int)nrows);

	width = size / snrows;

	if (width <= 19) {
		return 12;
	} else if (width <= 27) {
		return 16;
	} else if (width <= 35) {
		return 24;
	} else if (width <= 41) {
		return 32;
	}

	return width - 4;
#endif /* LIBXFCE4PANEL_CHECK_VERSION(4,14,0) */
}
