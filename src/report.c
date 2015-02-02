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
#include <stdio.h>
#include "criterion.h"
#include "report.h"

#define IMPL_CALL_REPORT_HOOKS(Kind)                        \
    static f_report_hook * const g_##Kind##_section_start = \
        &__start_criterion_hooks_##Kind;                    \
    static f_report_hook * const g_##Kind##_section_end =   \
        &__stop_criterion_hooks_##Kind;                     \
    void call_report_hooks_##Kind(void *data) {             \
        for (f_report_hook *hook = g_##Kind##_section_start;\
             hook < g_##Kind##_section_end;                 \
             ++hook) {                                      \
            (*hook)(data);                                  \
        }                                                   \
    }

IMPL_CALL_REPORT_HOOKS(PRE_EVERYTHING);
IMPL_CALL_REPORT_HOOKS(PRE_INIT);
IMPL_CALL_REPORT_HOOKS(PRE_TEST);
IMPL_CALL_REPORT_HOOKS(POST_TEST);
IMPL_CALL_REPORT_HOOKS(POST_FINI);
IMPL_CALL_REPORT_HOOKS(POST_EVERYTHING);

ReportHook(PRE_INIT) {
    struct criterion_test *test = data;

    fprintf(stderr, "%s::%s: RUNNING\n", test->category, test->name);
}

ReportHook(POST_TEST) {
    struct criterion_test_stats *stats = data;

    int success = stats->failed == 0;

    fprintf(stderr, "%s::%s: %s\n", stats->test->category, stats->test->name, success ? "SUCCESS" : "FAILURE");
}

ReportHook(PRE_TEST) {}
ReportHook(POST_FINI) {}

ReportHook(PRE_EVERYTHING) {}
ReportHook(POST_EVERYTHING) {}