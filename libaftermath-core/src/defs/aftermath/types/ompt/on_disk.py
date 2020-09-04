# Author: Andi Drebes <andi@drebesium.org>
# Author: Igor Wodiany <igor.wodiany@manchester.ac.uk>
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

from aftermath.types import TypeList, Field, FieldList
from aftermath import relations
from aftermath import tags
from aftermath.types.on_disk import Frame, EventFrame
import aftermath.types.on_disk

am_dsk_ompt_thread = EventFrame(
    name = "am_dsk_ompt_thread",
    entity = "on-disk OpenMP thread execution interval",
    comment = "An OpenMP thread execution interval",
    fields = FieldList([
        Field(
            name = "interval",
            field_type = aftermath.types.on_disk.am_dsk_interval,
            comment = "Start and end of the execution interval"),
        Field(
            name = "type",
            field_type = aftermath.types.builtin.uint64_t,
            comment = "Thread type")]))

tags.dsk.tomem.add_per_event_collection_tags(
    am_dsk_ompt_thread,
    aftermath.types.ompt.in_memory.am_ompt_thread,
    "collection_id")

################################################################################

am_dsk_ompt_parallel = EventFrame(
    name = "am_dsk_ompt_parallel",
    entity = "on-disk OpenMP parallel section execution interval",
    comment = "An OpenMP parallel section execution interval",
    fields = FieldList([
        Field(
            name = "interval",
            field_type = aftermath.types.on_disk.am_dsk_interval,
            comment = "Start and end of the execution interval"),
        Field(
            name = "requested_parallelism",
            field_type = aftermath.types.builtin.uint32_t,
            comment = "Number of threads or teams in the region"),
        Field(
              name = "flags",
              field_type = aftermath.types.builtin.uint16_t,
              comment = "Type of the parallel region")]))

tags.dsk.tomem.add_per_event_collection_tags(
    am_dsk_ompt_parallel,
    aftermath.types.ompt.in_memory.am_ompt_parallel,
    "collection_id")

################################################################################

am_dsk_ompt_task_create = EventFrame(
    name = "am_dsk_ompt_task_create",
    entity = "on-disk OpenMP task create event",
    comment = "An event generated when OpenMP task regions are generated",
    fields = FieldList([
        Field(
            name = "timestamp",
            field_type = aftermath.types.builtin.uint64_t,
            comment = "Time of the event"),
        Field(
            name = "task_id",
            field_type = aftermath.types.builtin.uint64_t,
            comment = "Unique task id"),
        Field(
            name = "new_task_id",
            field_type = aftermath.types.builtin.uint64_t,
            comment = "Unique new task id"),
        Field(
            name = "flags",
            field_type = aftermath.types.builtin.uint16_t,
            comment = "Kind of the task"),
        Field(
            name = "has_dependences",
            field_type = aftermath.types.builtin.uint8_t,
            comment = "True if the task has dependences"),
        Field(
            name = "codeptr_ra",
            field_type = aftermath.types.builtin.uint64_t,
            comment = "OpenMP region code pointer")]))

tags.dsk.tomem.add_per_event_collection_tags(
    am_dsk_ompt_task_create,
    aftermath.types.ompt.in_memory.am_ompt_task_create,
    "collection_id")

################################################################################

am_dsk_ompt_task_schedule = EventFrame(
    name = "am_dsk_ompt_task_schedule",
    entity = "on-disk OpenMP task schedule event",
    comment = "An event generated when an OpenMP task is scheduled",
    fields = FieldList([
        Field(
            name = "timestamp",
            field_type = aftermath.types.builtin.uint64_t,
            comment = "Time of the event"),
        Field(
            name = "prior_task_id",
            field_type = aftermath.types.builtin.uint64_t,
            comment = "Unique id of the encountering task"),
        Field(
            name = "next_task_id",
            field_type = aftermath.types.builtin.uint64_t,
            comment = "Unique id of the next task"),
        Field(
            name = "prior_task_status",
            field_type = aftermath.types.builtin.uint8_t,
            comment = "Status of the task at the scheduling point")]))

