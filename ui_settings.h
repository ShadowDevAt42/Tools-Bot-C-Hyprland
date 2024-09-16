#ifndef UI_SETTINGS_H
#define UI_SETTINGS_H

#include <gtk/gtk.h>

void create_settings_ui(GtkWidget *stack);
void switch_theme(GtkWidget *widget, gpointer user_data); // Déclaration de switch_theme

#endif // UI_SETTINGS_H
