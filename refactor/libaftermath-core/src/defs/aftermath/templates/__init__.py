#!/usr/bin/env python

# Author: Andi Drebes <andi@drebesium.org>
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as published
# by the Free Software Foundation.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301,
# USA.

from aftermath.types import Type, FieldList, Field, CompoundType, EnumType
from aftermath.util import AbstractFunction, \
    enforce_type, \
    enforce_type_dict_values, \
    enforce_type_list
from aftermath import tags
import aftermath.types.builtin
import aftermath.types.aux
import jinja2
import os
import inspect

def leadingws(s):
    """Returns the leading whitespace of s"""

    enforce_type(s, str)

    ret = ""

    for c in s:
        if c == ' ' or c == '\t':
            ret += c
        else:
            return ret

    return ret

def trimlws(s):
    """Trims leading whitespace at the beginning of the first non-empty lines
    from all lines of the string `s`"""

    enforce_type(s, str)

    prefix_set = False
    prefix_len = 0
    prefix = ""
    ret = ""

    for i, line in enumerate(s.split("\n")):
        if not prefix_set and line:
            prefix = leadingws(line)
            prefix_set = True
            prefix_len = len(prefix)

        if prefix_set and line.startswith(prefix):
            ret += line[prefix_len:]+"\n"
        else:
            ret += line + "\n"

    return ret

class Template(object):
    """A simple template generating C code"""

    def __init__(self, default_args = {}):
        """`default_args` is a dictionary, for which each key-value pair will
        define a variable in the template whose name is the key and whose value
        is the value.
        """
        enforce_type(default_args, dict)

        self.__default_args = {
            "template" : self,
            "aftermath" : aftermath,
            "isinstance" : isinstance
        }
        self.__default_args.update(default_args)
        self.__required_tags = {}

    def getDefaultArgs(self):
        """Returns the default arguments for the template as a dictionary"""

        return dict(self.__default_args)

    def addDefaultArguments(self, **kwargs):
        """Creates a variable for each key = value pair passed to this function
        for the template"""

        self.__default_args.update(kwargs)

    def getName(self):
        """Returns the name of this template class as a string"""

        return self.__class__.__name__

    @AbstractFunction
    def render(self, **kwargs):
        """Returns the contents of the filled template as a string. All
        parameters are passed as key-value pairs to the template code."""

        pass

    def __str__(self):
        return self.render()

    def requireTags(self, t, tags_names, inherit = True):
        """Checks whether the type t has instances of all tag classes specified as
        values in the dictionary `tags_names`. The return value is a dictionary
        with the same keys as tags_names, but where the values are the instances
        of the specified tag classes.
        """

        enforce_type(t, Type)
        enforce_type_dict_values(tags_names, type)

        def_args = {}

        for tagname, tagclass in tags_names.iteritems():
            taginst = t.getTag(tagclass, inherit = inherit)

            if not taginst:
                raise Exception("Template '" + self.getName() + "' "
                                "requires tag " + \
                                "'" + tagclass.__name__ + "', but no such tag " +
                                "has been associated with type " +
                                "'" + t.getName() + "'.")
            else:
                def_args[tagname] = taginst

        return def_args

class Jinja2Template(Template):
    """A template based on the Jinja2 template engine"""

    def __init__(self, template_directory = None, *args, **kwargs):
        """`template_directory` is an optional string, indicating in which
        directory template files are searched (e.g., when using
        {% include ... %}) within the template"""

        super(Jinja2Template, self).__init__()

        enforce_type(template_directory, [str, type(None)])

        if template_directory:
            loader = jinja2.FileSystemLoader(template_directory)
        else:
            loader = jinja2.FileSystemLoader(
                os.path.dirname(inspect.getfile(self.__class__)))

        self.__env = jinja2.Environment(loader = loader,
                                        undefined = jinja2.StrictUndefined)

    def renderJinjaTemplate(self, template, **kwargs):
        """Renders a Jinja2.Template with the given arguments"""

        enforce_type(template, jinja2.Template)

        args = self.getDefaultArgs()
        args.update(**kwargs)
        return template.render(args)

    def getLoader(self):
        """Returns the Jinja2 loader used with this template"""

        return self.__loader

    def getEnvironment(self):
        """Returns the Jinja2 environment used with this template"""

        return self.__env

