#include <gtk/gtk.h>
#include <libxfce4util/libxfce4util.h>
#include <libxfce4panel/libxfce4panel.h>
#include <mjson/mjson.h>

#include "utils.h"
#include "watson.h"


/**
 * config_path
 * -----------
 * Returns a NULL terminated string with the path to the watson
 * configuration directory. The returned string must be free'd with
 * `g_free()`.
 *
 * @param const char * Optional file name inside Watson config
 * @return char *
 */
static gchar *
config_path(const gchar *file)
{
	const gchar *config_dir;
	gchar *path;

	config_dir = g_get_user_config_dir();
	path = g_build_filename(config_dir, "watson", file, NULL);

	return path;
}

/**
 * icon_new
 * --------
 * Creates a GTK Widget of the current icon type.
 *
 * @param enum watson_icon
 * @param int
 * @return GtkWidget *
 */
static GtkWidget *
icon_new(enum watson_icon icon, int size)
{
	GError *error = NULL;
	GdkPixbuf *pix = NULL;
	gchar *file = NULL;
	GtkWidget *image;

	switch (icon) {
	case ICON_ACTIVE:
		file = g_build_filename(WATSON_DATADIR, "active.svg", NULL);
		break;
	case ICON_INACTIVE:
		file = g_build_filename(WATSON_DATADIR, "inactive.svg", NULL);
		break;
	default:
		/* do nothing */
		break;
	}

	if(file) {
		pix = gdk_pixbuf_new_from_file_at_scale(file, size, size, FALSE, &error);
	}

	if (pix) {
		image = gtk_image_new_from_pixbuf(pix);
		g_object_unref(pix);
	} else {
		if (file) {
			DBG("gdk_pixbuf_new_from_file_at_scale(%s): %s", file, error->message);
			g_error_free(error);
		}

		image = gtk_image_new_from_icon_name("image-missing", 0);
	}

	if (file) {
		g_free(file);
	}

	return image;
}

/**
 * icon_update
 * ------------------
 * Updates the displayed icon in the plugin.
 *
 * @param struct watson_plugin *
 * @param enum WatsonIconType
 * @return void
 */
static void
icon_update(struct watson_plugin *watson, enum watson_icon icon)
{
	int size;

	if (watson->icon) {
		gtk_widget_destroy(watson->icon);
	}

	size = plugin_get_icon_size(watson->plugin);
	watson->icon = icon_new(icon, size);

	gtk_container_add(GTK_CONTAINER(watson->box), watson->icon);
	gtk_widget_show(watson->icon);
}

/**
 * state_free
 * ----------
 * Free memory for `struct watson_state`.
 *
 * @param WatsonState *
 * @return void
 */
static void
state_free(struct watson_state *state)
{
	g_slice_free(struct watson_state, state);
}

/**
 * state_new
 * ---------
 * get the state of the external Watson program. Free using
 * `state_free`.
 *
 * @return WatsonState *
 */
static struct watson_state *
state_new(void)
{
	size_t content_len;
	gchar *path, *content;
	int json_size, scontent_len;
	const char *json_ptr;
	enum mjson_tok json_tok;
	struct watson_state *state;

	state = g_slice_new0(struct watson_state);
	path = config_path("state");

	if (g_file_get_contents(path, &content, &content_len, NULL) == FALSE) {
		state_free(state);
		state = NULL;
		goto clean;
	}

	/**
	 * Parse JSON
	 */
	scontent_len = (content_len > INT_MAX ? INT_MAX : (int)content_len);
	json_tok = mjson_find(content, scontent_len, "$.project", &json_ptr, &json_size);
	if (json_tok == MJSON_TOK_STRING) {
		state->status = STATUS_ACTIVE;
	} else {
		state->status = STATUS_INACTIVE;
	}

	/**
	 * Cleanup
	 */
	clean:
	g_free(path);
	g_free(content);

	return state;
}

/**
 * watson_update
 * -------------
 * Updates the plugin based on it's currently saved state.
 *
 * @param struct watson_plugin *
 * @return void
 */
static void
watson_update(struct watson_plugin *watson)
{
	enum watson_icon icon;

	switch (watson->status) {
	case STATUS_ACTIVE:
		icon = ICON_ACTIVE;
		break;
	case STATUS_INACTIVE:
		icon = ICON_INACTIVE;
		break;
	default:
		icon = ICON_INACTIVE;
		break;
	}

	icon_update(watson, icon);
}

/**
 * watson_process
 * --------------
 * Processes external data to update the plugin state.
 *
 * @param struct watson_plugin *
 * @return void
 */
static void
watson_process(struct watson_plugin *watson)
{
	struct watson_state *state;

	state = state_new();
	if (!state) {
		watson->status = STATUS_INACTIVE;
	} else {
		watson->status = state->status;
	}

	state_free(state);
}

/**
 * watson_notify_event
 * ------------------
 * Callback for when the notify library. Handles events for the Watson
 * configuration files and then updates the plugin state.
 *
 * @param NotifyEvent *
 * @param struct watson_plugin *
 * @return void
 */
