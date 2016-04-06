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

#ifndef TRACE_WIDGET_H
#define TRACE_WIDGET_H

#include <gtk/gtk.h>
#include "multi_event_set.h"
#include "omp_for.h"
#include "omp_for_instance.h"
#include "omp_for_chunk_set.h"
#include "omp_task.h"
#include "omp_task_instance.h"
#include "filter.h"
#include "export.h"
#include "./contrib/linux-kernel/list.h"

G_BEGIN_DECLS

#define GTK_TRACE(obj) GTK_CHECK_CAST(obj, gtk_trace_get_type (), GtkTrace)
#define GTK_TRACE_CLASS(class) GTK_CHECK_CLASS_CAST(class, gtk_trace_get_type(), GtkCpuClass)
#define GTK_IS_TRACE(obj) GTK_CHECK_TYPE(obj, gtk_trace_get_type())

typedef struct _GtkTrace GtkTrace;
typedef struct _GtkTraceClass GtkTraceClass;

struct trace_marker {
	int cpu;
	uint64_t time;
	double color_r;
	double color_g;
	double color_b;
};

enum gtk_trace_signals {
	GTK_TRACE_BOUNDS_CHANGED = 0,
	GTK_TRACE_STATE_EVENT_UNDER_POINTER_CHANGED,
	GTK_TRACE_STATE_EVENT_SELECTION_CHANGED,
	GTK_TRACE_OMP_CHUNK_SET_PART_SELECTION_CHANGED,
	GTK_TRACE_YBOUNDS_CHANGED,
	GTK_TRACE_RANGE_SELECTION_CHANGED,
	GTK_TRACE_CREATE_ANNOTATION,
	GTK_TRACE_EDIT_ANNOTATION,
	GTK_TRACE_MAX_SIGNALS
};

enum gtk_trace_modes {
	GTK_TRACE_MODE_NORMAL = 0,
	GTK_TRACE_MODE_NAVIGATE,
	GTK_TRACE_MODE_SELECT_RANGE_START,
	GTK_TRACE_MODE_SELECT_RANGE,
	GTK_TRACE_MODE_ZOOM
};

enum gtk_trace_map_mode {
	GTK_TRACE_MAP_MODE_STATES = 0,
	GTK_TRACE_MAP_MODE_HEAT_TASKLEN,
	GTK_TRACE_MAP_MODE_NUMA_READS,
	GTK_TRACE_MAP_MODE_NUMA_WRITES,
	GTK_TRACE_MAP_MODE_TASK_TYPE,
	GTK_TRACE_MAP_MODE_HEAT_NUMA,
	GTK_TRACE_MAP_MODE_OMP_FOR_LOOPS,
	GTK_TRACE_MAP_MODE_OMP_FOR_INSTANCES,
	GTK_TRACE_MAP_MODE_OMP_FOR_CHUNK_SETS,
	GTK_TRACE_MAP_MODE_OMP_FOR_CHUNK_SET_PARTS,
	GTK_TRACE_MAP_MODE_OMP_TASKS,
	GTK_TRACE_MAP_MODE_OMP_TASK_INSTANCES,
	GTK_TRACE_MAP_MODE_OMP_TASK_PARTS
};

struct _GtkTrace {
	GtkWidget widget;
	long double left;
	long double right;
	float cpu_height;
	float cpu_offset;
	int axis_width;
	int tick_width;
	int minor_tick_width;
	float scroll_amount;
	float cpu_scroll_px;
	float zoom_factor;
	enum gtk_trace_modes mode;
	int draw_states;
	int draw_comm_size;
	int draw_steals;
	int draw_pushes;
	int draw_data_reads;
	int draw_data_writes;
	int draw_single_events;
	int draw_counters;
	int draw_annotations;
	int draw_measurement_intervals;
	enum gtk_trace_map_mode map_mode;
	int heatmap_shades;
	int moved_during_navigation;

	uint64_t heatmap_min;
	uint64_t heatmap_max;

	int64_t range_selection_start;
	int64_t range_selection_end;
	int range_selection;

	int double_buffering;
	cairo_surface_t* back_buffer;

	struct multi_event_set* event_sets;

	double last_mouse_x;
	double last_mouse_y;

	struct filter* filter;
	struct state_event* highlight_state_event;
	struct single_event* highlight_task_texec_start;
	struct annotation* highlight_annotation;
	struct omp_for_chunk_set_part* highlight_omp_chunk_set_part;
	struct omp_task_part* highlight_omp_task_part;
	struct trace_marker* markers;

	struct list_head* highlight_predecessor_inst;
	int* num_predecessor_inst;
	int predecessor_inst_max_depth;

	int num_markers;
};

