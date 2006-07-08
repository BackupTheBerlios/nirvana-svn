#include "WebCoreCache.h"

#include "misc/loader.h"

using khtml::Cache;


void WebCoreCache::statistics()
{
#if 0
    Cache::Statistics s = Cache::getStatistics();
    return [NSArray arrayWithObjects:
        [NSDictionary dictionaryWithObjectsAndKeys:
            [NSNumber numberWithInt:s.images.count], @"images",
            [NSNumber numberWithInt:s.movies.count], @"movies",
            [NSNumber numberWithInt:s.styleSheets.count], @"style sheets",
            [NSNumber numberWithInt:s.scripts.count], @"scripts",
            [NSNumber numberWithInt:s.other.count], @"other",
            nil],
        [NSDictionary dictionaryWithObjectsAndKeys:
            [NSNumber numberWithInt:s.images.size], @"images",
            [NSNumber numberWithInt:s.movies.size], @"movies",
            [NSNumber numberWithInt:s.styleSheets.size] ,@"style sheets",
            [NSNumber numberWithInt:s.scripts.size], @"scripts",
            [NSNumber numberWithInt:s.other.size], @"other",
            nil],
        nil];
#endif
}


void WebCoreCache::empty()
{
    Cache::flushAll();
}

void WebCoreCache::setDisabled(bool disabled)
{
    Cache::setCacheDisabled(disabled);
}
