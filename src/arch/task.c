#include <aplus.h>
#include <aplus/spinlock.h>
#include <aplus/task.h>
#include <aplus/mm.h>
#include <aplus/list.h>
#include <aplus/fs.h>


__asm__ (
	".global task_context_switch		\n"
	"task_context_switch:				\n"
	"	push ebp						\n"
	"	mov ebp, esp					\n"
	"	pushf							\n"
	"	push edx						\n"
	"	push eax						\n"	
	"	push ecx						\n"
	"	push ebx						\n"
	"	push esi						\n"
	"	push edi						\n"
	"	mov eax, [ebp + 8]				\n"
	"	mov edx, [ebp + 12]				\n"
	"	mov [eax], esp					\n"
	"	mov esp, [edx]					\n"
	"	pop edi							\n"
	"	pop esi							\n"
	"	pop ebx							\n"
	"	pop ecx							\n"
	"	pop eax							\n"
	"	pop edx							\n"
	"	popf							\n"
	"	pop ebp							\n"
	"	sti								\n"
	"ret								\n"
);


extern task_t* current_task;
extern task_t* kernel_task;
extern list_t* task_queue;

extern uint32_t* kernel_vmm;
extern uint32_t* current_vmm;

extern volatile inode_t* vfs_root;
extern void task_context_switch(task_env_t** old, task_env_t** new);


task_t* task_fork() {
	if(!current_task)
		return NULL;

	task_t* child = (task_t*) kmalloc(sizeof(task_t));
	memset(child, 0, sizeof(task_t));

	child->pid = schedule_nextpid();
	child->cwd = current_task->cwd;
	child->exe = current_task->exe;
	child->uid = current_task->uid;
	child->gid = current_task->gid;
	
	child->state = TASK_STATE_ALIVE;
	child->priority = current_task->priority;
	child->clock = 0;
	child->parent = current_task;

	child->signal_handler = current_task->signal_handler;
	child->signal_sig = current_task->signal_sig;

	
	for(int i = 0; i < TASK_MAX_FD; i++)
		child->fd[i] = current_task->fd[i];


	child->context.cr3 = vmm_create();
	vmm_mapkernel(child->context.cr3);


	child->image.vaddr = current_task->image.vaddr;
	child->image.length = current_task->image.length;

	
	if(current_task->image.ptr) {
		void* addr = (void*) kmalloc(child->image.length);
		memcpy(addr, (void*) child->image.vaddr, child->image.length);

		vmm_map(child->context.cr3, mm_paddr(addr), child->image.vaddr, child->image.length);
		child->image.ptr = (uint32_t) mm_paddr(addr);
	}

	
	
	child->context.stack = current_task->context.stack;
	child->context.env = current_task->context.env;

	void* stack = (void*) mm_align(kmalloc(TASK_STACKSIZE * 2));
	memcpy(stack, (void*) child->context.stack, TASK_STACKSIZE);

	vmm_umap(child->context.cr3, child->context.stack, TASK_STACKSIZE);
	vmm_map(child->context.cr3, mm_paddr(stack), child->context.stack, TASK_STACKSIZE, VMM_FLAGS_DEFAULT | VMM_FLAGS_USER);
	

	/* TODO: Set EIP for child */
	
	if(current_task == child)
		return 0;
	

	list_add(task_queue, (listval_t) child);
	return child;
}



task_t* task_clone(void* entry, void* arg, void* stack) {
	if(entry == NULL)
		return NULL;

	if(stack == NULL)
		stack = (void*) ((int) mm_align(kmalloc(TASK_STACKSIZE * 2)) + TASK_STACKSIZE);

	memset(stack, 0, TASK_STACKSIZE);


	task_t* child = (task_t*) kmalloc(sizeof(task_t));
	memset(child, 0, sizeof(task_t));

	child->pid = schedule_nextpid();
	child->cwd = current_task->cwd;
	child->exe = current_task->exe;
	child->uid = current_task->uid;
	child->gid = current_task->gid;
	
	child->state = TASK_STATE_ALIVE;
	child->priority = current_task->priority;
	child->clock = 0;
	child->parent = current_task;

	child->signal_handler = current_task->signal_handler;
	child->signal_sig = current_task->signal_sig;

	child->image.vaddr = current_task->image.vaddr;
	child->image.length = current_task->image.length;
	child->image.ptr = current_task->image.ptr;
		
	
	child->context.cr3 = current_task->context.cr3;
	child->context.stack = (uint32_t) stack - TASK_STACKSIZE;
	child->context.env = (task_env_t*) ((uint32_t) stack - sizeof(task_env_t));

	child->context.env->eax = (uint32_t) arg;
	child->context.env->eip = (uint32_t) entry;
	child->context.env->ebp = (uint32_t) child->context.env; 

	
	for(int i = 0; i < TASK_MAX_FD; i++)
		child->fd[i] = current_task->fd[i];


	list_add(task_queue, (listval_t) child);
	return child;
}


void task_switch(task_t* newtask) {
	
	task_t* old = current_task;
	current_task = newtask;


	vmm_switch(current_task->context.cr3);
	outb(0x20, 0x20);

	task_context_switch(&old->context.env, &current_task->context.env);
}



int task_init() {

	extern uint32_t kernel_stack;
	
	kernel_task = current_task = (task_t*) kmalloc(sizeof(task_t));
	memset(current_task, 0, sizeof(task_t));
	
	
	current_task->context.env = (task_env_t*) 0;
	current_task->context.cr3 = (uint32_t) kernel_vmm;
	current_task->context.stack = (uint32_t) &kernel_stack;
	
	
	current_task->pid = schedule_nextpid();
	current_task->cwd = (inode_t*) vfs_root;
	current_task->uid = (uid_t) 0;
	current_task->gid = (gid_t) 0;
	
	current_task->state = TASK_STATE_ALIVE;
	current_task->priority = TASK_PRIORITY_REGULAR;
	current_task->parent = NULL;


	list_add(task_queue, (listval_t) current_task);
	task_switch(current_task);


	return 0;
}
