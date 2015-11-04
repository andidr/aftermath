/**
 * Copyright (C) 2013 Andi Drebes <andi.drebes@lip6.fr>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as published
 * by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "dialogs.h"
#include "glade_extras.h"
#include "cpu_list.h"
#include "task_list.h"
#include "histogram_widget.h"
#include <glade/glade.h>
#include <math.h>
#include <inttypes.h>
#include <pthread.h>

/**
 * A simple message dialog. Format is a printf-style format string.
 */
void show_error_message(char* format, ...)
{
	char buff[1024];
	va_list ap;

	va_start(ap, format);
	vsnprintf(buff, sizeof(buff), format, ap);
	va_end(ap);

	GtkWidget* dialog = gtk_message_dialog_new (NULL,
				GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_MESSAGE_ERROR,
				GTK_BUTTONS_CLOSE,
				buff);
	gtk_dialog_run (GTK_DIALOG (dialog));
	gtk_widget_destroy (dialog);
}

/**
 * Displays a simple dialog with three buttons: yes, no and cancel.
 * @param format A printf-style format string
 * @return 0 if the answer was no, 1 if yes and 2 if cancel was pressed.
 */
enum yes_no_cancel_dialog_response show_yes_no_cancel_dialog(char* format, ...)
{
	char buff[1024];
	va_list ap;
	enum yes_no_cancel_dialog_response ret = DIALOG_RESPONSE_CANCEL;

	va_start(ap, format);
	vsnprintf(buff, sizeof(buff), format, ap);
	va_end(ap);

	GtkWidget* dialog = gtk_message_dialog_new (NULL,
						    GTK_DIALOG_DESTROY_WITH_PARENT,
						    GTK_MESSAGE_QUESTION,
						    GTK_BUTTONS_YES_NO,
						    buff);
	/* FIXME: use a button from stock */
	gtk_dialog_add_button(GTK_DIALOG(dialog),
			      "Cancel",
			      GTK_RESPONSE_REJECT);

	gint resp = gtk_dialog_run(GTK_DIALOG (dialog));

	switch(resp) {
		case GTK_RESPONSE_NO:
			ret = DIALOG_RESPONSE_NO;
			break;
		case GTK_RESPONSE_YES:
			ret = DIALOG_RESPONSE_YES;
			break;
		case GTK_RESPONSE_REJECT:
			ret = DIALOG_RESPONSE_CANCEL;
			break;
	}

	gtk_widget_destroy (dialog);

	return ret;
}

/**
 * Displays a simple dialog with two buttons: yes and no.
 * @param format A printf-style format string
 * @return 0 if the answer was no, 1 if yes.
 */
enum yes_no_cancel_dialog_response show_yes_no_dialog(char* format, ...)
{
	char buff[1024];
	va_list ap;
	enum yes_no_cancel_dialog_response ret = DIALOG_RESPONSE_NO;

	va_start(ap, format);
	vsnprintf(buff, sizeof(buff), format, ap);
	va_end(ap);

	GtkWidget* dialog = gtk_message_dialog_new (NULL,
						    GTK_DIALOG_DESTROY_WITH_PARENT,
						    GTK_MESSAGE_QUESTION,
						    GTK_BUTTONS_YES_NO,
						    buff);

	gint resp = gtk_dialog_run(GTK_DIALOG (dialog));

	switch(resp) {
		case GTK_RESPONSE_NO:
			ret = DIALOG_RESPONSE_NO;
			break;
		case GTK_RESPONSE_YES:
			ret = DIALOG_RESPONSE_YES;
			break;
	}

	gtk_widget_destroy (dialog);

	return ret;
}

/**
 * Runs a simple file selection dialog.
 * @param title The string shown in the title bar of the dialog
 * @param mode GTK_FILE_CHOOSER_ACTION_OPEN or GTK_FILE_CHOOSER_ACTION_SAVE
 * @param filter_name Name for the file extension filter (e.g. "WAVE-Files")
 * @param filter_extension The allowed file extension (e.g. "*.wav")
 * @param default_dir The directory which is displayed by default
 * @return The filename including the full path of the selected file or NULL if the dialog was cancelled
 */
char* load_save_file_dialog(const char* title, GtkFileChooserAction mode, const char* filter_name, const char* filter_extension, const char* default_dir)
{
	char *filename = NULL;

	GtkWidget* dialog = gtk_file_chooser_dialog_new (title,
							 NULL,
							 mode,
							 GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
							 GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
							 NULL);

	GtkFileFilter *filter = gtk_file_filter_new ();
	gtk_file_filter_set_name(filter, filter_name);
	gtk_file_filter_add_pattern(filter, filter_extension);
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);

	if(default_dir)
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), default_dir);

	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
		filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));

	gtk_widget_destroy (dialog);

	return filename;
}


