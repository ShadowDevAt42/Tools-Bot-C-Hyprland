#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <curl/curl.h>
#include <json-c/json.h>
#include <gtk/gtk.h>
#include "../ui_chat.h"
#include "ollama.h"


// Structure pour stocker la réponse de Ollama
struct memory {
    char *response;
    size_t size;
};


// Fonction pour gérer la réponse de Ollama, écrire les données dans un buffer
static size_t write_callback(void *data, size_t size, size_t nmemb, void *userp) {
    size_t total_size = size * nmemb;
    struct memory *mem = (struct memory *)userp;

    char *ptr = realloc(mem->response, mem->size + total_size + 1);
    if (ptr == NULL) {
        printf("Erreur d'allocation mémoire\n");
        return 0;
    }

    mem->response = ptr;
    memcpy(&(mem->response[mem->size]), data, total_size);
    mem->size += total_size;
    mem->response[mem->size] = 0;  // Null terminate la chaîne

    return total_size;
}

// Fonction pour insérer du texte dans le thread principal GTK de manière sécurisée
gboolean insert_text_safe(gpointer data) {
    struct ollama_request_data *req_data = (struct ollama_request_data *)data;
    if (!req_data || !req_data->full_response) return FALSE;

    // Ajouter le message à l'interface
    add_message_to_chat(req_data->full_response, FALSE);

    // Vider la réponse pour le prochain fragment
    free(req_data->full_response);
    req_data->full_response = calloc(1, sizeof(char));

    return FALSE;
}

gboolean display_thinking_message(gpointer user_data) {
    add_message_to_chat("IA thinking...", FALSE);
    return FALSE;
}

gboolean update_ia_response(gpointer user_data) {
    char *response = (char *)user_data;
    
    // Supprimer le message "IA thinking..."
    GtkListBoxRow *last_row = gtk_list_box_get_row_at_index(GTK_LIST_BOX(chat_list), -1);
    if (last_row) {
        gtk_container_remove(GTK_CONTAINER(chat_list), GTK_WIDGET(last_row));
    }
    
    // Ajouter la réponse complète
    add_message_to_chat(response, FALSE);
    
    g_free(response);
    return FALSE;
}

void parse_and_display_full_response(const char *full_response, GtkWidget *chat_view) {
    struct json_object *parsed_json;
    struct json_object *message;
    struct json_object *content;
    GString *full_content = g_string_new("");

    const char *line = full_response;
    const char *next_line;
    while (line && *line) {
        next_line = strchr(line, '\n');
        if (next_line) {
            char *temp_line = g_strndup(line, next_line - line);
            parsed_json = json_tokener_parse(temp_line);
            g_free(temp_line);
        } else {
            parsed_json = json_tokener_parse(line);
        }

        if (parsed_json && json_object_object_get_ex(parsed_json, "message", &message)) {
            if (json_object_object_get_ex(message, "content", &content)) {
                const char *text = json_object_get_string(content);
                g_string_append(full_content, text);
            }
        }
        json_object_put(parsed_json);
        
        if (next_line) line = next_line + 1;
        else break;
    }

    // Remplacer le message "IA thinking..." par la réponse complète
    gdk_threads_add_idle(update_ia_response, g_strdup(full_content->str));
    g_string_free(full_content, TRUE);
}

// Fonction pour parser et afficher la réponse
void parse_and_display_response(const char *json_response, GtkWidget *chat_view, char **full_response) {
    struct json_object *parsed_json;
    struct json_object *message;
    struct json_object *content;

    parsed_json = json_tokener_parse(json_response);
    if (parsed_json == NULL) {
        printf("Erreur lors du parsing JSON\n");
        return;
    }

    if (json_object_object_get_ex(parsed_json, "message", &message)) {
        if (json_object_object_get_ex(message, "content", &content)) {
            const char *text = json_object_get_string(content);
            size_t new_len = strlen(*full_response) + strlen(text) + 2;
            *full_response = realloc(*full_response, new_len);
            strcat(*full_response, text);
            
            // Ajouter le nouveau texte à l'interface utilisateur immédiatement
            gdk_threads_add_idle(insert_text_safe, *full_response);
        }
    }

    json_object_put(parsed_json);
}

// Fonction pour envoyer une requête de chat à Ollama sans streaming (fonction exécutée dans un thread)
void* query_ollama_thread(void *data) {
    struct ollama_request_data *req_data = (struct ollama_request_data *)data;
    if (!req_data || !req_data->prompt) {
        fprintf(stderr, "Invalid request data\n");
        return NULL;
    }

    // Afficher "IA thinking..." immédiatement
    gdk_threads_add_idle(display_thinking_message, NULL);

    CURL *curl;
    CURLcode res;
    struct memory chunk = {0};
    
    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "http://localhost:11434/api/chat");
        struct curl_slist *headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

        char *escaped_prompt = curl_easy_escape(curl, req_data->prompt, 0);
        char *json_data = NULL;
        asprintf(&json_data, "{\"model\": \"llama3.1\", \"messages\": [{\"role\": \"user\", \"content\": \"%s\"}], \"stream\": true}", escaped_prompt);
        curl_free(escaped_prompt);

        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, json_data);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        } else {
            parse_and_display_full_response(chunk.response, req_data->chat_view);
        }

        free(json_data);
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }

    if (chunk.response) free(chunk.response);
    free(req_data->prompt);
    free(req_data);

    return NULL;
}