tags.dsk.tomem.add_per_event_collection_tags(
    am_dsk_ompt_task_schedule,
    aftermath.types.ompt.in_memory.am_ompt_task_schedule,
    "collection_id")

################################################################################

am_dsk_ompt_implicit_task = EventFrame(
    name = "am_dsk_ompt_implicit_task",
    entity = "on-disk OpenMP initial or implicit task execution interval",
    comment = "An OpenMP initial or implicit task execution interval",
    fields = FieldList([
        Field(
            name = "interval",
            field_type = aftermath.types.on_disk.am_dsk_interval,
            comment = "Start and end of the execution interval"),
        Field(
            name = "actual_parallelism",
            field_type = aftermath.types.builtin.uint32_t,
            comment = "Number of threads or teams in the region"),
        Field(
            name = "index",
            field_type = aftermath.types.builtin.uint32_t,
            comment = "Thread or team number of the calling thread"),
        Field(
            name = "flags",
            field_type = aftermath.types.builtin.uint8_t,
            comment = "Indication if the task is initial or implicit")]))

tags.dsk.tomem.add_per_event_collection_tags(
    am_dsk_ompt_implicit_task,
    aftermath.types.ompt.in_memory.am_ompt_implicit_task,
    "collection_id")

################################################################################

am_dsk_ompt_sync_region_wait = EventFrame(
    name = "am_dsk_ompt_sync_region_wait",
    entity = "on-disk OpenMP sync region wait execution interval",
    comment = "An OpenMP sync region wait execution interval",
    fields = FieldList([
        Field(
            name = "interval",
            field_type = aftermath.types.on_disk.am_dsk_interval,
            comment = "Start and end of the execution interval"),
        Field(
            name = "kind",
            field_type = aftermath.types.builtin.uint8_t,
            comment = "Argument indicates a type of the synchronization "
                      + "construct")]))

tags.dsk.tomem.add_per_event_collection_tags(
    am_dsk_ompt_sync_region_wait,
    aftermath.types.ompt.in_memory.am_ompt_sync_region_wait,
    "collection_id")

################################################################################

am_dsk_ompt_mutex_released = EventFrame(
    name = "am_dsk_ompt_mutex_released",
    entity = "on-disk OpenMP mutex released event",
    comment = "An event generated when an OpenMP mutex is released",
    fields = FieldList([
        Field(
            name = "timestamp",
            field_type = aftermath.types.builtin.uint64_t,
            comment = "Time of the event"),
        Field(
            name = "wait_id",
            field_type = aftermath.types.builtin.uint64_t,
            comment = "ID of the object that is being released"),
        Field(
            name = "kind",
            field_type = aftermath.types.builtin.uint8_t,
            comment = "Type of the mutual exclusion event")]))

tags.dsk.tomem.add_per_event_collection_tags(
    am_dsk_ompt_mutex_released,
    aftermath.types.ompt.in_memory.am_ompt_mutex_released,
    "collection_id")

###############################################################################

am_dsk_ompt_dependences = EventFrame(
    name = "am_dsk_ompt_dependences",
    entity = "on-disk OpenMP dependences event",
    comment = "An event generated when new tasks and ordered constructs with "
              + "dependences are dispatched",
    fields = FieldList([
        Field(
            name = "timestamp",
            field_type = aftermath.types.builtin.uint64_t,
            comment = "Time of the event"),
        Field(
            name = "ndeps",
            field_type = aftermath.types.builtin.uint32_t,
            comment = "Number of dependences")]))

tags.dsk.tomem.add_per_event_collection_tags(
    am_dsk_ompt_dependences,
    aftermath.types.ompt.in_memory.am_ompt_dependences,
    "collection_id")

################################################################################

am_dsk_ompt_task_dependence = EventFrame(
    name = "am_dsk_ompt_task_dependence",
    entity = "on-disk OpenMP task dependence event",
    comment = "An event generated when unfulfilled task dependences are "
              + "encountered",
    fields = FieldList([
        Field(
            name = "timestamp",
            field_type = aftermath.types.builtin.uint64_t,
            comment = "Time of the event"),
        Field(
            name = "src_task_id",
            field_type = aftermath.types.builtin.uint64_t,
            comment = "Unique id of the source task"),
        Field(
            name = "sink_task_id",
            field_type = aftermath.types.builtin.uint64_t,
            comment = "Unique id of the sink task")]))

