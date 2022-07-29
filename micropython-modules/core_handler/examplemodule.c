// Include MicroPython API.
#include "py/runtime.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h" // Defined here: xSemaphoreTake, xSemaphoreGive
#include "freertos/queue.h"

// This is the function which will be called from Python as cexample.add_ints(a, b).
STATIC mp_obj_t example_add_ints(mp_obj_t a_obj, mp_obj_t b_obj) {
    // Extract the ints from the micropython input objects.
    int a = mp_obj_get_int(a_obj);
    int b = mp_obj_get_int(b_obj);

    xTaskCreatePinnedToCore(coreTaskZero, "coreTaskZero", 10000, NULL, 1, NULL, taskCoreZero);

    delay(500);

    // Calculate the addition and convert to MicroPython object.
    return mp_obj_new_int(a + b);
}
void task1code( void * pvParameters ) {
    for(;;){
        mp_printf(MICROPY_ERROR_PRINTER, "Task1code is cool core 1");
        delay(500);
    }
}
void task2code( void * pvParameters ) {
    for(;;){
        mp_printf(MICROPY_ERROR_PRINTER, "Task2code is cool core 0");
        delay(700);
    }
}

STATIC mp_obj_t mod_core_function(size_t n_args, const mp_obj_t *args) {
    // This structure holds the Python function and arguments for thread entry.
    // We copy all arguments into this structure to keep ownership of them.
    // We must be very careful about root pointers because this pointer may
    // disappear from our address space before the thread is created.
    thread_entry_args_t *th_args;
    
    // get positional arguments
    size_t pos_args_len;
    mp_obj_t *pos_args_items;
    mp_obj_get_array(args[1], &pos_args_len, &pos_args_items);

    // just position arguments
    th_args = m_new_obj_var(thread_entry_args_t, mp_obj_t, pos_args_len);
    th_args->n_kw = 0;
    
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
    
    // Gets the core id
    int core_id = mp_obj_get_int(args[2]);

//    BaseType_t result = xTaskCreatePinnedToCore(
//        freertos_entry,
//        name,
//        *stack_size / sizeof(StackType_t),
//        th_args,
//        priority,
//        &th->id,
//        core_id,
//    );
    xTaskCreatePinnedToCore(task1code, "Task1", 10000, NULL, 1, &Task1, 1);
    xTaskCreatePinnedToCore(task2code, "Task2", 10000, NULL, 1, &Task1, 0);

    return mp_const_none;
}

// Define a Python reference to the function above.
STATIC MP_DEFINE_CONST_FUN_OBJ_2(example_add_ints_obj, example_add_ints);
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(mod_core_function_obj, 2, 3, mod_core_function);

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
const mp_obj_module_t example_user_cmodule = {
        .base = { &mp_type_module },
        .globals = (mp_obj_dict_t *)&example_module_globals,
};

// Register the module to make it available in Python.
MP_REGISTER_MODULE(MP_QSTR_core_handler, example_user_cmodule, 1);