int show_goto_dialog(double start, double end, double curr_value, double* time)
{
	int ret = 0;
	GladeXML* xml = glade_xml_new(DATA_PATH "/goto_dialog.glade", NULL, NULL);
	glade_xml_signal_autoconnect(xml);
	IMPORT_GLADE_WIDGET(xml, dialog);
	IMPORT_GLADE_WIDGET(xml, spin);

	printf("set range to %f - %f\n", start, end);
	gtk_spin_button_set_range(GTK_SPIN_BUTTON(spin), start, end);
	gtk_spin_button_set_increments(GTK_SPIN_BUTTON(spin), pow(10, floor(log(fabs(curr_value+0.1)) / log(10))), end-start);
	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin), curr_value);

	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
		*time = gtk_spin_button_get_value(GTK_SPIN_BUTTON(spin));
		ret = 1;
	}

	gtk_widget_destroy(dialog);
	g_object_unref(G_OBJECT(xml));

	return ret;
}

int show_task_graph_texec_dialog(unsigned int* depth_down, unsigned int* depth_up)
{
	int ret = 0;
	GladeXML* xml = glade_xml_new(DATA_PATH "/task_graph_texec_dialog.glade", NULL, NULL);
	glade_xml_signal_autoconnect(xml);
	IMPORT_GLADE_WIDGET(xml, dialog);
	IMPORT_GLADE_WIDGET(xml, spin_down);
	IMPORT_GLADE_WIDGET(xml, spin_up);

	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
		*depth_down = gtk_spin_button_get_value(GTK_SPIN_BUTTON(spin_down));
		*depth_up = gtk_spin_button_get_value(GTK_SPIN_BUTTON(spin_up));
		ret = 1;
	}

	gtk_widget_destroy(dialog);
	g_object_unref(G_OBJECT(xml));

	return ret;
}

enum annotation_dialog_response show_annotation_dialog(struct annotation* a, int edit)
{
	int ret = ANNOTATION_DIALOG_RESPONSE_CANCEL;
	char buffer[128];
	static GdkColor color = {.red = 65535, .green = 32767, .blue = 32767};

	GladeXML* xml = glade_xml_new(DATA_PATH "/annotation_dialog.glade", NULL, NULL);
	glade_xml_signal_autoconnect(xml);
	IMPORT_GLADE_WIDGET(xml, dialog);
	IMPORT_GLADE_WIDGET(xml, label_cpu);
	IMPORT_GLADE_WIDGET(xml, label_time);
	IMPORT_GLADE_WIDGET(xml, text);
	IMPORT_GLADE_WIDGET(xml, colorbutton);
	IMPORT_GLADE_WIDGET(xml, button_delete);

	GtkTextBuffer* text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(text));
	GtkTextIter start;
	GtkTextIter end;

	snprintf(buffer, sizeof(buffer), "%d", a->cpu);
	gtk_label_set_text(GTK_LABEL(label_cpu), buffer);

	snprintf(buffer, sizeof(buffer), "%"PRIu64, a->time);
	gtk_label_set_text(GTK_LABEL(label_time), buffer);

	if(edit) {
		gtk_text_buffer_set_text(text_buffer, a->text, strlen(a->text));
		gtk_window_set_title(GTK_WINDOW(dialog), "Edit annotation");
		gtk_widget_show(button_delete);

		color.red = 65535.0*a->color_r;
		color.green = 65535.0*a->color_g;
		color.blue = 65535.0*a->color_b;

		gtk_color_button_set_color(GTK_COLOR_BUTTON(colorbutton), &color);
	} else {
		gtk_window_set_title(GTK_WINDOW(dialog), "Create annotation");
		gtk_widget_hide(button_delete);

		gtk_color_button_set_color(GTK_COLOR_BUTTON(colorbutton), &color);
	}

	switch(gtk_dialog_run(GTK_DIALOG(dialog))) {
		case GTK_RESPONSE_ACCEPT:
			gtk_text_buffer_get_start_iter(text_buffer, &start);
			gtk_text_buffer_get_end_iter(text_buffer, &end);
			annotation_set_text(a, gtk_text_buffer_get_text(text_buffer, &start, &end, 0));

			gtk_color_button_get_color(GTK_COLOR_BUTTON(colorbutton), &color);
			a->color_r = (double)color.red / 65535.0;
			a->color_g = (double)color.green / 65535.0;
			a->color_b = (double)color.blue / 65535.0;

			ret = ANNOTATION_DIALOG_RESPONSE_OK;
			break;
		case -99:
			ret = ANNOTATION_DIALOG_RESPONSE_DELETE;
			break;
	}

	gtk_widget_destroy(dialog);
	g_object_unref(G_OBJECT(xml));

	return ret;
}