tags.dsk.tomem.add_per_event_collection_tags(
    am_dsk_ompt_task_dependence,
    aftermath.types.ompt.in_memory.am_ompt_task_dependence,
    "collection_id")

################################################################################

am_dsk_ompt_work = EventFrame(
    name = "am_dsk_ompt_work",
    entity = "on-disk OpenMP work execution interval",
    comment = "An OpenMP work execution interval",
    fields = FieldList([
        Field(
            name = "interval",
            field_type = aftermath.types.on_disk.am_dsk_interval,
            comment = "Start and end of the execution interval"),
        Field(
            name = "type",
            field_type = aftermath.types.builtin.uint8_t,
            comment = "Type of the work"),
        Field(
            name = "count",
            field_type = aftermath.types.builtin.uint64_t,
            comment = "Measure of quantity involved in work")]))

tags.dsk.tomem.add_per_event_collection_tags(
    am_dsk_ompt_work,
    aftermath.types.ompt.in_memory.am_ompt_work,
    "collection_id")

################################################################################

am_dsk_ompt_master = EventFrame(
    name = "am_dsk_ompt_master",
    entity = "on-disk OpenMP master region execution interval",
    comment = "An OpenMP master region execution interval",
    fields = FieldList([
        Field(
            name = "interval",
            field_type = aftermath.types.on_disk.am_dsk_interval,
            comment = "Start and end of the execution interval")]))

tags.dsk.tomem.add_per_event_collection_tags(
    am_dsk_ompt_master,
    aftermath.types.ompt.in_memory.am_ompt_master,
    "collection_id")

################################################################################

am_dsk_ompt_sync_region = EventFrame(
    name = "am_dsk_ompt_sync_region",
    entity = "on-disk OpenMP sync region execution interval",
    comment = "An OpenMP sync region execution interval",
    fields = FieldList([
        Field(
            name = "interval",
            field_type = aftermath.types.on_disk.am_dsk_interval,
            comment = "Start and end of the execution interval"),
        Field(
            name = "kind",
            field_type = aftermath.types.builtin.uint8_t,
            comment = "Type of the synchronization construct")]))

tags.dsk.tomem.add_per_event_collection_tags(
    am_dsk_ompt_sync_region,
    aftermath.types.ompt.in_memory.am_ompt_sync_region,
    "collection_id")

################################################################################

am_dsk_ompt_lock_init = EventFrame(
    name = "am_dsk_ompt_lock_init",
    entity = "on-disk OpenMP lock init event",
    comment = "An event generated when an OpenMP lock is initialized",
    fields = FieldList([
        Field(
            name = "timestamp",
            field_type = aftermath.types.builtin.uint64_t,
            comment = "Time of the event"),
        Field(
            name = "wait_id",
            field_type = aftermath.types.builtin.uint64_t,
            comment = "ID of the lock that is being initialized"),
        Field(
            name = "kind",
            field_type = aftermath.types.builtin.uint8_t,
            comment = "Type of the lock")]))

tags.dsk.tomem.add_per_event_collection_tags(
    am_dsk_ompt_lock_init,
    aftermath.types.ompt.in_memory.am_ompt_lock_init,
    "collection_id")

################################################################################

am_dsk_ompt_lock_destroy = EventFrame(
    name = "am_dsk_ompt_lock_destroy",
    entity = "on-disk OpenMP lock destroyed event",
    comment = "An event generated when an OpenMP lock is destroyed",
    fields = FieldList([
        Field(
            name = "timestamp",
            field_type = aftermath.types.builtin.uint64_t,
            comment = "Time of the event"),
        Field(
            name = "wait_id",
            field_type = aftermath.types.builtin.uint64_t,
            comment = "ID of the lock that is being destroyed"),
        Field(
            name = "kind",
            field_type = aftermath.types.builtin.uint8_t,
            comment = "Type of the lock")]))

