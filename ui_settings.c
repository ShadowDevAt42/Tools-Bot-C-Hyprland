#include <gtk/gtk.h>
#include "ui_settings.h"

// Création de l'UI des options
void create_settings_ui(GtkWidget *stack) {
    GtkWidget *options_box;
    GtkWidget *theme_switch;

    // Vue des options
    options_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_stack_add_titled(GTK_STACK(stack), options_box, "options", "Options");

    // Bouton pour changer de thème dans les options
    theme_switch = gtk_button_new_with_label("Switch Theme");
    g_signal_connect(theme_switch, "clicked", G_CALLBACK(switch_theme), NULL);
    gtk_box_pack_start(GTK_BOX(options_box), theme_switch, TRUE, TRUE, 0);
}

// Fonction pour changer entre les thèmes sombre et clair
void switch_theme(GtkWidget *widget, gpointer user_data) {
    static gboolean dark_mode = FALSE;
    GtkCssProvider *cssProvider = gtk_css_provider_new();

    if (dark_mode) {
        gtk_css_provider_load_from_path(cssProvider, "light.css", NULL);
    } else {
        gtk_css_provider_load_from_path(cssProvider, "dark.css", NULL);
    }

    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(), GTK_STYLE_PROVIDER(cssProvider), GTK_STYLE_PROVIDER_PRIORITY_USER);

    dark_mode = !dark_mode;
}
