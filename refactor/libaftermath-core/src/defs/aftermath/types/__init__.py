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

from aftermath.util import AbstractFunction, \
    enforce_type, \
    enforce_type_list, \
    enforce_type_tuple
from aftermath import tags
from aftermath import relations
import aftermath
import copy
import types
import re

#################################################################################

class Type(object):
    """Base class for all type definitions for the Aftermath type system"""

    def __init__(self,
                 name,
                 entity,
                 comment,
                 ident = None,
                 tags = None):
        """`name` is the internal name of the type and is used to prefix
        functions and types in C. It must therefore be a valid prefix for C type
        names and identifiers.

        `entity` is a human-readable name for the type describing what an
        instance of the type represents.

        `comment` is a human-readable comment of any length describing the
        defined type.

        `tags` is a list with type tags guiding templates using the defined
        type.
        """

        enforce_type(name, str)
        enforce_type(entity, [str, type(None)])
        enforce_type(comment, [str, type(None)])
        enforce_type(ident, [str, type(None)])
        enforce_type(tags, [list, type(None)])

        self.__tags = []
        self.__tags_hashed = {}

        if tags:
            for d in tags:
                self.addTag(d)

        self.__name = name
        self.__entity = entity
        self.__comment = comment
        self.__filter_predicates = {}

        self.__ident = ident

        self.__relations = []
        self.__relations_hashed = {}

    def finalize(self):
        """Called by the type system when all types have been collected and the
        system is about to be used for code generation"""

        if self.hasDestructor():
            if not self.hasTag(tags.GenerateDestructor):
                self.addTag(tags.GenerateDestructor())

        if self.hasDefaultConstructor():
            if not self.hasTag(tags.GenerateDefaultConstructor):
                self.addTag(tags.GenerateDefaultConstructor())

    def getComment(self):
        return self.__comment

    def getEntity(self):
        return self.__entity

    def getName(self):
        return self.__name

    def getStripName(self):
        """Returns the name without preceding "am_" (if present)"""
        return re.sub("^am_", "", self.__name)

    def getIdent(self):
        if not self.__ident:
            raise Exception("Identifier string for type " + \
                            "'" + self.getName() + "' not set.")
        else:
            return self.__ident

    @AbstractFunction
    def getCType(self):
        """Generates a string representing the type in C"""
        pass

    @AbstractFunction
    def isInteger(self):
        """Returns true if the defined type is an integer or an alias for an
        integer type"""

        pass

    def isCompound(self):
        """Returns true if the defined type is a structure"""
        return self.hasTag(tags.Compound)

    def hasDestructor(self):
        """Returns True if a destructor must be invoked when an instance of this
        type is destroyed"""

        return self.hasTag(tags.Destructor)

    def getDestructorName(self):
        """Returns the name of the destructor for this type"""

        if not self.hasDestructor():
            raise Exception("Destructor name requested, but the type doesn't " +\
                            "have a destructor")

        return self.getTag(tags.Destructor).getFunctionName()

    def hasDefaultConstructor(self):
        """Returns True if a default constructor must be invoked when an instance
        of this type is created"""

        return self.hasTag(tags.DefaultConstructor)

    def getDefaultConstructorName(self):
        """Returns the name of the default constructor for this type"""

        if not self.hasDefaultConstructor():
            raise Exception("Default constructor name requested, but the type "+
                            "doesn't have a default constructor")

        return self.getTag(tags.DefaultConstructor).getFunctionName()

    def __addMultiTag(self, tag):
        """Add a tag to the type that can be associated multiple times"""

        if tag.__class__ in self.__tags_hashed.keys():
            self.__tags_hashed[tag.__class__].append(tag)
        else:
            self.__tags_hashed[tag.__class__] = [ tag ]

    def __addUniqueTag(self, tag):
        """Add a tag to the type that can only be associated once"""

        if tag.__class__ in self.__tags_hashed.keys():
            raise Exception("Unique tag '" + \
                            tag.__module__ + "." + \
                            tag.__class__.__name__ + \
                            "' already added for type '" + \
                            self.getName() + "'")

        self.__tags_hashed[tag.__class__] = tag

    def addTag(self, tag):
        """Adds a type tag to the list of tags of this type."""

        enforce_type(tag, tags.Tag)

        if isinstance(tag, tags.MultiTag):
            self.__addMultiTag(tag)
        else:
            self.__addUniqueTag(tag)

        self.__tags.append(tag)
        tag.setType(self)

    def addTags(self, *tags):
        """Adds a list of type tags to this type."""
        for tag in tags:
            self.addTag(tag)

    def getOrAddTagInheriting(self, tagclass, *args, **kwargs):
        """Returns the tag that is an direct or inheriting instance of `tagclass`
        associated to the type. If no such tag exists, the tag class is
        instantiated using the arguments provided in *args and **kwargs.
        """

        (t, _) = self.getOrAddTagInheritingRes(tagclass, *args, **kwargs)
        return t

    def getOrAddTagInheritingRes(self, tagclass, *args, **kwargs):
        """Same as `getOrAddTagInheriting`, but returns a pair (t, create),
        where `t` is the tag instance and `create` a boolean indicating whether
        the tag needed to be created (create = True) or already existed
        (create = False).
        """
        enforce_type(tagclass, type)

        create = False
        tag = self.getTagInheriting(tagclass)

        if not tag:
            create = True
            tag = tagclass(*args, **kwargs)
            self.addTag(tag)

        return (tag, create)

    def hasTag(self, c):
        """Checks whether a tag of the given class has been associated with this
        type"""

        return self.getTag(c) != None

    def getTag(self, c, inherit = True, multiple = False):
        """Returns the type tag of the given class or None if no such tag has
        been associated with this type. If `inherit` is True, the function also
        considers tags which are instances of classes inheriting from `c`.

        If `multiple` and `inherit` are both True and multiple tags inherit from
        `c`, the entire list of tags is returned. If `multiple` is False and
        `inherit` is True and more than one tag meets the criterion, an
        exception is thrown.
        """

        enforce_type(c, type)

        if inherit:
            tags = filter(lambda tag: isinstance(tag, c), self.getTags())

            if multiple:
                return tags
            else:
                if len(tags) > 1:
                    raise Exception("Type '" + self.getName() + "' has more "
                                    "than one tag that inherits " +
                                    c.__name__)
                elif len(tags) == 1:
                    return tags[0]
                else:
                    return None
        else:
            if multiple and not issubclass(c, tags.MultiTag):
                raise Exception("Parameter `multiple` may only be True if " +
                                "`inherit` is True or if `c` is a class that " +
                                "inherits from MultiTag")

            if c in self.__tags_hashed.keys():
                return self.__tags_hashed[c]
            else:
                return None

    def getTagInheriting(self, c, multiple = False):
        """Same as getTag, but with parameter inherit set to True"""

        return self.getTag(c, inherit = True, multiple = multiple)

    def getAllTagsInheriting(self, c):
        """Same as getTagi, but with parameter multiple set to True"""

        ret = self.getTag(c, inherit = True, multiple = True)
        return ret

    def removeTag(self, c):
        """Removes a type tag of the given class c from the list of tags
        associated to this type."""

        enforce_type(c, type)

        tag = self.__tags_hashed[c]
        self.__tags.remove(tag)
        del self.__tags_hashed[c]

    def removeTags(self, *cs):
        """Removes all tags of the classes specified as arguments from the list
        of tags associated to this type.
        """

        enforce_type_tuple(cs, type)

        for c in cs:
            self.removeTag(c)

    def getTags(self):
        return self.__tags

    def getDependencies(self, exclude_pointers = False):
        """Returns a list of Types with one entry per type this type depends
        on."""

        # By default a type has no dependencies; how the list of dependencies is
        # created must be defined in the inheriting class
        return []

    def getRelations(self):
        """Returns the list of Relations associated to this type"""

        return self.__relations

    def addRelation(self, r):
        """Adds the relation r to this type"""

        enforce_type(r, relations.Relation)

        if r.__class__ in self.__relations_hashed.keys():
            self.__relations_hashed[r.__class__].append(r)
        else:
            self.__relations_hashed[r.__class__] = [ r ]

        self.__relations.append(r)

    def addRelations(self, *relations):
        """Adds all relations passed as arguments to the list of relations
        associated with this type"""

        for r in relations:
            self.addRelation(r)

    def getRelation(self, c, single = False):
        """Returns the relations of type `c` associated to this type. If no such
        relation exists, None is returned. Otherwise, a list with all instances
        of `c` is returned. If `single` is True and if only one relation of the
        type exists, the instance is returned directly rather than a list with a
        single entry.
        """

        if r.__class__ in self.__relations_hashed.keys():
            ret = self.__relations_hashed[r.__class__]
        else:
            return None

        if single:
            if len(ret) != 1:
                raise Exception("Requested a single instance of a relation, " + \
                                "but type '"+ self.getName()+"' has " + \
                                "multiple instances of relation " + \
                                "'" + str(r.__class__) + "'")
            else:
                return ret[0]
        else:
            return ret

    def getRelationsInheriting(self, c):
        """Returns the relations inheriting `c` associated to this type. If no
        such relation exists, an empty list is returned."""

        return filter(lambda r: isinstance(r, c), self.__relations)

