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
#pragma once

#include <iosfwd>
#include <sys/types.h>
#include <sys/un.h>

namespace seastar {

static constexpr int un_path_size = sizeof(::sockaddr_un::sun_path);

///  the sockaddr_un structure, but with an additional 'length' attribute
///  (required for abstract-namespace support)
struct unix_domain_addr {
    // DO NOT CHANGE THE MEMBERS' ORDER:
    __SOCKADDR_COMMON (local_);
    char local_path[un_path_size];   //!< Path name
    size_t path_byte_count;          //!< Meaningful bytes in 'local_path'. NOT the same as strlen()!

    explicit unix_domain_addr(const std::string& fn);

    explicit unix_domain_addr(const char* fn);

    unix_domain_addr() = default;   // required, as 'socket_addr' is sometimes aggregate-initialized

    bool operator==(const unix_domain_addr& a) const {
        return (path_byte_count == a.path_byte_count) &&
                !memcmp(local_path, a.local_path, path_byte_count);
    }
  
    bool operator!=(const unix_domain_addr& a) const {
        return !(*this == a);
    }
};
} // namespace seastar