from aftermath.core._aftermath_core import ffi, lib

class Trace(object):
    """Wrapper for a trace"""

    def __init__(self, filename):
        """Creates a trace with the contents from the file whose paths is provided in
        `filename`"""

        self.__trace = ffi.gc(lib.am_py_trace_load(filename.encode("utf-8")),
                              lib.am_py_trace_destroy_and_free)

    def getTraceArray(self, ident):
        """Returns the trace array whose identifier is `ident`"""

        import aftermath.core.array

        arr = lib.am_py_trace_find_trace_array(self.__trace,
                                               ident.encode("utf-8"))

        if arr:
            return aftermath.core.array.wrap(ident, arr, self)
        else:
            raise Exception("No Trace array with identifier '" + ident + "' found.")

    def getEventCollections(self):
        num_collections = lib.am_py_trace_get_num_event_collections(self.__trace)
        arr = lib.am_py_trace_get_event_collections(self.__trace)

        import aftermath.core.array

        return aftermath.core.array.Array(arr,
                     num_collections,
                     lib.am_py_event_collection_array_get_element,
                     lambda cffi_ecoll, owner: EventCollection(cffi_ecoll, owner),
                     self)

class EventCollection(object):
    """Wrapper for an event collection"""

    def __init__(self, cffi_ecoll, owner):
        self.__owner = owner
        self.__cffi_ecoll = cffi_ecoll

    def getEventArray(self, ident):
        """Returns the event array whose identifier is `ident` or None if no such array
        exists for the event collection"""

        import aftermath.core.array

        arr = lib.am_py_event_collection_find_event_array(self.__cffi_ecoll,
                                                          ident.encode("utf-8"))

        if arr != ffi.NULL:
            return aftermath.core.array.wrap(ident, arr, self)
        else:
            raise Exception("No event array with identifier '" + ident + "' found.")