tags.dsk.tomem.add_per_event_collection_tags(
    am_dsk_ompt_lock_destroy,
    aftermath.types.ompt.in_memory.am_ompt_lock_destroy,
    "collection_id")

################################################################################

am_dsk_ompt_mutex_acquire = EventFrame(
    name = "am_dsk_ompt_mutex_acquire",
    entity = "on-disk OpenMP mutex acquire event",
    comment = "An event generated when an OpenMP mutex is being acquired",
    fields = FieldList([
        Field(
            name = "timestamp",
            field_type = aftermath.types.builtin.uint64_t,
            comment = "Time of the event"),
        Field(
            name = "wait_id",
            field_type = aftermath.types.builtin.uint64_t,
            comment = "ID of the object that is being locked"),
        Field(
            name = "kind",
            field_type = aftermath.types.builtin.uint8_t,
            comment = "Type of the mutual exclusion"),
        Field(
            name = "hint",
            field_type = aftermath.types.builtin.uint8_t,
            comment = "Hint provided at mutex initialization"),
        Field(
            name = "implementation",
            field_type = aftermath.types.builtin.uint32_t,
            comment = "Mechanism chosen by runtime to implement the mutex")]))

tags.dsk.tomem.add_per_event_collection_tags(
    am_dsk_ompt_mutex_acquire,
    aftermath.types.ompt.in_memory.am_ompt_mutex_acquire,
    "collection_id")

################################################################################

am_dsk_ompt_mutex_acquired = EventFrame(
    name = "am_dsk_ompt_mutex_acquired",
    entity = "on-disk OpenMP mutex acquired event",
    comment = "An event when an OpenMP mutex is acquired",
    fields = FieldList([
        Field(
            name = "timestamp",
            field_type = aftermath.types.builtin.uint64_t,
            comment = "Time of the event"),
        Field(
            name = "wait_id",
            field_type = aftermath.types.builtin.uint64_t,
            comment = "ID of the object that is being locked"),
        Field(
            name = "kind",
            field_type = aftermath.types.builtin.uint8_t,
            comment = "Type of the mutual exclusion")]))

tags.dsk.tomem.add_per_event_collection_tags(
    am_dsk_ompt_mutex_acquired,
    aftermath.types.ompt.in_memory.am_ompt_mutex_acquired,
    "collection_id")

################################################################################

am_dsk_ompt_nest_lock = EventFrame(
    name = "am_dsk_ompt_nest_lock",
    entity = "on-disk OpenMP nest lock execution interval",
    comment = "An OpenMP nested lock interval when the nested lock was held",
    fields = FieldList([
        Field(
            name = "interval",
            field_type = aftermath.types.on_disk.am_dsk_interval,
            comment = "Start and end of the execution interval"),
        Field(
            name = "wait_id",
            field_type = aftermath.types.builtin.uint64_t,
            comment = "ID of the object that is locked")]))

tags.dsk.tomem.add_per_event_collection_tags(
    am_dsk_ompt_nest_lock,
    aftermath.types.ompt.in_memory.am_ompt_nest_lock,
    "collection_id")

################################################################################

am_dsk_ompt_flush = EventFrame(
    name = "am_dsk_ompt_flush",
    entity = "on-disk OpenMP flush event",
    comment = "A memory flush event generated by the OpenMP flush construct",
    fields = FieldList([
        Field(
            name = "timestamp",
            field_type = aftermath.types.builtin.uint64_t,
            comment = "Time of the event")]))

tags.dsk.tomem.add_per_event_collection_tags(
    am_dsk_ompt_flush,
    aftermath.types.ompt.in_memory.am_ompt_flush,
    "collection_id")

################################################################################

am_dsk_ompt_cancel = EventFrame(
    name = "am_dsk_ompt_cancel",
    entity = "on-disk OpenMP cancel event",
    comment = "An event generated by OpenMP cancel construct",
    fields = FieldList([
        Field(
            name = "timestamp",
            field_type = aftermath.types.builtin.uint64_t,
            comment = "Time of the event"),
        Field(
            name = "flags",
            field_type = aftermath.types.builtin.uint8_t,
            comment = "Cancellation source and cancelled construct information")]))