struct derived_counter_dialog_context {
	GtkWidget* dialog;
	GtkWidget* table_properties;
	GtkWidget* combo_type;
	GtkWidget* combo_cpu;
	GtkWidget* combo_counter;
	GtkWidget* combo_state;
	GtkWidget* combo_numa_node;
	GtkWidget* scale_samples;
	GtkWidget* entry_name;
	GtkWidget* radio_reads_only;
	GtkWidget* radio_writes_only;
	GtkWidget* radio_reads_and_writes;
	GtkWidget* radio_remote_to_local;
	GtkWidget* radio_local_to_local;
	GtkWidget* radio_local_to_remote;
	GtkWidget* radio_remote_and_local_to_local;
	GtkWidget* radio_everything_involving_local_node;
	GtkWidget* radio_spikes;
	GtkWidget* radio_linear;
	GtkWidget* label_type;
	GtkWidget* label_cpu;
	GtkWidget* label_counter;
	GtkWidget* label_state;
	GtkWidget* label_samples;
	GtkWidget* label_name;
	GtkWidget* label_numa_node;
	GtkWidget* label_reads_only;
	GtkWidget* label_writes_only;
	GtkWidget* label_reads_and_writes;
	GtkWidget* label_spikes;
	GtkWidget* label_linear;
	GtkWidget* label_remote_to_local;
	GtkWidget* label_local_to_local;
	GtkWidget* label_local_to_remote;
	GtkWidget* label_remote_and_local_to_local;
	GtkWidget* label_everything_involving_local_node;
	GtkWidget* check_exclude_node;
	GtkWidget* combo_exclude_node;
	GtkWidget* hsep_contention1;
	GtkWidget* hsep_contention2;
	GtkWidget* hsep_contention3;
	GtkWidget* hsep_contention4;
	GtkWidget* combo_divcounter;
	GtkWidget* label_divcounter;
	GtkWidget* label_divmode;
	GtkWidget* vbox_divmode;
	GtkWidget* radio_plain_div;
	GtkWidget* radio_div_sum;
	GtkWidget* label_cpus;
	GtkWidget* scroll_cpus;
	GtkWidget* treeview_cpus;
	GtkWidget* label_tasks;
	GtkWidget* scroll_tasks;
	GtkWidget* treeview_tasks;
};

G_MODULE_EXPORT void derived_counter_dialog_type_changed(GtkComboBox* widget, gpointer user_data)
{
	struct derived_counter_dialog_context* ctx;
	int curr_type = gtk_combo_box_get_active(widget);

	ctx = g_object_get_data(G_OBJECT(widget), "context");
	gtk_widget_hide_all(ctx->table_properties);
	gtk_widget_show(ctx->table_properties);

	GtkWidget** visible_widgets[5];

	visible_widgets[DERIVED_COUNTER_PARALLELISM] =
		(GtkWidget*[]) {
		ctx->combo_type, ctx->label_type,
		ctx->combo_cpu, ctx->label_cpu,
		ctx->combo_state, ctx->label_state,
		ctx->scale_samples, ctx->label_samples,
		ctx->entry_name, ctx->label_name,
		NULL };

	visible_widgets[DERIVED_COUNTER_AGGREGATE] =
		(GtkWidget*[]){
		ctx->combo_type, ctx->label_type,
		ctx->combo_cpu, ctx->label_cpu,
		ctx->combo_counter, ctx->label_counter,
		ctx->scale_samples, ctx->label_samples,
		ctx->entry_name, ctx->label_name,
		NULL };

	visible_widgets[DERIVED_COUNTER_NUMA_CONTENTION] =
		(GtkWidget*[]){
		ctx->combo_type, ctx->label_type,
		ctx->combo_cpu, ctx->label_cpu,
		ctx->combo_numa_node, ctx->label_numa_node,
		ctx->scale_samples, ctx->label_samples,
		ctx->entry_name, ctx->label_name,
		ctx->label_reads_only,
		ctx->label_writes_only,
		ctx->label_reads_and_writes,
		ctx->radio_reads_only,
		ctx->radio_writes_only,
		ctx->radio_reads_and_writes,
		ctx->radio_spikes,
		ctx->radio_linear,
		ctx->label_spikes,
		ctx->label_linear,
		ctx->radio_remote_to_local,
		ctx->radio_local_to_local,
		ctx->radio_local_to_remote,
		ctx->radio_remote_and_local_to_local,
		ctx->radio_everything_involving_local_node,
		ctx->label_remote_to_local,
		ctx->label_local_to_local,
		ctx->label_local_to_remote,
		ctx->label_remote_and_local_to_local,
		ctx->label_everything_involving_local_node,
		ctx->hsep_contention1,
		ctx->hsep_contention2,
		ctx->hsep_contention3,
		ctx->hsep_contention4,
		ctx->check_exclude_node,
		ctx->combo_exclude_node,
		NULL };

	visible_widgets[DERIVED_COUNTER_RATIO] =
		(GtkWidget*[]){
		ctx->combo_type, ctx->label_type,
		ctx->combo_counter, ctx->label_counter,
		ctx->combo_divcounter, ctx->label_divcounter,
		ctx->vbox_divmode, ctx->label_divmode,
		ctx->radio_plain_div, ctx->radio_div_sum,
		ctx->scale_samples, ctx->label_samples,
		ctx->entry_name, ctx->label_name,
		NULL };

	visible_widgets[DERIVED_COUNTER_TASK_LENGTH] =
		(GtkWidget*[]){
		ctx->combo_type, ctx->label_type,
		ctx->combo_cpu, ctx->label_cpu,
		ctx->label_cpus, ctx->treeview_cpus,
		ctx->scroll_cpus,
		ctx->label_tasks, ctx->treeview_tasks,
		ctx->scroll_tasks,
		ctx->scale_samples, ctx->label_samples,
		ctx->entry_name, ctx->label_name,
		NULL };

	for(int i = 0; visible_widgets[curr_type][i]; i++)
		gtk_widget_show_all(visible_widgets[curr_type][i]);

	gtk_container_check_resize(GTK_CONTAINER(ctx->table_properties));
}

