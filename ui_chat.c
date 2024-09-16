#include <gtk/gtk.h>
#include "ui_chat.h"
#include "ollama/ollama.h"

GtkWidget *chat_list;
static GtkWidget *input_box;

typedef struct {
    GtkWidget *label;
    gboolean is_user;
} MessageWidget;

void add_message_to_chat(const gchar *text, gboolean is_user) {
    GtkWidget *row = gtk_list_box_row_new();
    GtkWidget *row_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    GtkWidget *bubble = gtk_label_new(text);
    
    gtk_label_set_line_wrap(GTK_LABEL(bubble), TRUE);
    gtk_label_set_max_width_chars(GTK_LABEL(bubble), 50);
    
    GtkStyleContext *context = gtk_widget_get_style_context(bubble);
    gtk_style_context_add_class(context, "message-bubble");
    gtk_style_context_add_class(context, is_user ? "user-message" : "assistant-message");
    
    if (is_user) {
        gtk_box_pack_start(GTK_BOX(row_box), bubble, FALSE, FALSE, 0);
        gtk_box_pack_end(GTK_BOX(row_box), gtk_label_new(""), TRUE, TRUE, 0);
    } else {
        gtk_box_pack_start(GTK_BOX(row_box), gtk_label_new(""), TRUE, TRUE, 0);
        gtk_box_pack_end(GTK_BOX(row_box), bubble, FALSE, FALSE, 0);
    }
    
    gtk_container_add(GTK_CONTAINER(row), row_box);
    gtk_style_context_add_class(gtk_widget_get_style_context(row), "message-row");
    
    gtk_list_box_insert(GTK_LIST_BOX(chat_list), row, -1);
    gtk_widget_show_all(row);
    
    // Scroll to bottom
    GtkAdjustment *adj = gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(gtk_widget_get_ancestor(chat_list, GTK_TYPE_SCROLLED_WINDOW)));
    gtk_adjustment_set_value(adj, gtk_adjustment_get_upper(adj) - gtk_adjustment_get_page_size(adj));
}

static void on_send_message(GtkWidget *widget, gpointer user_data) {
    const gchar *text = gtk_entry_get_text(GTK_ENTRY(input_box));
    if (g_strcmp0(text, "") == 0) return;
    
    add_message_to_chat(text, TRUE);
    
    // Créer la structure de données pour la requête Ollama
    struct ollama_request_data *req_data = malloc(sizeof(struct ollama_request_data));
    req_data->prompt = g_strdup(text);
    req_data->chat_view = chat_list;
    
    // Lancer le thread pour la requête Ollama
    pthread_t thread_id;
    pthread_create(&thread_id, NULL, query_ollama_thread, (void *)req_data);
    pthread_detach(thread_id);
    
    // Effacer l'entrée utilisateur
    gtk_entry_set_text(GTK_ENTRY(input_box), "");
}

void create_chat_ui(GtkWidget *stack) {
    GtkWidget *chat_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_style_context_add_class(gtk_widget_get_style_context(chat_box), "chat-container");
    
    // Liste des messages
    chat_list = gtk_list_box_new();
    gtk_list_box_set_selection_mode(GTK_LIST_BOX(chat_list), GTK_SELECTION_NONE);
    
    GtkWidget *scroll_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scroll_window), chat_list);
    gtk_box_pack_start(GTK_BOX(chat_box), scroll_window, TRUE, TRUE, 0);
    
    // Zone de saisie et bouton d'envoi
    GtkWidget *input_area = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_style_context_add_class(gtk_widget_get_style_context(input_area), "input-area");
    
    input_box = gtk_entry_new();
    gtk_widget_set_name(input_box, "message-input");
    gtk_box_pack_start(GTK_BOX(input_area), input_box, TRUE, TRUE, 0);
    
    GtkWidget *send_button = gtk_button_new_with_label("Envoyer");
    gtk_widget_set_name(send_button, "send-button");
    g_signal_connect(send_button, "clicked", G_CALLBACK(on_send_message), NULL);
    gtk_box_pack_start(GTK_BOX(input_area), send_button, FALSE, FALSE, 0);
    
    gtk_box_pack_end(GTK_BOX(chat_box), input_area, FALSE, FALSE, 0);
    
    gtk_stack_add_titled(GTK_STACK(stack), chat_box, "chat", "Chat");
}

// Fonction à appeler pour ajouter une réponse d'Ollama
void add_ollama_response(const gchar *text) {
    add_message_to_chat(text, FALSE);
}