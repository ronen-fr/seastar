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
/*
 * Copyright (C) 2019 Red Hat, Inc.
 */ 
/*! \file
  \brief unix-domain address structures, to be used for creating socket_address-es for unix-domain
         sockets.

  Note that the path in a unix-domain address may start with a null character.
*/

#include <ostream>
#include <functional>
#include <seastar/net/socket_defs.hh>
#include <cassert>

namespace seastar {

socket_address::socket_address(const unix_domain_addr& s) {
    u.un.sun_family = AF_UNIX;
    memset(u.un.sun_path, '\0', sizeof(u.un.sun_path));
    auto path_length = std::min((int)sizeof(u.un.sun_path), s.path_length());
    memcpy(u.un.sun_path, s.path_bytes(), path_length);
    addr_length = path_length + ((size_t) (((struct ::sockaddr_un *) 0)->sun_path));
}

std::ostream& operator<<(std::ostream& os, const unix_domain_addr& addr) {
    if (addr.path_length() == 0) {
        return os << "{unnamed}";
    }
    if (addr.name[0]) {
        // regular (filesystem-namespace) path
        return os << addr.name;
    }

    os << '@';
    const char* src = addr.path_bytes() + 1;

    for (auto k = addr.path_length(); --k > 0; src++) {
        os << (std::isprint(*src) ? *src : '_');
    }
    return os;
}

std::string printable_ud_addr(const socket_address& sa) {
    using namespace std::string_literals;
    assert(sa.u.un.sun_family == AF_UNIX);

    if (sa.length() <= ((size_t) (((struct ::sockaddr_un *) 0)->sun_path))) {
        return "{unnamed}"s;
    }
    if (sa.u.un.sun_path[0]) {
        // regular (filesystem-namespace) path
        return std::string{sa.u.un.sun_path};
    }

    const size_t  path_length{sa.length() - ((size_t) (((struct ::sockaddr_un *) 0)->sun_path))};
    char ud_path[1 + path_length];
    char* targ = ud_path;
    *targ++ = '@';
    const char* src = sa.u.un.sun_path + 1;
    int k = (int)path_length;

    for (; --k > 0; src++) {
        *targ++ = std::isprint(*src) ? *src : '_';
    }
    return std::string{ud_path, path_length};
}

} // namespace seastar

size_t std::hash<seastar::unix_domain_addr>::operator()(const seastar::unix_domain_addr& a) const {
    return std::hash<std::string>()(a.name);
}
