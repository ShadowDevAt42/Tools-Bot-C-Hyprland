#include <gtk/gtk.h>
#include "ui_chat.h"
#include "ui_settings.h"

// Prototypes
void on_menu_item_clicked(GtkWidget *widget, gpointer stack);

// Fonction pour activer l'UI principale
void activate_ui(GtkApplication *app, gpointer user_data) {
    GtkWidget *window;
    GtkWidget *main_box;
    GtkWidget *sidebar_box;
    GtkWidget *stack;
    GtkWidget *menu_button_chat, *menu_button_options;
    GtkCssProvider *cssProvider;

    // Fenêtre principale
    window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Ollama Assistant");
    gtk_window_set_default_size(GTK_WINDOW(window), 1200, 700); // Taille plus large pour une meilleure lisibilité

    // Conteneur principal : Horizontal box
    main_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_container_add(GTK_CONTAINER(window), main_box);

    // Sidebar (Menu latéral permanent)
    sidebar_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10); // Espacement plus large entre les éléments
    gtk_widget_set_size_request(sidebar_box, 150, -1); // Menu latéral plus large
    gtk_box_pack_start(GTK_BOX(main_box), sidebar_box, FALSE, FALSE, 0);

    // CSS pour styliser le menu
    cssProvider = gtk_css_provider_new();
    gtk_css_provider_load_from_path(cssProvider, "style.css", NULL); // Fichier de style dark uniquement
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(), GTK_STYLE_PROVIDER(cssProvider), GTK_STYLE_PROVIDER_PRIORITY_USER);

    // Stack pour changer de vue entre Chat et Options
    stack = gtk_stack_new();
    gtk_stack_set_transition_type(GTK_STACK(stack), GTK_STACK_TRANSITION_TYPE_SLIDE_LEFT_RIGHT);
    gtk_box_pack_start(GTK_BOX(main_box), stack, TRUE, TRUE, 0);

    // Bouton menu pour Chat avec icône
    menu_button_chat = gtk_button_new_with_label("Chat");
    GtkWidget *chat_icon = gtk_image_new_from_icon_name("mail-send", GTK_ICON_SIZE_BUTTON);
    gtk_button_set_image(GTK_BUTTON(menu_button_chat), chat_icon);
    gtk_button_set_relief(GTK_BUTTON(menu_button_chat), GTK_RELIEF_NONE); // Style plat
    g_signal_connect(menu_button_chat, "clicked", G_CALLBACK(on_menu_item_clicked), stack);
    gtk_box_pack_start(GTK_BOX(sidebar_box), menu_button_chat, FALSE, FALSE, 0);

    // Bouton menu pour Options avec icône
    menu_button_options = gtk_button_new_with_label("Options");
    GtkWidget *options_icon = gtk_image_new_from_icon_name("preferences-system", GTK_ICON_SIZE_BUTTON);
    gtk_button_set_image(GTK_BUTTON(menu_button_options), options_icon);
    gtk_button_set_relief(GTK_BUTTON(menu_button_options), GTK_RELIEF_NONE); // Style plat
    g_signal_connect(menu_button_options, "clicked", G_CALLBACK(on_menu_item_clicked), stack);
    gtk_box_pack_start(GTK_BOX(sidebar_box), menu_button_options, FALSE, FALSE, 0);

    // Charge l'interface du chat et des options
    create_chat_ui(stack);
    create_settings_ui(stack);

    gtk_widget_show_all(window);
}

// Fonction pour changer la vue en fonction du bouton de menu sélectionné
void on_menu_item_clicked(GtkWidget *widget, gpointer stack) {
    const gchar *name = gtk_button_get_label(GTK_BUTTON(widget));

    if (g_strcmp0(name, "Chat") == 0) {
        gtk_stack_set_visible_child_name(GTK_STACK(stack), "chat");
    } else if (g_strcmp0(name, "Options") == 0) {
        gtk_stack_set_visible_child_name(GTK_STACK(stack), "options");
    }
}