int show_derived_counter_dialog(struct multi_event_set* mes, struct derived_counter_options* opt)
{
	GladeXML* xml = glade_xml_new(DATA_PATH "/derived_counter_dialog.glade", NULL, NULL);
	char buffer[256];
	int type_idx;
	int cpu_idx;
	int ctr_idx = 0;
	int divctr_idx = 0;
	int numa_node_idx = 0;
	int numa_node;
	int exclude_idx;
	int ret = 0;
	const char* name;
	int state_desc_idx = 0;
	struct derived_counter_dialog_context ctx;

	GtkTreeIter iter;
	GtkListStore *liststore;
	GtkCellRenderer *column;

	struct event_set* es;

	glade_xml_signal_autoconnect(xml);
	IMPORT_GLADE_WIDGET_ASSIGN_STRUCT(xml, &ctx, dialog);
	IMPORT_GLADE_WIDGET_ASSIGN_STRUCT(xml, &ctx, table_properties);
	IMPORT_GLADE_WIDGET_ASSIGN_STRUCT(xml, &ctx, combo_type);
	IMPORT_GLADE_WIDGET_ASSIGN_STRUCT(xml, &ctx, combo_cpu);
	IMPORT_GLADE_WIDGET_ASSIGN_STRUCT(xml, &ctx, combo_counter);
	IMPORT_GLADE_WIDGET_ASSIGN_STRUCT(xml, &ctx, combo_numa_node);
	IMPORT_GLADE_WIDGET_ASSIGN_STRUCT(xml, &ctx, scale_samples);
	IMPORT_GLADE_WIDGET_ASSIGN_STRUCT(xml, &ctx, entry_name);
	IMPORT_GLADE_WIDGET_ASSIGN_STRUCT(xml, &ctx, radio_reads_only);
	IMPORT_GLADE_WIDGET_ASSIGN_STRUCT(xml, &ctx, radio_writes_only);
	IMPORT_GLADE_WIDGET_ASSIGN_STRUCT(xml, &ctx, radio_reads_and_writes);
	IMPORT_GLADE_WIDGET_ASSIGN_STRUCT(xml, &ctx, radio_spikes);
	IMPORT_GLADE_WIDGET_ASSIGN_STRUCT(xml, &ctx, radio_linear);
	IMPORT_GLADE_WIDGET_ASSIGN_STRUCT(xml, &ctx, label_type);
	IMPORT_GLADE_WIDGET_ASSIGN_STRUCT(xml, &ctx, label_cpu);
	IMPORT_GLADE_WIDGET_ASSIGN_STRUCT(xml, &ctx, label_counter);
	IMPORT_GLADE_WIDGET_ASSIGN_STRUCT(xml, &ctx, label_state);
	IMPORT_GLADE_WIDGET_ASSIGN_STRUCT(xml, &ctx, label_samples);
	IMPORT_GLADE_WIDGET_ASSIGN_STRUCT(xml, &ctx, label_name);
	IMPORT_GLADE_WIDGET_ASSIGN_STRUCT(xml, &ctx, label_numa_node);
	IMPORT_GLADE_WIDGET_ASSIGN_STRUCT(xml, &ctx, label_reads_only);
	IMPORT_GLADE_WIDGET_ASSIGN_STRUCT(xml, &ctx, label_writes_only);
	IMPORT_GLADE_WIDGET_ASSIGN_STRUCT(xml, &ctx, label_reads_and_writes);
	IMPORT_GLADE_WIDGET_ASSIGN_STRUCT(xml, &ctx, label_spikes);
	IMPORT_GLADE_WIDGET_ASSIGN_STRUCT(xml, &ctx, label_linear);
	IMPORT_GLADE_WIDGET_ASSIGN_STRUCT(xml, &ctx, radio_remote_to_local);
	IMPORT_GLADE_WIDGET_ASSIGN_STRUCT(xml, &ctx, radio_local_to_local);
	IMPORT_GLADE_WIDGET_ASSIGN_STRUCT(xml, &ctx, radio_local_to_remote);
	IMPORT_GLADE_WIDGET_ASSIGN_STRUCT(xml, &ctx, radio_remote_and_local_to_local);
	IMPORT_GLADE_WIDGET_ASSIGN_STRUCT(xml, &ctx, radio_everything_involving_local_node);
	IMPORT_GLADE_WIDGET_ASSIGN_STRUCT(xml, &ctx, label_remote_to_local);
	IMPORT_GLADE_WIDGET_ASSIGN_STRUCT(xml, &ctx, label_local_to_local);
	IMPORT_GLADE_WIDGET_ASSIGN_STRUCT(xml, &ctx, label_local_to_remote);
	IMPORT_GLADE_WIDGET_ASSIGN_STRUCT(xml, &ctx, label_remote_and_local_to_local);
	IMPORT_GLADE_WIDGET_ASSIGN_STRUCT(xml, &ctx, label_everything_involving_local_node);
	IMPORT_GLADE_WIDGET_ASSIGN_STRUCT(xml, &ctx, check_exclude_node);
	IMPORT_GLADE_WIDGET_ASSIGN_STRUCT(xml, &ctx, combo_exclude_node);
	IMPORT_GLADE_WIDGET_ASSIGN_STRUCT(xml, &ctx, hsep_contention1);
	IMPORT_GLADE_WIDGET_ASSIGN_STRUCT(xml, &ctx, hsep_contention2);
	IMPORT_GLADE_WIDGET_ASSIGN_STRUCT(xml, &ctx, hsep_contention3);
	IMPORT_GLADE_WIDGET_ASSIGN_STRUCT(xml, &ctx, hsep_contention4);
	IMPORT_GLADE_WIDGET_ASSIGN_STRUCT(xml, &ctx, label_divcounter);
	IMPORT_GLADE_WIDGET_ASSIGN_STRUCT(xml, &ctx, combo_divcounter);
	IMPORT_GLADE_WIDGET_ASSIGN_STRUCT(xml, &ctx, label_divmode);
	IMPORT_GLADE_WIDGET_ASSIGN_STRUCT(xml, &ctx, radio_plain_div);
	IMPORT_GLADE_WIDGET_ASSIGN_STRUCT(xml, &ctx, radio_div_sum);
	IMPORT_GLADE_WIDGET_ASSIGN_STRUCT(xml, &ctx, vbox_divmode);
	IMPORT_GLADE_WIDGET_ASSIGN_STRUCT(xml, &ctx, label_cpus);
	IMPORT_GLADE_WIDGET_ASSIGN_STRUCT(xml, &ctx, scroll_cpus);
	IMPORT_GLADE_WIDGET_ASSIGN_STRUCT(xml, &ctx, treeview_cpus);
	IMPORT_GLADE_WIDGET_ASSIGN_STRUCT(xml, &ctx, label_tasks);
	IMPORT_GLADE_WIDGET_ASSIGN_STRUCT(xml, &ctx, scroll_tasks);
	IMPORT_GLADE_WIDGET_ASSIGN_STRUCT(xml, &ctx, treeview_tasks);

	liststore = gtk_list_store_new(1, G_TYPE_STRING);

	for(int i = 0; i < mes->num_states; i++)
		gtk_list_store_insert_with_values(liststore, &iter, i, 0, mes->states[i].name, -1);

	ctx.combo_state = gtk_combo_box_new_with_model(GTK_TREE_MODEL(liststore));
	column = gtk_cell_renderer_text_new();
	gtk_cell_layout_pack_start(GTK_CELL_LAYOUT(ctx.combo_state), column, TRUE);
	gtk_cell_layout_set_attributes( GTK_CELL_LAYOUT(ctx.combo_state), column, "text", 0, NULL );
	gtk_table_attach(GTK_TABLE(ctx.table_properties), ctx.combo_state, 1, 2, 1, 2, GTK_FILL, 0, 0, 0);

	gtk_combo_box_remove_text(GTK_COMBO_BOX(ctx.combo_cpu), 0);

	for_each_event_set(mes, es) {
		snprintf(buffer, sizeof(buffer), "CPU %d", es->cpu);
		gtk_combo_box_append_text(GTK_COMBO_BOX(ctx.combo_cpu), buffer);
	}

	gtk_combo_box_remove_text(GTK_COMBO_BOX(ctx.combo_counter), 0);
	gtk_combo_box_remove_text(GTK_COMBO_BOX(ctx.combo_divcounter), 0);
	for(ctr_idx = 0; ctr_idx < mes->num_counters; ctr_idx++) {
		strncpy(buffer, mes->counters[ctr_idx].name, sizeof(buffer));
		gtk_combo_box_append_text(GTK_COMBO_BOX(ctx.combo_counter), buffer);
		gtk_combo_box_append_text(GTK_COMBO_BOX(ctx.combo_divcounter), buffer);
	}

	gtk_combo_box_remove_text(GTK_COMBO_BOX(ctx.combo_numa_node), 0);
	gtk_combo_box_remove_text(GTK_COMBO_BOX(ctx.combo_exclude_node), 0);
	for(numa_node = 0; numa_node <= mes->max_numa_node_id; numa_node++) {
		snprintf(buffer, sizeof(buffer), "Node %d", numa_node);
		gtk_combo_box_append_text(GTK_COMBO_BOX(ctx.combo_numa_node), buffer);
		gtk_combo_box_append_text(GTK_COMBO_BOX(ctx.combo_exclude_node), buffer);
	}

	cpu_list_init(GTK_TREE_VIEW(ctx.treeview_cpus));
	cpu_list_fill(GTK_TREE_VIEW(ctx.treeview_cpus), multi_event_get_max_cpu(mes));

	task_list_init(GTK_TREE_VIEW(ctx.treeview_tasks));
	task_list_fill(GTK_TREE_VIEW(ctx.treeview_tasks), mes->tasks, mes->num_tasks);
	task_list_uncheck_all(GTK_TREE_VIEW(ctx.treeview_tasks));

	g_object_set_data(G_OBJECT(ctx.combo_type), "context", &ctx);

	gtk_combo_box_set_active(GTK_COMBO_BOX(ctx.combo_type), 0);
	gtk_combo_box_set_active(GTK_COMBO_BOX(ctx.combo_cpu), 0);
	gtk_combo_box_set_active(GTK_COMBO_BOX(ctx.combo_counter), 0);
	gtk_combo_box_set_active(GTK_COMBO_BOX(ctx.combo_divcounter), 0);
	gtk_combo_box_set_active(GTK_COMBO_BOX(ctx.combo_numa_node), 0);
	gtk_combo_box_set_active(GTK_COMBO_BOX(ctx.combo_exclude_node), 0);
	gtk_combo_box_set_active(GTK_COMBO_BOX(ctx.combo_state), 0);

retry:
	if(gtk_dialog_run(GTK_DIALOG(ctx.dialog)) == GTK_RESPONSE_ACCEPT) {
		if((type_idx = gtk_combo_box_get_active(GTK_COMBO_BOX(ctx.combo_type))) == -1) {
			show_error_message("Please select a type for the derived counter.");
			goto retry;
		}

		if((cpu_idx = gtk_combo_box_get_active(GTK_COMBO_BOX(ctx.combo_cpu))) == -1) {
			show_error_message("Please select a CPU to attach the counter to.");
			goto retry;
		}

		if(type_idx == DERIVED_COUNTER_PARALLELISM &&
		   (state_desc_idx = gtk_combo_box_get_active(GTK_COMBO_BOX(ctx.combo_state))) == -1)
		{
			show_error_message("Please select a state.");
			goto retry;
		}

		if((type_idx == DERIVED_COUNTER_AGGREGATE || type_idx == DERIVED_COUNTER_RATIO) &&
		   (ctr_idx = gtk_combo_box_get_active(GTK_COMBO_BOX(ctx.combo_counter))) == -1)
		{
			show_error_message("Please select a counter.");
			goto retry;
		}

		if(type_idx == DERIVED_COUNTER_RATIO &&
		   (divctr_idx = gtk_combo_box_get_active(GTK_COMBO_BOX(ctx.combo_divcounter))) == -1)
		{
			show_error_message("Please select a counter.");
			goto retry;
		}

		if(type_idx == DERIVED_COUNTER_NUMA_CONTENTION &&
		   (numa_node_idx = gtk_combo_box_get_active(GTK_COMBO_BOX(ctx.combo_numa_node))) == -1)
		{
			show_error_message("Please select a NUMA node.");
			goto retry;
		}

		if(type_idx == DERIVED_COUNTER_TASK_LENGTH) {
			cpu_list_build_bitvector(GTK_TREE_VIEW(ctx.treeview_cpus), &opt->cpus);

			if(bitvector_num_bits_set(&opt->cpus) == 0) {
				show_error_message("Please select at least one CPU.");
				goto retry;
			}
		}

		filter_clear_tasks(&opt->task_filter);

		if(type_idx == DERIVED_COUNTER_TASK_LENGTH) {
			task_list_build_filter(GTK_TREE_VIEW(ctx.treeview_tasks), &opt->task_filter);

			if(opt->task_filter.num_tasks == 0) {
				show_error_message("Please select at least one task.");
				goto retry;
			}
		}

		name = gtk_entry_get_text(GTK_ENTRY(ctx.entry_name));

		if(strlen(name) == 0) {
			show_error_message("Please specify a name for the counter.");
			goto retry;
		}

		if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ctx.radio_reads_only)))
			opt->contention_type = ACCESS_TYPE_READS_ONLY;
		else if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ctx.radio_writes_only)))
			opt->contention_type = ACCESS_TYPE_WRITES_ONLY;
		else if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ctx.radio_reads_and_writes)))
			opt->contention_type = ACCESS_TYPE_READS_AND_WRITES;

		if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ctx.radio_local_to_local)))
			opt->data_direction = DATA_DIRECTION_LOCAL_TO_LOCAL;
		else if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ctx.radio_local_to_remote)))
			opt->data_direction = DATA_DIRECTION_LOCAL_TO_REMOTE;
		else if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ctx.radio_remote_to_local)))
			opt->data_direction = DATA_DIRECTION_REMOTE_TO_LOCAL;
		else if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ctx.radio_remote_and_local_to_local)))
			opt->data_direction = DATA_DIRECTION_REMOTE_AND_LOCAL_TO_LOCAL;
		else if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ctx.radio_everything_involving_local_node)))
			opt->data_direction = DATA_DIRECTION_EVERYTHING_INVOLVING_LOCAL_NODE;

		if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ctx.radio_spikes)))
			opt->contention_model = ACCESS_MODEL_SPIKES;
		else if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ctx.radio_linear)))
			opt->contention_model = ACCESS_MODEL_LINEAR;

		if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ctx.radio_plain_div)))
			opt->ratio_type = RATIO_TYPE_PLAIN_DIV;
		else if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ctx.radio_div_sum)))
			opt->ratio_type = RATIO_TYPE_DIV_SUM;

		if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ctx.check_exclude_node))) {
			if((exclude_idx = gtk_combo_box_get_active(GTK_COMBO_BOX(ctx.combo_exclude_node))) == -1)
			{
				show_error_message("Please select a Node to exclude.");
				goto retry;
			}
		} else {
			exclude_idx = -1;
		}

		opt->type = type_idx;
		opt->cpu = mes->sets[cpu_idx].cpu;
		opt->num_samples = gtk_range_get_value(GTK_RANGE(ctx.scale_samples));
		opt->name = malloc(strlen(name)+1);
		opt->state = mes->states[state_desc_idx].state_id;
		opt->counter_idx = ctr_idx;
		opt->divcounter_idx = divctr_idx;
		opt->numa_node = numa_node_idx;
		opt->exclude_node = exclude_idx;

		if(!opt->name) {
			show_error_message("Could not allocate space for counter name.");
			goto retry;
		}

		strcpy(opt->name, name);
		ret = 1;
	}

	gtk_widget_destroy(ctx.dialog);
	g_object_unref(G_OBJECT(xml));

	return ret;
}

