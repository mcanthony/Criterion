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
#include <csptr/smalloc.h>

#include "criterion/assert.h"
#include "pipe.h"
#include "internal.h"

struct pipe_handle {
#ifdef VANILLA_WIN32
    HANDLE fhs[2];
#else
    int fds[2];
#endif
};

FILE *pipe_in(s_pipe_handle *p, int do_close) {
#ifdef VANILLA_WIN32
    if (do_close)
        CloseHandle(p->fhs[1]);
    int fd = _open_osfhandle((intptr_t) p->fhs[0], _O_RDONLY);
    if (fd == -1)
        return NULL;
    FILE *in = _fdopen(fd, "r");
#else
    if (do_close)
        close(p->fds[1]);
    FILE *in = fdopen(p->fds[0], "r");
#endif
    if (!in)
        return NULL;

    setvbuf(in, NULL, _IONBF, 0);
    return in;
}

FILE *pipe_out(s_pipe_handle *p, int do_close) {
#ifdef VANILLA_WIN32
    if (do_close)
        CloseHandle(p->fhs[0]);
    int fd = _open_osfhandle((intptr_t) p->fhs[1], _O_WRONLY);
    if (fd == -1)
        return NULL;
    FILE *out = _fdopen(fd, "w");
#else
    if (do_close)
        close(p->fds[0]);
    FILE *out = fdopen(p->fds[1], "w");
#endif
    if (!out)
        return NULL;

    setvbuf(out, NULL, _IONBF, 0);
    return out;
}

int stdpipe_stack(s_pipe_handle *out) {
#ifdef VANILLA_WIN32
    HANDLE fhs[2];
    SECURITY_ATTRIBUTES attr = {
        .nLength = sizeof (SECURITY_ATTRIBUTES),
        .bInheritHandle = TRUE
    };
    if (!CreatePipe(fhs, fhs + 1, &attr, 0))
        return -1;
    *out = (s_pipe_handle) {{ fhs[0], fhs[1] }};
#else
    int fds[2] = { -1, -1 };
    if (pipe(fds) == -1)
        return -1;
    *out = (s_pipe_handle) {{ fds[0], fds[1] }};
#endif
    return 0;
}

s_pipe_handle *stdpipe() {
    s_pipe_handle *handle = smalloc(sizeof (s_pipe_handle));
    if (stdpipe_stack(handle) < 0)
        return NULL;
    return handle;
}

int stdpipe_options(s_pipe_handle *handle, int id, int noblock) {
#ifdef VANILLA_WIN32
    HANDLE fhs[2];
    SECURITY_ATTRIBUTES attr = {
        .nLength = sizeof (SECURITY_ATTRIBUTES),
        .bInheritHandle = TRUE
    };
    char pipe_name[256] = {0};
    snprintf(pipe_name, sizeof (pipe_name),
        "\\\\.\\pipe\\criterion_%lu_%d", GetCurrentProcessId(), id);
    fhs[0] = CreateNamedPipe(pipe_name,
            PIPE_ACCESS_INBOUND,
            PIPE_TYPE_BYTE | PIPE_READMODE_BYTE
                           | (noblock ? PIPE_NOWAIT : PIPE_WAIT),
            1,
            4096 * 4,
            4096 * 4,
            0,
            &attr);

    if (fhs[0] == INVALID_HANDLE_VALUE)
        return 0;

    fhs[1] = CreateFile(pipe_name,
            GENERIC_WRITE,
            0,
            &attr,
            OPEN_EXISTING,
            0,
            NULL);

    if (fhs[1] == INVALID_HANDLE_VALUE) {
        CloseHandle(fhs[0]);
        return 0;
    }

    *handle = (s_pipe_handle) {{ fhs[0], fhs[1] }};
#else
    (void) id;

    int fds[2] = { -1, -1 };
    if (pipe(fds) == -1)
        return 0;

    if (noblock)
        for (int i = 0; i < 2; ++i)
            fcntl(fds[i], F_SETFL, fcntl(fds[i], F_GETFL) | O_NONBLOCK);

    *handle = (s_pipe_handle) {{ fds[0], fds[1] }};
#endif
    return 1;
}

void pipe_std_redirect(s_pipe_handle *pipe, enum criterion_std_fd fd) {
    enum pipe_end end = fd == CR_STDIN ? PIPE_READ : PIPE_WRITE;
#ifdef VANILLA_WIN32
    int stdfd = _open_osfhandle((intptr_t) pipe->fhs[end], end == PIPE_READ ? _O_RDONLY : _O_WRONLY);
    if (stdfd == -1)
        cr_assert_fail("Could not redirect standard file descriptor.");

    _close(fd);
    _dup2(stdfd, fd);
    _close(stdfd);

    setvbuf(get_std_file(fd), NULL, _IONBF, 0);

    static int handles[] = {
        [CR_STDIN]  = STD_INPUT_HANDLE,
        [CR_STDOUT] = STD_OUTPUT_HANDLE,
        [CR_STDERR] = STD_ERROR_HANDLE,
    };
    SetStdHandle(handles[fd], pipe->fhs[end]);
#else
    close(fd);
    dup2(pipe->fds[end], fd);
    close(pipe->fds[end]);
#endif
}

static s_pipe_handle stdout_redir_;
static s_pipe_handle stderr_redir_;
static s_pipe_handle stdin_redir_;

s_pipe_handle *stdout_redir = &stdout_redir_;
s_pipe_handle *stderr_redir = &stderr_redir_;
s_pipe_handle *stdin_redir  = &stdin_redir_;