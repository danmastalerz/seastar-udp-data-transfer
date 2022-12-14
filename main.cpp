#include <seastar/core/app-template.hh>
#include <seastar/core/reactor.hh>
#include <iostream>

static size_t read_bytes = 0;
static size_t read_bytes_now = 0;
static seastar::timer<> timer;

seastar::future<> service_loop();


seastar::future<> f() {
    return seastar::parallel_for_each(boost::irange<unsigned>(0, seastar::smp::count),
                                      [] (unsigned c) {
                                          return seastar::smp::submit_to(c, service_loop);
                                      });
}

int main(int argc, char** argv) {
    seastar::app_template app;
    app.run(argc, argv, f);
}

seastar::future<> service_loop() {
    // Create udp_channel
    auto channel = seastar::make_udp_channel(seastar::make_ipv4_address({2121}));

    // Catch SIGZSTP signal
    seastar::engine().handle_signal(SIGTSTP, [&channel] {
        channel.shutdown_input();
        channel.shutdown_output();
        seastar::engine_exit();
    });

    // Catch seg fault
    seastar::engine().handle_signal(SIGSEGV, [&channel] {
        seastar::engine_exit();
    });

    // Get number of a shard.
    using namespace std::chrono_literals;
    timer.set_callback([] {
        std::cout << "\033[2J\033[1;1H";
        int received_kilobytes = read_bytes / 1024;
        int received_kilobytes_now = read_bytes_now / 1024;
        double received_megabytes = read_bytes / 1024.0 / 1024.0;
        double received_megabytes_now = read_bytes_now / 1024.0 / 1024.0;
        std::cout << "Received " << received_kilobytes << " kilobytes (" << received_megabytes << " megabytes) in total." << std::endl;
        std::cout << "Current downloading speed: " << received_kilobytes_now << " kilobytes (" << received_megabytes_now << " megabytes) per second." << std::endl;
        read_bytes_now = 0;
    });
    timer.arm_periodic(1s);

    // In loop read all the incoming packets
    return seastar::do_with(std::move(channel), [](auto& channel) {
        return seastar::keep_doing([&channel] {
            return channel.receive().then([](seastar::net::udp_datagram dgram) {
                auto fragment_array = dgram.get_data().fragment_array();
                read_bytes += fragment_array->size;
                read_bytes_now += fragment_array->size;
                return seastar::make_ready_future<>();
            });
        });
    });
}