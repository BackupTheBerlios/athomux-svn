/* Author: Roland Niese
 * Copyright: Roland Niese
 * License: see files SOFTWARE-LICENSE, PATENT-LICENSE
 *
 * Purpose:
 *   Graphical console for input from/ output to the
 *   device_console_ulinux brick.
 *   Simply a program that displays input from stdin and writes
 *   user input to stdout.
 */

#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <gtk/gtk.h>

#define MAX_INPUT_LENGTH (0x0800)
#define OUTPUT_BUF_SIZE (0x2000)
#define THREAD_STACK_SIZE (0x0200)

/* GTK callback functions, see descriptions below. */
static void input_available(gpointer data, gint src, GdkInputCondition cond);
static gboolean close_main_win(GtkWidget *wdg, GdkEvent *event, gpointer data);
static void destroy_main_win(GtkWidget *wdg, gpointer data);
static void text_entered(GtkWidget *wdg, GtkWidget *entry);

/* Main window and widgets contained in it. */
static GtkWidget *main_win, *vbox, *input_field, *output_pane, *output_area;
static GtkTextBuffer *output_text_buf;
/* Necessary for unhooking the input handler when stdin gets closed. */
static gint input_handle;
/* Read buffer for stdin. */
static guchar input_buf[OUTPUT_BUF_SIZE];
/* Program cannot close via [X] button as long as this is false. */
static gboolean close_enabled;

/* Main function, builds and runs the main window. */
int main(int argc, char *argv[]) {	
	static const char *title_str = "Console";
	
	if (argc > 1) {
		title_str = argv[1];
	}
	close_enabled = FALSE;
	
	gtk_init(&argc, &argv);
	
	input_field = gtk_entry_new();
	gtk_entry_set_max_length(GTK_ENTRY(input_field), MAX_INPUT_LENGTH);
	g_signal_connect(G_OBJECT(input_field), "activate", G_CALLBACK(text_entered), (gpointer)input_field);
	gtk_widget_show(input_field);
	
	output_area = gtk_text_view_new();
	gtk_text_view_set_editable(GTK_TEXT_VIEW(output_area), FALSE);
	gtk_text_view_set_cursor_visible(GTK_TEXT_VIEW(output_area), FALSE);
	output_text_buf = gtk_text_view_get_buffer(GTK_TEXT_VIEW(output_area));
	gtk_widget_show(output_area);
	
	output_pane = gtk_scrolled_window_new(NULL, NULL);
	gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(output_pane), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
	gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(output_pane), output_area);
	gtk_widget_show(output_pane);
	
	vbox = gtk_vbox_new(FALSE, 5);
	gtk_box_pack_start(GTK_BOX(vbox), input_field, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(vbox), output_pane, TRUE, TRUE, 0);
	gtk_widget_show(vbox);
	
	main_win = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	g_signal_connect(G_OBJECT(main_win), "delete_event", G_CALLBACK(close_main_win), NULL);
	g_signal_connect(G_OBJECT(main_win), "destroy", G_CALLBACK(destroy_main_win), NULL);		
	gtk_container_set_border_width(GTK_CONTAINER(main_win), 8);
	gtk_window_set_title(GTK_WINDOW(main_win), title_str);
	gtk_window_set_default_size(GTK_WINDOW(main_win), 512, 256);
	gtk_container_add(GTK_CONTAINER(main_win), vbox);
	gtk_widget_show(main_win);

	/* Nifty: install input surveillance (no extra thread necessary). */
	input_handle = gdk_input_add(STDIN_FILENO, GDK_INPUT_READ, input_available, NULL);
	
	gtk_main();
	return 0;
}

/* Called by GTK when there's input available on stdin. */
void input_available(gpointer data, gint src, GdkInputCondition cond) {
	size_t buf_size = read(src, input_buf, sizeof input_buf - 1);
	if (buf_size <= 0) {
		/* End of stream reached, unhook callback func
		 * and make program closeable now. */
		close_enabled = TRUE;
		gdk_input_remove(input_handle);
		return;
	}
	input_buf[buf_size] = '\0';
	/* Avoid any trouble with encodings. */
	guchar *c;
	for (c = input_buf; *c != '\0'; c++) {
		if (*c >= 128) {
			*c = '?';
		}
	}
	GtkTextIter end_iter;
	gtk_text_buffer_get_end_iter(output_text_buf, &end_iter);
	gtk_text_buffer_insert(output_text_buf, &end_iter, input_buf, strlen(input_buf));
}

/* Called by GTK when the main window is to be closed. This is only allowed,
 * when the input stream has been closed. */
static gboolean close_main_win(GtkWidget *wdg, GdkEvent *event, gpointer data) {
	return !close_enabled;
}

/* Called by GTK when the main window is about to be killed. */
static void destroy_main_win(GtkWidget *wdg, gpointer data) {
	gtk_main_quit();
}

/* Called by GTK when the user presses ENTER on the input line. */
static void text_entered(GtkWidget *wdg, GtkWidget *entry) {
	const char *text = gtk_entry_get_text(GTK_ENTRY(entry));
	char buf[2048];
	strncpy(buf, text, sizeof buf - 2);
	buf[sizeof buf - 2] = '\0';
	strcat(buf, "\n");
	write(STDOUT_FILENO, buf, strlen(buf));
	gtk_entry_set_text(GTK_ENTRY(entry), "");
}
