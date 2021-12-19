use std::env;
use std::net::SocketAddr;
use std::net::ToSocketAddrs;

fn main() {
    if env::args().len() < 2 {
        eprintln!(
            "Usage {} hostname [hostname...]",
            env::args().next().unwrap()
        );
        return;
    }

    for h in env::args().skip(1) {
        println!("{}", h);

        match format!("{}:443", h).to_socket_addrs() {
            Ok(socket_addrs) => {
                // For some reason the IPv6 addresses are not showing up. For example, when a
                // resolution is made for "www.example.com", getaddrinfo() returns the IPv6 version
                // of the IP but does not in this case.
                for sa in socket_addrs {
                    let addr_type: &str = match sa {
                        SocketAddr::V4(_) => "IPv4",
                        SocketAddr::V6(_) => "IPv6",
                    };

                    println!("\t{}: {}", addr_type, sa.ip());
                }
            }
            Err(e) => {
                eprintln!("Failed to resolve address {}, err: {}", h, e);
            }
        }
    }
}
