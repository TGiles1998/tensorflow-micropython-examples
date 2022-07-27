
#include <string.h>
// Include MicroPython API.
#include "py/runtime.h"
#include "py/stackctrl.h"

//#include "py/stackctrl.h"
//#include "py/mpconfig.h"
//#include "py/modthread.c"
//#include "py/mpthread.h"
//#include <mpthreadport.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h" // Defined here: xSemaphoreTake, xSemaphoreGive
#include "freertos/queue.h"

#define MP_MY_TASK_STACK_SIZE      (16 * 1024)
#define ESP_TASK_PRIO_MIN (0)

typedef struct _mp_my_thread_mutex_t {
    SemaphoreHandle_t handle;
    StaticSemaphore_t buffer;
} mp_my_thread_mutex_t;

int mp_my_thread_mutex_lock(mp_my_thread_mutex_t *mutex, int wait) {
    return pdTRUE == xSemaphoreTake(mutex->handle, wait ? portMAX_DELAY : 0);
}

void mp_my_thread_mutex_unlock(mp_my_thread_mutex_t *mutex) {
    xSemaphoreGive(mutex->handle);
}

#define MP_THREAD_MIN_STACK_SIZE                        (4 * 1024)
#define MP_THREAD_DEFAULT_STACK_SIZE                    (MP_THREAD_MIN_STACK_SIZE + 1024)
#define MP_THREAD_PRIORITY                              (ESP_TASK_PRIO_MIN + 1)

// this structure forms a linked list, one node per active thread
typedef struct _mp_my_thread_t {
    TaskHandle_t id;        // system id of thread
    int ready;              // whether the thread is ready and running
    void *arg;              // thread Python args, a GC root pointer
    void *stack;            // pointer to the stack
    size_t stack_len;       // number of words in the stack
    struct _mp_my_thread_t *next;
} mp_my_thread_t;

// the mutex controls access to the linked list
STATIC mp_my_thread_mutex_t thread_mutex;
STATIC mp_my_thread_t thread_entry0;
STATIC mp_my_thread_t *thread = NULL; // root pointer, handled by mp_my_thread_gc_others

void mp_my_thread_set_state(mp_state_thread_t *state) {
    vTaskSetThreadLocalStoragePointer(NULL, 1, state);
}

void mp_my_thread_mutex_init(mp_my_thread_mutex_t *mutex) {
    // Need a binary semaphore so a lock can be acquired on one Python thread
    // and then released on another.
    mutex->handle = xSemaphoreCreateBinaryStatic(&mutex->buffer);
    xSemaphoreGive(mutex->handle);
}


void mp_my_thread_init(void *stack, uint32_t stack_len) {
    mp_my_thread_set_state(&mp_state_ctx.thread);
    // create the first entry in the linked list of all threads
    thread_entry0.id = xTaskGetCurrentTaskHandle();
    thread_entry0.ready = 1;
    thread_entry0.arg = NULL;
    thread_entry0.stack = stack;
    thread_entry0.stack_len = stack_len;
    thread_entry0.next = NULL;
    mp_my_thread_mutex_init(&thread_mutex);

    // memory barrier to ensure above data is committed
    __sync_synchronize();

    // vPortCleanUpTCB needs the thread ready after thread_mutex is ready
    thread = &thread_entry0;
}

void mp_my_thread_start(void) {
    mp_my_thread_mutex_lock(&thread_mutex, 1);
    for (mp_my_thread_t *th = thread; th != NULL; th = th->next) {
        if (th->id == xTaskGetCurrentTaskHandle()) {
            th->ready = 1;
            break;
        }
    }
    mp_my_thread_mutex_unlock(&thread_mutex);
}

STATIC void *(*ext_thread_entry)(void *) = NULL;

STATIC void freertos_entry(void *arg) {
    if (ext_thread_entry) {
        ext_thread_entry(arg);
    }
    vTaskDelete(NULL);
    for (;;) {;
    }
}

void mp_my_thread_finish(void) {
    mp_my_thread_mutex_lock(&thread_mutex, 1);
    for (mp_my_thread_t *th = thread; th != NULL; th = th->next) {
        if (th->id == xTaskGetCurrentTaskHandle()) {
            th->ready = 0;
            break;
        }
    }
    mp_my_thread_mutex_unlock(&thread_mutex);
}

