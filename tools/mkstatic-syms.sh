#!/bin/sh

readelf=$1
shift

tmp=${TMPDIR:-/tmp}/musl-static-syms-$$
trap 'rm -f "$tmp"' EXIT HUP INT TERM

LC_ALL=C "$readelf" -sW "$@" | awk '
	($4 == "FUNC" || $4 == "OBJECT" || $4 == "NOTYPE") &&
	($5 == "GLOBAL" || $5 == "WEAK") &&
	($6 == "DEFAULT" || $6 == "PROTECTED") &&
	$7 != "UND" && $7 != "ABS" {
		print $8, $3, $4
	}
' | sort -k1,1 -u > "$tmp"

# The address references in this table make the archive linker retain every
# public libc definition when, and only when, the static loader is selected.
cat <<'EOF'
#include <stddef.h>
#include <stdint.h>
#include "dynlink.h"
#include "static_dlopen.h"

EOF

while read -r name size type; do
	if test "$type" = FUNC; then
		printf 'extern void %s(void);\n' "$name"
	else
		printf 'extern char %s[];\n' "$name"
	fi
done < "$tmp"

cat <<'EOF'

const struct __dl_static_sym __dl_static_syms[] = {
EOF

while read -r name size type; do
	printf '\t{ { .st_info=(STB_GLOBAL<<4)|STT_%s, .st_shndx=SHN_ABS, .st_value=(size_t)%s, .st_size=%s }, "%s" },\n' \
		"$type" "$name" "$size" "$name"
done < "$tmp"

cat <<'EOF'
	{ { 0 }, 0 }
};
EOF
