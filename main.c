#include <gtk/gtk.h>
#include <curl/curl.h>

#include <stdio.h>
#include <stdlib.h>

typedef struct
{
  GtkWidget *window;
  GtkWidget *urlBarField;
  GtkWidget *responseField;
  GtkComboBox *methodType;
  GtkWidget *data;
} Window;

struct MemoryStruct {
  char *memory;
  size_t size;
};

static void *myrealloc(void *ptr, size_t size);
 
static void *myrealloc(void *ptr, size_t size)
{
  /* There might be a realloc() out there that doesn't like reallocing
   *      NULL pointers, so we take care of it here */ 
  if(ptr)
    return realloc(ptr, size);
  else
    return malloc(size);
}
 
static size_t
WriteMemoryCallback(void *ptr, size_t size, size_t nmemb, void *data)
{
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)data;

  mem->memory = myrealloc(mem->memory, mem->size + realsize + 1);
  if (mem->memory) {
    memcpy(&(mem->memory[mem->size]), ptr, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;
  }
  return realsize;
}

void
on_window1_destroy(GtkObject *object, gpointer user_data)
{
  gtk_main_quit();
}

void
on_SendButton_clicked(GtkObject *object, Window *window)
{
  CURL *curl;
  CURLcode res;
  GtkTextBuffer *buffer;
  GtkTextBuffer *dataBuffer;
  const gchar *url, *data2;
  gchar *postType=NULL;
  GtkTreeIter iter, start, end;
  GtkTreeModel *model;

  struct MemoryStruct chunk;

  chunk.memory=NULL;
  chunk.size=0;

  if( gtk_combo_box_get_active_iter( window->methodType, &iter ) )
  {
    model = gtk_combo_box_get_model( window->methodType);
    gtk_tree_model_get( model, &iter, 0, &postType, -1 );
  }

  curl_global_init(CURL_GLOBAL_ALL);

  curl = curl_easy_init();

  if(curl) {
    url = gtk_entry_get_text(GTK_ENTRY(window->urlBarField));
    dataBuffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(window->data));
    gtk_text_buffer_get_start_iter(dataBuffer, &start);
    gtk_text_buffer_get_end_iter(dataBuffer, &end);
    data2 = gtk_text_buffer_get_text(dataBuffer, &start, &end, FALSE);

    curl_easy_setopt(curl, CURLOPT_URL, url);

    if(g_strcmp0(postType, "POST") == 0) {
      printf("%s\n", data2);
      curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data2);
      curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long)strlen(data2));
    }

    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

    curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");

    res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);

    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(window->responseField));
    gtk_text_buffer_set_text(buffer, chunk.memory, -1);

    if(chunk.memory)
      free(chunk.memory);
  }

  curl_global_cleanup();
}

void
populate_combobox( GtkComboBox *combo )
{
  GtkListStore *store;
  GtkTreeIter iter, first_item;
  GtkCellRenderer *cell;

  store = gtk_list_store_new( 1, G_TYPE_STRING );

  gtk_list_store_append( store, &iter );
  first_item = iter;
  gtk_list_store_set( store, &iter, 0, "GET", -1 );
  gtk_list_store_append( store, &iter );
  gtk_list_store_set( store, &iter, 0, "POST", -1 );

  gtk_combo_box_set_model( combo, GTK_TREE_MODEL( store ));
  gtk_combo_box_set_active_iter( combo, &first_item );
  //window->methodType = gtk_combo_box_new_with_model( GTK_TREE_MODEL( store ) );
  g_object_unref( G_OBJECT( store ) );
  //gtk_combo_box_append_text( combo, "hello world" );
  cell = gtk_cell_renderer_text_new();

  gtk_cell_layout_pack_start( GTK_CELL_LAYOUT( combo ), cell, TRUE );

  gtk_cell_layout_set_attributes( GTK_CELL_LAYOUT( combo ), cell, "text", 0, NULL );

}

int
main(int argc, char *argv[])
{

  Window *window;
  GtkBuilder *builder;

  window = g_slice_new(Window);

  gtk_init(&argc, &argv);

  builder = gtk_builder_new();
  gtk_builder_add_from_file(builder, "http_client_gtk.glade", NULL);

  window->window = GTK_WIDGET(gtk_builder_get_object(builder, "window1"));
  window->urlBarField = GTK_WIDGET(gtk_builder_get_object(builder, "URLBarField"));
  window->responseField = GTK_WIDGET(gtk_builder_get_object(builder, "ResponseField"));
  window->data = GTK_WIDGET(gtk_builder_get_object(builder, "data"));
  window->methodType = GTK_COMBO_BOX(gtk_builder_get_object(builder, "methodType"));

  populate_combobox( window->methodType );

  gtk_builder_connect_signals(builder, window);

  g_object_unref(G_OBJECT(builder));

  gtk_widget_show(window->window);
  gtk_main();

  g_slice_free(Window, window);

  return 0;
}
