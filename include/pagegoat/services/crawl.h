#ifndef PAGEGOAT_CRAWL_H
#define PAGEGOAT_CRAWL_H

#include "pagegoat/coregoat/context.h"
#include "pagegoat/coregoat/status.h"

/*
 * Extract contact information from the homepage and up to nine
 * prioritized same-origin pages discovered in its links.
 */
PGStatus pg_crawl_contact_pages(PGContext *ctx);

#endif
