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
 * Copyright (C) 2019 Cloudius Systems, Ltd. RRR which copyright to use?
 */

#include <seastar/testing/test_case.hh>
#include <seastar/net/api.hh>
#include <seastar/net/inet_address.hh>
#include <seastar/core/reactor.hh>
#include <seastar/core/thread.hh>
#include <seastar/util/log.hh>
#include <seastar/util/std-compat.hh>

using namespace seastar;
using std::string;
using namespace std::string_literals;

static logger iplog("unix_domain");

class ud_server_client {
public:
    ud_server_client(string server_path, compat::optional<string> client_path, int rounds) :
        server_addr{unix_domain_addr{server_path}}, client_path{client_path}, rounds{rounds},
        rounds_left{rounds} {}

    future<> run();
    ud_server_client(ud_server_client&&) = default;
    ud_server_client(const ud_server_client&) = default;

private:
    const string test_message{"are you still the same?"s};
    future<> init_server();
    future<> client_round();
    //future<> wait_for_data();
    const socket_address server_addr;

    const compat::optional<string> client_path;
    api_v2::server_socket server;
    const int rounds;
    int rounds_left;
};

future<> ud_server_client::init_server() {
    return do_with(engine().listen(server_addr), [this](server_socket& lstn) mutable {
        std::cout << "b4 accept" << std::endl;

        //  start the clients here, where we know the server is listening

        (void)async([this]{
            for (int i=0; i<rounds; ++i) {
                
                std::cout << "c rnd " << i << " left: " << rounds_left << std::endl;
                (void)client_round().get0();
            } 
            std::cout << "cln ------" << std::endl;
        });

        return do_until([this](){return rounds_left<=0;}, [&lstn,this]() {
            return lstn.accept().then([this](accept_result from_accept) {
                connected_socket cn    = std::move(from_accept.connection);
                socket_address cn_addr = std::move(from_accept.remote_address);
                --rounds_left;
                std::cout << "accepted " << rounds_left << " clnt: " << cn_addr << std::endl;
                //  verify the client address
                if (client_path) {
                    std::cout << "Incoming path " << cn_addr << "-" << cn_addr.family() << "-" << cn_addr.length() << std::endl;
                    socket_address tmmp{unix_domain_addr{*client_path}};
                    std::cout << ") vs: " << *client_path << "-" << tmmp.family() << "-" << tmmp.length() << std::endl;
                    BOOST_REQUIRE_EQUAL(cn_addr, socket_address{unix_domain_addr{*client_path}});
                }

                return do_with(std::move(cn.input()), std::move(cn.output()), [](auto& inp, auto& out) {

                    return inp.read().then([&out](auto bb) {
                        string ans = "-"s;
                        if (bb && bb.size()) {
                            ans = "+"s + string{bb.get(), bb.size()};
                        }
                        std::cout << "-> " << ans << std::endl;
                        return out.write(ans).then([&out](){out.flush();}).
                        then([&out](){return out.close();});
                    }).then([&inp]() { return inp.close(); }).
                    then([]() { return make_ready_future<>(); });

                }).then([]{ return make_ready_future<>();});
            });
        });
    });
}

/// Send a message to the server, and expect (almost) the same string back.
/// If 'client_path' is set, the client binds to the named path.
future<> ud_server_client::client_round() {

    std::cerr << "cln " << (client_path? *client_path : "x") << std::endl;
    auto cc = client_path ? 
        engine().net().connect(server_addr, socket_address{unix_domain_addr{*client_path}}).get0() :
        engine().net().connect(server_addr).get0();

    std::cout << "cln connected " << std::endl;
    return do_with(std::move(cc.input()), std::move(cc.output()), [this](auto& inp, auto& out) {

        return out.write(test_message).then(
            [this,&out,&inp](){return out.flush();}).then(
            [this,&out,&inp](){
                std::cout << "b4 read" << std::endl;
                return inp.read();
            }).then(
            [this,&out,&inp](auto bb){
                BOOST_REQUIRE_EQUAL(bb.get(), "+"s+test_message);
                return inp.close();
            }).then([&out](){return out.close();}).then(
            [&out,&inp]{ 
                std::cout << "cln done" << std::endl;
                return make_ready_future<>();
            });
    });

}