class Jinja2FileTemplate(Jinja2Template):
    """A Jinja2 template whose contents are generated from a template file"""

    def __init__(self, filename, *args, **kwargs):
        """`filename` specified from which file the template code is loaded"""

        enforce_type(filename, str)

        super(Jinja2FileTemplate, self).__init__(*args, **kwargs)
        self.__filename = filename

    def render(self, **kwargs):
        template = self.getEnvironment().get_template(self.getFilename())
        return self.renderJinjaTemplate(template, **kwargs)

    def getFilename(self):
        """Returns the filename the template code will be loaded from"""
        return self.__filename

class Jinja2StringTemplate(Jinja2Template):
    """A Jinja2 template whose contents are generated from a string"""

    def __init__(self, template_content, *args, **kwargs):
        """`template_content` is a string that contains directly the Jinja2
        template code"""

        enforce_type(template_content, str)
        super(Jinja2StringTemplate, self).__init__(*args, **kwargs)
        self.__template_content = trimlws(template_content)

    def render(self, **kwargs):
        template = self.getEnvironment().from_string(self.__template_content)
        return self.renderJinjaTemplate(template, **kwargs)

class StructFieldDefinition(Template):
    """A simple template that returns the C representation of a Field without the
    trailing semicolon."""

    def __init__(self, field, *args, **kwargs):
        super(StructFieldDefinition, self).__init__(*args, **kwargs)

        self.__field = field

    def render(self):
        ret = ""

        if self.__field.isConst():
            ret += "const "

        ret += self.__field.getType().getCType();

        if self.__field.isPointer():
            ret += "*" * self.__field.getPointerDepth()

        ret += " "+self.__field.getName()
        return ret

class StructDefinition(Jinja2StringTemplate):
    """Template generating the structure definition in C for a compound type"""

    def __init__(self, type, *args, **kwargs):
        enforce_type(type, CompoundType)

        template_content = trimlws("""
        /* {{t.getComment()}} */
        {{t.getCType()}} {
        {%- for field in t.getFields() %}
        	/* {{field.getComment()}} */
        	{{aftermath.templates.StructFieldDefinition(field)}};
        {% endfor -%}
        {%- if t.hasTag(aftermath.tags.Packed) -%}
        } __attribute__((packed));
        {%- else -%}
        };
        {%- endif %}""")

        super(StructDefinition, self).__init__(
            template_content = template_content, *args, **kwargs)
        self.addDefaultArguments(t = type)

class EnumDefinition(Jinja2StringTemplate):
    """Template generating the definition in C for an enum"""

    def __init__(self, type, *args, **kwargs):
        enforce_type(type, EnumType)

        template_content = trimlws("""
        /* {{t.getComment()}} */
        {{t.getCType()}} {
        {%- for variant in t.getVariants() %}
        	{% if variant.getComment() -%}/* {{variant.getComment()}} */{% endif %}
        	{{variant.getName()}}{% if variant.getValue() is not none %} = {{ variant.getValue() }}{% endif %}
        	{%- if not loop.last %},
{% endif %}
        {%- endfor %}
        };""")

        super(EnumDefinition, self).__init__(
            template_content = template_content, *args, **kwargs)
        self.addDefaultArguments(t = type)

