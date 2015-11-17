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

#include "color_scheme.h"
#include "ansi_extras.h"
#include "color.h"
#include "object_notation.h"
#include <regex.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

/* Human-readable names for rule types */
const char* color_scheme_rule_type_names[] = {
	"task symbol",
	"state name",
	"counter name"
};

/* Machine-readable names for rule types */
const char* color_scheme_rule_type_syms[] = {
	"task_symbol",
	"state_name",
	"counter_name"
};

/* Returns a human-readable string for a scheme rule type. */
const char* color_scheme_rule_type_name(enum color_scheme_rule_type type)
{
	return color_scheme_rule_type_names[type];
}

/* Writes the prefix nprefix times to fp before writing the string str
 * to fp. */
static int fputs_prefix(const char* str, const char* prefix, int nprefix, FILE* fp)
{
	for(int i = 0; i < nprefix; i++)
		if(fputs(prefix, fp) < 0)
			return 1;

	if(fputs(str, fp) < 0)
		return 1;

	return 0;
}

/* Checks whether the regular expression of a color scheme rule
 * matches the string str. The check uses POSIX extended regular
 * expressions and is case-sensitive. */
int color_scheme_rule_regex_matches(struct color_scheme_rule* csr, const char* str)
{
	regex_t regex;
	int ret;

	/* TODO: this silently ignores invalid regex */
	if(regcomp(&regex, csr->regex, REG_EXTENDED | REG_NOSUB))
		return 0;

	ret = !regexec(&regex, str, 0, NULL, 0);
	regfree(&regex);

	return ret;
}

/* Returns the type for the machine-readable scheme rule type
 * string. If the string does not represent a valid type the function
 * returns 1, otherwise 0. */
int color_scheme_rule_type_from_string(const char* str, enum color_scheme_rule_type* type)
{
	for(int i = 0; i < COLOR_SCHEME_RULE_TYPE_MAX; i++) {
		if(strcmp(color_scheme_rule_type_syms[i], str) == 0) {
			*type = i;
			return 0;
		}
	}

	return 1;
}

/* Returns the type for the human-readable scheme rule type string. If
 * the string does not represent a valid type the function returns 1,
 * otherwise 0. */
int color_scheme_rule_type_from_name(const char* str, enum color_scheme_rule_type* type)
{
	for(int i = 0; i < COLOR_SCHEME_RULE_TYPE_MAX; i++) {
		if(strcmp(color_scheme_rule_type_names[i], str) == 0) {
			*type = i;
			return 0;
		}
	}

	return 1;
}

/* Sets the regex of a color scheme rule. If the rule already has a
 * regular expression, the memory for the regular expression is
 * extended / shrunk automatically. If memory allocation is sucessful
 * the function returns 0, otherwise 1. */
int color_scheme_rule_set_regex(struct color_scheme_rule* csr, const char* regex)
{
	char* tmp = realloc(csr->regex, strlen(regex)+1);

	if(!tmp)
		return 1;

	csr->regex = tmp;
	strcpy(csr->regex, regex);

	return 0;
}

/* Sets the color of a color scheme rule. The color components must be
 * values between 0.0 and 1.0. */
void color_scheme_rule_set_color(struct color_scheme_rule* csr, double r, double g, double b)
{
	csr->color_r = r;
	csr->color_g = g;
	csr->color_b = b;
}

/* Initializes a color scheme rule with a type and regular
 * expression. Returns 0 on success, otherwise 1. */
int color_scheme_rule_init(struct color_scheme_rule* csr, enum color_scheme_rule_type type, const char* regex)
{
	csr->type = type;
	csr->regex = NULL;
	return color_scheme_rule_set_regex(csr, regex);
}

/* Adds the color scheme rule csr to the color scheme cs. Ownership
 * for memory is transferred to the color scheme (i.e., the rule will
 * be destroyed and freed automatically upon destruction of the color
 * scheme and therefore must have been allocated on the heap). Returns
 * 0 on success, otherwise 1. */
int color_scheme_add_color_scheme_rule(struct color_scheme* cs, struct color_scheme_rule* csr)
{
	struct color_scheme_rule** tmp = realloc(cs->rules, (cs->num_rules+1) * sizeof(struct color_scheme_rule*));

	if(!tmp)
		return 1;

	cs->rules = tmp;
	cs->rules[cs->num_rules] = csr;
	cs->num_rules++;

	return 0;
}

/* Removes a rule from a color scheme. Ownership for memory of the
 * rule is transferred to the caller.  Returns 0 on success, otherwise
 * 1.*/
