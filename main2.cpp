#include <seastar/core/app-template.hh>
#include <seastar/core/reactor.hh>
#include <iostream>

#define CHUNK_SIZE 65000

static std::vector<char> buffer(CHUNK_SIZE, 'a');
static int socket_fd;
static struct sockaddr_in server_address;

seastar::future<> service_loop();

seastar::future<> f() {
    return seastar::parallel_for_each(boost::irange<unsigned>(0, seastar::smp::count),
                                      [] (unsigned c) {
                                            std::cout << "shards " << c << std::endl;
                                            return seastar::smp::submit_to(c, service_loop);
                                      });
}

int main(int argc, char** argv) {
    seastar::app_template app;
    app.run(argc, argv, f);
}

seastar::future<> service_loop() {
    buffer[CHUNK_SIZE - 1] = '\0';

    for(int i = 0; i < 5; i++) {
        (void) seastar::do_with(seastar::make_udp_channel(), [](auto& channel) {
            return seastar::keep_doing([&channel] {
                return channel.send(seastar::make_ipv4_address({2121}), seastar::temporary_buffer<char>(buffer.data(), CHUNK_SIZE));
            });
        });
    }


    return seastar::make_ready_future<>();
}