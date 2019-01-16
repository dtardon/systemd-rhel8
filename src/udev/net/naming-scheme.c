/* SPDX-License-Identifier: LGPL-2.1+ */
#include "alloc-util.h"
#include "naming-scheme.h"
#include "proc-cmdline.h"
#include "string-util.h"

static const NamingScheme naming_schemes[] = {
        { "v238", NAMING_V238 },
        { "v239", NAMING_V239 },
        { "rhel-8.0", NAMING_RHEL_8_0 },
        { "rhel-8.1", NAMING_RHEL_8_1 },
        { "rhel-8.2", NAMING_RHEL_8_2 },
        { "rhel-8.3", NAMING_RHEL_8_3 },
        { "rhel-8.4", NAMING_RHEL_8_4 },
        { "rhel-8.5", NAMING_RHEL_8_5 },
        { "rhel-8.6", NAMING_RHEL_8_6 },
        { "rhel-8.7", NAMING_RHEL_8_7 },
        /* … add more schemes here, as the logic to name devices is updated … */
};

static const NamingScheme* naming_scheme_from_name(const char *name) {
        size_t i;

        if (streq(name, "latest"))
                return naming_schemes + ELEMENTSOF(naming_schemes) - 1;

        for (i = 0; i < ELEMENTSOF(naming_schemes); i++)
                if (streq(naming_schemes[i].name, name))
                        return naming_schemes + i;

        return NULL;
}

const NamingScheme* naming_scheme(void) {
        static const NamingScheme *cache = NULL;
        _cleanup_free_ char *buffer = NULL;
        const char *e, *k;

        if (cache)
                return cache;

        /* Acquire setting from the kernel command line */
        (void) proc_cmdline_get_key("net.naming-scheme", 0, &buffer);

        /* Also acquire it from an env var */
        e = getenv("NET_NAMING_SCHEME");
        if (e) {
                if (*e == ':') {
                        /* If prefixed with ':' the kernel cmdline takes precedence */
                        k = buffer ?: e + 1;
                } else
                        k = e; /* Otherwise the env var takes precedence */
        } else
                k = buffer;

        if (k) {
                cache = naming_scheme_from_name(k);
                if (cache) {
                        log_info("Using interface naming scheme '%s'.", cache->name);
                        return cache;
                }

                log_warning("Unknown interface naming scheme '%s' requested, ignoring.", k);
        }

        cache = naming_scheme_from_name(DEFAULT_NET_NAMING_SCHEME);
        assert(cache);
        log_info("Using default interface naming scheme '%s'.", cache->name);

        return cache;
}