void show_progress_window_persistent(struct progress_window_widgets* widgets)
{
	GladeXML* xml = glade_xml_new(DATA_PATH "/progress_dialog.glade", NULL, NULL);
	glade_xml_signal_autoconnect(xml);
	IMPORT_GLADE_WIDGET(xml, window);
	IMPORT_GLADE_WIDGET(xml, progressbar);
	IMPORT_GLADE_WIDGET(xml, label_trace_bytes);
	IMPORT_GLADE_WIDGET(xml, label_bytes_loaded);
	IMPORT_GLADE_WIDGET(xml, label_seconds_remaining);
	IMPORT_GLADE_WIDGET(xml, label_throughput);

	widgets->window = GTK_WINDOW(window);
	widgets->progressbar = GTK_PROGRESS_BAR(progressbar);
	widgets->label_trace_bytes = GTK_LABEL(label_trace_bytes);
	widgets->label_bytes_loaded = GTK_LABEL(label_bytes_loaded);
	widgets->label_seconds_remaining = GTK_LABEL(label_seconds_remaining);
	widgets->label_throughput = GTK_LABEL(label_throughput);

	gtk_widget_show_all(window);
	g_object_unref(G_OBJECT(xml));
}

void show_about_dialog(void)
{
	GladeXML* xml = glade_xml_new(DATA_PATH "/about_dialog.glade", NULL, NULL);
	glade_xml_signal_autoconnect(xml);
	IMPORT_GLADE_WIDGET(xml, dialog);

	gtk_dialog_run(GTK_DIALOG(dialog));

	gtk_widget_destroy(dialog);
	g_object_unref(G_OBJECT(xml));
}

