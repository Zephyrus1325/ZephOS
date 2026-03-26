#include "file_explorer.h"

#include "include/syscall.h"

static void main(){
    while(1){
        delay(100000);
    }
}


// Deixar isso aqui na parte de baixo de tudo
task_t file_explorer = {
    main, // Ponto de entrada da tarefa
    1500  // Quantas words são armazenadas no stack
};

