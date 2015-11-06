/**
 * Copyright (C) 2015 Andi Drebes <andi.drebes@lip6.fr>
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

#ifndef COLOR_SCHEME_H
#define COLOR_SCHEME_H

#include <stdint.h>
#include <stdio.h>

/* A color scheme defines a set of color scheme rules for coloring
 * tasks, states and counters. Color schemes can be grouped in a color
 * scheme set. */

enum color_scheme_rule_type {
	COLOR_SCHEME_RULE_TYPE_TASKNAME,
	COLOR_SCHEME_RULE_TYPE_STATENAME,
	COLOR_SCHEME_RULE_TYPE_COUNTERNAME,
	COLOR_SCHEME_RULE_TYPE_MAX
};

struct color_scheme_rule {
	enum color_scheme_rule_type type;
	char* regex;

	double color_r;
	double color_g;
	double color_b;
};

struct color_scheme {
	char* name;
	unsigned int num_rules;
	struct color_scheme_rule** rules;
};

struct color_scheme_set {
	unsigned int num_schemes;
	struct color_scheme** schemes;
};

const char* color_scheme_rule_type_name(enum color_scheme_rule_type type);
int color_scheme_rule_type_from_string(const char* str, enum color_scheme_rule_type* type);
int color_scheme_rule_type_from_name(const char* str, enum color_scheme_rule_type* type);
int color_scheme_rule_regex_matches(struct color_scheme_rule* csr, const char* str);
int color_scheme_rule_is_valid_regex(const char* sregex);
int color_scheme_rule_is_valid_color(const char* color);
int color_scheme_rule_set_regex(struct color_scheme_rule* csr, const char* regex);
void color_scheme_rule_set_color(struct color_scheme_rule* csr, double r, double g, double b);
int color_scheme_rule_init(struct color_scheme_rule* csr, enum color_scheme_rule_type type, const char* regex);
void color_scheme_rule_destroy(struct color_scheme_rule* csr);

int color_scheme_set_name(struct color_scheme* cs, const char* name);
int color_scheme_add_color_scheme_rule(struct color_scheme* cs, struct color_scheme_rule* csr);
int color_scheme_remove_color_scheme_rule(struct color_scheme* cs, struct color_scheme_rule* csr);
int color_scheme_init(struct color_scheme* cs, const char* name);
void color_scheme_destroy(struct color_scheme* cs);

void color_scheme_set_init(struct color_scheme_set* css);
int color_scheme_set_add_color_scheme(struct color_scheme_set* css, struct color_scheme* cs);
int color_scheme_set_remove_color_scheme(struct color_scheme_set* css, struct color_scheme* cs);
void color_scheme_set_destroy(struct color_scheme_set* css);

int color_scheme_set_save_fp(struct color_scheme_set* css, FILE* fp);
int color_scheme_set_save(struct color_scheme_set* css, const char* filename);
int color_scheme_set_load(struct color_scheme_set* css, const char* filename);
int color_scheme_set_from_string(struct color_scheme_set* css, const char* str, size_t len);

#endif
