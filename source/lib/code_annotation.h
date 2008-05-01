/**
 * =========================================================================
 * File        : code_annotation.h
 * Project     : 0 A.D.
 * Description : macros for code annotation.
 * =========================================================================
 */

// license: GPL; see lib/license.txt

#ifndef INCLUDED_CODE_ANNOTATION
#define INCLUDED_CODE_ANNOTATION

/**
 * mark a function local variable or parameter as unused and avoid
 * the corresponding compiler warning.
 * use inside the function body, e.g. void f(int x) { UNUSED2(x); }
 **/
#define UNUSED2(param) (void)param;

/**
 * mark a function parameter as unused and avoid
 * the corresponding compiler warning.
 * wrap around the parameter name, e.g. void f(int UNUSED(x))
 **/
#define UNUSED(param)


/**
"unreachable code" helpers

unreachable lines of code are often the source or symptom of subtle bugs.
they are flagged by compiler warnings; however, the opposite problem -
erroneously reaching certain spots (e.g. due to missing return statement)
is worse and not detected automatically.

to defend against this, the programmer can annotate their code to
indicate to humans that a particular spot should never be reached.
however, that isn't much help; better is a sentinel that raises an
error if if it is actually reached. hence, the UNREACHABLE macro.

ironically, if the code guarded by UNREACHABLE works as it should,
compilers may flag the macro's code as unreachable. this would
distract from genuine warnings, which is unacceptable.

even worse, compilers differ in their code checking: GCC only complains if
non-void functions end without returning a value (i.e. missing return
statement), while VC checks if lines are unreachable (e.g. if they are
preceded by a return on all paths).

our implementation of UNREACHABLE solves this dilemna as follows:
- on GCC: call abort(); since it has the noreturn attributes, the
  "non-void" warning disappears.
- on VC: avoid generating any code. we allow the compiler to assume the
  spot is actually unreachable, which incidentally helps optimization.
  if reached after all, a crash usually results. in that case, compile with
  CONFIG_PARANOIA, which will cause an error message to be displayed.

this approach still allows for the possiblity of automated
checking, but does not cause any compiler warnings.
**/
#define UNREACHABLE	// actually defined below.. this is for
# undef UNREACHABLE	// CppDoc's benefit only.

// 1) final build: optimize assuming this location cannot be reached.
//    may crash if that turns out to be untrue, but removes checking overhead.
#if CONFIG_FINAL
# define UNREACHABLE ASSUME_UNREACHABLE
// 2) normal build:
#else
//    a) normal implementation: includes "abort", which is declared with
//       noreturn attribute and therefore avoids GCC's "execution reaches
//       end of non-void function" warning.
# if !MSC_VERSION || ICC_VERSION || CONFIG_PARANOIA
#  define UNREACHABLE\
	STMT(\
		debug_assert(0);	/* hit supposedly unreachable code */\
		abort();\
	)
//    b) VC only: don't generate any code; squelch the warning and optimize.
# else
#  define UNREACHABLE ASSUME_UNREACHABLE
# endif
#endif

/**
convenient specialization of UNREACHABLE for switch statements whose
default can never be reached. example usage:
int x;
switch(x % 2)
{
	case 0: break;
	case 1: break;
	NODEFAULT;
}
**/
#define NODEFAULT default: UNREACHABLE


/**
 * equivalent to strcpy, but indicates that the programmer checked usage and
 * promises it is safe.
 *
 * (this macro prevents actually-safe instances of the function from
 * showing up in searches)
 **/
#define SAFE_STRCPY str##cpy
#define SAFE_WCSCPY wcs##cpy


// generate a symbol containing the line number of the macro invocation.
// used to give a unique name (per file) to types made by cassert.
// we can't prepend __FILE__ to make it globally unique - the filename
// may be enclosed in quotes. PASTE3_HIDDEN__ is needed to make sure
// __LINE__ is expanded correctly.
#define PASTE3_HIDDEN__(a, b, c) a ## b ## c
#define PASTE3__(a, b, c) PASTE3_HIDDEN__(a, b, c)
#define UID__  PASTE3__(LINE_, __LINE__, _)
#define UID2__ PASTE3__(LINE_, __LINE__, _2)

/**
 * compile-time debug_assert. causes a compile error if the expression
 * evaluates to zero/false.
 *
 * no runtime overhead; may be used anywhere, including file scope.
 * especially useful for testing sizeof types.
 *
 * @param expression that is expected to evaluate to non-zero at compile-time.
 **/
#define cassert(expr) typedef detail::static_assert<(expr)>::type UID__;
namespace detail
{
	template<bool> struct static_assert;
	template<> struct static_assert<true>
	{
		typedef int type;
	};
}

/**
 * compile-time debug_assert. causes a compile error if the expression
 * evaluates to zero/false.
 *
 * no runtime overhead; may be used anywhere, including file scope.
 * especially useful for testing sizeof types.
 *
 * this version has a less helpful error message, but redefinition doesn't
 * trigger warnings.
 *
 * @param expression that is expected to evaluate to non-zero at compile-time.
 **/
#define cassert2(expr) extern char CASSERT_FAILURE[1][(expr)]


// copied from boost::noncopyable; this definition avoids warnings when
// an exported class derives from noncopyable.

namespace noncopyable_  // protection from unintended ADL
{
	class LIB_API noncopyable
	{
	protected:
		noncopyable() {}
		~noncopyable() {}
	private:  // emphasize the following members are private
		noncopyable(const noncopyable&);
		const noncopyable& operator=(const noncopyable&);
	};
}

typedef noncopyable_::noncopyable noncopyable;

#endif	// #ifndef INCLUDED_CODE_ANNOTATION
