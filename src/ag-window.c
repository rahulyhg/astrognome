#include <string.h>
#include <glib/gi18n.h>
#include <libgd/gd.h>

#include "ag-app.h"
#include "ag-window.h"

struct _AgWindowPrivate {
    GtkWidget *grid;
    GtkWidget *header_bar;
    GtkWidget *notebook;
    GtkWidget *name;
    GtkWidget *north_lat;
    GtkWidget *south_lat;
    GtkWidget *latitude;
    GtkWidget *east_long;
    GtkWidget *west_long;
    GtkWidget *longitude;
    GtkWidget *year;
    GtkWidget *month;
    GtkWidget *day;
    GtkWidget *hour;
    GtkWidget *minute;
    GtkWidget *second;
    GtkBuilder *builder;

    gint tab_chart;
    gint tab_aspects;
    gint tab_points;
    gint tab_edit;
};

G_DEFINE_TYPE(AgWindow, ag_window, GTK_TYPE_APPLICATION_WINDOW);

#define GET_PRIVATE(instance) (G_TYPE_INSTANCE_GET_PRIVATE((instance), AG_TYPE_WINDOW, AgWindowPrivate))

static void
gear_menu_cb(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
    GVariant *state;

    state = g_action_get_state(G_ACTION(action));
    g_action_change_state(G_ACTION(action), g_variant_new_boolean(!g_variant_get_boolean(state)));

    g_variant_unref(state);
}

static void
close_cb(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
    AgWindow *window = user_data;

    // TODO: Save unsaved changes!
    gtk_widget_destroy(GTK_WIDGET(window));
}

static void
set_tab_cb(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
    AgWindow *window = AG_WINDOW(user_data);
    const gchar *target = g_variant_get_string(parameter, NULL);
    gint target_tab = 0;

    g_action_change_state(G_ACTION(action), parameter);

    if (strcmp(target, "chart") == 0) {
        target_tab = window->priv->tab_chart;
    } else if (strcmp(target, "aspects") == 0) {
        target_tab = window->priv->tab_aspects;
    } else if (strcmp(target, "points") == 0) {
        target_tab = window->priv->tab_points;
    } else if (strcmp(target, "edit") == 0) {
        target_tab = window->priv->tab_edit;
    } else {
        g_warning("Unknown tab!");

        return;
    }

    gtk_notebook_set_current_page(GTK_NOTEBOOK(window->priv->notebook), target_tab);
}

static void
change_tab_cb(GSimpleAction *action, GVariant *state, gpointer user_data)
{
    g_simple_action_set_state(action, state);
}

static GActionEntry win_entries[] = {
    { "close",      close_cb,     NULL, NULL,      NULL },
    { "gear-menu",  gear_menu_cb, NULL, "false",   NULL },
    { "tab-change", set_tab_cb,   "s",  "'edit'", change_tab_cb },
};

static void
ag_window_init(AgWindow *window)
{
    AgWindowPrivate *priv;
    GtkAccelGroup *accel_group;
    GError *err = NULL;

    window->priv = priv = GET_PRIVATE(window);

    gtk_window_set_hide_titlebar_when_maximized(GTK_WINDOW(window), TRUE);

    priv->builder = gtk_builder_new();

    if (!gtk_builder_add_from_resource(priv->builder, "/eu/polonkai/gergely/astrognome/astrognome.ui", &err)) {
        g_error("Cannot add resource to builder: '%s'", (err) ? err->message : "unknown error");
        g_clear_error(&err);
    }

    priv->grid = gtk_grid_new();
    gtk_widget_show(priv->grid);

    gtk_container_add(GTK_CONTAINER(window), priv->grid);

    g_action_map_add_action_entries(G_ACTION_MAP(window), win_entries, G_N_ELEMENTS(win_entries), window);

    accel_group = gtk_accel_group_new();
    gtk_window_add_accel_group(GTK_WINDOW(window), accel_group);
}

