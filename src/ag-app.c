#include <glib/gi18n.h>

#include "ag-app.h"
#include "config.h"

struct _AgAppPrivate {
};

G_DEFINE_TYPE(AgApp, ag_app, GTK_TYPE_APPLICATION);

GtkWindow *
ag_app_peek_first_window(AgApp *self)
{
    GList *l;

    for (l = gtk_application_get_windows(GTK_APPLICATION(self)); l; l = g_list_next(l)) {
        if (GTK_IS_WINDOW(l->data)) {
            return (GTK_WINDOW(l->data));
        }
    }

    ag_app_new_window(self);

    return ag_app_peek_first_window(self);
}

void
ag_app_new_window(AgApp *self)
{
    g_action_group_activate_action(G_ACTION_GROUP(self), "new-window", NULL);
}

void
ag_app_quit(AgApp *self)
{
    g_action_group_activate_action(G_ACTION_GROUP(self), "quit", NULL);
}

void
ag_app_raise(AgApp *self)
{
    g_action_group_activate_action(G_ACTION_GROUP(self), "raise", NULL);
}

static void
new_window_cb(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
    AgApp *self = AG_APP(user_data);
    GtkWidget *window;

    window = gtk_application_window_new(GTK_APPLICATION(self));
    gtk_application_add_window(GTK_APPLICATION(self), GTK_WINDOW(window));
    gtk_widget_show_all(window);
}

static void
preferences_cb(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
    //ag_preferences_show_dialog();
}

static void
about_cb(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
    const gchar *authors[] = {
        "Gergely Polonkai <gergely@polonkai.eu>",
        "Jean-André Santoni <jean.andre.santoni@gmail.com>",
        NULL
    };

    const gchar **documentors = NULL;
    const gchar *translator_credits = _("translator_credits");

    /* i18n: Please don't translate "Astrognome" (it's marked as translatable for transliteration only */
    gtk_show_about_dialog(NULL,
            "name", _("Astrognome"),
            "version", PACKAGE_VERSION,
            "comments", _("Astrologers' software for GNOME"),
            "authors", authors,
            "documentors", documentors,
            "translator_credits", ((strcmp(translator_credits, "translator_credits") != 0) ? translator_credits : NULL),
            "website", PACKAGE_URL,
            "website-label", _("Astrognome Website"),
            "logo-icon-name", PACKAGE_TARNAME,
            NULL);
}

static void
quit_cb(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
    AgApp *self = AG_APP(user_data);
    GList *l;

    while ((l = gtk_application_get_windows(GTK_APPLICATION(self)))) {
        gtk_application_remove_window(GTK_APPLICATION(self), GTK_WINDOW(l->data));
    }
}

static void
raise_cb(GSimpleAction *action, GVariant *parameter, gpointer user_data)
{
    AgApp *self = AG_APP(user_data);
    GtkWindow *window;

    window = ag_app_peek_first_window(self);
    gtk_window_present(window);
}

static GActionEntry app_entries[] = {
    { "new-window",  new_window_cb,  NULL, NULL, NULL },
    { "preferences", preferences_cb, NULL, NULL, NULL },
    { "about",       about_cb,       NULL, NULL, NULL },
    { "quit",        quit_cb,        NULL, NULL, NULL },
    { "raise",       raise_cb,       NULL, NULL, NULL },
};

static void
setup_actions(AgApp *self)
{
    g_action_map_add_action_entries(G_ACTION_MAP(self), app_entries, G_N_ELEMENTS(app_entries), self);
}

static void
setup_accelerators(AgApp *self)
{
    gtk_application_add_accelerator(GTK_APPLICATION(self), "<Primary>w", "win.close",     NULL);
    gtk_application_add_accelerator(GTK_APPLICATION(self), "F10",        "win.gear-menu", NULL);
}

static void
setup_menu(AgApp *self)
{
    GtkBuilder *builder;
    GMenuModel *model;
    GError *err = NULL;

    builder = gtk_builder_new();

    if (!gtk_builder_add_from_resource(builder, "/eu/polonkai/gergely/astrognome/astrognome.ui", &err)) {
        g_error("%s", (err) ? err->message : "unknown error");
    }

    model = G_MENU_MODEL(gtk_builder_get_object(builder, "app-menu"));
    gtk_application_set_app_menu(GTK_APPLICATION(self), model);

    g_object_unref(builder);
}

static void
startup(GApplication *app)
{
    AgApp *self = AG_APP(app);

    G_APPLICATION_CLASS(ag_app_parent_class)->startup(app);

    setup_actions(self);
    setup_menu(self);
    setup_accelerators(self);
}

AgApp *
ag_app_new(void)
{
    AgApp *app;

    /* i18n: Please don't translate "Astrognome" (it's marked as translatable for transliteration only */
    g_set_application_name(_("Astrognome"));

    app = g_object_new(AG_TYPE_APP,
            "application-id",   "eu.polonkai.gergely.Astrognome",
            "flags",            G_APPLICATION_FLAGS_NONE,
            "register-session", TRUE,
            NULL);

    return app;
}

static void
ag_app_init(AgApp *self)
{
}

static void
ag_app_class_init(AgAppClass *klass)
{
    GApplicationClass *application_class = G_APPLICATION_CLASS(klass);

    application_class->startup = startup;
}