typedef struct _thread_entry_args_t {
    mp_obj_dict_t *dict_locals;
    mp_obj_dict_t *dict_globals;
    size_t stack_size;
    mp_obj_t fun;
    size_t n_args;
    size_t n_kw;
    mp_obj_t args[];
} thread_entry_args_t;


STATIC void *thread_entry(void *args_in) {
    // Execution begins here for a new thread.  We do not have the GIL.

    thread_entry_args_t *args = (thread_entry_args_t *)args_in; // good

    mp_state_thread_t ts; // Maybe
    mp_my_thread_set_state(&ts); // Good

    mp_stack_set_top(&ts + 1); // need to include ts in root-pointer scan
    mp_stack_set_limit(args->stack_size);

    #if MICROPY_ENABLE_PYSTACK
    // TODO threading and pystack is not fully supported, for now just make a small stack
    mp_obj_t mini_pystack[128];
    mp_pystack_init(mini_pystack, &mini_pystack[128]);
    #endif

    // The GC starts off unlocked on this thread.
    ts.gc_lock_depth = 0;

    ts.mp_pending_exception = MP_OBJ_NULL;

    // set locals and globals from the calling context
    mp_locals_set(args->dict_locals); // Runtime
    mp_globals_set(args->dict_globals); // Runtime

    MP_THREAD_GIL_ENTER();

    // signal that we are set up and running
    mp_my_thread_start();

    // TODO set more thread-specific state here:
    //  cur_exception (root pointer)

    DEBUG_printf("[thread] start ts=%p args=%p stack=%p\n", &ts, &args, MP_STATE_THREAD(stack_top));

    nlr_buf_t nlr;
    if (nlr_push(&nlr) == 0) {
        mp_call_function_n_kw(args->fun, args->n_args, args->n_kw, args->args);
        nlr_pop();
    } else {
        // uncaught exception
        // check for SystemExit
        mp_obj_base_t *exc = (mp_obj_base_t *)nlr.ret_val;
        if (mp_obj_is_subclass_fast(MP_OBJ_FROM_PTR(exc->type), MP_OBJ_FROM_PTR(&mp_type_SystemExit))) {
            // swallow exception silently
        } else {
            // print exception out
            mp_printf(MICROPY_ERROR_PRINTER, "Unhandled exception in thread started by ");
            mp_obj_print_helper(MICROPY_ERROR_PRINTER, args->fun, PRINT_REPR);
            mp_printf(MICROPY_ERROR_PRINTER, "\n");
            mp_obj_print_exception(MICROPY_ERROR_PRINTER, MP_OBJ_FROM_PTR(exc));
        }
    }

    DEBUG_printf("[thread] finish ts=%p\n", &ts);

    // signal that we are finished
    mp_my_thread_finish();

    MP_THREAD_GIL_EXIT();

    return NULL;
}

void mp_my_thread_create_ex(void *(*entry)(void *), void *arg, size_t *stack_size, int priority, char *name) {
    // store thread entry function into a global variable so we can access it
    ext_thread_entry = entry;

    if (*stack_size == 0) {
        *stack_size = MP_THREAD_DEFAULT_STACK_SIZE; // default stack size
    } else if (*stack_size < MP_THREAD_MIN_STACK_SIZE) {
        *stack_size = MP_THREAD_MIN_STACK_SIZE; // minimum stack size
    }

    // Allocate linked-list node (must be outside thread_mutex lock)
    mp_my_thread_t *th = m_new_obj(mp_my_thread_t);

    mp_my_thread_mutex_lock(&thread_mutex, 1);

    // create thread
    BaseType_t result = xTaskCreatePinnedToCore(freertos_entry, name, *stack_size / sizeof(StackType_t), arg, priority, &th->id, 0);//MP_TASK_COREID
    if (result != pdPASS) {
        mp_my_thread_mutex_unlock(&thread_mutex);
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("can't create thread"));
    }

    // add thread to linked list of all threads
    th->ready = 0;
    th->arg = arg;
    th->stack = pxTaskGetStackStart(th->id);
    th->stack_len = *stack_size / sizeof(uintptr_t);
    th->next = thread;
    thread = th;

    // adjust the stack_size to provide room to recover from hitting the limit
    *stack_size -= 1024;

    mp_my_thread_mutex_unlock(&thread_mutex);
}

