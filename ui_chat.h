#ifndef UI_CHAT_H
#define UI_CHAT_H

#include <gtk/gtk.h>

extern GtkWidget *chat_list;

void create_chat_ui(GtkWidget *stack);
void add_message_to_chat(const gchar *text, gboolean is_user);

#endif // UI_CHAT_H