G_MODULE_EXPORT gint accept_dialog(GtkWidget *widget, gpointer data)
{
        gtk_dialog_response(GTK_DIALOG(widget), GTK_RESPONSE_ACCEPT);

        return 0;
}

int show_settings_dialog(struct settings* s)
{
	int ret = 0;
	GladeXML* xml = glade_xml_new(DATA_PATH "/settings.glade", NULL, NULL);
	glade_xml_signal_autoconnect(xml);
	IMPORT_GLADE_WIDGET(xml, dialog);
	IMPORT_GLADE_WIDGET(xml, external_editor_check);
	IMPORT_GLADE_WIDGET(xml, external_editor_entry);

	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(external_editor_check), s->use_external_editor);
	gtk_widget_set_sensitive(external_editor_entry, s->use_external_editor);

	if(s->external_editor_command != NULL)
		gtk_entry_set_text(GTK_ENTRY(external_editor_entry), s->external_editor_command);

	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
		s->use_external_editor = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(external_editor_check));

		if(s->use_external_editor)
			settings_set_string(&s->external_editor_command,
					    gtk_entry_get_text(GTK_ENTRY(external_editor_entry)));
		ret = 1;
	}

	gtk_widget_destroy(dialog);
	g_object_unref(G_OBJECT(xml));

	return ret;
}