#################################################################################

class NonCompoundType(Type):
    """A type that is not composed of any other type"""

    def __init__(self,
                 format_string = None,
                 pformat_string = None,
                 *args,
                 **kwargs):
        """`format_string` is the C format string that needs to be used when
        passing an instance of the type to C's printf function by value.

        `pformat_string` is the C format string that needs to be used when
        passing an instance of the type to C's printf function by address (e.g.,
        "%s" for char, since char* should be interpreted by printf as a string).
        """
        enforce_type(format_string, [str, type(None)])
        enforce_type(pformat_string, [str, type(None)])

        super(NonCompoundType, self).__init__(*args, **kwargs)
        self.__format_string = format_string
        self.__pformat_string = pformat_string

    def getFormatStringSym(self):
        """Returns a symbolic expression for the string in C containing the
        format string for printf (e.g., PRIu64)"""

        return self.__format_string

    def setFormatStringSym(self, s):
        """Sets the symbolic expression for the format string for this type"""

        self.__format_string = s

    def getPointerFormatStringSym(self):
        """Returns a symbolic expression for the string in C containing the
        format string for printf (e.g., PRIu64) for a pointer to this type"""

        return self.__pformat_string

    def setPointerFormatStringSym(self, s):
        """Sets the symbolic expression for the format string for pointers of
        this type"""

        self.__pformat_string = s

    def getCType(self):
        return self.getName()

