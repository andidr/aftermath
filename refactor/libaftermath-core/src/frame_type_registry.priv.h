/* These definitions have been separated from the rest of the code in order to
 * avoid warnings from clang about unused static inline functions. By moving the
 * definitions into a file that is included in the main source file, clang does
 * not complain anymore.
 *
 * Since the definitions are only used locally by the code in the C file, they
 * should not be public in the header file.
 */

#include <string.h>

#define AM_FRAME_TYPE_REGISTRY_NODE_ACC_NAME(e) ((e).name)
#define AM_FRAME_TYPE_REGISTRY_NODE_CMP_NAME(a, b) strcmp((a), (b))

AM_DECL_TYPED_RBTREE_OPS(am_frame_type_tree,
			 struct am_frame_type_tree,
			 entries_name,
			 struct am_frame_type,
			 node,
			 const char*,
			 AM_FRAME_TYPE_REGISTRY_NODE_ACC_NAME,
			 AM_FRAME_TYPE_REGISTRY_NODE_CMP_NAME)

#define am_frame_type_tree_for_each_safe(ftt, iter, tmp)		\
	for(iter = am_frame_type_tree_first_postorder(ftt),		\
		    tmp = (!iter) ?					\
		          NULL :					\
		          am_frame_type_tree_next_postorder(iter);	\
	    iter;							\
	    iter = tmp,						\
		    tmp = (!tmp) ?					\
		          NULL :					\
		          am_frame_type_tree_next_postorder(tmp))