int show_color_dialog(GdkColor* color)
{
	int ret = 0;
	int foo;
	GladeXML* xml = glade_xml_new(DATA_PATH "/color_dialog.glade", NULL, NULL);
	glade_xml_signal_autoconnect(xml);
	IMPORT_GLADE_WIDGET(xml, dialog);
	IMPORT_GLADE_WIDGET(xml, color_selection);

	gtk_color_selection_set_current_color(GTK_COLOR_SELECTION(color_selection), color);

	if((foo = gtk_dialog_run(GTK_DIALOG(dialog))) == GTK_RESPONSE_OK) {
		gtk_color_selection_get_current_color(GTK_COLOR_SELECTION(color_selection), color);
		ret = 1;
	}

	gtk_widget_destroy(dialog);
	g_object_unref(G_OBJECT(xml));

	return ret;
}

struct counter_offset_dialog_params {
	struct multi_event_set* mes;
	struct counter_description* cd;
	GtkWidget* trace_widget;
	int64_t last_offset;
};

G_MODULE_EXPORT void counter_offset_dialog_offset_changed(GtkSpinButton* spin, gpointer user_data)
{
	struct counter_offset_dialog_params* params = user_data;
	int64_t curr_offset = gtk_spin_button_get_value(GTK_SPIN_BUTTON(spin));
	int64_t effective_offset = curr_offset - params->last_offset;

	multi_event_event_set_add_counter_offset(params->mes, params->cd->counter_id, effective_offset);
	gtk_widget_queue_draw(params->trace_widget);

	params->last_offset = curr_offset;
}

