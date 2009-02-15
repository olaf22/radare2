/* radare - LGPL - Copyright 2008-2009 pancake<nopcode.org> */

#include "r_search.h"
#include <regex.h>

int r_search_regexp_update(struct r_search_t *s, u64 from, const u8 *buf, int len)
{
	struct list_head *pos;
	char *buffer = malloc(len+1);
	char *skipz, *end;
	int i, count = 0;

	memcpy(buffer, buf, len);
	buffer[len]='\0';

	list_for_each_prev(pos, &s->kws) {
		struct r_search_kw_t *kw = list_entry(pos, struct r_search_kw_t, list);
		int reflags = REG_EXTENDED;
		int delta = 0;
		regmatch_t matches[10];
		regex_t compiled;

		if (strchr(kw->binmask, 'i'))
			reflags |= REG_ICASE;

		regcomp(&compiled, kw->keyword, reflags);
		foo:
		while (!regexec(&compiled, buffer+delta, 1, &matches, 0)) {
			if (s->callback)
				s->callback(kw, s->user, (u64)from+matches[0].rm_so+delta);
			else printf("hit%d_%d 0x%08llx ; %s\n",
				count, kw->count, (u64)from+matches[0].rm_so,
				buf+matches[0].rm_so+delta);
			delta += matches[0].rm_so+1;
			kw->count++;
			count++;
		}

		/* TODO: check if skip 0 works */
		skipz = strchr(buffer, '\0');
		end = buffer+len;
		if (skipz && skipz+1 < end) {
			for(;!*skipz&&end;skipz=skipz+1);
			delta = skipz-buffer;
			goto foo;
		}
	}
	return count;
}
