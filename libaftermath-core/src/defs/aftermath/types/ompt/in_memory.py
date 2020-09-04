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

from aftermath.types import TypeList, Field, FieldList, EnumType, EnumVariant
from aftermath.types.in_memory import InMemoryCompoundType
from aftermath import tags
import aftermath.types.base

am_ompt_sync_region_t = EnumType(
    name = "enum am_ompt_sync_region_t",
    entity = "An OpenMP valid synchronization region kind",
    comment = "Type defines the valid kind of the synchronization region",
    variants = [
        EnumVariant(
            "AM_OMPT_SYNC_REGION_BARRIER",
            1,
            "Barrier"),

        EnumVariant(
            "AM_OMPT_SYNC_REGION_BARRIER_IMPLICIT",
            2,
            "Implicit Barrier"),

        EnumVariant(
            "AM_OMPT_SYNC_REGION_BARRIER_EXPLICIT",
            3,
            "Explicit barrier"),

        EnumVariant(
            "AM_OMPT_SYNC_REGION_BARRIER_IMPLEMENTATION",
            4,
            "Barrier implementation"),

        EnumVariant(
            "AM_OMPT_SYNC_REGION_TASKWAIT",
            5,
            "Taskwait"),

        EnumVariant(
            "AM_OMPT_SYNC_REGION_TASKGROUP",
            6,
            "Taskgroup"),

        EnumVariant(
            "AM_OMPT_SYNC_REGION_REDUCTION",
            7,
            "Reduction")
    ])

################################################################################

am_ompt_work_t = EnumType(
    name = "enum am_ompt_work_t",
    entity = "OpenMP work types",
    comment = "Type defines the valid OpenMP work types",
    variants = [
        EnumVariant(
            "AM_OMPT_WORK_LOOP",
            1,
            "Loop"),

        EnumVariant(
            "AM_OMPT_WORK_SECTIONS",
            2,
            "Sections"),

        EnumVariant(
            "AM_OMPT_WORK_SINGLE_EXECUTOR",
            3,
            "Single executor"),

        EnumVariant(
            "AM_OMPT_WORK_SINGLE_OTHER",
            4,
            "Single other"),

        EnumVariant(
            "AM_OMPT_WORK_WORKSHARE",
            5,
            "Workshare"),

        EnumVariant(
            "AM_OMPT_WORK_DISTRIBUTE",
            6,
            "Distribute"),

        EnumVariant(
            "AM_OMPT_WORK_TASKLOOP",
            7,
            "Taskloop")
    ])

################################################################################

am_ompt_thread_t = EnumType(
    name = "enum am_ompt_thread_t",
    entity = "OpenMP thread types",
    comment = "Type defines the valid OpenMP thread types",
    variants = [
        EnumVariant(
            "AM_OMPT_THREAD_INITIAL",
            1,
            "Initial thread"),

        EnumVariant(
            "AM_OMPT_THREAD_WORKER",
            2,
            "Worker thread"),

        EnumVariant(
            "AM_OMPT_THREAD_OTHER",
            3,
            "Other thread"),

        EnumVariant(
            "AM_OMPT_THREAD_UNKNOWN",
            4,
            "Unknown thread")
    ])

################################################################################

am_ompt_parallel_flag_t = EnumType(
    name = "enum am_ompt_parallel_flag_t",
    entity = "OpenMP parallel region types",
    comment = "Type defines valid OpenMP parallel region types",
    variants = [
        EnumVariant(
            "AM_OMPT_INVOKER_PROGRAM_LEAGUE",
            0x40000001,
            "Invoker program and league"),

        EnumVariant(
            "AM_OMPT_INVOKER_PROGRAM_TEAM",
            0x80000001,
            "Invoker program and team"),

        EnumVariant(
            "AM_OMPT_INVOKER_RUNTIME_LEAGUE",
            0x40000002,
            "Invoker runtime and league"),

        EnumVariant(
            "AM_OMPT_INVOKER_RUNTIME_TEAM",
            0x80000002,
            "Invoker runtime and league")
    ])

################################################################################

