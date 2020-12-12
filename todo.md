# TODO

## Defines

Cleanup all the defines so they are easier to find. I'd like all related defines to have the same prefix.
e.g. CONFIG_GLOBAL_TABLE_SIZE, CONFIG_STRING_TABLE_SIZE. Types and functions should follow a similar pattern.

## More Headers

Split up the mega headers into smaller pieces. We can keep a single header entry point but each module should
be a distinct header and implementation pair.

## Abstraction

Create a types.h that puts all the typedefs in one place. This will make platform porting easier so 
stray defs don't cause surprises.

## Testing

As mentioned in the readme, we need the tests ported to gtest before we make any major architecture changes.