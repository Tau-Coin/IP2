#!/usr/bin/env python3
# vim: tabstop=8 expandtab shiftwidth=4 softtabstop=4

import sys

# to use this script, first run 'sample' to sample your ip2 based process
# the output can then be passed to this script to auto-fold call stacks at
# relevant depths and to filter out low sample counts
f = open(sys.argv[1])


def parse_line(line):
    indentation = 0
    while indentation < len(line) and line[indentation] == ' ':
        indentation += 1
    if indentation == 0:
        return (0, 0, '')

    line = line.strip().split(' ')
    samples = int(line[0])
    fun = ' '.join(line[1:])

    return (indentation, samples, fun)


fold = -1

try:
    sample_limit = int(sys.argv[2])
except Exception:
    sample_limit = 5

fun_samples = {}

for line in f:
    if 'Sort by top of stack' in line:
        break

    indentation, samples, fun = parse_line(line)
    if samples < sample_limit:
        continue
    if fold != -1 and indentation > fold:
        continue
    fold = -1

    if '__gnu_cxx::__normal_iterator<' in fun:
        fold = indentation - 1
        continue

    if 'boost::_bi::bind_t' in fun:
        continue
    if 'boost::_bi::list' in fun:
        continue
    if 'boost::_mfi::mf' in fun:
        continue
    if 'boost::_bi::storage' in fun:
        continue

# should only add leaves
    if fun in fun_samples:
        fun_samples[fun] += samples
    else:
        fun_samples[fun] = samples

    output = '%s%-4d %s' % (' ' * (indentation / 2), samples, fun)
    if len(output) > 200:
        output = output[0:200]
    print(output)

    if 'invariant_checker_impl' in fun:
        fold = indentation
    if 'free_multiple_buffers' in fun:
        fold = indentation
    if 'ip2::condition::wait' in fun:
        fold = indentation
    if 'allocate_buffer' in fun:
        fold = indentation
    if '::find_POD' in fun:
        fold = indentation
    if 'SHA1_Update' in fun:
        fold = indentation
    if 'boost::detail::function::basic_vtable' in fun:
        fold = indentation
    if 'operator new' in fun:
        fold = indentation
    if 'malloc' == fun:
        fold = indentation
    if 'free' == fun:
        fold = indentation
    if 'std::_Rb_tree' in fun:
        fold = indentation
    if 'pthread_cond_wait' in fun:
        fold = indentation
    if 'mp_exptmod' == fun:
        fold = indentation
    if '::check_invariant()' in fun:
        fold = indentation
    if 'ip2::condition::wait' in fun:
        fold = indentation
    if '_sigtramp' in fun:
        fold = indentation
    if 'time_now_hires' in fun:
        fold = indentation
    if 'ip2::sleep' in fun:
        fold = indentation
    if 'puts' == fun:
        fold = indentation
    if 'boost::asio::basic_stream_socket' in fun:
        fold = indentation
    if 'recvmsg' == fun:
        fold = indentation
    if 'sendmsg' == fun:
        fold = indentation
    if 'semaphore_signal_trap' == fun:
        fold = indentation
    if 'boost::detail::atomic_count::operator' in fun:
        fold = indentation
    if 'pthread_mutex_lock' == fun:
        fold = indentation
    if 'pthread_mutex_unlock' == fun:
        fold = indentation
    if '>::~vector()' == fun:
        fold = indentation
    if 'szone_free_definite_size' == fun:
        fold = indentation
    if 'snprintf' == fun:
        fold = indentation
    if 'usleep' == fun:
        fold = indentation
    if 'pthread_mutex_lock' == fun:
        fold = indentation
    if 'pthread_mutex_unlock' == fun:
        fold = indentation
    if 'std::string::append' in fun:
        fold = indentation
    if 'getipnodebyname' == fun:
        fold = indentation
    if '__gnu_debug::_Safe_iterator<std::' in fun:
        fold = indentation
    if 'fflush' == fun:
        fold = indentation
    if 'vfprintf' == fun:
        fold = indentation
    if 'fprintf' == fun:
        fold = indentation
    if 'BN_mod_exp' == fun:
        fold = indentation
    if 'BN_CTX_free' == fun:
        fold = indentation
    if 'cerror' == fun:
        fold = indentation
    if '0xffffffff' == fun:
        fold = indentation

list = []
for k in fun_samples:
    list.append((fun_samples[k], k))

list = sorted(list, reverse=True)

for i in list:
    print('%-4d %s' % (i[0], i[1]))