int color_scheme_remove_color_scheme_rule(struct color_scheme* cs, struct color_scheme_rule* csr)
{
	struct color_scheme_rule** tmp;
	int idx = -1;

	for(int i = 0; i < cs->num_rules; i++) {
		if(cs->rules[i] == csr) {
			idx = i;
			break;
		}
	}

	if(idx == -1)
		return 0;

	memmove(&cs->rules[idx], &cs->rules[idx+1], (cs->num_rules-idx-1)*sizeof(struct color_scheme_rule*));
	cs->num_rules--;

	tmp = realloc(cs->rules, cs->num_rules * sizeof(struct color_scheme_rule*));

	if(!tmp && cs->num_rules)
		return 1;

	cs->rules = tmp;

	return 0;
}

/* Writes the string representation in object notation of a color
 * scheme rule to fp. The parameter indent specifies by how many tabs
 * each line should be indented. Returns 0 on success, otherwise 1. */
int color_scheme_rule_save_fp(struct color_scheme_rule* csr, FILE* fp, int indent)
{
	char htmlrgb[8];
	char* escaped;

	if(fputs_prefix("color_scheme_rule {\n", "\t", indent, fp))
		return 1;

	if(fputs_prefix("type: \"", "\t", indent+1, fp))
		return 1;

	if(fputs(color_scheme_rule_type_syms[csr->type], fp) < 0)
		return 1;

	if(fputs("\",\n", fp) < 0)
		return 1;

	if(fputs_prefix("regex: \"", "\t", indent+1, fp) < 0)
		return 1;

	if(!(escaped = escape_string(csr->regex)))
		return 1;

	if(fputs(escaped, fp) < 0) {
		free(escaped);
		return 1;
	}

	free(escaped);

	if(fputs("\",\n", fp) < 0)
		return 1;

	if(fputs_prefix("color: \"", "\t", indent+1, fp) < 0)
		return 1;

	color_to_htmlrgb(csr->color_r, csr->color_g, csr->color_b, htmlrgb);

	if(fputs(htmlrgb, fp) < 0)
		return 1;

	if(fputs("\"\n", fp) < 0)
		return 1;

	if(fputs_prefix("}", "\t", indent, fp))
		return 1;

	return 0;
}

/* Destroys a color scheme rule */
void color_scheme_rule_destroy(struct color_scheme_rule* csr)
{
	free(csr->regex);
}

/* Sets the name of a color scheme. If the scheme already has a name,
 * the memory for the name is extended / shrunk automatically. If
 * memory allocation is sucessful the function returns 0, otherwise
 * 1. */
int color_scheme_set_name(struct color_scheme* cs, const char* name)
{
	char* tmp = realloc(cs->name, strlen(name)+1);

	if(!tmp)
		return 1;

	cs->name = tmp;
	strcpy(cs->name, name);

	return 0;
}

/* Initializes a color scheme with the name passed as a
 * parameter. Returns 0 on success, otherwise 1. */
int color_scheme_init(struct color_scheme* cs, const char* name)
{
	cs->name = NULL;
	cs->rules = NULL;
	cs->num_rules = 0;

	return color_scheme_set_name(cs, name);
}

/* Writes the string representation in object notation of a color
 * scheme to fp. The parameter indent specifies by how many tabs each
 * line should be indented. Returns 0 on success, otherwise 1. */
int color_scheme_save_fp(struct color_scheme* cs, FILE* fp, int indent)
{
	char* escaped;

	if(fputs_prefix("color_scheme {\n", "\t", indent, fp))
		return 1;

	if(fputs_prefix("name: \"", "\t", indent+1, fp))
		return 1;

	if(!(escaped = escape_string(cs->name)))
		return 1;

	if(fputs(escaped, fp) < 0) {
		free(escaped);
		return 1;
	}

	free(escaped);

	if(fputs("\",\n", fp) < 0)
		return 1;

	if(fputs_prefix("rules: [\n", "\t", indent+1, fp))
		return 1;

	for(int i = 0; i < cs->num_rules; i++) {
		if(color_scheme_rule_save_fp(cs->rules[i], fp, indent+2))
			return 1;

		if(i == cs->num_rules-1) {
			if(fputs("\n", fp) < 0)
				return 1;
		} else {
			if(fputs(",\n", fp) < 0)
				return 1;
		}
	}

	if(fputs_prefix("]\n", "\t", indent+1, fp))
		return 1;

	if(fputs_prefix("}", "\t", indent, fp))
		return 1;

	return 0;
}

/* Destroys a color scheme */
void color_scheme_destroy(struct color_scheme* cs)
{
	for(int i = 0; i < cs->num_rules; i++) {
		color_scheme_rule_destroy(cs->rules[i]);
		free(cs->rules[i]);
	}

	free(cs->rules);
	free(cs->name);
}

/* Initializes a color scheme set */
void color_scheme_set_init(struct color_scheme_set* css)
{
	css->schemes = NULL;
	css->num_schemes = 0;
}