class FunctionTemplate(Template):
    """A template implementing a function"""

    def __init__(self,
                 function_name,
                 return_type,
                 arglist,
                 inline = False,
                 returns_pointer = False,
                 *args,
                 **kwargs):
        """`function_name` is a string containing the name of function that will
        be used in C.

        `return_type` must be an instance of aftermath.types.Type, defining the
        function's return type.

        `arglist` is must be an instance of aftermath.types.FieldList (which may
        be empty), which will be used for the declaration of the function's
        parameters. That is, the field names become the names for the formal
        parameters and the associated field types become the parameter types.

        If `inline` is true, the function will be declared inline.

        If `returns_pointer` is true, the return type of the generated function
        will be the <return_type>* instead of <return_type>.
        """

        enforce_type(function_name, str)
        enforce_type(return_type, Type)
        enforce_type(arglist, FieldList)
        enforce_type(inline, bool)
        enforce_type(returns_pointer, bool)

        Template.__init__(self, *args, **kwargs)

        self.__function_name = function_name
        self.__return_type = return_type
        self.__arglist = arglist
        self.__inline = inline
        self.__returns_pointer = returns_pointer

    def getFunctionName(self):
        """Returns the function name as a string"""

        return self.__function_name

    def getReturnType(self):
        """Returns the function's return type as a Type"""

        return self.__return_type

    def returnsPointer(self):
        """Returns true if the return value is a pointer"""

        return self.__returns_pointer

    def getArgumentList(self):
        """Returns the function's argument list as FieldList"""

        return self.__arglist

    def getSignature(self):
        """Returns a string with the function signature"""

        return ("static inline " if self.isInline() else "") + \
            self.__return_type.getCType() + \
            ("*" if self.returnsPointer() else "") + \
            " " + \
            self.getFunctionName() + "(" + \
            ", ".join([StructFieldDefinition(f).render() \
                       for f in self.getArgumentList()]) + ")"

    def isInline(self):
        """Returns True if this is a function marked as inline for the
        compiler"""

        return self.__inline

    def getPrototype(self):
        """Returns a string with the function signature followed by a
        semicolon"""

        return self.getSignature()+";"

class Destructor(FunctionTemplate, Jinja2StringTemplate):
    """A template implementing a destructor"""

    def __init__(self, type):
        enforce_type(type, CompoundType)

        if not type.hasTag(tags.Destructor):
            raise Exception("Destructor template requires destructor tag")

        dtag = type.getTag(tags.Destructor)

        FunctionTemplate.__init__(
            self,
            function_name = dtag.getFunctionName(),
            return_type = aftermath.types.builtin.void,
            arglist = FieldList([
                Field(name = "e",
                      type = type,
                      is_pointer = dtag.takesAddress())
            ]))

        template_content = trimlws("""
        /* Destroys a {{ t.getEntity() }} */
        {{template.getSignature()}}
        {
        	{%- for field in t.getFields() -%}
        	{%- if field.hasCustomDestructor() %}
        	{%- if field.isPointer() %}
        	{{field.getCustomDestructorName()}}(e->{{field.getName()}});
        	{%- else %}
        	{{field.getCustomDestructorName()}}(&e->{{field.getName()}});
        	{%- endif %}
        	{%- else %}
        	{%- if field.getType().hasDestructor() %}
        	{%- set dtag = field.getType().getTag(aftermath.tags.Destructor) %}

        	{%- if not field.isPointer() or field.isOwned() %}
        	{%- if field.isPointer() or not dtag.takesAddress()%}
        	{{dtag.getFunctionName()}}(e->{{field.getName()}});
        	{%- else %}
        	{{dtag.getFunctionName()}}(&e->{{field.getName()}});
        	{%- endif -%}
        	{%- endif -%}
        	{%- endif -%}
        	{%- endif -%}
        	{%- endfor -%}
        {# #}
        }""")

        Jinja2StringTemplate.__init__(self, template_content)
        self.addDefaultArguments(t = type)