future<> ud_server_client::run() {
    //return do_with(this, [this](){
    //    return init_server();
    //});
    return async([this] {
        auto serverfut = init_server();
        (void)serverfut.get();
    });

    
}

//  testing the various address types, both on the server and on the
//  client side

SEASTAR_TEST_CASE(unixdomain_server) {
    system("rm -f /tmp/ry");
    ud_server_client uds("/tmp/ry", compat::nullopt, 3);
    return do_with(std::move(uds),[](auto& uds){
        return uds.run();
    });
    return make_ready_future<>();
}


SEASTAR_TEST_CASE(unixdomain_abs) {
    char sv_name[]{'\0', '1', '1', '1'};
    //ud_server_client uds(string{"\0111",4}, string{"\0112",4}, 1);
    ud_server_client uds(string{sv_name,4}, compat::nullopt, 4);
    return do_with(std::move(uds),[](auto& uds){
        return uds.run();
    });
    //return make_ready_future<>();
}

SEASTAR_TEST_CASE(unixdomain_abs_bind) {
    char sv_name[]{'\0', '1', '1', '1'};
    char cl_name[]{'\0', '1', '1', '2'};
    ud_server_client uds(string{sv_name,4}, string{cl_name,4}, 1);
    return do_with(std::move(uds),[](auto& uds){
        return uds.run();
    });
}

SEASTAR_TEST_CASE(unixdomain_abs_bind_2) {
    char sv_name[]{'\0', '1', '\0', '\12', '1'};
    char cl_name[]{'\0', '1', '\0', '\12', '2'};
    ud_server_client uds(string{sv_name,5}, string{cl_name,5}, 2);
    return do_with(std::move(uds),[](auto& uds){
        return uds.run();
    });
}

SEASTAR_TEST_CASE(unixdomain_bind) {
    system("rm -f 111 112");
    ud_server_client uds("111"s, "112"s, 1);
    return do_with(std::move(uds),[](auto& uds){
        return uds.run();
    });
}

SEASTAR_TEST_CASE(unixdomain_short) {
    system("rm -f 3");
    ud_server_client uds("3"s, compat::nullopt, 10);
    return do_with(std::move(uds),[](auto& uds){
        return uds.run();
    });
}

#if 0

SEASTAR_TEST_CASE(udp_packet_test) {
    if (!check_ipv6_support()) {
        return make_ready_future<>();
    }

    auto sc = engine().net().make_udp_channel(ipv6_addr{"::1"});

    BOOST_REQUIRE_EQUAL(sc.local_address().addr().is_ipv6());

    auto cc = engine().net().make_udp_channel(ipv6_addr{"::1"});

    auto f1 = cc.send(sc.local_address(), "apa");

    return f1.then([cc = std::move(cc), sc = std::move(sc)]() mutable {
        auto src = cc.local_address();
        cc.close();
        auto f2 = sc.receive();

        return f2.then([sc = std::move(sc), src](auto pkt) mutable {
            auto a = sc.local_address();
            sc.close();
            BOOST_REQUIRE_EQUAL(src, pkt.get_src());
            auto dst = pkt.get_dst();
            // Don't always get a dst address.
            if (dst != socket_address()) {
                BOOST_REQUIRE_EQUAL(a, pkt.get_dst());
            }
        });
    });
}

SEASTAR_TEST_CASE(tcp_packet_test) {
    if (!check_ipv6_support()) {
        return make_ready_future<>();
    }

    return async([] {
        auto sc = api_v2::server_socket(engine().net().listen(ipv6_addr{"::1"}, {}));
        auto la = sc.local_address();

        BOOST_REQUIRE(la.addr().is_ipv6());

        auto cc = engine().net().connect(la).get0();
        auto lc = std::move(sc.accept().get0().connection);

        auto strm = cc.output();
        strm.write("los lobos").get();
        strm.flush().get();

        auto in = lc.input();

        using consumption_result_type = typename input_stream<char>::consumption_result_type;
        using stop_consuming_type = typename consumption_result_type::stop_consuming_type;
        using tmp_buf = stop_consuming_type::tmp_buf;

        in.consume([](tmp_buf buf) {
            return make_ready_future<consumption_result_type>(stop_consuming<char>({}));
        }).get();

        strm.close().get();
        in.close().get();
        sc.abort_accept();
    });
}

#endif
