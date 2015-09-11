/*
 * The MIT License (MIT)
 *
 * Copyright © 2015 Franklin "Snaipe" Mathieu <http://snai.pe/>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#ifndef CRITERION_REDIRECT_H_
# define CRITERION_REDIRECT_H_

# include "common.h"
# include "assert.h"

# ifdef __cplusplus
#  include <cstdio>
# else
#  include <stdio.h>
# endif

CR_BEGIN_C_API

CR_API void cr_redirect_stdout(void);
CR_API void cr_redirect_stderr(void);
CR_API void cr_redirect_stdin(void);

CR_API CR_STDN FILE* cr_get_redirected_stdout(void);
CR_API CR_STDN FILE* cr_get_redirected_stderr(void);
CR_API CR_STDN FILE* cr_get_redirected_stdin(void);

CR_API int cr_file_match_str(CR_STDN FILE* f, const char *str);

CR_END_C_API

# define cr_assert_redir_op_(Fail, Fun, Op, File, Str, ...)     \
    CR_EXPAND(cr_assert_impl(                                   \
            Fail,                                               \
            !(Fun((File), (Str)) Op 0),                         \
            dummy,                                              \
            CRITERION_ASSERT_MSG_FILE_STR_MATCH,                \
            (CR_STR(File), Str),                                \
            __VA_ARGS__                                         \
    ))

# define cr_assert_redir_op_va_(Fail, Fun, Op, ...)             \
    CR_EXPAND(cr_assert_redir_op_(                              \
            Fail,                                               \
            Fun,                                                \
            Op,                                                 \
            CR_VA_HEAD(__VA_ARGS__),                            \
            CR_VA_HEAD(CR_VA_TAIL(__VA_ARGS__)),                \
            CR_VA_TAIL(CR_VA_TAIL(__VA_ARGS__))                 \
    ))

# define cr_assert_file_contents_match_str(...) CR_EXPAND(cr_assert_redir_op_va_(CR_FAIL_ABORT_,     cr_file_match_str, ==, __VA_ARGS__))
# define cr_expect_file_contents_match_str(...) CR_EXPAND(cr_assert_redir_op_va_(CR_FAIL_CONTINUES_, cr_file_match_str, ==, __VA_ARGS__))

# define cr_assert_file_contents_not_match_str(...) CR_EXPAND(cr_assert_redir_op_va_(CR_FAIL_ABORT_,     cr_file_match_str, !=, __VA_ARGS__))
# define cr_expect_file_contents_not_match_str(...) CR_EXPAND(cr_assert_redir_op_va_(CR_FAIL_CONTINUES_, cr_file_match_str, !=, __VA_ARGS__))

#endif /* !CRITERION_REDIRECT_H_ */