am_ompt_implicit_task_t = EnumType(
    name = "enum am_ompt_implicit_task_t",
    entity = "OpenMP implicit task types",
    comment = "Type defines valid OpenMP implicit task types",
    variants = [
        EnumVariant(
            "AM_OMPT_IMPLICIT_TASK_INITIAL",
            1,
            "Initial task"),

        EnumVariant(
            "AM_OMPT_IMPLICIT_TASK_IMPLICIT",
            2,
            "Implicit task"),
    ])

################################################################################

am_ompt_mutex_t = EnumType(
    name = "enum am_ompt_mutex_t",
    entity = "OpenMP mutex types",
    comment = "Type defines valid OpenMP mutex types",
    variants = [
        EnumVariant(
            "AM_OMPT_MUTEX_LOCK",
            1,
            "Lock"),

        EnumVariant(
            "AM_OMPT_MUTEX_TEST_LOCK",
            2,
            "Test lock"),

        EnumVariant(
            "AM_OMPT_MUTEX_NEST_LOCK",
            3,
            "Nest lock"),

        EnumVariant(
            "AM_OMPT_MUTEX_TEST_NEST_LOCK",
            4,
            "Test nest lock"),

        EnumVariant(
            "AM_OMPT_MUTEX_CRITICAL",
            5,
            "Critical"),

        EnumVariant(
            "AM_OMPT_MUTEX_ATOMIC",
            6,
            "Atomic"),

        EnumVariant(
            "AM_OMPT_MUTEX_ORDERED",
            7,
            "Ordered")
    ])

################################################################################

am_ompt_sync_hint_t = EnumType(
    name = "enum am_ompt_sync_hint_t",
    entity = "OpenMP sync hint types",
    comment = "Type defines valid OpenMP sync hint types",
    variants = [
        EnumVariant(
            "AM_OMPT_SYNC_HINT_NONE",
            0x0,
            "None"),

        EnumVariant(
            "AM_OMPT_SYNC_HINT_UNCONTENDED",
            0x1,
            "Uncontended"),

        EnumVariant(
            "AM_OMPT_SYNC_HINT_CONTENDED",
            0x2,
            "Contended"),

        EnumVariant(
            "AM_OMPT_SYNC_HINT_NONSPECULATIVE",
            0x4,
            "Nonspeculative"),

        EnumVariant(
            "AM_OMPT_SYNC_HINT_SPECULATIVE",
            0x8,
            "Speculative"),

    ])

################################################################################

am_ompt_task_status_t = EnumType(
    name = "enum am_ompt_task_status_t",
    entity = "OpenMP task status",
    comment = "Type defines the valid OpenMP task status",
    variants = [
        EnumVariant(
            "AM_OMPT_TASK_COMPLETE",
            1,
            "Task complete"),

        EnumVariant(
            "AM_OMPT_TASK_YIELD",
            2,
            "Task yield"),

        EnumVariant(
            "AM_OMPT_TASK_CANCEL",
            3,
            "Task cancel"),

        EnumVariant(
            "AM_OMPT_TASK_DETACH",
            4,
            "Task detach"),

        EnumVariant(
            "AM_OMPT_TASK_EARLY_FULFILL",
            5,
            "Task early fulfill"),

        EnumVariant(
            "AM_OMPT_TASK_LATE_FULFILL",
            6,
            "Task late fulfill"),

        EnumVariant(
            "AM_OMPT_TASK_SWITCH",
            7,
            "Task switch")

    ])

################################################################################