#################################################################################

class IntegerType(NonCompoundType):
    """An alias for an integer type"""

    def __init__(self, signed, *args, **kwargs):
        """`signed` indicates whether the defined integer type can have negative
        values"""

        enforce_type(signed, bool)

        super(IntegerType, self).__init__(*args, **kwargs)
        self.__signed = signed

    def isInteger(self):
        return True

    def getNumBitsSym(self):
        """Returns the number of bits for this integer as a symbolic
        expression"""

        return "(8*sizeof("+self.getCType()+"))"

    def isSigned(self):
        """Returns true if the integer is signed"""
        return self.__signed

    def getSignedSym(self):
        """Returns the string '1' if the type is signed, otherwise the
        string '0'"""
        return "1" if self.isSigned() else "0"

#################################################################################

class FixedWidthIntegerType(IntegerType):
    """An alias for an integer with a fixed, implementation-independent width"""

    def __init__(self, num_bits, *args, **kwargs):
        """`num_bits` the maximum number of bits for values of the defined
        integer type"""

        enforce_type(num_bits, int)

        super(FixedWidthIntegerType, self).__init__(*args, **kwargs)
        self.__num_bits = num_bits

    def getNumBits(self):
        """Returns the number of bits for this integer (e.g., 8, 16, 32, 64)"""
        return self.__num_bits

    def getMinValueSym(self):
        """Returns a symbolic expression for the minimum value for the integer"""

        if self.isSigned():
            return "INT" + str(self.getNumBits())+"_MIN"
        else:
            return "0"

    def getMaxValueSym(self):
        """Returns a symbolic expression for the maximum value for the integer"""

        prefix = "INT" if self.isSigned() else "UINT"
        return prefix + str(self.getNumBits())+"_MAX"

    def getMaxDecimalDigits(self):
        """Returns the number of decimal digits needed to hold the integer's
        maximum value"""

        if self.isSigned():
            return len(str(2**self.getNumBits()))+1
        else:
            return len(str(2**self.getNumBits()))

    def getFormatStringSym(self):
        """Returns a symbolic expression for the string in C containing the
        format string for printf (e.g., PRIu64)"""

        custom_format = super(IntegerType, self).getFormatStringSym()

        if custom_format:
            return custom_format
        else:
            return "PRI" + ("d" if self.isSigned() else "u") + \
                str(self.getNumBits())

