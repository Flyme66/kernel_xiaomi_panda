#include "selinux.h"
#include "objsec.h"
#include "linux/version.h"
#include "../klog.h" // IWYU pragma: keep

#define KERNEL_SU_DOMAIN "u:r:su:s0"

static u32 ksu_sid;

static int transive_to_domain(const char *domain)
{
	struct cred *cred;
	struct task_security_struct *tsec;
	u32 sid;
	int error;

	cred = (struct cred *)__task_cred(current);

	tsec = cred->security;
	if (!tsec) {
		pr_err("tsec == NULL!\n");
		return -1;
	}

	error = security_secctx_to_secid(domain, strlen(domain), &sid);
	if (error) {
		pr_info("security_secctx_to_secid %s -> sid: %d, error: %d\n", domain, sid, error);
	}
	if (!error) {
		if (!ksu_sid)
			ksu_sid = sid;

		tsec->sid = sid;
		tsec->create_sid = 0;
		tsec->keycreate_sid = 0;
		tsec->sockcreate_sid = 0;
	}
	return error;
}

void setup_selinux(const char *domain)
{
	if (transive_to_domain(domain)) {
		pr_err("transive domain failed.");
		return;
	}

	/* we didn't need this now, we have change selinux rules when boot!
if (!is_domain_permissive) {
  if (set_domain_permissive() == 0) {
      is_domain_permissive = true;
  }
}*/
}

void setenforce(bool enforce)
{
#ifdef CONFIG_SECURITY_SELINUX_DEVELOP
	selinux_state.enforcing = enforce;
#endif
}

bool getenforce()
{
#ifdef CONFIG_SECURITY_SELINUX_DISABLE
	if (selinux_state.disabled) {
		return false;
	}
#endif

#ifdef CONFIG_SECURITY_SELINUX_DEVELOP
	return selinux_state.enforcing;
#else
	return true;
#endif
}

bool is_ksu_domain()
{
	return ksu_sid && current_sid() == ksu_sid;
}