static void
ag_window_class_init(AgWindowClass *klass)
{
    g_type_class_add_private(klass, sizeof(AgWindowPrivate));
}

static GtkWidget *
notebook_edit(AgWindow *window)
{
    GtkWidget *grid;
    GtkWidget *label;
    AgWindowPrivate *priv = window->priv;

    grid = gtk_grid_new();

    label = gtk_label_new(_("Name"));
    gtk_grid_attach(GTK_GRID(grid), label, 0, 0, 1, 1);

    priv->name = gtk_entry_new();
    gtk_grid_attach(GTK_GRID(grid), priv->name, 1, 0, 6, 1);

    label = gtk_label_new(_("Country"));
    gtk_grid_attach(GTK_GRID(grid), label, 0, 1, 1, 1);

    label = gtk_label_new(_("City"));
    gtk_grid_attach(GTK_GRID(grid), label, 0, 2, 1, 1);

    label = gtk_label_new(_("Latitude"));
    gtk_grid_attach(GTK_GRID(grid), label, 0, 3, 2, 1);

    priv->north_lat = gtk_radio_button_new_with_label(NULL, _("North"));
    gtk_grid_attach(GTK_GRID(grid), priv->north_lat, 0, 4, 1, 1);

    priv->south_lat = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(priv->north_lat), _("South"));
    gtk_grid_attach(GTK_GRID(grid), priv->south_lat, 1, 4, 1, 1);

    priv->latitude = gtk_spin_button_new_with_range(0.0, 90.0, 0.1);
    gtk_spin_button_set_digits(GTK_SPIN_BUTTON(priv->latitude), 6);
    gtk_grid_attach(GTK_GRID(grid), priv->latitude, 0, 5, 2, 1);

    label = gtk_label_new(_("Longitude"));
    gtk_grid_attach(GTK_GRID(grid), label, 2, 3, 2, 1);

    priv->east_long = gtk_radio_button_new_with_label(NULL, _("East"));
    gtk_grid_attach(GTK_GRID(grid), priv->east_long, 2, 4, 1, 1);

    priv->west_long = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(priv->east_long), _("West"));
    gtk_grid_attach(GTK_GRID(grid), priv->west_long, 3, 4, 1, 1);

    priv->longitude = gtk_spin_button_new_with_range(0.0, 180.0, 0.1);
    gtk_spin_button_set_digits(GTK_SPIN_BUTTON(priv->longitude), 6);
    gtk_grid_attach(GTK_GRID(grid), priv->longitude, 2, 5, 2, 1);

    label = gtk_label_new(_("Year"));
    gtk_grid_attach(GTK_GRID(grid), label, 4, 1, 1, 1);

    priv->year = gtk_spin_button_new_with_range(G_MININT, G_MAXINT, 1);
    gtk_spin_button_set_digits(GTK_SPIN_BUTTON(priv->year), 0);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(priv->year), 0);
    gtk_grid_attach(GTK_GRID(grid), priv->year, 4, 2, 1, 1);

    label = gtk_label_new(_("Month"));
    gtk_grid_attach(GTK_GRID(grid), label, 5, 1, 1, 1);

    priv->month = gtk_spin_button_new_with_range(1, 12, 1);
    gtk_spin_button_set_digits(GTK_SPIN_BUTTON(priv->month), 0);
    gtk_grid_attach(GTK_GRID(grid), priv->month, 5, 2, 1, 1);

    label = gtk_label_new(_("Day"));
    gtk_grid_attach(GTK_GRID(grid), label, 6, 1, 1, 1);

    priv->day = gtk_spin_button_new_with_range(1, 31, 1);
    gtk_spin_button_set_digits(GTK_SPIN_BUTTON(priv->day), 0);
    gtk_grid_attach(GTK_GRID(grid), priv->day, 6, 2, 1, 1);

    label = gtk_label_new(_("Hour"));
    gtk_grid_attach(GTK_GRID(grid), label, 4, 3, 1, 1);

    priv->hour = gtk_spin_button_new_with_range(0, 23, 1);
    gtk_spin_button_set_digits(GTK_SPIN_BUTTON(priv->hour), 0);
    gtk_grid_attach(GTK_GRID(grid), priv->hour, 4, 4, 1, 1);

    label = gtk_label_new(_("Minute"));
    gtk_grid_attach(GTK_GRID(grid), label, 5, 3, 1, 1);

    priv->minute = gtk_spin_button_new_with_range(0, 59, 1);
    gtk_spin_button_set_digits(GTK_SPIN_BUTTON(priv->minute), 0);
    gtk_grid_attach(GTK_GRID(grid), priv->minute, 5, 4, 1, 1);

    label = gtk_label_new(_("Second"));
    gtk_grid_attach(GTK_GRID(grid), label, 6, 3, 1, 1);

    priv->second = gtk_spin_button_new_with_range(0, 61, 1);
    gtk_spin_button_set_digits(GTK_SPIN_BUTTON(priv->second), 0);
    gtk_grid_attach(GTK_GRID(grid), priv->second, 6, 4, 1, 1);

    gtk_widget_show_all(grid);

    return grid;
}