am_ompt_task_flag_t = EnumType(
    name = "enum am_ompt_task_flag_t",
    entity = "An OpenMP task flag",
    comment = "Type defines the valid OpenMP task flags",
    variants = [
        EnumVariant(
            "AM_OMPT_TASK_INITIAL_UNDEFERRED",
            0x08000001,
            "Task initial undeferred"),

        EnumVariant(
            "AM_OMPT_TASK_INITIAL_UNTIED",
            0x10000001,
            "Task initial untied"),

        EnumVariant(
            "AM_OMPT_TASK_INITIAL_FINAL",
            0x20000001,
            "Task initial final"),

        EnumVariant(
            "AM_OMPT_TASK_INITIAL_MERGEABLE",
            0x40000001,
            "Task initial mergeable"),

        EnumVariant(
            "AM_OMPT_TASK_INITIAL_MERGED",
            0x80000001,
            "Task initial merged"),

        EnumVariant(
            "AM_OMPT_TASK_IMPLICIT_UNDEFERRED",
            0x08000002,
            "Task implicit undeferred"),

        EnumVariant(
            "AM_OMPT_TASK_IMPLICIT_UNTIED",
            0x10000002,
            "Task implicit untied"),

        EnumVariant(
            "AM_OMPT_TASK_IMPLICIT_FINAL",
            0x20000002,
            "Task implicit final"),

        EnumVariant(
            "AM_OMPT_TASK_IMPLICIT_MERGEABLE",
            0x40000002,
            "Task implicit mergeable"),

        EnumVariant(
            "AM_OMPT_TASK_IMPLICIT_MERGED",
            0x80000002,
            "Task implicit merged"),

        EnumVariant(
            "AM_OMPT_TASK_EXPLICIT_UNDEFERRED",
            0x08000004,
            "Task explicit undeferred"),

        EnumVariant(
            "AM_OMPT_TASK_EXPLICIT_UNTIED",
            0x10000004,
            "Task explicit untied"),

        EnumVariant(
            "AM_OMPT_TASK_EXPLICIT_FINAL",
            0x20000004,
            "Task explicit final"),

        EnumVariant(
            "AM_OMPT_TASK_EXPLICIT_MERGEABLE",
            0x40000004,
            "Task explicit mergeable"),

        EnumVariant(
            "AM_OMPT_TASK_EXPLICIT_MERGED",
            0x80000004,
            "Task explicit merged"),

        EnumVariant(
            "AM_OMPT_TASK_TARGET_UNDEFERRED",
            0x08000008,
            "Task target undeferred"),

        EnumVariant(
            "AM_OMPT_TASK_TARGET_UNTIED",
            0x10000008,
            "Task target untied"),

        EnumVariant(
            "AM_OMPT_TASK_TARGET_FINAL",
            0x20000008,
            "Task target final"),

        EnumVariant(
            "AM_OMPT_TASK_TARGET_MERGEABLE",
            0x40000008,
            "Task target mergeable"),

        EnumVariant(
            "AM_OMPT_TASK_TARGET_MERGED",
            0x80000005,
            "Task target merged")

    ])

################################################################################

am_ompt_cancel_flag_t = EnumType(
    name = "enum am_ompt_cancel_flag_t",
    entity = "An OpenMP cancel flag",
    comment = "Type defines valid OpenMP cancel flags",
    variants = [
        EnumVariant(
            "AM_OMPT_CANCEL_PARALLEL",
            0x01,
            "Cancel parallel"),

        EnumVariant(
            "AM_OMPT_CANCEL_SECTIONS",
            0x02,
            "Cancel sections"),

        EnumVariant(
            "AM_OMPT_CANCEL_LOOP",
            0x04,
            "Cancel loop"),

        EnumVariant(
            "AM_OMPT_CANCEL_TASKGROUP",
            0x08,
            "Cancel taskgroup"),

        EnumVariant(
            "AM_OMPT_CANCEL_ACTIVATED",
            0x10,
            "Cancel activated"),

        EnumVariant(
            "AM_OMPT_CANCEL_DETECTED",
            0x20,
            "Cancel detected"),

        EnumVariant(
            "AM_OMPT_CANCEL_DISCARDED_TASK",
            0x40,
            "Cancel discarded task")

    ])

################################################################################

am_ompt_thread = InMemoryCompoundType(
    name = "am_ompt_thread",
    entity = "An OpenMP thread execution interval",
    comment = "An OpenMP thread execution interval",
    ident = "am::ompt::thread",
        fields = FieldList([
            Field(
                name = "kind",
                field_type = am_ompt_thread_t,
                comment = "Type of the thread"),
            Field(
                name = "interval",
                field_type = aftermath.types.in_memory.am_interval,
                comment = "Interval of the execution")]))

################################################################################

am_ompt_parallel = InMemoryCompoundType(
    name = "am_ompt_parallel",
    entity = "An OpenMP parallel section execution interval",
    comment = "An OpenMP parallel section execution interval",
    ident = "am::ompt::parallel",
        fields = FieldList([
            Field(
                name = "requested_parallelism",
                field_type = aftermath.types.builtin.uint32_t,
                comment = "Number of threads or teams in the region"),
            Field(
                  name = "flags",
                  field_type = am_ompt_parallel_flag_t,
                  comment = "Type of the parallel region"),
            Field(
                name = "interval",
                field_type = aftermath.types.in_memory.am_interval,
                comment = "Interval of the execution")]))

