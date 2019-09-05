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
 * Copyright 2015 Cloudius Systems
 */

#include <seastar/net/stack.hh>
#include <seastar/net/inet_address.hh>
#include <seastar/core/reactor.hh>

namespace seastar {

net::udp_channel::udp_channel()
{}

net::udp_channel::udp_channel(std::unique_ptr<udp_channel_impl> impl) : _impl(std::move(impl))
{}

net::udp_channel::~udp_channel()
{}

net::udp_channel::udp_channel(udp_channel&&) = default;
net::udp_channel& net::udp_channel::operator=(udp_channel&&) = default;

socket_address net::udp_channel::local_address() const {
    return _impl->local_address();
}

future<net::udp_datagram> net::udp_channel::receive() {
    return _impl->receive();
}

future<> net::udp_channel::send(const socket_address& dst, const char* msg) {
    return _impl->send(dst, msg);
}

future<> net::udp_channel::send(const socket_address& dst, packet p) {
    return _impl->send(dst, std::move(p));
}

bool net::udp_channel::is_closed() const {
    return _impl->is_closed();
}

void net::udp_channel::shutdown_input() {
    _impl->shutdown_input();
}

void net::udp_channel::shutdown_output() {
    _impl->shutdown_output();
}


void net::udp_channel::close() {
    return _impl->close();
}

connected_socket::connected_socket()
{}

connected_socket::connected_socket(
        std::unique_ptr<net::connected_socket_impl> csi)
        : _csi(std::move(csi)) {
}

connected_socket::connected_socket(connected_socket&& cs) noexcept = default;
connected_socket& connected_socket::operator=(connected_socket&& cs) noexcept = default;

connected_socket::~connected_socket()
{}

input_stream<char> connected_socket::input() {
    return input_stream<char>(_csi->source());
}

output_stream<char> connected_socket::output(size_t buffer_size) {
    // TODO: allow user to determine buffer size etc
    return output_stream<char>(_csi->sink(), buffer_size, false, true);
}

void connected_socket::set_nodelay(bool nodelay) {
    _csi->set_nodelay(nodelay);
}

bool connected_socket::get_nodelay() const {
    return _csi->get_nodelay();
}
void connected_socket::set_keepalive(bool keepalive) {
    _csi->set_keepalive(keepalive);
}
bool connected_socket::get_keepalive() const {
    return _csi->get_keepalive();
}
void connected_socket::set_keepalive_parameters(const net::keepalive_params& p) {
    _csi->set_keepalive_parameters(p);
}
net::keepalive_params connected_socket::get_keepalive_parameters() const {
    return _csi->get_keepalive_parameters();
}

void connected_socket::shutdown_output() {
    _csi->shutdown_output();
}

void connected_socket::shutdown_input() {
    _csi->shutdown_input();
}

socket::~socket()
{}

socket::socket(
        std::unique_ptr<net::socket_impl> si)
        : _si(std::move(si)) {
}

socket::socket(socket&&) noexcept = default;
socket& socket::operator=(socket&&) noexcept = default;

future<connected_socket> socket::connect(socket_address sa, socket_address local, transport proto) {
    return _si->connect(sa, local, proto);
}

void socket::shutdown() {
    _si->shutdown();
}

server_socket::server_socket() {
}

server_socket::server_socket(std::unique_ptr<net::server_socket_impl> ssi)
        : _ssi(std::move(ssi)) {
}
server_socket::server_socket(server_socket&& ss) noexcept = default;
server_socket& server_socket::operator=(server_socket&& cs) noexcept = default;

server_socket::~server_socket() {
}

future<connected_socket, socket_address> server_socket::accept() {
    if (_aborted) {
        return make_exception_future<connected_socket, socket_address>(std::system_error(ECONNABORTED, std::system_category()));
    }
    return _ssi->accept();
}

void server_socket::abort_accept() {
    _ssi->abort_accept();
    _aborted = true;
}

socket_address server_socket::local_address() const {
    return _ssi->local_address();
}

socket_address::socket_address()
    : socket_address(ipv4_addr())
{}

socket_address::socket_address(uint16_t p)
    : socket_address(ipv4_addr(p))
{}

socket_address::socket_address(ipv4_addr addr)
{
    u.in.sin_family = AF_INET;
    u.in.sin_port = htons(addr.port);
    u.in.sin_addr.s_addr = htonl(addr.ip);
}

socket_address::socket_address(const ipv6_addr& addr)
{
    u.in6.sin6_family = AF_INET6;
    u.in6.sin6_port = htons(addr.port);
    std::copy(addr.ip.begin(), addr.ip.end(), u.in6.sin6_addr.s6_addr);
}

socket_address::socket_address(const unix_domain_addr& addr)
{
    u.un.sun_family = AF_UNIX;
    
    //  Note: I am limiting the socket name to 107, while Linux supports
    //  the non-portable limit of 108 (allowing strings without the terminating
    //  '\0'. 
    strncpy(u.un.sun_path, addr.sfile_.c_str(), sizeof(u.un.sun_path)-1);
    u.un.sun_path[sizeof(u.un.sun_path)-1] = '\0';
}

socket_address::socket_address(uint32_t ipv4, uint16_t p)
    : socket_address(make_ipv4_address(ipv4, p))
{}

bool socket_address::operator==(const socket_address& a) const {
    if (u.sa.sa_family != a.u.sa.sa_family) {
        return false;
    }
    if (u.sa.sa_family == AF_UNIX) {
        // no support yet for abstract-namespace u.d. sockets
        return !strcmp(u.un.sun_path, a.u.un.sun_path);
    }
    if (u.in.sin_port != a.u.in.sin_port) {
        return false;
    }
    switch (u.sa.sa_family) {
    case AF_INET:
        return u.in.sin_addr.s_addr == a.u.in.sin_addr.s_addr;
    case AF_INET6:
        break;
    default:
        return false;
    }

    auto& in1 = as_posix_sockaddr_in6();
    auto& in2 = a.as_posix_sockaddr_in6();

    return IN6_ARE_ADDR_EQUAL(&in1, &in2);
}

future<connected_socket>
network_stack::connect(socket_address sa, socket_address local, transport proto) {
    if (local == socket_address()) {
        local = net::inet_address(sa.addr().in_family());
    }
    return do_with(socket(), [sa, local, proto](::seastar::socket& s) {
        return s.connect(sa, local, proto);
    });
}

}
