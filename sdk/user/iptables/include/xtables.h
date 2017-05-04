#define true	1
#define false	0

extern struct in6_addr *xtables_numeric_to_ip6addr(const char *);
extern const char *xtables_ip6addr_to_numeric(const struct in6_addr *);

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof(*(x)))
#endif

enum xtables_exittype {
	//OTHER_PROBLEM = 1,
	//PARAMETER_PROBLEM,
	//VERSION_PROBLEM,
	RESOURCE_PROBLEM = 4,
	XTF_ONLY_ONCE,
	XTF_NO_INVERT,
	XTF_BAD_VALUE,
	XTF_ONE_ACTION,
};