################################################################################

am_ompt_task_create = InMemoryCompoundType(
    name = "am_ompt_task_create",
    entity = "An OpenMP task create event",
    comment = "An OpenMP task create event",
    ident = "am::ompt::task_create",
        fields = FieldList([
            Field(
                name = "timestamp",
                field_type = aftermath.types.base.am_timestamp_t,
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
                field_type = am_ompt_task_flag_t,
                comment = "Kind of task"),
            Field(
                name = "has_dependence",
                field_type = aftermath.types.base.am_bool_t,
                comment = "True if task has dependences"),
            Field(
                name = "codeptr_ra",
                field_type = aftermath.types.builtin.uint64_t,
                comment = "OpenMP region code pointer")]))

################################################################################

am_ompt_task_schedule = InMemoryCompoundType(
    name = "am_ompt_task_schedule",
    entity = "An OpenMP task schedule event",
    comment = "An OpenMP task schedule event",
    ident = "am::ompt::task_schedule",
        fields = FieldList([
            Field(
                name = "timestamp",
                field_type = aftermath.types.base.am_timestamp_t,
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
                field_type = am_ompt_task_status_t,
                comment = "Status of the task at the scheduling point")]))

################################################################################

am_ompt_implicit_task = InMemoryCompoundType(
    name = "am_ompt_implicit_task",
    entity = "An OpenMP implicit execution interval",
    comment = "An OpenMP implicit task execution interval",
    ident = "am::ompt::implicit_task",
        fields = FieldList([
            Field(
                name = "actual_parallelism",
                field_type = aftermath.types.builtin.uint32_t,
                comment = "Number of threads or teams in the region"),
            Field(
                name = "index",
                field_type = aftermath.types.builtin.uint32_t,
                comment = "Thread or team number of calling thread"),
            Field(
                name = "flags",
                field_type = am_ompt_implicit_task_t,
                comment = "Indication if the task is initial or implicit"),
            Field(
                name = "interval",
                field_type = aftermath.types.in_memory.am_interval,
                comment = "Interval of the execution")]))

################################################################################

am_ompt_sync_region_wait = InMemoryCompoundType(
    name = "am_ompt_sync_region_wait",
    entity = "An OpenMP sync region wait execution interval",
    comment = "An OpenMP sync region wait execution interval",
    ident = "am::ompt::sync_region_wait",
        fields = FieldList([
            Field(
                name = "kind",
                field_type = am_ompt_sync_region_t,
                comment = "Type of the sync region"),
            Field(
                name = "interval",
                field_type = aftermath.types.in_memory.am_interval,
                comment = "Interval of the execution")]))

################################################################################

am_ompt_mutex_released = InMemoryCompoundType(
    name = "am_ompt_mutex_released",
    entity = "An OpenMP mutex release event",
    comment = "An OpenMP mutex release event",
    ident = "am::ompt::mutex_released",
      fields = FieldList([
          Field(
              name = "timestamp",
              field_type = aftermath.types.base.am_timestamp_t,
              comment = "Time of the event"),
          Field(
              name = "wait_id",
              field_type = aftermath.types.builtin.uint64_t,
              comment = "ID of the object that is being locked"),
          Field(
              name = "kind",
              field_type = am_ompt_mutex_t,
              comment = "Type of the mutual exclusion")]))

################################################################################

am_ompt_work = InMemoryCompoundType(
    name = "am_ompt_work",
    entity = "An OpenMP worksharing region execution interval",
    comment = "An OpenMP worksharing region execution interval",
    ident = "am::ompt::work",
        fields = FieldList([
            Field(
                name = "type",
                field_type = am_ompt_work_t,
                comment = "Type of work"),
            Field(
                name = "count",
                field_type = aftermath.types.builtin.uint64_t,
                comment = "Measure of the quantity involved in work"),
            Field(
                name = "interval",
                field_type = aftermath.types.in_memory.am_interval,
                comment = "Interval of the execution")]))

################################################################################

