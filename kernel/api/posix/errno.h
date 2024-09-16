/*
 * Copyright (c) 2018-2020, Andreas Kling <kling@serenityos.org>
 * Copyright (c) 2021, sin-ack <sin-ack@protonmail.com>
 *
 * SPDX-License-Identifier: BSD-2-Clause
 */

#pragma once

#define ENUMERATE_ERRNO_CODES(X)                                      \
    X(ESUCCESS, "Success (not an error)")                             \
    X(EPERM, "Operation not permitted")                               \
    X(ENOENT, "No such file or directory")                            \
    X(ESRCH, "No such process")                                       \
    X(EINTR, "Interrupted syscall")                                   \
    X(EIO, "I/O error")                                               \
    X(ENXIO, "No such device or address")                             \
    X(E2BIG, "Argument list too long")                                \
    X(ENOEXEC, "Exec format error")                                   \
    X(EBADF, "Bad fd number")                                         \
    X(ECHILD, "No child processes")                                   \
    X(EAGAIN, "Try again")                                            \
    X(ENOMEM, "Out of memory")                                        \
    X(EACCES, "Permission denied")                                    \
    X(EFAULT, "Bad address")                                          \
    X(ENOTBLK, "Block device required")                               \
    X(EBUSY, "Device or resource busy")                               \
    X(EEXIST, "File already exists")                                  \
    X(EXDEV, "Cross-device link")                                     \
    X(ENODEV, "No such device")                                       \
    X(ENOTDIR, "Not a directory")                                     \
    X(EISDIR, "Is a directory")                                       \
    X(EINVAL, "Invalid argument")                                     \
    X(ENFILE, "File table overflow")                                  \
    X(EMFILE, "Too many open files")                                  \
    X(ENOTTY, "Not a TTY")                                            \
    X(ETXTBSY, "Text file busy")                                      \
    X(EFBIG, "File too large")                                        \
    X(ENOSPC, "No space left on device")                              \
    X(ESPIPE, "Illegal seek")                                         \
    X(EROFS, "Read-only filesystem")                                  \
    X(EMLINK, "Too many links")                                       \
    X(EPIPE, "Broken pipe")                                           \
    X(ERANGE, "Range error")                                          \
    X(ENAMETOOLONG, "Name too long")                                  \
    X(ELOOP, "Too many symlinks")                                     \
    X(EOVERFLOW, "Overflow")                                          \
    X(EOPNOTSUPP, "Operation not supported")                          \
    X(ENOSYS, "No such syscall")                                      \
    X(ENOTIMPL, "Not implemented")                                    \
    X(EAFNOSUPPORT, "Address family not supported")                   \
    X(ENOTSOCK, "Not a socket")                                       \
    X(EADDRINUSE, "Address in use")                                   \
    X(ENOTEMPTY, "Directory not empty")                               \
    X(EDOM, "Math argument out of domain")                            \
    X(ECONNREFUSED, "Connection refused")                             \
    X(EHOSTDOWN, "Host is down")                                      \
    X(EADDRNOTAVAIL, "Address not available")                         \
    X(EISCONN, "Already connected")                                   \
    X(ECONNABORTED, "Connection aborted")                             \
    X(EALREADY, "Connection already in progress")                     \
    X(ECONNRESET, "Connection reset")                                 \
    X(EDESTADDRREQ, "Destination address required")                   \
    X(EHOSTUNREACH, "Host unreachable")                               \
    X(EILSEQ, "Illegal byte sequence")                                \
    X(EMSGSIZE, "Message size")                                       \
    X(ENETDOWN, "Network down")                                       \
    X(ENETUNREACH, "Network unreachable")                             \
    X(ENETRESET, "Network reset")                                     \
    X(ENOBUFS, "No buffer space")                                     \
    X(ENOLCK, "No lock available")                                    \
    X(ENOMSG, "No message")                                           \
    X(ENOPROTOOPT, "No protocol option")                              \
    X(ENOTCONN, "Not connected")                                      \
    X(ESHUTDOWN, "Transport endpoint has shutdown")                   \
    X(ETOOMANYREFS, "Too many references")                            \
    X(ESOCKTNOSUPPORT, "Socket type not supported")                   \
    X(EPROTONOSUPPORT, "Protocol not supported")                      \
    X(EDEADLK, "Resource deadlock would occur")                       \
    X(ETIMEDOUT, "Timed out")                                         \
    X(EPROTOTYPE, "Wrong protocol type")                              \
    X(EINPROGRESS, "Operation in progress")                           \
    X(ENOTHREAD, "No such thread")                                    \
    X(EPROTO, "Protocol error")                                       \
    X(ENOTSUP, "Not supported")                                       \
    X(EPFNOSUPPORT, "Protocol family not supported")                  \
    X(EDIRINTOSELF, "Cannot make directory a subdirectory of itself") \
    X(EDQUOT, "Quota exceeded")                                       \
    X(ENOTRECOVERABLE, "State not recoverable")                       \
    X(ECANCELED, "Operation cancelled")                               \
    X(EPROMISEVIOLATION, "The process has a promise violation")       \
    X(ESTALE, "Stale network file handle")                            \
    X(ESRCNOTFOUND, "System resource not found")                      \
    X(EMAXERRNO, "The highest errno +1 :^)")

enum ErrnoCode {
#define __ENUMERATE_ERRNO_CODE(c, s) c,
    ENUMERATE_ERRNO_CODES(__ENUMERATE_ERRNO_CODE)
#undef __ENUMERATE_ERRNO_CODE
};