/* Adds the color scheme cs to the color scheme set css. Ownership for
 * memory is transferred to the color scheme set (i.e., the scheme
 * will be destroyed and freed automatically upon destruction of the
 * color scheme set and therefore must have been allocated on the
 * heap). Returns 0 on success, otherwise 1. */
int color_scheme_set_add_color_scheme(struct color_scheme_set* css, struct color_scheme* cs)
{
	struct color_scheme** tmp = realloc(css->schemes, (css->num_schemes+1) * sizeof(struct color_scheme*));

	if(!tmp)
		return 1;

	css->schemes = tmp;
	css->schemes[css->num_schemes] = cs;
	css->num_schemes++;

	return 0;
}

/* Removes a scheme from a color scheme set. Ownership for memory of
 * the scheme is transferred to the caller.  Returns 0 on success,
 * otherwise 1.*/
int color_scheme_set_remove_color_scheme(struct color_scheme_set* css, struct color_scheme* cs)
{
	struct color_scheme** tmp;
	int idx = -1;

	for(int i = 0; i < css->num_schemes; i++) {
		if(css->schemes[i] == cs) {
			idx = i;
			break;
		}
	}

	if(idx == -1)
		return 0;

	memmove(&css->schemes[idx], &css->schemes[idx+1], (css->num_schemes-idx-1)*sizeof(struct color_scheme*));
	css->num_schemes--;

	tmp = realloc(css->schemes, css->num_schemes * sizeof(struct color_scheme*));

	if(!tmp && css->num_schemes)
		return 1;

	css->schemes = tmp;

	return 0;
}

/* Destroys a color scheme set. */
void color_scheme_set_destroy(struct color_scheme_set* css)
{
	for(int i = 0; i < css->num_schemes; i++) {
		color_scheme_destroy(css->schemes[i]);
		free(css->schemes[i]);
	}

	free(css->schemes);
}

/* Initializes a color scheme rule from an object notation node
 * containing the description of a color scheme rule. Validity of the
 * object notation is checked. The function returns 0 if the node
 * contains a valid color scheme rule description and if the
 * initialization of the rule has been successful. In case of an error
 * the function returns 1. */
int color_scheme_rule_from_object_notation(struct color_scheme_rule* csr, struct object_notation_node* root)
{
	struct object_notation_node_group* root_group;
	struct object_notation_node_member* member;
	struct object_notation_node_string* member_str;

	if(root->type != OBJECT_NOTATION_NODE_TYPE_GROUP)
		return 1;

	root_group = (struct object_notation_node_group*)root;

	if(!object_notation_node_group_has_members(root_group, 1,
						   "type", "regex",
						   "color", NULL))
	{
		goto out_err;
	}

	csr->regex = NULL;

	object_notation_for_each_group_member(root_group, member) {
		if(member->def->type != OBJECT_NOTATION_NODE_TYPE_STRING)
			return 1;

		member_str = (struct object_notation_node_string*)member->def;

		if(strcmp(member->name, "regex") == 0) {
			if(!(csr->regex = strdup(member_str->value)))
				goto out_err_regex;
		} else if(strcmp(member->name, "type") == 0) {
			if(color_scheme_rule_type_from_string(member_str->value, &csr->type))
				goto out_err_regex;
		} else if(strcmp(member->name, "color") == 0) {
			if(htmlrgb_to_double(member_str->value, &csr->color_r, &csr->color_g, &csr->color_b))
				goto out_err_regex;
		}
	}

	return 0;

out_err_regex:
	free(csr->regex);
out_err:
	return 1;
}

/* Initializes a color scheme from an object notation node containing
 * the description of a color scheme. Validity of the object notation
 * is checked. The function returns 0 if the node contains a valid
 * color scheme description and if the initialization of the scheme
 * has been successful. In case of an error the function returns 1. */
int color_scheme_from_object_notation(struct color_scheme* cs, struct object_notation_node* root)
{
	struct object_notation_node_group* root_group;
	struct object_notation_node_list* list;
	struct object_notation_node* iter;
	struct object_notation_node* member_def;
	struct object_notation_node_string* member_str;
	struct color_scheme_rule* csr;

	if(root->type != OBJECT_NOTATION_NODE_TYPE_GROUP)
		goto out_err;

	root_group = (struct object_notation_node_group*)root;

	if(strcmp(root_group->name, "color_scheme") != 0)
		goto out_err;

	if(!object_notation_node_group_has_members(root_group, 1, "rules", "name", NULL))
		goto out_err;

	member_def = object_notation_node_group_get_member_def(root_group, "name");
	member_str = (struct object_notation_node_string*)member_def;

	if(color_scheme_init(cs, member_str->value))
		goto out_err;

	member_def = object_notation_node_group_get_member_def(root_group, "rules");
	list = (struct object_notation_node_list*)member_def;

	object_notation_for_each_list_item(list, iter) {
		if(!(csr = malloc(sizeof(*csr))))
			goto out_err_destroy;

		if(color_scheme_rule_from_object_notation(csr, iter))
			goto out_err_free;

		if(color_scheme_add_color_scheme_rule(cs, csr))
			goto out_err_destroy_csr;
	}

	return 0;

out_err_destroy_csr:
	color_scheme_rule_destroy(csr);
out_err_free:
	free(csr);
out_err_destroy:
	color_scheme_destroy(cs);
out_err:
	return 1;
}

