#ifndef OLLAMA_H
#define OLLAMA_H

#include <gtk/gtk.h>
#include "../ui_chat.h"  // Ajout de cette ligne

// Structure pour stocker les données de requête Ollama dans un thread
struct ollama_request_data {
    char *prompt;
    GtkWidget *chat_view;
    char *full_response;
};

// Fonction pour envoyer une requête de chat à Ollama dans un thread (non-streaming)
void* query_ollama_thread(void *data);
gboolean display_thinking_message(gpointer user_data);
void parse_and_display_full_response(const char *full_response, GtkWidget *chat_view);
gboolean update_ia_response(gpointer user_data);
gboolean insert_text_safe(gpointer data);

#endif // OLLAMA_H