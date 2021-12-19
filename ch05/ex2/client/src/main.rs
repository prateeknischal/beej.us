use std::env;
use std::io::prelude::*;
use std::net::TcpStream;
use std::net::{SocketAddr, ToSocketAddrs};

fn main() {
    if env::args().len() < 2 || env::args().len() > 3 {
        eprintln!("Usage{} host [port:80]", env::args().next().unwrap());
        return;
    }

    let args: Vec<String> = env::args().skip(1).collect();
    let host = &args[0];
    let mut port = "80";
    if args.len() == 2 {
        port = &args[1];
    }

    let addr: SocketAddr;
    addr = match format!("{}:{}", host, port).to_socket_addrs() {
        Ok(mut addrs) => addrs.next().unwrap(),
        Err(e) => {
            eprintln!("failed to resolve the host {}", e);
            return;
        }
    };

    let mut stream = match TcpStream::connect(addr) {
        Ok(s) => s,
        Err(e) => {
            eprintln!("failed to open tcp stream, err: {}", e);
            return;
        }
    };

    let req = format!("GET / HTTP/1.1\r\nHost: {}\r\n\r\n", host);
    if let Err(e) = stream.write(req.as_bytes()) {
        eprintln!("failed to write to socket: {}", e);
        return;
    }

    let mut buf = [0u8; 1024];
    loop {
        for v in buf.iter_mut() {
            *v = 0;
        }

        match stream.read(&mut buf) {
            Ok(bytes_read) => {
                println!("{}", String::from_utf8(buf.to_vec()).unwrap());
                if bytes_read != buf.len() {
                    // this was the last packet and there is nothing else left.
                    break;
                }
            }
            Err(e) => {
                eprintln!("failed to read: {}", e);
                break;
            }
        }
    }
}