struct _GtkTraceClass {
	GtkWidgetClass parent_class;

	void (* bounds_changed) (GtkTrace *t);
};

void gtk_trace_destroy(GtkObject *object);
GtkWidget* gtk_trace_new(struct multi_event_set* mes);
GtkType gtk_trace_get_type(void);
void gtk_trace_class_init(GtkTraceClass *class);
void gtk_trace_size_request(GtkWidget *widget, GtkRequisition *requisition);
void gtk_trace_size_allocate(GtkWidget *widget, GtkAllocation *allocation);
void gtk_trace_realize(GtkWidget *widget);
gboolean gtk_trace_expose(GtkWidget *widget, GdkEventExpose *event);
void gtk_trace_init(GtkTrace *trace);
void gtk_trace_paint(GtkWidget *widget);
void gtk_trace_enter_range_selection_mode(GtkWidget *widget);
void gtk_trace_clear_range_selection(GtkWidget *widget);
void gtk_trace_set_range_selection(GtkWidget *widget, int64_t start, int64_t end);
int gtk_trace_has_range_selection(GtkWidget *widget);
int gtk_trace_get_range_selection(GtkWidget *widget, int64_t* left, int64_t* right);
void gtk_trace_set_bounds(GtkWidget *widget, long double left, long double right);
void gtk_trace_set_cpu_offset(GtkWidget *widget, long double cpu_offset);
void gtk_trace_set_left(GtkWidget *widget, long double left);
void gtk_trace_set_right(GtkWidget *widget, long double right);
void gtk_trace_get_bounds(GtkWidget *widget, long double* left, long double* right);
void gtk_trace_set_draw_states(GtkWidget *widget, int val);
void gtk_trace_set_draw_comm_size(GtkWidget *widget, int val);
void gtk_trace_set_draw_steals(GtkWidget *widget, int val);
void gtk_trace_set_draw_pushes(GtkWidget *widget, int val);
void gtk_trace_set_draw_data_reads(GtkWidget *widget, int val);
void gtk_trace_set_draw_data_writes(GtkWidget *widget, int val);
void gtk_trace_set_draw_single_events(GtkWidget *widget, int val);
void gtk_trace_set_draw_counters(GtkWidget *widget, int val);
void gtk_trace_set_draw_annotations(GtkWidget *widget, int val);
void gtk_trace_set_draw_measurement_intervals(GtkWidget *widget, int val);
void gtk_trace_set_double_buffering(GtkWidget *widget, int val);
void gtk_trace_set_heatmap_num_shades(GtkWidget *widget, int num_shades);
void gtk_trace_set_heatmap_task_length_bounds(GtkWidget *widget, uint64_t min_length, uint64_t max_length);
void gtk_trace_set_map_mode(GtkWidget *widget, enum gtk_trace_map_mode model);
void gtk_trace_set_filter(GtkWidget *widget, struct filter* f);
struct filter* gtk_trace_get_filter(GtkWidget *widget);
void gtk_trace_set_highlighted_state_event(GtkWidget *widget, struct state_event* se);
struct state_event* gtk_trace_get_highlighted_state_event(GtkWidget *widget);
void gtk_trace_set_highlighted_task(GtkWidget *widget, struct single_event* texec_start);
struct single_event* gtk_trace_get_highlighted_task(GtkWidget *widget);
double gtk_trace_get_time_at(GtkWidget *widget, int x);
struct state_event* gtk_trace_get_state_event_at(GtkWidget *widget, int x, int y, int* cpu, int* worker);
struct annotation* gtk_trace_get_nearest_annotation_at(GtkWidget *widget, int x, int y);
void gtk_trace_set_markers(GtkWidget *widget, struct trace_marker* m, int num_markers);
int gtk_trace_get_cpu_at_y(GtkWidget *widget, int y);
struct event_set* gtk_trace_get_event_set_at_y(GtkWidget *widget, int y);
struct omp_for_chunk_set_part* gtk_trace_get_omp_chunk_set_part_at(GtkWidget *widget, int x, int y, int* cpu, int* worker);
struct omp_task_part* gtk_trace_get_omp_task_part_at(GtkWidget *widget, int x, int y, int* cpu, int* worker);
int gtk_trace_save_to_file(GtkWidget *widget, enum export_file_format format, const char* filename);
void gtk_trace_fit_all_cpus(GtkWidget *widget);

void gtk_trace_set_highlighted_predecessors(GtkWidget *widget, struct list_head* predecessors, int* num_predecessors, int max_depth);
void gtk_trace_reset_highlighted_predecessors(GtkWidget *widget);

extern gint gtk_trace_signals[GTK_TRACE_MAX_SIGNALS];

G_END_DECLS

#endif
