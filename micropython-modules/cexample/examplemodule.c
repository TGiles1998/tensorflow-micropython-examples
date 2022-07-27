// Include MicroPython API.
#include "py/runtime.h"
#include "py/stackctrl.h"
//#include "py/mpthread.h"
#include "py/mpconfig.h"


#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"

typedef struct _mp_thread_mutex_t {
    SemaphoreHandle_t handle;
    StaticSemaphore_t buffer;
} mp_thread_mutex_t;

int mp_thread_mutex_lock(mp_thread_mutex_t *mutex, int wait) {
    return pdTRUE == xSemaphoreTake(mutex->handle, wait ? portMAX_DELAY : 0);
}

void mp_thread_mutex_unlock(mp_thread_mutex_t *mutex) {
    xSemaphoreGive(mutex->handle);
}

void mp_thread_create_ex(void *(*entry)(void *), void *arg, size_t *stack_size, int priority, char *name) {
    // store thread entry function into a global variable so we can access it
    ext_thread_entry = entry;

    if (*stack_size == 0) {
        *stack_size = MP_THREAD_DEFAULT_STACK_SIZE; // default stack size
    } else if (*stack_size < MP_THREAD_MIN_STACK_SIZE) {
        *stack_size = MP_THREAD_MIN_STACK_SIZE; // minimum stack size
    }

    // Allocate linked-list node (must be outside thread_mutex lock)
    mp_thread_t *th = m_new_obj(mp_thread_t);

    mp_thread_mutex_lock(&thread_mutex, 1);

    // create thread
    BaseType_t result = xTaskCreatePinnedToCore(freertos_entry, name, *stack_size / sizeof(StackType_t), arg, priority, &th->id, MP_TASK_COREID);
    if (result != pdPASS) {
        mp_thread_mutex_unlock(&thread_mutex);
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

    mp_thread_mutex_unlock(&thread_mutex);
}

void mp_thread_create(void *(*entry)(void *), void *arg, size_t *stack_size) {
    mp_thread_create_ex(entry, arg, stack_size, MP_THREAD_PRIORITY, "mp_thread");
}

// This is the function which will be called from Python as cexample.add_ints(a, b).
STATIC mp_obj_t example_add_ints(mp_obj_t a_obj, mp_obj_t b_obj) {
    // Extract the ints from the micropython input objects.
    int a = mp_obj_get_int(a_obj);
    int b = mp_obj_get_int(b_obj);

    // Calculate the addition and convert to MicroPython object.
    return mp_obj_new_int(a + b);
}

STATIC mp_obj_t mod_thread_start_new_thread(size_t n_args, const mp_obj_t *args) {
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
    mp_thread_create(thread_entry, th_args, &th_args->stack_size);

    return mp_const_none;
}

// Define a Python reference to the function above.
STATIC MP_DEFINE_CONST_FUN_OBJ_2(example_add_ints_obj, example_add_ints);
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_thread_start_new_thread_obj, 2, 3, mod_thread_start_new_thread);

// Define all properties of the module.
// Table entries are key/value pairs of the attribute name (a string)
// and the MicroPython object reference.
// All identifiers and strings are written as MP_QSTR_xxx and will be
// optimized to word-sized integers by the build system (interned strings).
STATIC const mp_rom_map_elem_t example_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_cexample) },
    { MP_ROM_QSTR(MP_QSTR_add_ints), MP_ROM_PTR(&example_add_ints_obj) },
    { MP_ROM_QSTR(MP_QSTR_start_new_thread), MP_ROM_PTR(&mod_thread_start_new_thread_obj) },
};
STATIC MP_DEFINE_CONST_DICT(example_module_globals, example_module_globals_table);

// Define module object.
const mp_obj_module_t example_user_cmodule = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t *)&example_module_globals,
};

// Register the module to make it available in Python.
MP_REGISTER_MODULE(MP_QSTR_cexample, example_user_cmodule, 1);