#################################################################################

class CompoundType(Type):
    """A type that is composed of other types"""

    def __init__(self, fields, tags = None, *args, **kwargs):
        """`fields` must be an instance of FieldList defining the fields of the defined
        compound type."""

        enforce_type(fields, FieldList)

        self.__fields = fields
        self.__fields.setCompoundType(self)

        if not tags:
            tags = []

        tags.insert(0, aftermath.tags.Compound())

        super(CompoundType, self).__init__(tags = tags, *args, **kwargs)

    def finalize(self):
        # Add destructor tag if necessary
        if not self.hasTag(tags.Destructor):
            for field in self.getFields():
                if field.hasCustomDestructor():
                    self.addTag(aftermath.tags.Destructor())
                    break
                if not field.isPointer() or field.isOwned():
                    if field.getType().hasTag(aftermath.tags.Destructor):
                        self.addTag(aftermath.tags.Destructor())
                        break

        # Add default constructor tag if necessary
        if not self.hasTag(tags.DefaultConstructor):
            for field in self.getFields():
                if not field.isPointer() or field.isOwned():
                    if field.getType().hasTag(aftermath.tags.DefaultConstructor):
                        self.addTag(aftermath.tags.DefaultConstructor())
                        break

        Type.finalize(self)

    def getFields(self):
        return self.__fields

    def referencesType(self, t, types_checked = None):
        """Recursively checks whether this compound type contains a field whose
        type is `t` or if it references another compound type that contains such
        a field.

        `types_checked` is an optional list of types for which is has already
        been checked if they reference t. All of these types will be excluded
        when recursively invoking referencesType() for each field.
        """

        enforce_type(t, Type)
        enforce_type(types_checked, [ list, type(None) ])

        if types_checked is None:
            types_checked = [ ]

        types_checked.append(self)

        for f in self.getFields():
            field_type = f.getType()

            if not f.isPointer() or f.isOwned():
                if field_type == t:
                    return True
                if field_type.isCompound() and (field_type not in types_checked):
                    if field_type.referencesType(t, types_checked):
                        return True

        return False

    def getDependencies(self, exclude_pointers = False):
        """Returns a set with all types, for which a non-forward declaration is
        required for the definition of this type in C"""

        enforce_type(exclude_pointers, bool)

        return self.__fields.getDependencies(exclude_pointers = exclude_pointers)

    def addField(self, *args, **kwargs):
        """Same as FieldList.addField"""
        self.__fields.addField(*args, **kwargs)

    def prependField(self, *args, **kwargs):
        """Same as FieldList.prependField"""
        self.__fields.prependField(*args, **kwargs)

    def appendField(self, *args, **kwargs):
        """Same as FieldList.appendField"""
        self.__fields.appendField(*args, **kwargs)

    def getCType(self):
        return "struct "+self.getName()

    def hasDestructor(self):
        for f in self.__fields:
            # If there is at least one field with a custom destructor, a
            # destructor for the whole structure is needed
            if f.hasCustomDestructor():
                return True

            # Destructor is only invoked for fields that are located in the
            # structure and for pointer fields for which the structure has
            # been declared the owner
            if (not f.isPointer()) or f.isOwned():
                if f.getType().hasDestructor():
                    return True

        return self.hasTag(tags.Destructor)

    def hasDefaultConstructor(self):
        for f in self.__fields:
            # Constructor is only invoked for fields that are located in the
            # structure and for pointer fields for which the structure has
            # been declared the owner
            if (not f.isPointer()) or f.isOwned():
                if f.getType().hasDefaultConstructor():
                    return True

        return self.hasTag(tags.DefaultConstructor)

