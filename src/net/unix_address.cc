/*
 * This file is open source software, licensed to you under the terms
 * of the Apache License, Version 2.0 (the "License").  See the NOTICE file
 * distributed with this work for additional information regarding copyright
 * ownership.  You may not use this file except in compliance with the License.
 *
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */
/*! \file
  \brief unix-domain address structures, used when overlaying the general seastar::socket_address

  Note that the path in unix-domain address may start with a null character. The actual length of
  the address must be kept as a separate data item.
*/

#include <ostream>
#include <functional>
#include <seastar/net/socket_defs.hh>
#include <cstring>

namespace seastar {

unix_domain_addr::unix_domain_addr(const std::string& fn) : local_family{AF_UNIX} {
    memset(local_path, '\0', sizeof(local_path));
    path_byte_count = std::min(fn.length(), sizeof(local_path)-1);
    fn.copy(local_path, path_byte_count);
}

unix_domain_addr::unix_domain_addr(const char* fn) : local_family{AF_UNIX} {
    memset(local_path, '\0', sizeof(local_path));
    path_byte_count = std::min(strlen(fn), sizeof(local_path)-1);
    memcpy(local_path, fn, path_byte_count);
}

socket_address::socket_address(const unix_domain_addr& s) {
    u.ud.local_family = AF_UNIX;
    memset(u.ud.local_path, '\0', sizeof(u.ud.local_path));
    u.ud.path_byte_count = s.path_byte_count;
    memcpy(u.ud.local_path, s.local_path, sizeof(u.ud.local_path));
}    

std::ostream& operator<<(std::ostream& os, const unix_domain_addr& addr) {
    if (addr.path_byte_count == 0) {
        return os << "{unnamed}";
    }
    if (addr.local_path[0]) {
        // regular (filesystem-namespace) path
        return os << addr.local_path;
    }

    os << '@';
    const char* src = addr.local_path+1;
    auto k = (int)addr.path_byte_count;

    for (; --k > 0; src++)
        os << (std::isprint(*src) ? *src : '_');
    return os;
}

} // namespace seastar

size_t std::hash<seastar::unix_domain_addr>::operator()(const seastar::unix_domain_addr& a) const {
    // note that special treatment is required for abstract-namespace, where 'a.local_path' is
    // definitely *not* a null-terminated string. In C++17: use std::byte here
    std::string keep_length{a.local_path, a.path_byte_count};
    return std::hash<std::string>()(keep_length);
}