am_ompt_dependences = InMemoryCompoundType(
    name = "am_ompt_dependences",
    entity = "An OpenMP dependences event",
    comment = "An OpenMP dependences event",
    ident = "am::ompt::dependences",
        fields = FieldList([
            Field(
                name = "timestamp",
                field_type = aftermath.types.base.am_timestamp_t,
                comment = "Timestamp of the event"),
            Field(
                name = "ndeps",
                field_type = aftermath.types.builtin.uint32_t,
                comment = "Number of dependences")]))

################################################################################

am_ompt_task_dependence = InMemoryCompoundType(
    name = "am_ompt_task_dependence",
    entity = "An OpenMP task dependence event",
    comment = "An OpenMP task dependence event",
    ident = "am::ompt::task_dependence",
        fields = FieldList([
            Field(
                name = "timestamp",
                field_type = aftermath.types.base.am_timestamp_t,
                comment = "Timestamp of the event"),
            Field(
                name = "src_task_id",
                field_type = aftermath.types.builtin.uint64_t,
                comment = "Unique id of the source task"),
            Field(
                name = "sink_task_id",
                field_type = aftermath.types.builtin.uint64_t,
                comment = "Unique id of the sink task")]))

################################################################################

am_ompt_master = InMemoryCompoundType(
    name = "am_ompt_master",
    entity = "An OpenMP master region execution interval",
    comment = "An OpenMP master region execution interval",
    ident = "am::ompt::master",
        fields = FieldList([
            Field(
                name = "interval",
                field_type = aftermath.types.in_memory.am_interval,
                comment = "Interval of the execution")]))

################################################################################

am_ompt_sync_region = InMemoryCompoundType(
    name = "am_ompt_sync_region",
    entity = "an OpenMP sync region execution interval",
    comment = "An OpenMP sync region execution interval",
    ident = "am::ompt::sync_region",
        fields = FieldList([
            Field(
                name = "kind",
                field_type = am_ompt_sync_region_t,
                comment = "Type of the sync region"),
            Field(
                name = "interval",
                field_type = aftermath.types.in_memory.am_interval,
                comment = "Interval of the execution")]))

################################################################################

am_ompt_lock_init = InMemoryCompoundType(
    name = "am_ompt_lock_init",
    entity = "An OpenMP lock init event",
    comment = "An OpenMP lock init event",
    ident = "am::ompt::lock_init",
      fields = FieldList([
          Field(
              name = "timestamp",
              field_type = aftermath.types.base.am_timestamp_t,
              comment = "Time of the event"),
          Field(
              name = "wait_id",
              field_type = aftermath.types.builtin.uint64_t,
              comment = "ID of the object that is being locked"),
          Field(
              name = "kind",
              field_type = am_ompt_mutex_t,
              comment = "Type of the lock")]))

################################################################################

am_ompt_lock_destroy = InMemoryCompoundType(
    name = "am_ompt_lock_destroy",
    entity = "An OpenMP lock destroy event",
    comment = "An OpenMP lock destroy event",
    ident = "am::ompt::lock_destroy",
      fields = FieldList([
          Field(
              name = "timestamp",
              field_type = aftermath.types.base.am_timestamp_t,
              comment = "Time of the event"),
          Field(
              name = "wait_id",
              field_type = aftermath.types.builtin.uint64_t,
              comment = "ID of the object that is being locked"),
          Field(
              name = "kind",
              field_type = am_ompt_mutex_t,
              comment = "Type of the lock")]))

################################################################################

am_ompt_mutex_acquire = InMemoryCompoundType(
    name = "am_ompt_mutex_acquire",
    entity = "An OpenMP mutex acquire event",
    comment = "An OpenMP mutex acquire event",
    ident = "am::ompt::mutex_acquire",
      fields = FieldList([
          Field(
              name = "timestamp",
              field_type = aftermath.types.base.am_timestamp_t,
              comment = "Time of the event"),
          Field(
              name = "wait_id",
              field_type = aftermath.types.builtin.uint64_t,
              comment = "ID of the object that is being locked"),
          Field(
              name = "kind",
              field_type = am_ompt_mutex_t,
              comment = "Type of the mutual exclusion"),
          Field(
              name = "hint",
              field_type = am_ompt_sync_hint_t,
              comment = "Hint provided at the lock initialization"),
          Field(
              name = "implementation",
              field_type = aftermath.types.builtin.uint32_t,
              comment = "Mechanism chosen by runtime to implement the lock")]))

