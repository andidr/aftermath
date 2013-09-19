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
#include <glade/glade.h>
#include <math.h>

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
	GtkWidget* label_type;
	GtkWidget* label_cpu;
	GtkWidget* label_counter;
	GtkWidget* label_state;
	GtkWidget* label_samples;
	GtkWidget* label_name;
	GtkWidget* label_numa_node;
};

G_MODULE_EXPORT void derived_counter_dialog_type_changed(GtkComboBox* widget, gpointer user_data)
{
	struct derived_counter_dialog_context* ctx;
	int curr_type = gtk_combo_box_get_active(widget);

	ctx = g_object_get_data(G_OBJECT(widget), "context");
	gtk_widget_hide_all(ctx->table_properties);
	gtk_widget_show(ctx->table_properties);

	GtkWidget** visible_widgets[3];

	visible_widgets[0] = (GtkWidget*[]){
		ctx->combo_type, ctx->label_type,
		ctx->combo_cpu, ctx->label_cpu,
		ctx->combo_state, ctx->label_state,
		ctx->scale_samples, ctx->label_samples,
		ctx->entry_name, ctx->label_name,
		NULL };

	visible_widgets[1] = (GtkWidget*[]){
		ctx->combo_type, ctx->label_type,
		ctx->combo_cpu, ctx->label_cpu,
		ctx->combo_counter, ctx->label_counter,
		ctx->scale_samples, ctx->label_samples,
		ctx->entry_name, ctx->label_name,
		NULL };

	visible_widgets[2] = (GtkWidget*[]){
		ctx->combo_type, ctx->label_type,
		ctx->combo_cpu, ctx->label_cpu,
		ctx->combo_numa_node, ctx->label_numa_node,
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
	int ctr_idx;
	int numa_node_idx;
	int numa_node;
	int ret = 0;
	const char* name;
	enum worker_state state;
	struct derived_counter_dialog_context ctx;

	glade_xml_signal_autoconnect(xml);
	IMPORT_GLADE_WIDGET_ASSIGN_STRUCT(xml, &ctx, dialog);
	IMPORT_GLADE_WIDGET_ASSIGN_STRUCT(xml, &ctx, table_properties);
	IMPORT_GLADE_WIDGET_ASSIGN_STRUCT(xml, &ctx, combo_type);
	IMPORT_GLADE_WIDGET_ASSIGN_STRUCT(xml, &ctx, combo_cpu);
	IMPORT_GLADE_WIDGET_ASSIGN_STRUCT(xml, &ctx, combo_counter);
	IMPORT_GLADE_WIDGET_ASSIGN_STRUCT(xml, &ctx, combo_state);
	IMPORT_GLADE_WIDGET_ASSIGN_STRUCT(xml, &ctx, combo_numa_node);
	IMPORT_GLADE_WIDGET_ASSIGN_STRUCT(xml, &ctx, scale_samples);
	IMPORT_GLADE_WIDGET_ASSIGN_STRUCT(xml, &ctx, entry_name);
	IMPORT_GLADE_WIDGET_ASSIGN_STRUCT(xml, &ctx, label_type);
	IMPORT_GLADE_WIDGET_ASSIGN_STRUCT(xml, &ctx, label_cpu);
	IMPORT_GLADE_WIDGET_ASSIGN_STRUCT(xml, &ctx, label_counter);
	IMPORT_GLADE_WIDGET_ASSIGN_STRUCT(xml, &ctx, label_state);
	IMPORT_GLADE_WIDGET_ASSIGN_STRUCT(xml, &ctx, label_samples);
	IMPORT_GLADE_WIDGET_ASSIGN_STRUCT(xml, &ctx, label_name);
	IMPORT_GLADE_WIDGET_ASSIGN_STRUCT(xml, &ctx, label_numa_node);

	gtk_combo_box_remove_text(GTK_COMBO_BOX(ctx.combo_cpu), 0);
	for(cpu_idx = 0; cpu_idx < mes->num_sets; cpu_idx++) {
		snprintf(buffer, sizeof(buffer), "CPU %d", mes->sets[cpu_idx].cpu);
		gtk_combo_box_append_text(GTK_COMBO_BOX(ctx.combo_cpu), buffer);
	}

	gtk_combo_box_remove_text(GTK_COMBO_BOX(ctx.combo_counter), 0);
	for(ctr_idx = 0; ctr_idx < mes->num_counters; ctr_idx++) {
		strncpy(buffer, mes->counters[ctr_idx].name, sizeof(buffer));
		gtk_combo_box_append_text(GTK_COMBO_BOX(ctx.combo_counter), buffer);
	}

	gtk_combo_box_remove_text(GTK_COMBO_BOX(ctx.combo_numa_node), 0);
	for(numa_node = 0; numa_node <= mes->max_numa_node_id; numa_node++) {
		snprintf(buffer, sizeof(buffer), "Node %d", numa_node);
		gtk_combo_box_append_text(GTK_COMBO_BOX(ctx.combo_numa_node), buffer);
	}

	g_object_set_data(G_OBJECT(ctx.combo_type), "context", &ctx);

	gtk_combo_box_set_active(GTK_COMBO_BOX(ctx.combo_type), 0);
	gtk_combo_box_set_active(GTK_COMBO_BOX(ctx.combo_cpu), 0);
	gtk_combo_box_set_active(GTK_COMBO_BOX(ctx.combo_counter), 0);
	gtk_combo_box_set_active(GTK_COMBO_BOX(ctx.combo_numa_node), 0);
	gtk_combo_box_set_active(GTK_COMBO_BOX(ctx.combo_state), WORKER_STATE_TASKEXEC);

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
		   (state = gtk_combo_box_get_active(GTK_COMBO_BOX(ctx.combo_state))) == -1)
		{
			show_error_message("Please select a state.");
			goto retry;
		}

		if(type_idx == DERIVED_COUNTER_AGGREGATE &&
		   (ctr_idx = gtk_combo_box_get_active(GTK_COMBO_BOX(ctx.combo_counter))) == -1)
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

		name = gtk_entry_get_text(GTK_ENTRY(ctx.entry_name));

		if(strlen(name) == 0) {
			show_error_message("Please specify a name for the counter.");
			goto retry;
		}

		opt->type = type_idx;
		opt->cpu = mes->sets[cpu_idx].cpu;
		opt->num_samples = gtk_range_get_value(GTK_RANGE(ctx.scale_samples));
		opt->name = malloc(strlen(name)+1);
		opt->state = state;
		opt->counter_idx = ctr_idx;
		opt->numa_node = numa_node_idx;

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
