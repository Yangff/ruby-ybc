#include "general.h"

#ifndef true
	#define true 1
	#define false 0
#endif

// STACK CACHE
rb_stack_t *stack_cache;
rb_stack_t *stack_cache_top;
rb_stack_t *current_stack;

void stack_created() {
	asm("leaq 16(%%rbp), %%rax; mov %%rax, %0":"=m"(stack_cache_top->bp)::"rax");
	stack_cache_top->alive = true;
	current_stack = stack_cache_top;
	stack_cache_top += sizeof(struct rb_stack_struct);
}

void stack_destroying() {
	current_stack->alive = false;
	current_stack->bp = (uintptr_t) malloc(current_stack->size);
	// copy
	asm(
			// size
			"mov %1, %%rcx;"
			"lea 16(%%rbp), %%rsi;"
			"mov %0, %%rdi;"
			"cld;"										// decrement rdi, rsi
			"shr $3, %%rcx;"
			"rep movsq;"
			:"=m"(current_stack->bp):"m"(current_stack->size));
}

// NATIVE PROC
VALUE rb_cNativeProc;

VALUE rb_new_native_proc(VALUE(*func)(ANYARGS), int argc, uintptr_t locals_ptr, int size) {
	VALUE result = rb_funcall(rb_cNativeProc, rb_intern("new"), 0);
	rb_define_singleton_method(result, "call", func, argc);
	rb_iv_set(result, "@locals_ptr", LL2NUM(locals_ptr));
	rb_iv_set(result, "@locals_size", INT2NUM(size));
	return result;
}