static void
watson_file_changed(GFileMonitor *monitor, GFile *file, GFile *other,
                    GFileMonitorEvent evtype, struct watson_plugin *watson)
{
	(void) monitor;
	(void) file;
	(void) other;
	(void) evtype;

	TRACE("file changed");

	watson_process(watson);
	watson_update(watson);
}


/**
 * monitor_free
 * ------------
 * Free the monitor structure
 *
 * @param struct watson_monitor *
 * @return void
 */
void
monitor_free(struct watson_monitor *monitor)
{
	if (!monitor) { return; }

	g_object_unref(monitor->file);
	g_object_unref(monitor->monitor);

	g_slice_free(struct watson_monitor, monitor);
}

/**
 * monitor_new
 * -----------
 * Monitor a file
 *
 * @param gchar *
 * @return GFileMonitor
 */
static struct watson_monitor *
monitor_new(gchar *filename, GCallback cb, void *userp)
{
	struct watson_monitor *wm;
	GError *error = NULL;

	wm = g_slice_new0(struct watson_monitor);

	wm->file = g_file_new_for_path(filename);
	wm->monitor = g_file_monitor(wm->file, G_FILE_MONITOR_NONE, NULL, &error);
	if (!wm->monitor) {
		fprintf(stderr, "unable to monitor %s: %s\n", filename, error->message);
		g_error_free(error);
		monitor_free(wm);
		return NULL;
	}

	g_signal_connect(
		G_OBJECT(wm->monitor),
		"changed",
		cb,
		userp);

	return wm;
}

/**
 * watson_new
 * ----------
 * Creates the Watson Plugin for the XFCE4 Panel.
 *
 * @param XfcePanelPlugin *
 * @return struct watson_plugin *
 */
static struct watson_plugin *
watson_new(XfcePanelPlugin *plugin)
{
	GtkOrientation orientation;
	struct watson_plugin *watson;
	gchar *watson_state_file;

	watson = g_slice_new0(struct watson_plugin);
	watson->plugin = plugin;
	watson->status = STATUS_INACTIVE;

	/**
	 * File Monitor
	 */
	watson_state_file = config_path("state");
	watson->monitor = monitor_new(
		watson_state_file,
		G_CALLBACK(watson_file_changed),
		watson);

	/**
	 * Panel Variables
	 */
	orientation = xfce_panel_plugin_get_orientation(plugin);

	/**
	 * Create Container
	 */
	watson->evnt_box = gtk_event_box_new();
	watson->box = gtk_box_new(orientation, 2);

	gtk_widget_set_halign(watson->box, GTK_ALIGN_CENTER);
	gtk_widget_set_valign(watson->box, GTK_ALIGN_CENTER);

	gtk_container_add(GTK_CONTAINER(watson->evnt_box), watson->box);
	gtk_widget_show_all(watson->evnt_box);

	/**
	 * Cleanup
	 */
	g_free(watson_state_file);
	return watson;
}

/**
 * watson_free
 * -----------
 * Cleans up the Watson Plugin that was created using `watson_new`
 *
 * @param XfcePanelPlugin *
 * @param struct watson_plugin *
 * @return void
 */
static void
watson_free(XfcePanelPlugin *plugin, struct watson_plugin *watson)
{
	(void)plugin;

	GtkWidget *dialog;

	dialog = g_object_get_data(G_OBJECT(watson->plugin), "dialog");
	if (G_UNLIKELY(dialog != NULL)) {
		gtk_widget_destroy(dialog);
	}

	monitor_free(watson->monitor);

	gtk_widget_destroy(watson->box);
	g_slice_free(struct watson_plugin, watson);
}

/**
 * plugin_size_changed
 * -------------------
 * Callback for when the XFCE4 Panel size has changed.
 *
 * @param XfcePanelPlugin *
 * @param int
 * @param struct watson_plugin *
 * @return gboolean
 */
static gboolean
plugin_size_changed(XfcePanelPlugin *plugin, int size, struct watson_plugin *watson)
{
	(void)plugin;
	(void)size;

	watson_update(watson);
	return TRUE;
}


/**
 * plugin_construct
 * ----------------
 * The entry point for the XFCE4 Panel Plugin.
 *
 * @param XfcePanelPlugin *
 * @return void
 */
static void
plugin_construct(XfcePanelPlugin *plugin)
{
	struct watson_plugin *watson;

	/**
	 * Configure XFCE4 Panel Plugin
	 */
	xfce_panel_plugin_set_small(plugin, TRUE);

	/**
	 * Initialize Plugin
	 */
	watson = watson_new(plugin);
	gtk_container_add(GTK_CONTAINER(plugin), watson->evnt_box);

	watson_process(watson);
	watson_update(watson);

	/**
	 * Connect to signals
	 */
	g_signal_connect(
		G_OBJECT(plugin),
		"free-data",
		G_CALLBACK(watson_free),
		watson);

	g_signal_connect(
		G_OBJECT(plugin),
		"size-changed",
		G_CALLBACK(plugin_size_changed),
		watson);
}

/**
 * Register the plugin
 */
XFCE_PANEL_PLUGIN_REGISTER(plugin_construct);
