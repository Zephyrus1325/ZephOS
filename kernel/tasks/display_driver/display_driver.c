#include "display_driver.h"
#include "include/syscall.h"

/*
 *  Task Responsável por criar todo os gráficos básicos do sistema
 *  Boa parte das coisas do so devem ser um service para esta task
 */

static void main(){
    

    while(1){
        msleep(10000);
    }
}


// Deixar isso aqui na parte de baixo de tudo
task_t display_task = {
    main, // Ponto de entrada da tarefa
    1500  // Quantas words são armazenadas no stack
};

