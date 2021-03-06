#ifndef _KERNEL_ENV_H
#define _KERNEL_ENV_H

#include <include/env.h>
#include <kernel/cpu.h>

extern Env * envs;					// All environements
extern Segdesc gdt[];
#define curenv	(thiscpu->cpu_env)	// Current environment

void	env_init(void);
void	env_init_percpu(void);
int		env_alloc(Env **newenv_store, envid_t parent_id);
void	env_create(uint8_t *binary, enum EnvType type);
void	env_free(Env *e);
void	env_destroy(Env *e);

int		envid2env(envid_t envid, Env **env_store, bool checkperm);

void	env_run(Env *e) __attribute__((noreturn));
void	env_pop_tf(struct Trapframe *tf) __attribute__((noreturn));
// ENV_PASTE3 creates new token xyz.
#define ENV_PASTE3(x, y, z)		x ## y ## z

#define ENV_CREATE(x, type)										\
	do {														\
		extern uint8_t ENV_PASTE3(_binary_obj_, x, _start)[];	\
		env_create(ENV_PASTE3(_binary_obj_, x, _start), type);	\
	} while(0)

#endif
