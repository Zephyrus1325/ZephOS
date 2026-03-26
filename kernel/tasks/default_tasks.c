#include "tasks/default_tasks.h"
#include "core/task.h"
// Tarefas
#include "tasks/file_explorer/file_explorer.h"
#include "tasks/display_driver/display_driver.h"
#include "drivers/uart.h"


task_t* default_tasks[] = {
    &file_explorer,
    &display_task,
};


void start_default_tasks(){
    for(int i = 0; i < sizeof(default_tasks)/sizeof(task_t*); i++){
        k_task_create_no_interrupt(*default_tasks[i]->entry, default_tasks[i]->stack_size);
    }
}