#################################################################################

class TypeList(object):
    """Groups a set of type definitions"""

    def __init__(self, types = None):
        """`types` is an optional list of types that is added to this type list"""

        self.__types = []
        self.__types_hashed = {}

        if types:
            for t in types:
                self.addType(t)

    def getTypeNames(self):
        """Returns a list with the names fo all types in this type list"""

        return map(lambda x: x.getName(), self.__types)

    def getTypes(self):
        """Returns a new, plain list with all types of this list"""
        return self.__types[:]

    def addType(self, t):
        """Adds a Type instance t to the list of types"""

        enforce_type(t, Type)

        tname = t.getName()

        if tname in self.__types_hashed.keys():
            raise Exception("Type "+t.getName()+" defined multiple times")

        self.__types.append(t)
        self.__types_hashed[tname] = t

    def addTypes(self, *ts):
        """Adds all types passed as arguments to this list"""

        for t in ts:
            self.addType(t)

    def copy(self, types = None):
        """Returns a shallow copy of itself. If `types` != None, the copy
        receives types as the new list of type definitions."""

        if types == None:
            types = self.getTypes()

        return TypeList(types)

    def filterByTag(self, c):
        """Returns a TypeList with all types that have a tag of class c"""
        enforce_type(c, type)

        return TypeList(filter(lambda type: type.hasTag(c), self.getTypes()))

    def hasType(self, type):
        """Returns true if the type list contains the instance type."""

        enforce_type(type, Type)

        return type in self.__types

    def hasTypeName(self, tname):
        """Returns true if the type list contains a type instance whose name is
        n"""

        enforce_type(tname, str)

        return tname in self.__types_hashed.keys()

    def topologicalSort(self, local_only = True):
        """Determines the dependencies between type declarations and returns the
        types as a topologically sorted type list.

        If local_only is True, the returned list is composed solely of types
        that are included in self. Otherwise, the list may also contain
        references to types that were not in the original list, but referenced
        by types in the original list.
        """

        # Create a pair (type, dependencies) for each type
        if local_only:
            dep_pairs = map( \
                lambda t: (t, filter(self.hasType, t.getDependencies(exclude_pointers = True))), \
                self.__types)
        else:
            dep_pairs = map(lambda t: (t, t.getDependencies(exclude_pointers = True)), self.__types)

        keyfun = lambda x: len(x[1])
        dep_pairs = sorted(dep_pairs, key = keyfun)
        topo_list = []

        while dep_pairs:
            curr = dep_pairs.pop(0)
            t = curr[0]
            deps = curr[1]

            if len(deps) > 0:
                raise Exception("Topological sort impossible, no type without "
                                "dependencies; Current type is " +
                                t.getName() + " with remaining dependencies " +
                                str(map(lambda x: x.getName(), deps)))

            for other_dep_pair in dep_pairs:
                if t in other_dep_pair[1]:
                    other_dep_pair[1].remove(t)

            dep_pairs = sorted(dep_pairs, key = keyfun)

            topo_list.append(t)

        self_copy = copy.copy(self)
        self_copy.__types = topo_list

        return self_copy

    def __iter__(self):
        return self.__types.__iter__()

    def __add__(self, other):
        ret = self.copy()

        for t in other:
            ret.addType(t)

        return ret

    def __radd__(self, other):
        enforce_type(TypeList, other)
        return self.__add__(other)

    def __getitem__(self, key):
        enforce_type(key, str)
        return self.__types.__getitem__(key)

    def getType(self, tname):
        enforce_type(tname, str)

        if tname in self.__types_hashed.keys():
            return self.__types_hashed[tname]
        else:
            return None

    def __len__(self):
        return len(self.__types)