int show_counter_offset_dialog(struct multi_event_set* mes, struct counter_description* cd, GtkWidget* trace_widget, int64_t* offset)
{
	int ret = 0;
	GladeXML* xml = glade_xml_new(DATA_PATH "/counter_offset_dialog.glade", NULL, NULL);
	struct counter_offset_dialog_params params = {
		.mes = mes,
		.cd = cd,
		.trace_widget = trace_widget,
		.last_offset = 0
	};

	glade_xml_signal_autoconnect(xml);
	IMPORT_GLADE_WIDGET(xml, dialog);
	IMPORT_GLADE_WIDGET(xml, spin);

	g_signal_connect(G_OBJECT(spin), "value-changed", G_CALLBACK(counter_offset_dialog_offset_changed), &params);

	if(gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
		*offset = params.last_offset;
		ret = 1;
	} else {
		multi_event_event_set_add_counter_offset(mes, cd->counter_id, -params.last_offset);
		gtk_widget_queue_draw(trace_widget);
	}

	gtk_widget_destroy(dialog);
	g_object_unref(G_OBJECT(xml));

	return ret;
}

void show_parallelism_dialog(struct histogram* hist)
{
	char buffer[128];

	GladeXML* xml = glade_xml_new(DATA_PATH "/parallelism_dialog.glade", NULL, NULL);
	glade_xml_signal_autoconnect(xml);
	IMPORT_GLADE_WIDGET(xml, dialog);
	IMPORT_GLADE_WIDGET(xml, hist_box);
	IMPORT_GLADE_WIDGET(xml, label_top);
	IMPORT_GLADE_WIDGET(xml, label_right);

	snprintf(buffer, sizeof(buffer), "%d", hist->max_hist);
	gtk_label_set_text(GTK_LABEL(label_top), buffer);

	snprintf(buffer, sizeof(buffer), "%Lf", hist->right);
	gtk_label_set_text(GTK_LABEL(label_right), buffer);

	GtkWidget* histogram_widget = gtk_histogram_new();
	gtk_histogram_set_data(histogram_widget, hist);
	gtk_container_add(GTK_CONTAINER(hist_box), histogram_widget);

	gtk_widget_show_all(dialog);

	gtk_dialog_run(GTK_DIALOG(dialog));

	gtk_widget_destroy(dialog);
	g_object_unref(G_OBJECT(xml));
}

struct background_task_data {
	int status;
	int finished;
	void* data;
	int (*fun)(void* data);
};

void* background_task_run_thread(void* pdata)
{
	struct background_task_data* dlgst = pdata;
	dlgst->status = dlgst->fun(dlgst->data);
	dlgst->finished = 1;

	return NULL;
}

int background_task_with_modal_dialog(const char* message, const char* title, int (*fun)(void* data), void* data)
{
	struct background_task_data dlgst;
	pthread_t tid;
	int ret;
	GladeXML* xml = glade_xml_new(DATA_PATH "/message_dialog.glade", NULL, NULL);
	glade_xml_signal_autoconnect(xml);
	IMPORT_GLADE_WIDGET(xml, dialog);
	IMPORT_GLADE_WIDGET(xml, label_message);

	dlgst.status = 0;
	dlgst.finished = 0;
	dlgst.fun = fun;
	dlgst.data = data;

	gtk_window_set_title(GTK_WINDOW(dialog), title);
	gtk_label_set_text(GTK_LABEL(label_message), message);

	if(pthread_create(&tid, NULL, background_task_run_thread, &dlgst)) {
		ret = 1;
		goto out;
	}

	gtk_widget_show_all(dialog);

	while(!dlgst.finished) {
		while(gtk_events_pending())
			gtk_main_iteration();
	}

	pthread_join(tid, NULL);

	ret = dlgst.status;
out:
	gtk_widget_destroy(dialog);
	g_object_unref(G_OBJECT(xml));

	return ret;
}

gint delete_event_return_true(GtkWidget *widget, GdkEvent  *event, gpointer data)
{
	return TRUE;
}

GtkWidget* detach_dialog_create(const char* title, GtkWidget* child)
{
	GtkWidget* dlg = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(dlg), title);

	gtk_signal_connect(GTK_OBJECT(dlg), "delete_event", GTK_SIGNAL_FUNC(delete_event_return_true), NULL);

	gtk_widget_reparent(child, dlg);

	gtk_widget_show(child);
	gtk_widget_show(dlg);

	return dlg;
}

void detach_dialog_destroy(GtkWidget* dlg, GtkWidget* child, GtkWidget* new_parent)
{
	gtk_widget_reparent(child, new_parent);
	gtk_widget_destroy(dlg);
}