tags.dsk.tomem.add_per_event_collection_tags(
    am_dsk_ompt_cancel,
    aftermath.types.ompt.in_memory.am_ompt_cancel,
    "collection_id")

################################################################################

am_dsk_ompt_loop = EventFrame(
    name = "am_dsk_ompt_loop",
    entity = "on-disk OpenMP loop execution interval",
    comment = "An OpenMP loop execution interval",
    fields = FieldList([
        Field(
            name = "interval",
            field_type = aftermath.types.on_disk.am_dsk_interval,
            comment = "Start and end of the execution interval"),
        Field(
            name = "instance_id",
            field_type = aftermath.types.builtin.uint64_t,
            comment = "Loop instance id"),
        Field(
            name = "flags",
            field_type = aftermath.types.builtin.int64_t,
            comment = "Flags associated with the loop"),
        Field(
            name = "lower_bound",
            field_type = aftermath.types.builtin.int64_t,
            comment = "Lower bound of the loop"),
        Field(
            name = "upper_bound",
            field_type = aftermath.types.builtin.int64_t,
            comment = "Lower bound of the loop"),
        Field(
            name = "increment",
            field_type = aftermath.types.builtin.int64_t,
            comment = "Loop increment"),
        Field(
            name = "num_workers",
            field_type = aftermath.types.builtin.int32_t,
            comment = "Number of workers"),
        Field(
            name = "codeptr_ra",
            field_type = aftermath.types.builtin.uint64_t,
            comment = "Loop body address")]))

tags.dsk.tomem.add_per_event_collection_tags(
    am_dsk_ompt_loop,
    aftermath.types.ompt.in_memory.am_ompt_loop,
    "collection_id")

################################################################################

am_dsk_ompt_loop_chunk = EventFrame(
    name = "am_dsk_ompt_loop_chunk",
    entity = "on-disk OpenMP loop chunk dispatch event",
    comment = "An event generated by OpenMP loop chunk dispatch",
    fields = FieldList([
        Field(
            name = "timestamp",
            field_type = aftermath.types.builtin.uint64_t,
            comment = "Time of the event"),
        Field(
            name = "instance_id",
            field_type = aftermath.types.builtin.uint64_t,
            comment = "Associated loop instance id"),
        Field(
            name = "lower_bound",
            field_type = aftermath.types.builtin.int64_t,
            comment = "Lower bound of the chunk"),
        Field(
            name = "upper_bound",
            field_type = aftermath.types.builtin.int64_t,
            comment = "Lower bound of the chunk"),
        Field(
            name = "is_last",
            field_type = aftermath.types.builtin.int32_t,
            comment = "Is it end of the last period")]))

tags.dsk.tomem.add_per_event_collection_tags(
    am_dsk_ompt_loop_chunk,
    aftermath.types.ompt.in_memory.am_ompt_loop_chunk,
    "collection_id")

################################################################################

all_types = TypeList([
    am_dsk_ompt_thread,
    am_dsk_ompt_parallel,
    am_dsk_ompt_task_create,
    am_dsk_ompt_task_schedule,
    am_dsk_ompt_implicit_task,
    am_dsk_ompt_sync_region_wait,
    am_dsk_ompt_mutex_released,
    am_dsk_ompt_dependences,
    am_dsk_ompt_task_dependence,
    am_dsk_ompt_work,
    am_dsk_ompt_master,
    am_dsk_ompt_sync_region,
    am_dsk_ompt_lock_init,
    am_dsk_ompt_lock_destroy,
    am_dsk_ompt_mutex_acquire,
    am_dsk_ompt_mutex_acquired,
    am_dsk_ompt_nest_lock,
    am_dsk_ompt_flush,
    am_dsk_ompt_cancel,
    am_dsk_ompt_loop,
    am_dsk_ompt_loop_chunk
])

aftermath.config.addDskTypes(*all_types)
