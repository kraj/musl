#ifndef _STATIC_DLOPEN_H
#define _STATIC_DLOPEN_H

struct __dl_static_sym {
	Sym sym;
	const char *name;
};

hidden extern const struct __dl_static_sym __dl_static_syms[];

#endif
