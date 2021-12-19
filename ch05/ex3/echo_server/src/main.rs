use std::io::prelude::*;
use std::net::{SocketAddr, TcpListener, TcpStream};

fn main() {
    // create a listener for the host.
    let listener = match TcpListener::bind("0.0.0.0:8000") {
        Ok(listener) => listener,
        Err(e) => panic!("{}", e),
    };

    loop {
        // keep listening to the incoming requests and serve the requests on by one.
        if let Ok((mut conn, addr)) = listener.accept() {
            println!("Got connection from {}", addr);
            handle_client(&mut conn, &addr);
        }
    }
}

/// The echo handler for the server. This will accept the connection, read the request and then
/// reply with the same text as an echo server.
fn handle_client(stream: &mut TcpStream, addr: &SocketAddr) {
    let mut buf = [0u8; 8];
    loop {
        for v in buf.iter_mut() {
            *v = 0;
        }

        if let Err(e) = stream.read(&mut buf) {
            panic!("{}", e);
        }

        if buf.starts_with(b"QUIT") {
            println!("{} Closing the connection", addr);
            break;
        }

        if let Err(e) = stream.write_all(buf.as_slice()) {
            panic!("{}", e);
        }

        print!(
            "{} Echoing the request: {}",
            addr,
            String::from_utf8(buf.to_vec()).unwrap(),
        );

        if buf[buf.len() - 1] != b'\n' {
            println!();
        }
    }
}
