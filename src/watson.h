#ifndef X4WP_WATSON_H
#define X4WP_WATSON_H

#include <libxfce4panel/libxfce4panel.h>

G_BEGIN_DECLS

enum watson_icon
{
	ICON_ACTIVE,
	ICON_INACTIVE
};

enum watson_status
{
	STATUS_ACTIVE,
	STATUS_INACTIVE
};

struct watson_monitor
{
	GFile *file;
	GFileMonitor *monitor;
};

struct watson_plugin
{
	struct watson_monitor *monitor;
	GtkWidget *box;
	GtkWidget *evnt_box;
	GtkWidget *icon;
	XfcePanelPlugin *plugin;

	enum watson_status status;
};

struct watson_state
{
	enum watson_status status;
};

G_END_DECLS

#endif /* !defined(X4WP_WATSON_H) */