#################################################################################

class Field(object):
    """Field for compound types or function parameters; associates a name with a
    type and modifiers."""

    def __init__(self, name, type,
                 is_pointer = False,
                 pointer_depth = None,
                 is_const = False,
                 is_owned = None,
                 custom_destructor_name = None,
                 comment = None):
        """`name` is the name of the field as a string.

        `type` is an instance of Type indicating the field's type.

        If `is_pointer` is true, the field points to an instance of the
        specified type.

        If `pointer_depth` is not None, this field becomes a pointer of the
        specified depth (for a simple pointer the depth is 1, for a double
        pointer 2, and so on).

        If `is_const` is true, the field is read-only.

        If `is_owned` is true, the field is assumed to "belong" to the data
        structure using the field. This is even true if `is_pointer` is True and
        thus the memory region for the instance is not necessarily embedded into
        the structure itself.

        If `custom_destructor` is not None, a custom destructor is invoked upon
        destruction of the owning compound type.

        `comment` is a human-readable comment for the field.
        """

        enforce_type(type, Type)
        enforce_type(is_pointer, [ bool, types.NoneType ])
        enforce_type(pointer_depth, [ int, types.NoneType ])
        enforce_type(is_const, [ bool, types.NoneType ])
        enforce_type(is_owned, [ bool, types.NoneType ])
        enforce_type(custom_destructor_name, [ str, types.NoneType ])
        enforce_type(comment, [ str, types.NoneType ])

        if is_owned is None:
            if is_pointer:
                is_owned = False
            else:
                is_owned = True

        if is_pointer and pointer_depth is None:
            pointer_depth = 1

        if pointer_depth is not None:
            is_pointer = True

        if not is_pointer:
            pointer_depth = 0

        self.__name = name
        self.__comment = comment
        self.__type = type
        self.__is_pointer = is_pointer
        self.__pointer_depth = pointer_depth
        self.__is_const = is_const
        self.__is_owned = is_owned
        self.__compound_type = None
        self.__custom_destructor_name = custom_destructor_name

    def hasCustomDestructor(self):
        """Returns true if a custom destructor has been set for the field."""

        return self.getCustomDestructorName() is not None

    def getCustomDestructorName(self):
        """Returns the name of the field's custom destructor or None if no such
        destructor has been set."""

        return self.__custom_destructor_name

    def setCustomDestructorName(self, custom_destructor_name):
        """Sets the name of the field's custom destructor to be invoked upon
        destruction of an instance of the compound type."""

        enforce_type(custom_destructor_name, str)
        self.__custom_destructor_name = custom_destructor_name

    def getType(self):
        """Returns the type of this field. If this is a pointer, the base type of
        the pointer type is returned (e.g., the type for uint8_t instead of
        uint8_t*)
        """

        return self.__type

    def isPointer(self):
        """Returns true if the field is a pointer"""

        return self.__is_pointer

    def getPointerDepth(self):
        """Returns the pointer depth (i.e., 0 for a non-pointer, 1 for a simple pointer,
        2 for a double pointer, and so on)"""

        return self.__pointer_depth

    def isConst(self):
        """Returns true if the field is marked as const"""

        return self.__is_const

    def setType(self, type):
        enforce_type(type, Type)

        self.__type = type

    def getName(self):
        return self.__name

    def getComment(self):
        return self.__comment

    def isOwned(self):
        """Returns True if the structure this field belongs to (e.g., a compound
        type) owns the memory associated with this field."""

        return self.__is_owned

    def __hash__(self):
        return hash(self.name)

    def __eq__(self, other):
        return self.getName() == other.getName()

    def copy(self):
        """Returns a shallow copy of itself"""

        return copy.copy(self)

    def setCompoundType(self, t):
        """Sets the CompoundType this Field belongs to"""

        enforce_type(t, CompoundType)
        self.__compound_type = t

    def getCompoundType(self):
        """Returns the CompoundType this Field belongs to"""

        return self.__compound_type

