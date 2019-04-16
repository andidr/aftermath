import re


def to_camel_case(s):
    return s[0].upper() + \
        re.sub(r"_([a-zA-z0-9])", lambda pattern: pattern.group(1).upper(), s)[1:]


def to_stripped_camel_case(s):
    r = re.sub("^am_", "", s)
    return r[0].upper() + \
        re.sub(r"_([a-zA-z0-9])", lambda pattern: pattern.group(1).upper(), r)[1:]

def filters():
    return {
        "camel_case" : to_camel_case,
        "stripped_camel_case" : to_stripped_camel_case
    }
