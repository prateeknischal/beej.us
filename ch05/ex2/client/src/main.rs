use std::env;
use std::io::prelude::*;
use std::net::TcpStream;
use std::net::{SocketAddr, ToSocketAddrs};
use std::process;

fn usage() {
    eprintln!("Usage{} host [port:80]", env::args().next().unwrap());
}

fn main() {
    if env::args().len() < 2 || env::args().len() > 3 {
        usage();
        process::exit(1);
    }

    let mut args = env::args().skip(1);
    let host: String = match args.next() {
        Some(h) => h,
        None => {
            usage();
            process::exit(1);
        }
    };

    let port: String = match args.next() {
        Some(p) => p,
        None => "80".to_string(),
    };

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