void mp_my_thread_create(void *(*entry)(void *), void *arg, size_t *stack_size) {

    // Initialise my thread
    mp_my_thread_init(pxTaskGetStackStart(NULL), MP_MY_TASK_STACK_SIZE / sizeof(uintptr_t));

    mp_my_thread_create_ex(entry, arg, stack_size, MP_THREAD_PRIORITY, "mp_thread");
}

STATIC size_t thread_stack_size = 0;

// This is the function which will be called from Python as cexample.add_ints(a, b).
STATIC mp_obj_t example_add_ints(mp_obj_t a_obj, mp_obj_t b_obj) {
    // Extract the ints from the micropython input objects.
    int a = mp_obj_get_int(a_obj);
    int b = mp_obj_get_int(b_obj);

    // Calculate the addition and convert to MicroPython object.
    return mp_obj_new_int(a + b);
}


STATIC mp_obj_t mod_my_thread_start_new_thread(size_t n_args, const mp_obj_t *args) {
    // This structure holds the Python function and arguments for thread entry.
    // We copy all arguments into this structure to keep ownership of them.
    // We must be very careful about root pointers because this pointer may
    // disappear from our address space before the thread is created.
    thread_entry_args_t *th_args;

    // get positional arguments
    size_t pos_args_len;
    mp_obj_t *pos_args_items;
    mp_obj_get_array(args[1], &pos_args_len, &pos_args_items);

    // check for keyword arguments
    if (n_args == 2) {
        // just position arguments
        th_args = m_new_obj_var(thread_entry_args_t, mp_obj_t, pos_args_len);
        th_args->n_kw = 0;
    } else {
        // positional and keyword arguments
        if (mp_obj_get_type(args[2]) != &mp_type_dict) {
            mp_raise_TypeError(MP_ERROR_TEXT("expecting a dict for keyword args"));
        }
        mp_map_t *map = &((mp_obj_dict_t *)MP_OBJ_TO_PTR(args[2]))->map;
        th_args = m_new_obj_var(thread_entry_args_t, mp_obj_t, pos_args_len + 2 * map->used);
        th_args->n_kw = map->used;
        // copy across the keyword arguments
        for (size_t i = 0, n = pos_args_len; i < map->alloc; ++i) {
            if (mp_map_slot_is_filled(map, i)) {
                th_args->args[n++] = map->table[i].key;
                th_args->args[n++] = map->table[i].value;
            }
        }
    }

    // copy across the positional arguments
    th_args->n_args = pos_args_len;
    memcpy(th_args->args, pos_args_items, pos_args_len * sizeof(mp_obj_t));

    // pass our locals and globals into the new thread
    th_args->dict_locals = mp_locals_get();
    th_args->dict_globals = mp_globals_get();

    // set the stack size to use
    th_args->stack_size = thread_stack_size;

    // set the function for thread entry
    th_args->fun = args[0];

    // spawn the thread!
    mp_my_thread_create(thread_entry, th_args, &th_args->stack_size);

    return mp_const_none;
}

// Define a Python reference to the function above.
STATIC MP_DEFINE_CONST_FUN_OBJ_2(example_add_ints_obj, example_add_ints);
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_my_thread_start_new_thread_obj, 2, 3, mod_my_thread_start_new_thread);

// Define all properties of the module.
// Table entries are key/value pairs of the attribute name (a string)
// and the MicroPython object reference.
// All identifiers and strings are written as MP_QSTR_xxx and will be
// optimized to word-sized integers by the build system (interned strings).
STATIC const mp_rom_map_elem_t example_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_cexample) },
    { MP_ROM_QSTR(MP_QSTR_add_ints), MP_ROM_PTR(&example_add_ints_obj) },
    { MP_ROM_QSTR(MP_QSTR_start_new_thread), MP_ROM_PTR(&mod_my_thread_start_new_thread_obj) },
};
STATIC MP_DEFINE_CONST_DICT(example_module_globals, example_module_globals_table);

// Define module object.
const mp_obj_module_t example_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&example_module_globals,
};

// Register the module to make it available in Python.
MP_REGISTER_MODULE(MP_QSTR_cexample, example_user_cmodule, 1);

