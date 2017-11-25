#define __INCLUDE_FILE(x) #x
#define INCLUDE_FILE(val) __INCLUDE_FILE(UniqueLast ## val ## Digits)
#define MAKE_INCLUDE_STRING(val) INCLUDE_FILE(val)
(deffacts number
 		  (unique-numbers WIDTH
		   				  values:
#include MAKE_INCLUDE_STRING(WIDTH)
		  ))
