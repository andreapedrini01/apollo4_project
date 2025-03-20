#include <stdarg.h>
#include <string.h>
#include <am_bsp.h>
#include <am_util.h>
#include <stdio.h>
#include <../constants.h>
#include <alpaca.h>

GLOBAL_SB2(StateManager, manager);

/**
 * @brief double buffered context
 */
__nv context_t context_1 = { 0 };
/**
 * @brief double buffered context
 */
__nv context_t context_0 = { .task = TASK_REF(_entry_task), .needCommit = 0, };
/**
 * @brief current context
 */
__nv context_t *volatile curctx = &context_0;

__nv task_t *volatile curtsk = TASK_REF(_entry_task);

/**
 * @brief Function to be invoked at the beginning of every start
 */
void init_state_manager() {
	if(GV(signature) != INIT_SIGNATURE) {
    COPY_VALUE(&GV(needCommit), FALSE);
    COPY_VALUE(&GV(index), FALSE);

		int ind = GV(index);
		CritVar* buffer1 = &manager.buffer[1-ind];
		CritVar* buffer0 = &manager.buffer[ind];
		
		uint32_t *p_buffer1 = (uint32_t *)buffer1;
    uint32_t *p_buffer0 = (uint32_t *)buffer0;
		
		uint32_t n = sizeof(CritVar) / sizeof(uint32_t);
		am_util_stdio_printf("n = %X\n", n);
		for (uint32_t i = 0; i < n; i++) {
				COPY_VALUE(&p_buffer1[i], 0);
				COPY_VALUE(&p_buffer0[i], 0);
    }
		
		//init done, set signature
		COPY_VALUE(&GV(signature), INIT_SIGNATURE);
	}
}

/**
 * @brief Function to be invoked when changing indexes is needed
 */
void need_commit_buffer(int choice) {
	COPY_VALUE(&GV(needCommit), choice);
}

/**
 * @brief Function to be invoked to change original buffer
 */
void commit_state() { 
	switch(GV_STATE) {
		case READY: break;
		case COMMIT1:
			COPY_VALUE(&GV(newIndex), 1 ^ GV(index));
			update_buffer_state(COMMIT2);
		case COMMIT2:
			COPY_VALUE(&GV(index), GV(newIndex));
			need_commit_buffer(FALSE);
			update_buffer_state(READY);
	}
}

/**
 * @brief Function to be invoked when a buffers rollback is needed
 */
void rollback_state() {
		int ind = GV(index);
		CritVar* dest = &manager.buffer[1-ind];
		CritVar* src = &manager.buffer[ind];
		// Considering the struct of crit variables as an array
    uint32_t *p_dest = (uint32_t *)dest;
    const uint32_t *p_src = (const uint32_t *)src;
    // Calculate how many words are in CritVar
    uint32_t n = sizeof(CritVar) / sizeof(uint32_t);
	  //rollback buffer
    for (uint32_t i = 0; i < n; i++) {
        if (p_dest[i] != p_src[i]) {
            COPY_VALUE(&p_dest[i], p_src[i]);
        }
    }
}

void update_task_state(context_t* context, State newState) {
		COPY_VALUE(&(context->task->state), newState);
}

void update_buffer_state(State newState) {
	COPY_VALUE(&GV_STATE, newState);
}

/**
 * @brief Function to be invoked at the beginning of every task
 */
void task_prologue()
{	
	commit_state();
		
	switch(curctx->task->state){
		case READY: 
			rollback_state();
			//execute the task, to do in main
			break;
		case COMMIT1: 
			transition_to(curtsk);
			break;
		case COMMIT2:
			transition_to(curtsk);
	}
}

/**
 * @brief Transfer control to the given task
 * @details Finalize the current task and jump to the given task.
 *
 */
void transition_to(task_t *next_task)
{
		COPY_PTR(curtsk, next_task);
		context_t *next_ctx;
	
		switch(curctx->task->state) {
			case READY: 
				update_task_state(curctx, COMMIT1);
			case COMMIT1:
				update_buffer_state(COMMIT1);
			case COMMIT2:
				update_task_state(curctx, COMMIT2);
				next_ctx = (curctx == &context_0 ? &context_1 : &context_0);
				COPY_VALUE(&next_ctx->needCommit, TRUE);
				COPY_PTR(next_ctx->task, next_task);
				update_task_state(next_ctx, READY);
				COPY_PTR(curctx, next_ctx);
		}
		
}

/** @brief Entry point upon reboot */
//int main()
//{
////    setvbuf(stdout, NULL, _IONBF, 0);
//    __enable_irq();				//modified for apollo4 blue lite
//    init();

//    // check for update
//    task_prologue();

//    // jump to curctx
////    __asm__ volatile ( // volatile because output operands unused by C
////            "br %[nt]\n"
////            : /* no outputs */
////            : [nt] "r" (curctx->task->func)
////    );
//    return 0;
//}