class DefaultConstructor(FunctionTemplate, Jinja2StringTemplate):
    """A template implementing a default constructor"""

    def __init__(self, type):
        enforce_type(type, CompoundType)

        if not type.hasTag(tags.DefaultConstructor):
            raise Exception("Default constructor template requires "+
                            "default constructor tag")

        ctag = type.getTag(tags.DefaultConstructor)
        gen_tag = type.getTag(tags.GenerateDefaultConstructor)

        FunctionTemplate.__init__(
            self,
            function_name = ctag.getFunctionName(),
            return_type = aftermath.types.builtin.int,
            arglist = FieldList([
                Field(name = "e",
                      type = type,
                      is_pointer = True)
            ]))

        template_content = trimlws("""
        {%- set can_fail = {"value" : False} -%}
        /* Initializes a {{ t.getEntity() }} with default values*/
        {{template.getSignature()}}
        {
        	{%- for field in t.getFields() -%}
        	{%- if not field.isPointer() and field.getType().hasDefaultConstructor() %}
        	{%- set ctag = field.getType().getTag(aftermath.tags.DefaultConstructor) %}
        	if({{ctag.getFunctionName()}}(&e->{{field.getName()}}))
        		goto out_err_{{field.getName()}};

        	{%- if can_fail.update({"value": True}) %}{% endif %}
        	{%- endif %}
        	{%- endfor %}

        	{%- for (fieldname, val) in gen_tag.getFieldValues() %}
        	e->{{fieldname}} = {{val}};
        	{%- endfor %}

        	return 0;

        {%- if can_fail.value %}
        	{%- set is_first = True -%}
        	{%- for field in t.getFields()|reverse -%}
        	{%- if field.hasCustomDestructor() %}
        	{%- if field.isPointer() %}
        	{{field.getCustomDestructorName()}}(e->{{field.getName()}});
        	{%- else -%}
        	{{field.getCustomDestructorName()}}(&e->{{field.getName()}});
        	{%- endif -%}
        	{%- elif field.getType().hasDestructor() %}
        	{%- set dtag = field.getType().getTag(aftermath.tags.Destructor) %}
         out_err_{{field.getName()}}:
        	{% if not is_first %}
        	{% if not field.isPointer() or field.isOwned() %}
        	{%- if field.isPointer() or not dtag.takesAddress() %}
        	{{dtag.getFunctionName()}}(e->{{field.getName()}});
        	{%- else %}
        	{{dtag.getFunctionName()}}(&e->{{field.getName()}});
        	{%- endif -%}
        	{%- endif -%}
        	{%- endif -%}
        	{%- endif -%}
        	{% set is_first = False -%}
        	{%- endfor -%}
        	return 1;
        {%- endif %}
        {# #}
        }""")

        Jinja2StringTemplate.__init__(self, template_content)
        self.addDefaultArguments(t = type, gen_tag = gen_tag)

def gen_function_file_template_class(return_type,
                                     arglist,
                                     class_name,
                                     required_tags,
                                     directory,
                                     file_name = None,
                                     function_name_tag = "gen_tag",
                                     extra_base_classes = None):
    """Generates a template class with the name `class_name` generating a function
    with the return type `return_type` and the arguments specified in the
    FieldList `arglist` from a template file specified by `file_name` located in
    in `directory`.

    The dictionary `required_tags` is composed of names for tags and tag
    classes. The type on which the template is applied is queried for the
    specified tags.

    The name of the function is determined by calling getFunctionName() on the
    required tag with name `function_name_tag` on the associated tag.

    `extra_base_classes` may be a list with additional classes the generated
    class inherits from.
    """

    enforce_type(arglist, FieldList)
    enforce_type(return_type, Type)
    enforce_type(file_name, [str, type(None)])
    enforce_type(directory, str)
    enforce_type(required_tags, dict)
    enforce_type(function_name_tag, str)
    enforce_type(extra_base_classes, [list, type(None)])

    if extra_base_classes == None:
        extra_base_classes = []
    else:
        enforce_type_list(extra_base_classes, type)

    if file_name == None:
        file_name = class_name + ".tpl.c"

    def class_init(self, t):
        enforce_type(t, aftermath.types.Type)

        Jinja2FileTemplate.__init__(self,
                                    template_directory = directory,
                                    filename = file_name)

        reqtags = self.requireTags(t, required_tags)

        FunctionTemplate.__init__(
            self,
            function_name = reqtags[function_name_tag].getFunctionName(),
            return_type = return_type,
            inline = True,
            arglist = arglist)

        self.addDefaultArguments(t = t, **reqtags)

    base_classes = [ FunctionTemplate, Jinja2FileTemplate ] + extra_base_classes

    return type(class_name,
                tuple(base_classes),
                {"__init__": class_init })

def gen_function_file_template_class_int_ctx(*args, **kwargs):
    """Same as `gen_function_file_template_class`, but sets the return type of
    the function to int and specifies a single parameter ctx that is an
    am_io_context pointer
    """

    return gen_function_file_template_class(
        return_type = aftermath.types.builtin.int,
        arglist = FieldList([
            Field(name = "ctx",
                  type = aftermath.types.aux.am_io_context,
                  is_pointer = True)
        ]),
        *args,
        **kwargs)