static void
window_populate(AgWindow *window)
{
    AgWindowPrivate *priv = window->priv;
    GtkWidget *menu_button;
    GObject *menu;
    GtkWidget *placeholder;
    GtkWidget *tab;

    priv->header_bar = gd_header_bar_new();
    menu_button = gd_header_menu_button_new();
    gd_header_button_set_symbolic_icon_name(GD_HEADER_BUTTON(menu_button), "emblem-system-symbolic");
    gtk_actionable_set_action_name(GTK_ACTIONABLE(menu_button), "win.gear-menu");

    gd_header_bar_pack_end(GD_HEADER_BAR(priv->header_bar), menu_button);

    gtk_grid_attach(GTK_GRID(priv->grid), priv->header_bar, 0, 0, 1, 1);

    menu = gtk_builder_get_object(priv->builder, "window-menu");
    gtk_menu_button_set_menu_model(GTK_MENU_BUTTON(menu_button), G_MENU_MODEL(menu));

    priv->notebook = gtk_notebook_new();
    gtk_notebook_set_show_tabs(GTK_NOTEBOOK(priv->notebook), FALSE);
    gtk_grid_attach(GTK_GRID(priv->grid), priv->notebook, 0, 1, 1, 1);

    tab = notebook_edit(window);
    priv->tab_edit = gtk_notebook_append_page(GTK_NOTEBOOK(priv->notebook), tab, NULL);

    placeholder = gtk_label_new("PLACEHOLDER FOR THE CHART WEBKIT");
    priv->tab_chart = gtk_notebook_append_page(GTK_NOTEBOOK(priv->notebook), placeholder, NULL);

    placeholder = gtk_label_new("PLACEHOLDER FOR THE ASPECTS TABLE");
    priv->tab_aspects = gtk_notebook_append_page(GTK_NOTEBOOK(priv->notebook), placeholder, NULL);

    placeholder = gtk_label_new("PLACEHOLDER FOR THE POINTS TABLES");
    priv->tab_points = gtk_notebook_append_page(GTK_NOTEBOOK(priv->notebook), placeholder, NULL);

    /* TODO: change to the Chart tab if we are opening an existing chart! */
    gtk_notebook_set_current_page(GTK_NOTEBOOK(priv->notebook), priv->tab_edit);

    gtk_widget_show_all(priv->grid);
}

GtkWidget *
ag_window_new(AgApp *app)
{
    AgWindow *window;

    window = g_object_new(AG_TYPE_WINDOW, NULL);

    gtk_window_set_application(GTK_WINDOW(window), GTK_APPLICATION(app));

    window_populate(window);

    gtk_window_set_icon_name(GTK_WINDOW(window), "astrognome");

    return GTK_WIDGET(window);
}