/* Initializes a color scheme set from an object notation node
 * containing the description of a color scheme set. Validity of the
 * object notation is checked. The function returns 0 if the node
 * contains a valid color scheme set description and if the
 * initialization of the set has been successful. In case of an error
 * the function returns 1. */
int color_scheme_set_from_object_notation(struct color_scheme_set* css, struct object_notation_node* root)
{
	struct object_notation_node_group* root_group;
	struct object_notation_node* member;
	struct object_notation_node_list* list;
	struct object_notation_node* iter;
	struct color_scheme* cs;

	if(root->type != OBJECT_NOTATION_NODE_TYPE_GROUP)
		goto out_err;

	root_group = (struct object_notation_node_group*)root;

	if(strcmp(root_group->name, "color_scheme_set") != 0)
		goto out_err;

	if(!object_notation_node_group_has_members(root_group, 1, "schemes", NULL))
		goto out_err;

	member = object_notation_node_group_get_member_def(root_group, "schemes");
	list = (struct object_notation_node_list*)member;

	object_notation_for_each_list_item(list, iter) {
		if(!(cs = malloc(sizeof(*cs))))
			goto out_err_destroy;

		if(color_scheme_from_object_notation(cs, iter))
			goto out_err_free;

		if(color_scheme_set_add_color_scheme(css, cs))
			goto out_err_destroy_cs;
	}

	return 0;

out_err_destroy_cs:
	color_scheme_destroy(cs);
out_err_free:
	free(cs);
out_err_destroy:
	color_scheme_set_destroy(css);
out_err:
	return 1;
}

/* Initializes a color scheme from a string containing a description
 * of a color scheme in object notation. Validity of the object
 * notation is checked. The function returns 0 if the description is
 * valid and if the initialization of the set has been successful. In
 * case of an error the function returns 1. */
int color_scheme_set_from_string(struct color_scheme_set* css, const char* str, size_t len)
{
	struct object_notation_node* root;
	int ret = 1;

	if(!(root = object_notation_parse(str, len)))
		goto out_err;

	if(color_scheme_set_from_object_notation(css, root))
		goto out_err_destroy;

	ret = 0;

out_err_destroy:
	object_notation_node_destroy(root);
	free(root);
out_err:
	return ret;
}

/* Initializes a color scheme from a file containing string with a
 * description of a color scheme in object notation. Validity of the
 * file is checked. The function returns 0 if the description is valid
 * and if the initialization of the set has been successful. In case
 * of an error the function returns 1. */
int color_scheme_set_load(struct color_scheme_set* css, const char* filename)
{
	int fd;
	char* css_str;
	off_t size;
	int ret = 1;

	if((size = file_size(filename)) == -1)
		goto out;

	if((fd = open(filename, O_RDONLY)) == -1)
		goto out;

	css_str = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);

	if(css_str == MAP_FAILED)
		goto out_fd;

	ret = color_scheme_set_from_string(css, css_str, size);

	munmap(css_str, size);

	ret = 0;

out_fd:
	close(fd);
out:
	return ret;
}

/* Saves a color scheme set to fp using object notation. Returns 0 on
 * success, otherwise 1. */
int color_scheme_set_save_fp(struct color_scheme_set* css, FILE* fp)
{
	if(fputs("color_scheme_set {\n", fp) < 0)
		return 1;

	if(fputs_prefix("schemes: [\n", "\t", 1, fp))
		return 1;

	for(int i = 0; i < css->num_schemes; i++) {
		if(color_scheme_save_fp(css->schemes[i], fp, 2))
			return 1;

		if(i == css->num_schemes-1) {
			if(fputs("\n", fp) < 0)
				return 1;
		} else {
			if(fputs(",\n", fp) < 0)
				return 1;
		}
	}

	if(fputs_prefix("]\n", "\t", 1, fp))
		return 1;

	if(fputs("}\n", fp) < 0)
		return 1;

	return 0;
}

/* Saves a color scheme set to a file using object notation. The file
 * will be overwritten or created if necessary. Returns 0 on success,
 * otherwise 1. */
int color_scheme_set_save(struct color_scheme_set* css, const char* filename)
{
	FILE* fp = fopen(filename, "w+");
	int ret = 0;

	if(!fp)
		return 1;

	ret = color_scheme_set_save_fp(css, fp);
	fclose(fp);

	return ret;
}
