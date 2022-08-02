#include <string.h>
#include "py/runtime.h"
#include "py/stackctrl.h"
#include "py/nlr.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h" // Defined here: xSemaphoreTake, xSemaphoreGive
#include "freertos/queue.h"

//#define portTICK_PERIOD_MS = 10;

//#define MP_THREAD_MIN_STACK_SIZE                        (4 * 1024)
//#define MP_THREAD_DEFAULT_STACK_SIZE                    (MP_THREAD_MIN_STACK_SIZE + 1024)


// This is the function which will be called from Python as cexample.add_ints(a, b).
STATIC mp_obj_t example_add_ints(mp_obj_t a_obj, mp_obj_t b_obj) {
    // Extract the ints from the micropython input objects.
    int a = mp_obj_get_int(a_obj);
    int b = mp_obj_get_int(b_obj);

//    xTaskCreatePinnedToCore(coreTaskZero, "coreTaskZero", 10000, NULL, 1, NULL, taskCoreZero);

    // Calculate the addition and convert to MicroPython object.
    return mp_obj_new_int(a + b);
}
//void task1code( void * pvParameters ) {
//    for(;;){
//        mp_printf(MICROPY_ERROR_PRINTER, "\n Task1code is cool core 1 \n");
//        vTaskDelay(500 / 10);
//    }
//}
//void task2code( void * pvParameters ) {
//    for(;;){
//        mp_printf(MICROPY_ERROR_PRINTER, "\n Task2code is cool core 0 \n");
//        vTaskDelay(700 / 10);
//    }
//}

typedef struct _core_thread_entry_args_t {
    mp_obj_dict_t *dict_locals;
    mp_obj_dict_t *dict_globals;
    size_t stack_size;
    mp_obj_t fun;
    size_t n_args;
    size_t n_kw;
    mp_obj_t args[];
} core_thread_entry_args_t;

STATIC void run_function(void *args) {
    mp_call_function_n_kw(args->fun, args->n_args, args->n_kw, args->args);
//    nlr_buf_t nlr;
//    if (nlr_push(&nlr) == 0) {
//        ;
//        nlr_pop();
//    }
}

STATIC mp_obj_t mod_core_function(size_t n_args, const mp_obj_t *args) {
    // This structure holds the Python function and arguments for thread entry.
    // We copy all arguments into this structure to keep ownership of them.
    // We must be very careful about root pointers because this pointer may
    // disappear from our address space before the thread is created.
    core_thread_entry_args_t *th_args;

    // get positional arguments
    size_t pos_args_len;
    mp_obj_t *pos_args_items;
    mp_obj_get_array(args[1], &pos_args_len, &pos_args_items);

    // just position arguments
    th_args = m_new_obj_var(core_thread_entry_args_t, mp_obj_t, pos_args_len);
    th_args->n_kw = 0;

    // copy across the positional arguments
    th_args->n_args = pos_args_len;
    memcpy(th_args->args, pos_args_items, pos_args_len * sizeof(mp_obj_t));

    // pass our locals and globals into the new thread
    th_args->dict_locals = mp_locals_get();
    th_args->dict_globals = mp_globals_get();

    // set the function for thread entry
    th_args->fun = args[0];

    // Gets the core id
    const char* name = mp_obj_get_type_str(args[2]); //Task1
    mp_printf(MICROPY_ERROR_PRINTER, "\n Task1code is cool core 1 \n");
    int stack_size = mp_obj_get_int(args[3]);  // 10000
    int priority = mp_obj_get_int(args[4]);  // 1
    int core_id = mp_obj_get_int(args[5]);  // 1

    // set the stack size to use
    th_args->stack_size = stack_size;

    TaskHandle_t Task1;

    core_thread_entry_args_t *in_args = (core_thread_entry_args_t *)th_args;

    mp_printf(MICROPY_ERROR_PRINTER, "\n Starting task \n");
    xTaskCreatePinnedToCore(run_function, name, stack_size, in_args, priority, &Task1, core_id);
//    xTaskCreatePinnedToCore(task2code, "Task2", 10000, NULL, 1, &Task2, 0);

    return mp_const_none;
}

// Define a Python reference to the function above.
STATIC MP_DEFINE_CONST_FUN_OBJ_2(example_add_ints_obj, example_add_ints);
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_core_function_obj, 2, 6, mod_core_function);

// Define all properties of the module.
// Table entries are key/value pairs of the attribute name (a string)
// and the MicroPython object reference.
// All identifiers and strings are written as MP_QSTR_xxx and will be
// optimized to word-sized integers by the build system (interned strings).
STATIC const mp_rom_map_elem_t example_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_cexample) },
    { MP_ROM_QSTR(MP_QSTR_add_ints), MP_ROM_PTR(&example_add_ints_obj) },
    { MP_ROM_QSTR(MP_QSTR_start_new_thread), MP_ROM_PTR(&mod_core_function_obj) },
};
STATIC MP_DEFINE_CONST_DICT(example_module_globals, example_module_globals_table);

// Define module object.
const mp_obj_module_t core_handler = {
        .base = { &mp_type_module },
        .globals = (mp_obj_dict_t *)&example_module_globals,
};

// Register the module to make it available in Python.
MP_REGISTER_MODULE(MP_QSTR_core_handler, core_handler, 1);