#################################################################################

class FieldList(object):
    """An ordered list of fields (instances of Field)"""

    def __init__(self, fields, compound_type = None):
        """`fields` must be a list of instances of the class Field or empty.

        `compound_type` indicates to which compound type the fields belong
        to. May be None (e.g., if used for the parameters fo a functon instead
        of a compound type or if the compound type is not known, yet.
        """

        enforce_type_list(fields, Field)

        self.__fields = []
        self.__fields_hashed = {}
        self.__compound_type = compound_type

        enforce_type(fields, list)

        for f in fields:
            self.appendField(f)

    def addField(self, f, pos):
        """Adds a field at position pos (pos will be the 0-based index of the
        new field)"""

        enforce_type(f, Field)
        enforce_type(pos, int)

        if f.getName() in self.__fields_hashed.keys():
                raise Exception("Duplicate field \""+f.getName()+"\"")

        self.__fields_hashed[f.getName()] = f
        self.__fields.insert(pos, f)

        if self.__compound_type:
            f.setCompoundType(self.__compound_type)

    def appendField(self, f):
        """Adds a field at the last position"""
        self.addField(f, self.getNumFields())

    def appendFields(self, l):
        """Adds a list of field at the last position"""
        for f in l:
            self.addField(f, self.getNumFields())

    def prependField(self, f):
        """Adds a field at the first position"""
        self.addField(f, 0)

    def prependFields(self, fs):
        """Adds a list of fields at the first position"""
        for f in reversed(fs):
            self.prependField(f)

    def __iter__(self):
        return self.__fields.__iter__()

    def __getitem__(self, key):
        return self.__fields.__getitem__(key)

    def empty(self):
        """Returns true, if this field list does not contain any field"""

        return len(self.__fields) == 0

    def hasFieldByName(self, name):
        """Returns True if there is a field with the given name, otherwise
        False."""

        enforce_type(name, str)

        return name in self.__fields_hashed.keys()

    def getFieldByName(self, name):
        """Returns the field in the list whose name is `name`. If no such field
        exists, an exception is thrown."""

        enforce_type(name, str)

        if not name in self.__fields_hashed.keys():
            raise Exception("Unknown field '" + name + "'")

        return self.__fields_hashed[name]

    def getDependencies(self, exclude_pointers = False):
        """Returns a set with all types, for which a non-forward declaration is
        required when using the fields from this field list.

        If `exclude_pointers` is False, additionally all types only referenced
        by address are also included in the set.
        """

        enforce_type(exclude_pointers, bool)
        fields_set = set()

        for f in self.__fields:
            if not f.isPointer() or not exclude_pointers:
                fields_set.add(f.getType())

        return fields_set

    def setCompoundType(self, t):
        """Must be invoked if the field list is used for the definition of a compound
        type."""

        enforce_type(t, CompoundType)

        self.__compound_type = t

        for f in self.__fields:
            f.setCompoundType(t)

    def getNumFields(self):
        """Returns the number of fields in the list"""

        return len(self.__fields)