################################################################################

am_ompt_mutex_acquired = InMemoryCompoundType(
    name = "am_ompt_mutex_acquired",
    entity = "An OpenMP mutex acquire event",
    comment = "An OpenMP mutex acquire event",
    ident = "am::ompt::mutex_acquired",
      fields = FieldList([
          Field(
              name = "timestamp",
              field_type = aftermath.types.base.am_timestamp_t,
              comment = "Time of the event"),
          Field(
              name = "wait_id",
              field_type = aftermath.types.builtin.uint64_t,
              comment = "ID of the object that is being locked"),
          Field(
              name = "kind",
              field_type = am_ompt_mutex_t,
              comment = "Type of the mutual exclusion")]))

################################################################################

am_ompt_nest_lock = InMemoryCompoundType(
    name = "am_ompt_nest_lock",
    entity = "An OpenMP nest lock execution interval",
    comment = "An OpenMP nest lock execution interval",
    ident = "am::ompt::nest_lock",
        fields = FieldList([
            Field(
                name = "wait_id",
                field_type = aftermath.types.builtin.uint64_t,
                comment = "ID of the locked object"),
            Field(
                name = "interval",
                field_type = aftermath.types.in_memory.am_interval,
                comment = "Interval of the execution")]))

################################################################################

am_ompt_flush = InMemoryCompoundType(
    name = "am_ompt_flush",
    entity = "An OpenMP flush event",
    comment = "An OpenMP flush event",
    ident = "am::ompt::flush",
        fields = FieldList([
            Field(
                name = "timestamp",
                field_type = aftermath.types.base.am_timestamp_t,
                comment = "Timestamp of the event")]))

################################################################################

am_ompt_cancel = InMemoryCompoundType(
    name = "am_ompt_cancel",
    entity = "An OpenMP cancel event",
    comment = "An OpenMP cancel event",
    ident = "am::ompt::cancel",
        fields = FieldList([
            Field(
                name = "timestamp",
                field_type = aftermath.types.base.am_timestamp_t,
                comment = "Timestamp of the event"),
            Field(
                name = "flags",
                field_type = am_ompt_cancel_flag_t,
                comment = "Cancellation source and cancelled construct information")
]))

################################################################################

am_ompt_loop = InMemoryCompoundType(
    name = "am_ompt_loop",
    entity = "An OpenMP loop execution interval",
    comment = "An OpenMP loop execution interval",
    ident = "am::ompt::loop",
    fields = FieldList([
        Field(
            name = "interval",
            field_type = aftermath.types.in_memory.am_interval,
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

################################################################################

am_ompt_loop_chunk = InMemoryCompoundType(
    name = "am_ompt_loop_chunk",
    entity = "An OpenMP loop chunk dispatch event",
    comment = "An event generated by OpenMP loop chunk dispatch",
    ident = "am::ompt::loop_chunk",
    fields = FieldList([
        Field(
            name = "timestamp",
            field_type = aftermath.types.base.am_timestamp_t,
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
            field_type = aftermath.types.base.am_bool_t,
            comment = "End of the last period")]))

################################################################################

all_types = TypeList([
    am_ompt_sync_region_t,
    am_ompt_work_t,
    am_ompt_thread_t,
    am_ompt_parallel_flag_t,
    am_ompt_implicit_task_t,
    am_ompt_mutex_t,
    am_ompt_sync_hint_t,
    am_ompt_task_status_t,
    am_ompt_task_flag_t,
    am_ompt_cancel_flag_t,
    am_ompt_thread,
    am_ompt_parallel,
    am_ompt_task_create,
    am_ompt_task_schedule,
    am_ompt_implicit_task,
    am_ompt_sync_region_wait,
    am_ompt_mutex_released,
    am_ompt_work,
    am_ompt_dependences,
    am_ompt_task_dependence,
    am_ompt_master,
    am_ompt_sync_region,
    am_ompt_lock_init,
    am_ompt_lock_destroy,
    am_ompt_mutex_acquire,
    am_ompt_mutex_acquired,
    am_ompt_nest_lock,
    am_ompt_flush,
    am_ompt_cancel,
    am_ompt_loop,
    am_ompt_loop_chunk
])

aftermath.config.addMemTypes(*all_types)
