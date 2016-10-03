/*   -*- linux-c -*-
 *   arch/x86/include/asm/ipipe.h
 *
 *   Copyright (C) 2007 Philippe Gerum.
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, Inc., 675 Mass Ave, Cambridge MA 02139,
 *   USA; either version 2 of the License, or (at your option) any later
 *   version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef __X86_IPIPE_H
#define __X86_IPIPE_H

#ifdef CONFIG_IPIPE

#define IPIPE_CORE_RELEASE	7

struct ipipe_domain;
struct pt_regs;

struct ipipe_arch_sysinfo {
};

#define ipipe_processor_id()	raw_smp_processor_id()

#define ipipe_mm_switch_protect(flags)		\
	do { (flags) = hard_cond_local_irq_save(); } while (0)
#define ipipe_mm_switch_unprotect(flags)	\
	hard_cond_local_irq_restore(flags)

/* Private interface -- Internal use only */

#define __ipipe_early_core_setup()	do { } while(0)

#define __ipipe_enable_irq(irq)		irq_to_desc(irq)->chip->enable(irq)
#define __ipipe_disable_irq(irq)	irq_to_desc(irq)->chip->disable(irq)
#define __ipipe_enable_irqdesc(ipd, irq)	do { } while(0)
#define __ipipe_disable_irqdesc(ipd, irq)	do { } while(0)

#ifdef CONFIG_SMP
void __ipipe_hook_critical_ipi(struct ipipe_domain *ipd);
#else
#define __ipipe_hook_critical_ipi(ipd) do { } while(0)
#endif

void __ipipe_enable_pipeline(void);

int __ipipe_trap_prologue(struct pt_regs *regs, int trapnr,
			  unsigned long *flags);

#define IPIPE_DO_TRAP(__handler, __trapnr, __regs, __args...)			\
	({									\
		unsigned long __flags;						\
		int __ret = __ipipe_trap_prologue(__regs, __trapnr, &__flags);	\
		if (__ret <= 0) {						\
			__handler(__regs, ##__args);				\
			if (__ret == 0)						\
				ipipe_restore_root_nosync(__flags);		\
		}								\
		__ret > 0;							\
	})

#define __ipipe_root_tick_p(regs)	((regs)->flags & X86_EFLAGS_IF)

static inline void ipipe_mute_pic(void) { }

static inline void ipipe_unmute_pic(void) { }

static inline void ipipe_notify_root_preemption(void)
{
	__ipipe_notify_vm_preemption();
}

#else /* !CONFIG_IPIPE */

#define ipipe_mm_switch_protect(flags)		do { (void)(flags); } while(0)
#define ipipe_mm_switch_unprotect(flags)	do { (void)(flags); } while(0)

#define IPIPE_DO_TRAP(__handler, __trapnr, __regs, __args...)	\
	({							\
		__handler(__regs, ##__args);			\
		0;						\
	})

#endif /* CONFIG_IPIPE */

#if defined(CONFIG_SMP) && defined(CONFIG_IPIPE)
#define __ipipe_move_root_irq(irq)					\
	do {								\
		if (irq < NR_IRQS) {					\
			struct irq_desc *desc = irq_to_desc(irq);	\
			struct irq_chip *chip = desc->irq_data.chip;	\
			if (chip->irq_move)				\
				chip->irq_move(&desc->irq_data);	\
		}							\
	} while (0)
#else /* !(CONFIG_SMP && CONFIG_IPIPE) */
#define __ipipe_move_root_irq(irq)	do { } while (0)
#endif /* !(CONFIG_SMP && CONFIG_IPIPE) */

#endif	/* !__X86_IPIPE_H */
